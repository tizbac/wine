#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "winsock2.h"
#include "windef.h"
#include "winbase.h"
#include "winsock2.h"
#include "wine/debug.h"
#include <wine/server_protocol.h>
#include "xlivestructs.h"
#include "wine/library.h"
WINE_DEFAULT_DEBUG_CHANNEL(xlive_matchmaking);


// #5300: XSessionCreate
INT WINAPI XSessionCreate (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8);
    return -1;
}

// #5317: XSessionWriteStats
DWORD WINAPI XSessionWriteStats (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6);
    return 0;
}

// #5319: XSessionSearchEx
DWORD WINAPI XSessionSearchEx (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8, DWORD w9, DWORD w10, DWORD w11) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11);
    return 0;
}

// #5318: XSessionStart
INT WINAPI XSessionStart (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("stub: (%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #5320: XSessionSearchByID
INT WINAPI XSessionSearchByID(int a1, int a2, int a3, int a4, int a5, int *a6) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %p)\n", a1, a2, a3, a4, a5, a6);
    return 0;
}

// #5321: XSessionSearch
INT WINAPI XSessionSearch(int a3, int a4, int a5, __int16 a6, __int16 a7, int a8, int a9, int a10, int a11, int *a12) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %p)\n", a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    return 0;
}

// #5322: XSessionModify
DWORD WINAPI XSessionModify (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5323: XSessionMigrateHost
DWORD WINAPI XSessionMigrateHost (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}

// #5315: XInviteGetAcceptedInfo
INT WINAPI XInviteGetAcceptedInfo (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 1;
}

// #5316: XInviteSend
INT WINAPI XInviteSend (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5324 XOnlineGetNatType
INT WINAPI  XOnlineGetNatType (void) {
    return XONLINE_NAT_OPEN; //Since we are going to use it in LAN , NAT is always open
}

// #5325: XSessionLeaveLocal
DWORD WINAPI XSessionLeaveLocal (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}

// #5326: XSessionJoinRemote
DWORD WINAPI XSessionJoinRemote (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5327: XSessionJoinLocal
DWORD WINAPI XSessionJoinLocal (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5328: XSessionGetDetails
DWORD WINAPI XSessionGetDetails (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}

// #5329: XSessionFlushStats
INT WINAPI XSessionFlushStats (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 0;
}

// #5330: XSessionDelete
DWORD WINAPI XSessionDelete (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 0;
}