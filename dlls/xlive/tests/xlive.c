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

#include <stdio.h>
#include <windows.h>

#include "wine/test.h"

static BOOL (WINAPI *pXLIVE5002)(void);
static BOOL (WINAPI *pXLIVE5310)(void);

static HMODULE hmod;

static BOOL InitFunctionPtrs(void)
{
    hmod = LoadLibraryA("xlive.dll");
    if(!hmod)
    {
        win_skip("Could not load xlive.dll\n");
        return FALSE;
    }

    pXLIVE5002 = (void *)GetProcAddress(hmod, (LPSTR)5002);
    pXLIVE5310 = (void *)GetProcAddress(hmod, (LPSTR)5310);

    return TRUE;
}

static void test_XLIVE_5002(void)
{
    HRESULT hr;

    if(!pXLIVE5002)
    {
        win_skip("function not available, skipping test\n");
        return;
    }

    hr = pXLIVE5002();
    ok(hr == S_OK, "got %x, expected S_OK\n", hr);
}

static void test_XLIVE_5310(void)
{
    INT res;

    if(!pXLIVE5310)
    {
        win_skip("function not available, skipping test\n");
        return;
    }

    res = pXLIVE5310();
    ok(res == 10093, "got %d, expected 10093\n", res);
}

START_TEST(xlive)
{
    if(!InitFunctionPtrs())
        return;

    test_XLIVE_5002();
    test_XLIVE_5310();

    FreeLibrary(hmod);
}
