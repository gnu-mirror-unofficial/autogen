
/*
 *  agUtils.c
 *  $Id: agInit.c,v 3.2 2004/08/15 00:52:47 bkorb Exp $
 *  This is the main routine for autogen.
 */

/*
 *  AutoGen copyright 1992-2004 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */
STATIC void addSysEnv( char* pzEnvName );
#ifdef DAEMON_ENABLED
STATIC void spawnPipe( const char* pzFile );
#ifndef HAVE_SYS_SELECT_H
STATIC void spawnListens( long port );
#endif
STATIC void becomeDaemon( tCC*, tCC*, tCC*, tCC* );
#endif

#include "expr.ini"

EXPORT void
initialize( int arg_ct, char** arg_vec )
{
    /*
     *  Initialize all the Scheme functions.
     */
    ag_init();
    pzLastScheme = zSchemeInit;
    ag_scm_c_eval_string_from_file_line( zSchemeInit, "directive.h",
                                         SCHEME_INIT_LINE );
#ifndef scm_t_port
    {
        tSCC zInitRest[] =
            "(add-hook! before-error-hook error-source-line)\n"
            "(use-modules (ice-9 stack-catch))";
        pzLastScheme = zInitRest;
        ag_scm_c_eval_string_from_file_line( zInitRest, __FILE__, __LINE__-3 );
    }
#endif
    if (OPT_VALUE_TRACE > TRACE_NOTHING) {
        tSCC zBT[] = "(debug-enable 'backtrace)";
        pzLastScheme = zBT;
        ag_scm_c_eval_string_from_file_line( zBT, __FILE__, __LINE__ - 2 );
    }
    pzLastScheme = NULL;

    procState = PROC_STATE_OPTIONS;
    /*
     *  Set the last resort search directory first (lowest priority)
     */
    SET_OPT_TEMPL_DIRS( "$$/../share/autogen" );

    {
        char z[ 128 ] = "__autogen__";
#if defined( HAVE_POSIX_SYSINFO )
        static const int nm[] = {
            SI_SYSNAME, SI_HOSTNAME, SI_ARCHITECTURE, SI_HW_PROVIDER,
#ifdef      SI_PLATFORM
            SI_PLATFORM,
#endif
            SI_MACHINE };
        int ix;
        long sz;
        char* pz = z+2;

        addSysEnv( z );
        for (ix = 0; ix < sizeof(nm)/sizeof(nm[0]); ix++) {
            sz = sysinfo( nm[ix], z+2, sizeof( z ) - 2);
            if (sz > 0) {
                sz += 2;
                while (z[sz-1] == NUL)  sz--;
                strcpy( z + sz, "__" );
                addSysEnv( z );
            }
        }

#elif defined( HAVE_UNAME_SYSCALL )
        struct utsname unm;

        addSysEnv( z );
        if (uname( &unm ) != 0) {
            fprintf( stderr, "Error %d (%s) making uname(2) call\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );
        }

        sprintf( z+2, "%s__", unm.sysname );
        addSysEnv( z );

        sprintf( z+2, "%s__", unm.machine );
        addSysEnv( z );

        sprintf( z+2, "%s__", unm.nodename );
        addSysEnv( z );
#else

        addSysEnv( z );
#endif
    }

#ifndef DAEMON_ENABLED
    doOptions( arg_ct, arg_vec );

#else
    optionSaveState( &autogenOptions );
    doOptions( arg_ct, arg_vec );

    if (! HAVE_OPT( DAEMON ))
        return;

    if (0) {
#ifndef DEBUG_ENABLED
        tSCC zDevNull[] = "/dev/null";
#else
        tSCC zDevNull[] = "/tmp/AutoGenDebug.txt";
#endif /* DEBUG_ENABLED */
        becomeDaemon( "/", zDevNull, zDevNull, zDevNull );
    }

    {
        long  port;
        tCC*  pzS = OPT_ARG( DAEMON );
        char* pzE;
        errno = 0;
        port  = strtol( pzS, &pzE, 0 );

        if ((errno != 0) || (*pzE != NUL)) {
            spawnPipe( pzS );
        }
        else {
#ifndef HAVE_SYS_SELECT_H
            spawnListens( port );
#else
            AG_ABEND( "You cannot listen on a port." );
#endif
        }
    }
#endif /* DAEMON_ENABLED */
}


STATIC void
addSysEnv( char* pzEnvName )
{
    tSCC zFmt[] = "%s=1";
    int i = 2;

    for (;;) {
        if (isupper( pzEnvName[i] ))
            pzEnvName[i] = tolower( pzEnvName[i] );
        else if (! isalnum( pzEnvName[i] ))
            pzEnvName[i] = '_';

        if (pzEnvName[ ++i ] == NUL)
            break;
    }

    /*
     *  If the user already has something in the environment, do not
     *  override it.
     */
    if (getenv( pzEnvName ) == NULL) {
        char* pz;

        if (OPT_VALUE_TRACE > TRACE_NOTHING)
            fprintf( pfTrace, "Adding ``%s'' to environment\n", pzEnvName );
        pz = aprf( zFmt, pzEnvName );
        TAGMEM( pz, "Added environment var" );
        putenv( pz );
    }
}


#ifdef DAEMON_ENABLED
EXPORT void
handleSighup( int sig )
{
    redoOptions = AG_TRUE;
}


STATIC void
spawnPipe( tCC* pzFile )
{
#   define S_IRW_ALL \
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

#ifdef HAVE_FATTACH

    tFdPair fdpair;
    ag_bool removeDaemonFile = AG_FALSE;

    if (pipe( (int*)&fdpair ) != 0)
        AG_ABEND( aprf( zCannot, errno, "create", "a pipe", strerror(errno)));

    {
        struct stat sb;

        if (stat( pzFile, &sb ) != 0) {
            int fd;
            if (errno != ENOENT)
                AG_ABEND( aprf( zCannot, errno, "stat",
                                pzFile, strerror( errno )));
            fd = creat( pzFile, S_IRW_ALL );
            if (fd < 0)
                AG_ABEND( aprf( zCannot, errno, "creat(e)",
                                pzFile, strerror( errno )));
            close( fd );
            removeDaemonFile = AG_TRUE;
        }
    }

#ifdef HAVE_CONNLD
    (void)ioctl( fdpair.writeFd, I_PUSH, "connld" );
#endif /* HAVE_CONNLD */

    if (fattach( fdpair.writeFd, pzFile ) != 0)
        AG_ABEND( aprf( zCannot, errno, "fattach", pzFile, strerror(errno)));

    close( fdpair.writeFd );

    {
        struct pollfd polls[1];
        polls[0].fd     = fdpair.readFd;
        polls[0].events = POLLIN | POLLPRI;

        for (;;) {
            int ct = poll( polls, 1, -1 );
            struct strrecvfd recvfd;
            pid_t child;

            switch (ct) {
            case -1:
                if ((errno != EINTR) || (! redoOptions))
                    goto finish_with_fattach;

                optionRestore( &autogenOptions );
                doOptions( autogenOptions.origArgCt,
                           autogenOptions.origArgVect );
                SET_OPT_DEFINITIONS("-");
                break;

            case 1:
                if ((polls[0].revents & POLLIN) == 0)
                    continue;

                child = fork();
                switch (child) {
                default:
                    waitpid( child, &ct, 0 );
                    continue;

                case -1:
                    AG_ABEND( aprf(zCannot, errno, "fork", "", strerror(errno)));

                case 0:
                }
#ifdef HAVE_CONNLD
                if (ioctl( fdpair.readFd, I_RECVFD, &recvfd ) != 0)
                    AG_ABEND( aprf(zCannot, errno, "I_RECVFD", "on a pipe",
                                   strerror(errno)));
#else
                recvfd.fd = fdpair.readFd;
#endif /* ! HAVE_FATTACH */

                if (dup2( recvfd.fd, STDIN_FILENO ) != STDIN_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdin",
                                   strerror(errno)));

                if (dup2( fdpair.writeFd, STDOUT_FILENO ) != STDOUT_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdout",
                                   strerror(errno)));
                return;
            }
        }
    }

 finish_with_fattach:
    if (removeDaemonFile)
        unlink( pzFile );
#   undef S_IRW_ALL

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#else  /* ! HAVE_FATTACH */
    tFdPair fdpair;
    char* pzIn;
    char* pzOut;

    {
        size_t len = 2 * (strlen(pzFile) + 5);
        pzIn = AGALOC( len + 5, "fifo file name" );
        pzOut = pzIn + sprintf( pzIn, "%s-in", pzFile ) + 1;
    }

    if ((mkfifo( pzIn, S_IRW_ALL ) != 0) && (errno != EEXIST))
        AG_ABEND( aprf( zCannot, errno, "mkfifo",    pzIn, strerror(errno)));

    (void)sprintf( pzOut, "%s-out", pzFile );
    if ((mkfifo( pzOut, S_IRW_ALL ) != 0) && (errno != EEXIST))
        AG_ABEND( aprf( zCannot, errno, "mkfifo",    pzOut, strerror(errno)));

    fdpair.readFd = open( pzIn, O_RDONLY );
    if (fdpair.readFd < 0)
        AG_ABEND( aprf( zCannot, errno, "open fifo", pzIn, strerror(errno)));


    {
        struct pollfd polls[1];
        polls[0].fd     = fdpair.readFd;
        polls[0].events = POLLIN | POLLPRI;

        for (;;) {
            int ct = poll( polls, 1, -1 );
            struct strrecvfd recvfd;
            pid_t child;

            switch (ct) {
            case -1:
                if ((errno != EINTR) || (! redoOptions))
                    goto no_fattach_finish;

                optionRestore( &autogenOptions );
                doOptions( autogenOptions.origArgCt,
                           autogenOptions.origArgVect );
                SET_OPT_DEFINITIONS("-");
                break;

            case 1:
                if ((polls[0].revents & POLLIN) == 0)
                    continue;

                child = fork();
                switch (child) {
                default:
                    waitpid( child, &ct, 0 );
                    continue;

                case -1:
                    AG_ABEND( aprf(zCannot, errno, "fork", "", strerror(errno)));

                case 0:
                }

                if (dup2( fdpair.readFd, STDIN_FILENO ) != STDIN_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdin",
                                   strerror(errno)));

                fdpair.writeFd = open( pzOut, O_WRONLY );
                if (fdpair.writeFd < 0)
                    AG_ABEND( aprf( zCannot, errno, "open fifo", pzOut,
                                    strerror(errno)));

                polls[0].fd = fdpair.writeFd;
                polls[0].events = POLLOUT;
                if (poll( polls, 1, -1 ) != 1)
                    AG_ABEND( aprf(zCannot, errno, "poll", "write pipe",
                                   strerror( errno )));

                if (dup2( fdpair.writeFd, STDOUT_FILENO ) != STDOUT_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", pzOut,
                                   strerror(errno)));
                return;
            }
        }
    }

 no_fattach_finish:
    unlink( pzIn );
    unlink( pzOut );
    AGFREE( pzIn );
#endif /* ! HAVE_FATTACH */
#   undef S_IRW_ALL

    exit( EXIT_SUCCESS );
}


#ifndef HAVE_SYS_SELECT_H
STATIC void
spawnListens( long port )
{
    tSCC zPortFmt[] = "to port %d on INET stream sock";
    int socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if (socket_fd < 0)
        AG_ABEND( aprf( zCannot, errno, "socket", "AF_INET/SOCK_STREAM",
                        strerror( errno )));
    {
        struct sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons( (short)port );
        if (bind( socket_fd, &addr, sizeof(addr) ) < 0) {
            char* pz = aprf( zPortFmt, port );
            AG_ABEND( aprf( zCannot, errno, "bind", pz, strerror( errno )));
        }
    }

    if (fcntl( socket_fd, F_SETFL, O_NONBLOCK ) < 0) {
        AG_ABEND( aprf( zCannot, errno, "socket-fcntl", "FNDELAY",
                        strerror( errno )));
    }

    if (listen( socket_fd, 5 ) < 0) {
        char* pz = aprf( zPortFmt, port );
        AG_ABEND( aprf( zCannot, errno, "listen", pz, strerror( errno )));
    }

    for (;;) {
        fd_set fds;
        int    max_fd = socket_fd;
        int    new_conns;

        FD_ZERO( &fds );
        FD_SET( socket_fd, &fds );

        new_conns = select( max_fd, &fds, NULL, NULL, NULL );
        if (new_conns < 0) {
            if (errno == EINTR)
                continue;

            if (! redoOptions)
                exit( EXIT_SUCCESS );

            optionRestore( &autogenOptions );
            doOptions( autogenOptions.origArgCt,
                       autogenOptions.origArgVect );
            SET_OPT_DEFINITIONS("-");

            continue;
        }

        if (new_conns > 0) {
            switch (fork()) {
            default: continue;
            case -1: AG_ABEND(aprf(zCannot, errno, "fork", "", strerror(errno)));
            case 0:  break;
            }
            break;
        }
    }

    for (;;) {
        static int try_ct = 0;
        struct sockaddr addr;
        socklen_t addr_len;
        int fd = accept( socket_fd, &addr, &addr_len );
        if (fd < 0)
            switch (errno) {
            case EINTR:
            case EAGAIN:
            case EWOULDBLOCK:
                if (try_ct++ < 10000) {
                    sleep(1);
                    continue;
                }
            }
        socket_fd = fd;
        break;
    }

    if (dup2( socket_fd, STDOUT_FILENO ) != STDOUT_FILENO)
        AG_ABEND( aprf(zCannot, errno, "dup2", pzOut, strerror(errno)));
    if (dup2( socket_fd, STDIN_FILENO ) != STDIN_FILENO)
        AG_ABEND( aprf(zCannot, errno, "dup2", pzOut, strerror(errno)));
}
#endif


STATIC void
becomeDaemon( tCC* pzStdin,
              tCC* pzStdout,
              tCC* pzStderr,
              tCC* pzDaemonDir )
{
    tSCC zNoFork[] = "Error %d while forking:  %s\n";
    /*
     *  Become a daemon process by exiting the current process
     *  and allowing the child to continue.  Also, change stdin,
     *  stdout and stderr to point to /dev/null and change to
     *  the root directory ('/').
     */
    {
        int ret = fork();

        switch (ret) {
        case -1:
            fprintf( stderr, zNoFork, errno, strerror( errno ));
        default:
            exit( ret );

        case 0:
            break;
        }
    }

    /*
     *  Now, become a process group and session group leader.
     */
    if (setsid() == -1) {
        fprintf( stderr, "Error %d setting session ID:  %s\n",
                 errno, strerror( errno ));
        exit( 99 );
    }

    /*
     *  There is now no controlling terminal.  However, if we open anything
     *  that resembles a terminal, it will become our controlling terminal.
     *  So, we will fork again and the child will not be a group leader and
     *  thus cannot gain a controlling terminal.
     */
    switch (fork()) {
    case -1:
        fprintf( stderr, zNoFork, errno, strerror( errno ));
        exit( 99 );

    case 0:
        break;

    default:
        exit( 0 );  /* parent process - silently go away */
    }

    umask( 0 );
    if (pzDaemonDir == (char*)NULL)
        pzDaemonDir = "/";

    chdir( pzDaemonDir );

    /*
     *  Reopen the input, output and error files, unless we were told not to
     */
    if (pzStdin != (char*)NULL)
        freopen( pzStdin,  "r", stdin );

    if (pzStdout != (char*)NULL)
        freopen( pzStdout, "w", stdout );

    if (pzStderr != (char*)NULL)
        freopen( pzStderr, "w", stderr );

    /* We are a daemon now */
}
#endif
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agInit.c */