/*
 * Rtkit dbus calls
 * Copyright 2010 Maarten Lankhorst for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"
#include "wine/library.h"

#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SCHED_H
#include <sys/sched.h>
#endif
#include <sys/resource.h>

#if defined(HAVE_SETRLIMIT) && defined(__linux__) && defined(SONAME_LIBDBUS_1) && defined(HAVE_SCHED_H)

#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <syscall.h>
#include "object.h"
#include "thread.h"

#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif

#define FUNCPTR(fn) static typeof(fn) *p ##fn

FUNCPTR(dbus_error_init);
FUNCPTR(dbus_error_free);
FUNCPTR(dbus_bus_get);
FUNCPTR(dbus_message_new_method_call);
FUNCPTR(dbus_message_append_args);
FUNCPTR(dbus_connection_send_with_reply_and_block);
FUNCPTR(dbus_message_unref);
FUNCPTR(dbus_set_error_from_message);
#undef FUNCPTR

static struct list rt_thread_list = LIST_INIT(rt_thread_list);

static int translate_error( unsigned tid, const char *name )
{
    if (!strcmp( name, DBUS_ERROR_NO_MEMORY ))
        return -ENOMEM;
    if (!strcmp( name, DBUS_ERROR_SERVICE_UNKNOWN ) ||
        !strcmp( name, DBUS_ERROR_NAME_HAS_NO_OWNER ))
        return -ENOENT;
    if (!strcmp( name, DBUS_ERROR_ACCESS_DENIED ) ||
        !strcmp( name, DBUS_ERROR_AUTH_FAILED ))
        return -EACCES;

    if (debug_level)
        fprintf( stderr, "%04x: Could not map error \"%s\"\n", tid, name );
    return -EIO;
}

static void init_dbus(void)
{
#define FUNCPTR(fn) p ##fn = wine_dlsym( libdbus, #fn, NULL, 0 );
    char error[512];
    void *libdbus = wine_dlopen( SONAME_LIBDBUS_1, RTLD_NOW, error, sizeof( error ) );
    FUNCPTR(dbus_error_init);
    FUNCPTR(dbus_error_free);
    FUNCPTR(dbus_bus_get);
    FUNCPTR(dbus_message_new_method_call);
    FUNCPTR(dbus_message_append_args);
    FUNCPTR(dbus_connection_send_with_reply_and_block);
    FUNCPTR(dbus_message_unref);
    FUNCPTR(dbus_set_error_from_message);
#undef FUNCPTR
}

#define MSG_SIGXCPU "wineserver: SIGXCPU called on wineserver from kernel, realtime priority removed!\n"

static int sched_normal(struct thread *cur)
{
    int ret = 0;

    if (cur->unix_tid != -1) {
        struct sched_param parm;
        memset( &parm, 0, sizeof( parm ) );
        ret = sched_setscheduler(cur->unix_tid, SCHED_OTHER | SCHED_RESET_ON_FORK, &parm);
        if (ret < 0)
            ret = -errno;
    }

    list_remove(&cur->rt_entry);
    list_init(&cur->rt_entry);
    cur->rt_prio = 0;
    cur->priority = 0;
    return ret;
}

static void sigxcpu_handler(int sig, siginfo_t *si, void *ucontext)
{
    struct thread *cur, *tmp;
    int found = 0;
    int old_errno = errno;

    if (si->si_code & SI_KERNEL) {
        struct sched_param parm;
        memset( &parm, 0, sizeof( parm ) );

        sched_setscheduler(syscall( SYS_gettid ), SCHED_OTHER | SCHED_RESET_ON_FORK, &parm);

        write(2, MSG_SIGXCPU, sizeof(MSG_SIGXCPU)-1);
        goto restore_errno;
    }

    LIST_FOR_EACH_ENTRY_SAFE(cur, tmp, &rt_thread_list, struct thread, rt_entry)
    {
        if (si->si_pid == cur->unix_pid && cur->rt_prio == 1) {
            found = 1;
            sched_normal(cur);
        }
    }

    if (!found) {
        LIST_FOR_EACH_ENTRY_SAFE(cur, tmp, &rt_thread_list, struct thread, rt_entry)
        {
            if (si->si_pid == cur->unix_pid)
                sched_normal(cur);
        }
    }

restore_errno:
    errno = old_errno;
}

static void setup_rt(void)
{
    struct sigaction sa;
    struct rlimit rlimit;

    sa.sa_sigaction = sigxcpu_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGXCPU, &sa, NULL);

    if (!getrlimit( RLIMIT_RTTIME, &rlimit ))
    {
        /* wineserver can run for 1.5 seconds continuously at realtime before
         * it gets throttled down. At this point we probably hit a bug
         * somewhere.
         */
        if (rlimit.rlim_max > 2000000)
            rlimit.rlim_max = 2000000;
        if (rlimit.rlim_cur > 1500000)
            rlimit.rlim_cur = 1500000;

        setrlimit( RLIMIT_RTTIME, &rlimit );
    }
}

static DBusConnection *get_dbus(void)
{
    static DBusConnection *bus;
    DBusError error;

    if (bus)
        return bus;
    init_dbus();
    pdbus_error_init( &error );

    bus = pdbus_bus_get( DBUS_BUS_SYSTEM, &error );
    setup_rt();
    return bus;
}

int rtkit_make_realtime( struct thread *thread, pid_t unix_tid, int priority )
{
    DBusConnection *bus;
    DBusMessage *m = NULL, *r = NULL;
    dbus_uint64_t pid = thread ? thread->unix_pid : getpid();
    dbus_uint64_t tid = unix_tid;
    dbus_uint32_t rtprio = priority;
    sigset_t sigset;
    DBusError error;
    int ret;

    bus = get_dbus();
    if (!bus)
        return -ENOTSUP;

    pdbus_error_init( &error );
    m = pdbus_message_new_method_call( "org.freedesktop.RealtimeKit1",
                                       "/org/freedesktop/RealtimeKit1",
                                       "org.freedesktop.RealtimeKit1",
                                       "MakeThreadRealtimeWithPID" );
    if (!m)
    {
        ret = -ENOMEM;
        goto out;
    }

    ret = pdbus_message_append_args( m, DBUS_TYPE_UINT64, &pid,
                                     DBUS_TYPE_UINT64, &tid,
                                     DBUS_TYPE_UINT32, &rtprio,
                                     DBUS_TYPE_INVALID );
    if (!ret)
    {
        ret = -ENOMEM;
        goto out;
    }

    sigemptyset( &sigset );
    sigaddset( &sigset, SIGXCPU );
    sigprocmask( SIG_BLOCK, &sigset, NULL );

    r = pdbus_connection_send_with_reply_and_block( bus, m, -1, &error );
    if (!r)
    {
        ret = translate_error( tid, error.name );
        goto out_unblock;
    }
    if (pdbus_set_error_from_message( &error, r ))
        ret = translate_error( tid, error.name );
    else {
        ret = 0;
        if (thread) {
            if (list_empty(&thread->rt_entry))
                list_add_tail( &rt_thread_list, &thread->rt_entry );
            thread->rt_prio = rtprio;
        }
    }
out_unblock:
    sigprocmask( SIG_UNBLOCK, &sigset, NULL );
out:
    if (m)
        pdbus_message_unref( m );
    if (r)
        pdbus_message_unref( r );
    pdbus_error_free( &error );
    if (debug_level)
        fprintf( stderr, "%04x: Setting realtime priority of %u returns %i %m\n", (int)tid, rtprio, ret );
    return ret;
}

int rtkit_undo_realtime( struct thread *thread )
{
    sigset_t sigset;
    int ret = 0;

    sigemptyset( &sigset );
    sigaddset( &sigset, SIGXCPU );
    sigprocmask( SIG_BLOCK, &sigset, NULL );

    if (!list_empty(&thread->rt_entry))
        ret = sched_normal(thread);

    if (debug_level)
        fprintf( stderr, "%04x: Removing realtime priority of %u returns %i %m\n",
                 (int)thread->unix_tid, thread->rt_prio, ret );

    sigprocmask( SIG_UNBLOCK, &sigset, NULL );
    return ret < 0 ? -errno : 0;
}

#else

int rtkit_make_realtime( struct thread *thread, pid_t unix_tid, int priority )
{
    return -ENOTSUP;
}

int rtkit_undo_realtime( struct thread *thread )
{
    return -ENOTSUP;
}

#endif
