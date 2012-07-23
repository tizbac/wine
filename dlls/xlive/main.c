/*
 * Copyright 2010 Austin English
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

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winsock2.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(xlive);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    TRACE("(0x%p, %d, %p)\n", hinstDLL, fdwReason, lpvReserved);

    switch (fdwReason)
    {
        case DLL_WINE_PREATTACH:
            return FALSE;    /* prefer native version */
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

/* This DLL is completely undocumented. DLL ordinal functions were found using winedump
   and number of parameters determined using Python ctypes. See http://docs.python.org/library/ctypes.html */

INT WINAPI XLIVE_651(DWORD unknown, DWORD unknown2, DWORD unknown3, DWORD unknown4)
{
    FIXME("stub: %d %d %d %d\n", unknown, unknown2, unknown3, unknown4);
    return 0;
}

INT WINAPI XLIVE_652(DWORD unknown)
{
    FIXME("stub: %d\n", unknown);
    return 0;
}

INT WINAPI XLIVE_5000(DWORD unknown)
{
    FIXME("stub: %d\n", unknown);
    return 0;
}

INT WINAPI XLIVE_5001(DWORD unknown)
{
    FIXME("stub: %d\n", unknown);
    return 0;
}

INT WINAPI XLIVE_5002(void)
{
    FIXME("stub\n");
    return 0;
}

INT WINAPI XLIVE_5030(DWORD unknown)
{
    FIXME("stub: %d\n", unknown);
    return 0;
}

INT WINAPI XLIVE_5263(DWORD unknown, DWORD unknown2, DWORD unknown3)
{
    FIXME("stub: %d %d %d\n", unknown, unknown2, unknown3);
    return 0;
}

INT WINAPI XLIVE_5267(DWORD unknown, DWORD unknown2, DWORD unknown3)
{
    FIXME("stub: %d %d %d\n", unknown, unknown2, unknown3);
    return 0;
}

INT WINAPI XLIVE_5270(DWORD unknown, DWORD unknown2)
{
    FIXME("stub: %d %d\n", unknown, unknown2);
    return 0;
}

INT WINAPI XLIVE_5297(void * pXii, DWORD dwVersion)
{
    FIXME("stub: %p %d\n", pXii, dwVersion);
    return 0;
}

INT WINAPI XNetStartup(void* p)
{
    FIXME("stub: %p\n",p);
    return 0;
}
INT WINAPI XNetGetEthernetLinkStatus()
{
    FIXME("stub\n");
    return 1;
}
INT WINAPI XNetGetTitleXnAddr(DWORD * pAddr)
{
    *pAddr = 0x0100007F; //localhost
    FIXME("returning localhost\n");
    return 4;
}
INT WINAPI XUserGetSigninState(DWORD dwUserIndex)
{
    FIXME("stub: %d\n",dwUserIndex);
    return 1;
}
INT WINAPI XUserGetXUID(DWORD p0, DWORD * pXuid)
{
    pXuid[0] = pXuid[1] = 0x10001000; 
    FIXME("stub: %d %p\n",p0,pXuid);
	  return 0; // ???
}
INT WINAPI XWSAStartup(WORD wVersionRequested, LPWSADATA lpWsaData)
{
    FIXME("stub: %d %p\n",wVersionRequested,lpWsaData);
    lpWsaData->wVersion = wVersionRequested;
    return 0;
}
INT WINAPI XNetGetSystemLinkPort(DWORD * LinkPort)
{
    FIXME("stub: %p\n",LinkPort);
    *LinkPort = 0;
    return WSAEACCES;
}
short WINAPI NetDll_htons(short in)
{
    return htons(in);
}
short WINAPI NetDll_ntohs(short in)
{
    return ntohs(in);
}
INT WINAPI XNetSetSystemLinkPort(WORD newPort)
{
    FIXME("stub: %d\n",(int)newPort);
    return 0;
}
INT WINAPI XHVCreateEngine(DWORD p0, DWORD p1, void ** ppEngine)
{
  if ( ppEngine)
    *ppEngine = NULL;
  FIXME("stub: %d %d %p\n",p0,p1,ppEngine);
  return -1;

}
INT WINAPI XLIVE_5310(void)
{
    FIXME("stub\n");
    return 0;
}
