/*
 * Direct3D 9
 *
 * Copyright 2002-2003 Jason Edmeades
 * Copyright 2002-2003 Raphael Junqueira
 * Copyright 2005 Oliver Stieber
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
 *
 */

#include "config.h"
#include "initguid.h"
#include "d3d9_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d9);

static int D3DPERF_event_level = 0;

void WINAPI DebugSetMute(void) {
    /* nothing to do */
}

static BOOL try_native(void)
{
    HKEY defkey, appkey;
    DWORD type, data = 0;
    DWORD size = sizeof(DWORD);
    DWORD len;
    char buffer[MAX_PATH];

    /* @@ Wine registry key: HKCU\Software\Wine\Direct3D */
    if ( RegOpenKeyA( HKEY_CURRENT_USER, "Software\\Wine\\Direct3D", &defkey ) ) defkey = 0;

    len = GetModuleFileNameA( 0, buffer, MAX_PATH );
    if (len && len < MAX_PATH)
    {
        HKEY tmpkey;
        /* @@ Wine registry key: HKCU\Software\Wine\AppDefaults\app.exe\Direct3D */
        if (!RegOpenKeyA( HKEY_CURRENT_USER, "Software\\Wine\\AppDefaults", &tmpkey ))
        {
            char *p, *appname = buffer;
            if ((p = strrchr( appname, '/' ))) appname = p + 1;
            if ((p = strrchr( appname, '\\' ))) appname = p + 1;
            strcat( appname, "\\Direct3D" );
            TRACE("appname = [%s]\n", appname);
            if (RegOpenKeyA( tmpkey, appname, &appkey )) appkey = 0;
            RegCloseKey( tmpkey );
        }
    }

    if (!(appkey && !RegQueryValueExA(appkey, "UseNative", 0, &type, (BYTE *)&data, &size) && (type == REG_DWORD))) {
        if (!(defkey && !RegQueryValueExA(defkey, "UseNative", 0, &type, (BYTE *)&data, &size) && (type == REG_DWORD))) {
            data = 0;
        }
    }

    if (appkey) RegCloseKey( appkey );
    if (defkey) RegCloseKey( defkey );

    return data ? TRUE : FALSE;
}

IDirect3D9 * WINAPI DECLSPEC_HOTPATCH Direct3DCreate9(UINT sdk_version)
{
    struct d3d9 *object;

    TRACE("sdk_version %#x.\n", sdk_version);

    if (try_native()) {
        IDirect3D9 *native;
        if (SUCCEEDED(d3dadapter9_new(FALSE, (IDirect3D9Ex **)&native))) {
            return native;
        }
    }

    if (!(object = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*object))))
        return NULL;

    if (!d3d9_init(object, FALSE))
    {
        WARN("Failed to initialize d3d9.\n");
        HeapFree(GetProcessHeap(), 0, object);
        return NULL;
    }

    TRACE("Created d3d9 object %p.\n", object);

    return (IDirect3D9 *)&object->IDirect3D9Ex_iface;
}

HRESULT WINAPI DECLSPEC_HOTPATCH Direct3DCreate9Ex(UINT sdk_version, IDirect3D9Ex **d3d9ex)
{
    struct d3d9 *object;

    TRACE("sdk_version %#x, d3d9ex %p.\n", sdk_version, d3d9ex);

    if (try_native()) {
        if (SUCCEEDED(d3dadapter9_new(TRUE, d3d9ex))) { return D3D_OK; }
    }

    if (!(object = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*object))))
        return E_OUTOFMEMORY;

    if (!d3d9_init(object, TRUE))
    {
        WARN("Failed to initialize d3d9.\n");
        HeapFree(GetProcessHeap(), 0, object);
        return D3DERR_NOTAVAILABLE;
    }

    TRACE("Created d3d9 object %p.\n", object);
    *d3d9ex = &object->IDirect3D9Ex_iface;

    return D3D_OK;
}

/*******************************************************************
 *       Direct3DShaderValidatorCreate9 (D3D9.@)
 *
 * No documentation available for this function.
 * SDK only says it is internal and shouldn't be used.
 */
void* WINAPI Direct3DShaderValidatorCreate9(void)
{
    static int once;

    if (!once++) FIXME("stub\n");
    return NULL;
}

/*******************************************************************
 *       DllMain
 */
BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, void *reserved)
{
    /* At process attach */
    TRACE("fdwReason=%d\n", fdwReason);
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            d3dadapter9_init(hInstDLL);
            DisableThreadLibraryCalls(hInstDLL);
            break;

        case DLL_PROCESS_DETACH:
            d3dadapter9_destroy(hInstDLL);
            break;
    }
    return TRUE;
}

/***********************************************************************
 *              D3DPERF_BeginEvent (D3D9.@)
 */
int WINAPI D3DPERF_BeginEvent(D3DCOLOR color, const WCHAR *name)
{
    TRACE("color 0x%08x, name %s.\n", color, debugstr_w(name));

    return D3DPERF_event_level++;
}

/***********************************************************************
 *              D3DPERF_EndEvent (D3D9.@)
 */
int WINAPI D3DPERF_EndEvent(void) {
    TRACE("(void) : stub\n");

    return --D3DPERF_event_level;
}

/***********************************************************************
 *              D3DPERF_GetStatus (D3D9.@)
 */
DWORD WINAPI D3DPERF_GetStatus(void) {
    FIXME("(void) : stub\n");

    return 0;
}

/***********************************************************************
 *              D3DPERF_SetOptions (D3D9.@)
 *
 */
void WINAPI D3DPERF_SetOptions(DWORD options)
{
  FIXME("(%#x) : stub\n", options);
}

/***********************************************************************
 *              D3DPERF_QueryRepeatFrame (D3D9.@)
 */
BOOL WINAPI D3DPERF_QueryRepeatFrame(void) {
    FIXME("(void) : stub\n");

    return FALSE;
}

/***********************************************************************
 *              D3DPERF_SetMarker (D3D9.@)
 */
void WINAPI D3DPERF_SetMarker(D3DCOLOR color, const WCHAR *name)
{
    FIXME("color 0x%08x, name %s stub!\n", color, debugstr_w(name));
}

/***********************************************************************
 *              D3DPERF_SetRegion (D3D9.@)
 */
void WINAPI D3DPERF_SetRegion(D3DCOLOR color, const WCHAR *name)
{
    FIXME("color 0x%08x, name %s stub!\n", color, debugstr_w(name));
}
