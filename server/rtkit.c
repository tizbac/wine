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
#include "object.h"

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

static DBusConnection *get_dbus(void)
{
    static DBusConnection *bus;
    DBusError error;

    if (bus)
        return bus;
    init_dbus();
    pdbus_error_init( &error );

    bus = pdbus_bus_get( DBUS_BUS_SYSTEM, &error );
    return bus;
}

int rtkit_make_realtime( pid_t process, pid_t thread, int priority )
{
    DBusConnection *bus;
    DBusMessage *m = NULL, *r = NULL;
    dbus_uint64_t pid = process;
    dbus_uint64_t tid = thread;
    dbus_uint32_t rtprio = priority;
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
    r = pdbus_connection_send_with_reply_and_block( bus, m, -1, &error );
    if (!r)
    {
        ret = translate_error( tid, error.name );
        goto out;
    }
    if (pdbus_set_error_from_message( &error, r ))
        ret = translate_error( tid, error.name );
    else
        ret = 0;
out:
    if (m)
        pdbus_message_unref( m );
    if (r)
        pdbus_message_unref( r );
    pdbus_error_free( &error );
    if (debug_level)
        fprintf( stderr, "%04x: Setting realtime priority of %u returns %i\n", (int)tid, rtprio, ret );
    return ret;
}

int rtkit_undo_realtime( pid_t thread )
{
    struct sched_param parm;
    int ret;
    memset( &parm, 0, sizeof( parm ) );
    ret = sched_setscheduler( thread, SCHED_OTHER, &parm );
    if (ret < 0)
        return -errno;
    return ret;
}

#else

int rtkit_make_realtime( pid_t process, pid_t thread, int priority )
{
    return -ENOTSUP;
}

int rtkit_undo_realtime( pid_t thread )
{
    return -ENOTSUP;
}

#endif
