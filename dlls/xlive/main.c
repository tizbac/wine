/*
 * Copyright 2010 Austin English
 * Copyright 2010 Stanislav Golovin
 * Copyright 2012 Tiziano Bacocco
 * Copyright 2012 Giovanni Ongaro
 * Copyright 2013 Peter Keel
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
#include <stdio.h>
#include <unistd.h>
#include "winsock2.h"
#include "windef.h"
#include "winbase.h"
#include "winsock2.h"
#include "wine/debug.h"
#include <wine/server_protocol.h>
#include "crc32.c"
#include "xlivestructs.h"
#include "wine/library.h"
WINE_DEFAULT_DEBUG_CHANNEL(xlive);

const char xlivebasedir[] = "C:\\xlive\\"; //TODO: Make it tied to system
char xlivedir[512];
DWORD curr_titleId;
BOOL initialized = FALSE;
WINEXLIVEUSER Xliveusers[3];
DWORD handlecounter = 0;
unsigned short x_syslinkport = 3074;

int xnet_recv_bytes = 0;
int xnet_sent_bytes = 0;
XNetStartupParams xnetparams;
void _XNotifyPresenceChanged_Event(void);
void MatchMakingStartup();
INT WINAPI XUserReadProfileSettings(DWORD dwTitleId, DWORD dwUserIndex, DWORD dwNumSettingIds,
                                    DWORD * pdwSettingIds, DWORD * pcbResults, PXUSER_READ_PROFILE_SETTING_RESULT pResults, DWORD pOverlapped);
BOOL WINAPI XLivepIsUserIndexValid(DWORD userid,DWORD unk1, DWORD unk2)
{
    if ( userid < 3 ) {
        return TRUE;
    }
    return FALSE;
}
BOOL DirectoryExists(const char * szPath)
{
    DWORD dwAttrib = GetFileAttributesA(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
DWORD GetTitleID(void) {
    if ( !initialized )
    {
        ERR("Tried to retrieve titleid before init!");
        return -1;
    }
    return curr_titleId;
}
void HexDump(unsigned char * data, unsigned int len)
{
    int i = 0;

    char buf[1024];

    sprintf(buf,"HexDump(%u bytes): ",len);
    for ( i = 0; i < len && i < (1024-(128))/2; i++ )
    {
        sprintf(&buf[strlen(buf)],"%02X",(unsigned int)data[i]);
    }
    if ( i >= (1024-(128))/2 )
        sprintf(&buf[strlen(buf)],"...");
    sprintf(&buf[strlen(buf)],"\n");
    FIXME("%s",buf);
}


//TODO: TEMPORARY FOR DEBUGGING RE5
INT __cdecl Re5Log(void * logClassInstance,const char * format,char * funcname,void * p0,void * p1,void *p2,void * p3)
{

    printf(format,funcname,p0,p1,p2,p3);
    return 0;
}

//TODO: TEMPORARY FOR DEBUGGING RE5
INT __cdecl Re5Log0x20(void * logClassInstance,const char * UNK,char *f2,void * p0,void * p1,void *p2,void * p3)
{

    printf(f2,p0,p1,p2,p3);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    int i;
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
    const char * username = wine_get_user_name();
    for ( i = 0; i < 3; i++)
    {
        memset(&Xliveusers[i],0,sizeof(WINEXLIVEUSER));
        sprintf(Xliveusers[i].username,"Wineplayer%d",i);
        
        if ( i == 0 )
            Xliveusers[i].signedin = eXUserSigninState_SignedInToLive;
        else
            Xliveusers[i].signedin = eXUserSigninState_NotSignedIn;
        
        if ( Xliveusers[i].signedin == eXUserSigninState_SignedInToLive )
            Xliveusers[i].xuid = 0x2000100010001000L | ( (unsigned long long)(i+1) << 62L ) | XLIVEPRIVComputeCrc32(username,strlen(username),0x0);
        else if ( Xliveusers[i].signedin == eXUserSigninState_SignedInLocally )
            Xliveusers[i].xuid = 0x1000100010001000L | ( (unsigned long long)(i+1) << 62L ) | XLIVEPRIVComputeCrc32(username,strlen(username),0x0);
        else
            Xliveusers[i].xuid = 0x0;
        FIXME("Generated XUID For Player %s(%s) : %llx\n",Xliveusers[i].username,username,Xliveusers[i].xuid);
    }

    return TRUE;
}

/* This DLL is completely undocumented. DLL ordinal functions were found using winedump
   and number of parameters determined using Python ctypes. See http://docs.python.org/library/ctypes.html */

// #1: XWSAStartup
INT WINAPI XWSAStartup(WORD wVersionRequested, LPWSADATA lpWsaData) {
    TRACE("(%d, %p)\n",wVersionRequested,lpWsaData);
    return WSAStartup(wVersionRequested,lpWsaData);
}

// #2: XWSACleanup
void WINAPI XWSACleanup(void) {
    WSACleanup();
}

// #3: XSocketCreate
INT WINAPI XSocketCreate (int af, int type, int protocol) {
    TRACE("(%d,%d,%d)\n",af,type,protocol);
    return socket(af,type,protocol);
}

// #4: XSocketClose
INT WINAPI XSocketClose (long sock) {
    TRACE("(%ld)\n",sock);
    return closesocket(sock);
}

// #5: XSocketShutdown
INT WINAPI XSocketShutdown (long sock, int how) {
    TRACE("(%ld,%d)\n",sock,how);
    return shutdown(sock,how);
}

// #6: XSocketIOCTLSocket
INT WINAPI XSocketIOCTLSocket (long sock, long cmd, ULONG * argp) {
    TRACE("(%ld,%ld,%ul)\n",sock,cmd,*argp);
    return ioctlsocket(sock,cmd,argp);
}

// #7: XSocketSetSockOpt
INT WINAPI XSocketSetSockOpt (long sock, int level, int optname, const char * optval, int optlen) {
    TRACE("(%ld,%d,%d,%p%d)\n",sock,level,optname,optval,optlen);
    return setsockopt(sock,level,optname,optval,optlen);
}

// #8: XSocketGetSockOpt
INT WINAPI XSocketGetSockOpt (long sock, int level, int optname, char * optval, int * optlen) {
    TRACE("(%ld,%d,%d,%p%d)\n",sock,level,optname,optval,optlen);
    return getsockopt(sock,level,optname,optval,optlen);
}

// #9: XSocketGetSockName
INT WINAPI XSocketGetSockName (SOCKET sock, struct sockaddr * name, int * namelen) {
    TRACE("(%d,%p,%p)\n",sock,name,namelen);
    return getsockname(sock,name,namelen);
}

// #10: XSocketGetPeerName
INT WINAPI XSocketGetPeerName (SOCKET s, struct sockaddr * addr, int * addrlen) {
    TRACE("(%d,%p,%p)\n",s,addr,addrlen);
    return getpeername(s,addr,addrlen);
}

// #11: XSocketBind
INT WINAPI XSocketBind (SOCKET s, struct sockaddr * addr, int addrlen) {
    TRACE("(%d,%p,%p)\n",s,addr,addrlen);
    return bind(s,addr,addrlen);
}

// #12: XSocketConnect
INT WINAPI XSocketConnect (SOCKET s, struct sockaddr * addr, int addrlen) {
    TRACE("(%d,%p,%p)\n",s,addr,addrlen);
    return connect(s,addr,addrlen);
}

// #13: XSocketListen
INT WINAPI XSocketListen (SOCKET s, int backlog) {
    TRACE("(%d,%d)\n",s,backlog);
    return listen(s,backlog);
}

// #14: XSocketAccept
INT WINAPI XSocketAccept (SOCKET s, struct sockaddr * addr, int * addrlen) {
    TRACE("(%d,%p,%p)\n",s,addr,addrlen);
    return accept(s,addr,addrlen);
}

// #15: XSocketSelect
INT WINAPI XSocketSelect (int n, struct fd_set * readfds, struct fd_set * writefds, struct fd_set * exceptfds, struct timeval * timeout) {
    TRACE("(%d,%p,%p,%p,%p)\n",n,readfds,writefds,exceptfds,timeout);
    return select(n,readfds,writefds,exceptfds,timeout);
}

// #16: XWSAGetOverlappedResult
DWORD WINAPI XWSAGetOverlappedResult (SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags) {
    TRACE("(%d,%p,%p,%d,%p)\n",s,lpOverlapped,lpcbTransfer,fWait,lpdwFlags);
    return WSAGetOverlappedResult(s,lpOverlapped,lpcbTransfer,fWait,lpdwFlags);
}

// #17: XWSACancelOverlappedIO
DWORD WINAPI XWSACancelOverlappedIO (DWORD w1) {
    FIXME("stub: (%d)\n", w1);
    return 0;
    //return WSACancelOverlappedIO(w1); // TODO: Not implemented on wine at all??
}

// #18: XSocketRecv
INT WINAPI XSocketRecv (SOCKET s, char * buf, int len, int flags) {
    
    DWORD rb = recv(s,buf,len,flags);
    TRACE("(%d,%p,%d,0x%08x)=%d\n",s,buf,len,flags,rb);
    xnet_sent_bytes += rb;
    return rb;
}

// #19: XWSARecv
DWORD WINAPI XWSARecv (SOCKET s, LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    TRACE("(%d,%p,%d,%p,%p,%p,%p)\n",s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionRoutine);
    return WSARecv(s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionRoutine);
}

// #20: XSocketRecvFrom
INT WINAPI XSocketRecvFrom (SOCKET s, char * buf, int len, int flags, struct sockaddr * from, int * fromlen) {
    DWORD rb = recvfrom(s,buf,len,flags,from,fromlen);
    TRACE("(%d,%p,%d,0x%08x,%p,%p)=%d\n",s,buf,len,flags,from,fromlen,rb);
    TRACE("WSAGetLastError: %d\n",WSAGetLastError());
    xnet_sent_bytes += rb > 0 ? rb : 0;
    return rb;
}

// #21: XWSARecvFrom
DWORD WINAPI XWSARecvFrom (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8, DWORD w9) {
    FIXME("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8, w9);
    return 0;
}

// #22: XSocketSend
INT WINAPI XSocketSend (SOCKET s, char * buf, int len, int flags) {
    TRACE("(%d,%p,%d,0x%08x)=%d\n",s,buf,len,flags);
    xnet_sent_bytes += len;
    return send(s,buf,len,flags);
}

// #23: XWSASend
DWORD WINAPI XWSASend (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7) {
    FIXME("stub: (%d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7);
    return 0;
}

// #24: XSocketSendTo
INT WINAPI XSocketSendTo (SOCKET s, char * buf, int len, int flags, DWORD * to, int tolen) {
    FIXME("stub: (%p, %d, %d, %p, %d)\n", buf, len, flags, to, tolen);
    return 0;
}

// #25: XWSASendTo
DWORD WINAPI XWSASendTo (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8, DWORD w9) {
    FIXME("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8, w9);
    return 0;
}

// #26: XSocketInet_Addr
INT WINAPI XSocketInet_Addr (char * param) {
    FIXME ("stub: (%p)\n", param);
    return 0;
}

// #27 XWSAGetLastError
INT WINAPI XWSAGetLastError(void) {
    return WSAGetLastError();
}

// #28 XWSASetLastError
INT WINAPI XWSASetLastError(DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return WSAENETDOWN; // 0 ?
}

// #29: XWSACreateEvent
WSAEVENT WINAPI XWSACreateEvent(void) {
    return WSA_INVALID_EVENT;
}

// #30 XWSACloseEvent
INT WINAPI XWSACloseEvent(DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return WSAENETDOWN; // 0 ?
}

// #31 XWSASetEvent
INT WINAPI XWSASetEvent(DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return WSAENETDOWN; // 0 ?
}

// #32 XWSAResetEvent
INT WINAPI XWSAResetEvent(DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return WSAENETDOWN; // 0 ?
}

// #33: XWSAWaitForMultipleEvents
DWORD WINAPI XWSAWaitForMultipleEvents (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #34 __XWSAFDIsSet
INT WINAPI __XWSAFDIsSet(DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return WSAENETDOWN; // 0 ?
}

// 35: XWSAEventSelect
WSAEVENT WINAPI XWSAEventSelect(DWORD w1, DWORD w2, DWORD w3) {
    FIXME("stub: (%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #37:  XSocketNTOHS
short WINAPI XSocketNTOHS(short in) {
    return ntohs(in);
}

// #38: NetDll_ntohs
short WINAPI NetDll_ntohs(short in) {
    return ntohs(in);
}

// #39: XSocketNTOHL
DWORD WINAPI XSocketNTOHL (DWORD dw) {
    return ntohl(dw);
}

// #40: NetDll_htons
short WINAPI NetDll_htons(short in) {
    return htons(in);
}

// #51: XNetStartup
INT WINAPI XNetStartup(XNetStartupParams * p) {
    TRACE ("%d",p);
    memcpy(&xnetparams,p,sizeof(XNetStartupParams));
    
    
    
    
    //WARNING::: TEMPORARY HACK TO LOG RE5 internal stuff, hopefully xlive is initialized before xliveinit
    
    
    if ( curr_titleId == 0xc7db11d3 )
    {
        DWORD handle = (DWORD)GetModuleHandleA(NULL);

        BYTE * pImageBase = (BYTE*)(handle); 
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER) (handle);
        PIMAGE_NT_HEADERS pNtHeader  = (PIMAGE_NT_HEADERS) (pImageBase+pDosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeader);
        int iSection;
        for (iSection = 0; iSection < pNtHeader->FileHeader.NumberOfSections; ++iSection, ++pSection) 
        {
            char * pszSectionName = (char *)(pSection->Name);
            if (!strcmp (pszSectionName, ".text") || !strcmp (pszSectionName, ".rdata")) 
            {
                DWORD dwPhysSize = (pSection->Misc.VirtualSize + 4095) & ~4095;    
                FIXME("[EXE] unprotecting section '%s': addr = 0x%08x, size = 0x%08x\n", pSection->Name, pSection->VirtualAddress, dwPhysSize);

                DWORD oldProtect;
                DWORD newProtect = (pSection->Characteristics & IMAGE_SCN_MEM_EXECUTE) ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
                if (!VirtualProtect ((void*)(handle+pSection->VirtualAddress), dwPhysSize, newProtect, &oldProtect)) 
                {
                    FIXME("[EXE] Virtual protect error\n");
                    abort();
                }
            }
        }

    
        void * ptr1 = *(void **)(0x1361154);
        FIXME("ptr1 is %p\n",ptr1);
        void * ptr2 = *(void **)(ptr1);
        void * ptr3 = ptr2+0x24;
        FIXME("memory patch has to be applied on %p, new address : %p\n",ptr3,&Re5Log);
        *(void**)(ptr3) = (void*)&Re5Log;
        ptr3 = ptr2+0x20;
        FIXME("memory patch has to be applied on %p, new address : %p\n",ptr3,&Re5Log);
        *(void**)(ptr3) = (void*)&Re5Log0x20;
    }
    MatchMakingStartup();
    return 0;
}

// #52: XNetCleanup
INT WINAPI XNetCleanup (void) {
    return 0;
}

// #53: XNetRandom
INT WINAPI XNetRandom (BYTE * pb, DWORD cb) {
    DWORD i;
    FIXME ("(%p, %d)\n", pb, cb);
    if (cb) for (i = 0; i < cb; i++) pb[i] = (BYTE) (rand ());
    return 0;
}

// #54: XNetCreateKey
INT WINAPI XNetCreateKey (void * pxnkid, void * pxnkey) {
    FIXME ("(%p, %p)\n", pxnkid, pxnkey);
    return 0;
}

// #55: XNetRegisterKey
INT WINAPI XNetRegisterKey (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 0;
}

// #56: XNetUnregisterKey
INT WINAPI XNetUnregisterKey (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0;
}

// #57: XNetXnAddrToInAddr
INT WINAPI XNetXnAddrToInAddr (DWORD w1, DWORD w2, DWORD * p) {
    FIXME ("stub: (%d, %d, %p)\n", w1, w2, p);
    *p = 0;
    return 0;
}

// #58: XNetServerToInAddr
DWORD WINAPI XNetServerToInAddr (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("stub: (%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #60: XNetInAddrToXnAddr
DWORD WINAPI XNetInAddrToXnAddr (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("stub: (%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #62: XNetInAddrToString
INT WINAPI XNetInAddrToString(char a1, void *Dst, signed int Size) {
    FIXME("stub: (%d, %p, %d)\n", a1, Dst, Size);
    return 0;
}

// #63: XNetUnregisterInAddr
INT WINAPI XNetUnregisterInAddr (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0;
}

// #64: XNetXnAddrToMachineId
INT WINAPI XNetXnAddrToMachineId( void * addr1, void * pMachId) {
    FIXME("stub: (%p %p)\n",addr1,pMachId);
    return 0;
}

// #65: XNetConnect
INT WINAPI XNetConnect (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0;
}

// #66: XNetGetConnectStatus
INT WINAPI XNetGetConnectStatus (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0;
}

// #67: XNetDnsLookup
INT WINAPI XNetDnsLookup (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("stub: (%d, %d, %d)\n", w1, w2, w3);
    return w2; // might be a pointer, but arg 2 should be returned ?
}

// #68: XNetDnsLookup
INT WINAPI XNetDnsRelease (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 4;
}

// #69: XNetQosListen
DWORD WINAPI XNetQosListen (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #70: XNetQosLookup
DWORD WINAPI XNetQosLookup (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8, DWORD w9, DWORD w10, DWORD w11, DWORD w12) {
    FIXME("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12);
    return 0;
}

// #71: NetDll_XNetQosServiceLookup
INT WINAPI NetDll_XNetQosServiceLookup(DWORD flags, WSAEVENT hEvent, XNQOS ** ppxnqos) {
    FIXME("stub\n");// Commented out because this is called every frame
    XNQOS * xqos = (XNQOS *)malloc(sizeof(XNQOS));
    xqos->cxnqos = 1;
    xqos->cxnqosPending = 0;
    
    xqos->axnqosinfo[0].bFlags = XNET_XNQOSINFO_COMPLETE | XNET_XNQOSINFO_TARGET_CONTACTED ;
    xqos->axnqosinfo[0].cProbesXmit = 1;
    xqos->axnqosinfo[0].cProbesRecv = 1;
    xqos->axnqosinfo[0].cbData = 0;
    xqos->axnqosinfo[0].pbData = 0x0;
    xqos->axnqosinfo[0].wRttMinInMsecs = 100;
    xqos->axnqosinfo[0].wRttMedInMsecs = 100;
    xqos->axnqosinfo[0].dwDnBitsPerSec = 640000;
    xqos->axnqosinfo[0].dwUpBitsPerSec = 320000;
    *ppxnqos = xqos;
    return 0;
}

// #72: XNetQosRelease
DWORD WINAPI XNetQosRelease (XNQOS * pxnqos) {
    FIXME("stub\n");
    free(pxnqos);
    return 0;
}

// #73: XNetGetTitleXnAddr
INT WINAPI XNetGetTitleXnAddr(DWORD * pAddr) {
    *pAddr = 0x01FEFEFE; //localhost
    FIXME("localhost\n");
    return 4;
}

// #75: XNetGetEthernetLinkStatus
INT WINAPI XNetGetEthernetLinkStatus(void) {
    //FIXME("stub\n"); Commented out because this is called every frame
    return 1;
}

// #76: XNetGetBroadcastVersionStatus
DWORD WINAPI XNetGetBroadcastVersionStatus (BOOL fReset) {
    //FIXME("stub: (%d), returning 0\n",a1);
    return 0;// Zero means that there are no wrong-version clients around
}

// #77: XNetQosGetListenStats
DWORD WINAPI XNetQosGetListenStats (DWORD a1,DWORD a2) {
    FIXME("stub: (%d, %d), returning 0\n",a1,a2);
    return 8;
}

// #78: XNetGetOpt
DWORD WINAPI XNetGetOpt (DWORD dwOptId, BYTE *pbValue,DWORD * pdwValueSize) {
    switch ( dwOptId )
    {
        case XNET_OPTID_NIC_XMIT_BYTES:
            FIXME("NIC stats not implemented yet, returning since startup\n");
            if ( sizeof(ULONGLONG) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(ULONGLONG);
                return WSAEMSGSIZE;
            }
            *(ULONGLONG*)pbValue = xnet_sent_bytes;
            return 0;
        case XNET_OPTID_NIC_XMIT_FRAMES:
            FIXME("NIC stats not implemented yet, returning since startup\n");
            if ( sizeof(DWORD) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(DWORD);
                return WSAEMSGSIZE;
            }
            *(DWORD*)pbValue = xnet_sent_bytes/1500; //TODO
            return 0;
        case XNET_OPTID_NIC_RECV_BYTES:
            FIXME("NIC stats not implemented yet, returning since startup\n");
            if ( sizeof(ULONGLONG) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(ULONGLONG);
                return WSAEMSGSIZE;
            }
            *(ULONGLONG*)pbValue = xnet_recv_bytes;
            return 0;
        case XNET_OPTID_NIC_RECV_FRAMES:
            FIXME("NIC stats not implemented yet, returning since startup\n");
            if ( sizeof(DWORD) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(DWORD);
                return WSAEMSGSIZE;
            }
            *(DWORD*)pbValue = xnet_recv_bytes/1500; //TODO
            return 0;
        case XNET_OPTID_CALLER_XMIT_BYTES:
            if ( sizeof(ULONGLONG) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(ULONGLONG);
                return WSAEMSGSIZE;
            }
            *(ULONGLONG*)pbValue = xnet_sent_bytes;
            return 0;
        case XNET_OPTID_CALLER_RECV_BYTES:
            if ( sizeof(ULONGLONG) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(ULONGLONG);
                return WSAEMSGSIZE;
            }
            *(ULONGLONG*)pbValue = xnet_recv_bytes;
            return 0;
        case XNET_OPTID_CALLER_XMIT_FRAMES:
            if ( sizeof(DWORD) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(DWORD);
                return WSAEMSGSIZE;
            }
            *(DWORD*)pbValue = xnet_sent_bytes/1500; //TODO
            return 0;
        case XNET_OPTID_CALLER_RECV_FRAMES:
            if ( sizeof(DWORD) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(DWORD);
                return WSAEMSGSIZE;
            }
            *(DWORD*)pbValue = xnet_recv_bytes/1500; //TODO
            return 0;
        case XNET_OPTID_STARTUP_PARAMS:
            if ( sizeof(XNetStartupParams) < *pdwValueSize )
            {
                *pdwValueSize = sizeof(XNetStartupParams);
                return WSAEMSGSIZE;
            }
            memcpy(pbValue,&xnetparams,sizeof(XNetStartupParams));
            return 0;
        default:
            return WSAEINVAL;
    }
    return 0;
}

// #79: XNetSetOpt
DWORD WINAPI XNetSetOpt (DWORD a1,DWORD a2,DWORD a3) {
    FIXME("stub: (%d %d %d), returning 0\n",a1,a2,a3);
    return a2;
}

// #81: XNetReplaceKey
DWORD WINAPI XNetReplaceKey (DWORD a1,DWORD a2) {
    FIXME("stub: (%d %d), returning 0\n",a1,a2);
    return 8;
}

// #82: XNetGetXnAddrPlatform
DWORD WINAPI XNetGetXnAddrPlatform (DWORD a1,DWORD a2) {
    FIXME("stub: (%d %d), returning 0\n",a1,a2);
    return 8;
}

// #83: XNetGetSystemLinkPort
INT WINAPI XNetGetSystemLinkPort(WORD * LinkPort) {
    FIXME("trace: (%p)\n",LinkPort);
    *LinkPort = x_syslinkport;
    return ERROR_SUCCESS;
}

// #84: XNetSetSystemLinkPort
INT WINAPI XNetSetSystemLinkPort(WORD newPort) {
    FIXME("stub: (%d)\n",(int)newPort);
    x_syslinkport = newPort;
    return 0;
}

// #472: XCustomSetAction
INT WINAPI XCustomSetAction (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("(%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #473: XCustomGetLastActionPress
INT WINAPI XCustomGetLastActionPress (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("(%d, %d, %d)\n", w1, w2, w3);
    return 0;
}
// #474 XCustomSetDynamicActions
INT WINAPI XCustomSetDynamicActions (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #476 XCustomGetLastActionPressEx
INT WINAPI XCustomGetLastActionPressEx(DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #477 XCustomUnregisterDynamicActions
INT WINAPI XCustomUnregisterDynamicActions (void) {
    FIXME ("stub\n");
    return 0;
}

// #478 XCustomRegisterDynamicActions
INT WINAPI XCustomRegisterDynamicActions (void) {
    FIXME ("stub\n");
    return 0;
}

// #479 XCustomGetCurrentGamercard
INT WINAPI XCustomGetCurrentGamercard (DWORD w1, DWORD w2) {
    FIXME ("(%d, %d)\n", w1, w2);
    return 0;
}

// #1082: XGetOverlappedExtendedError
// INT WINAPI XGetOverlappedExtendedError (void * p0) {
INT WINAPI XGetOverlappedExtendedError (int a1) {
    DWORD result;
    result = 996;
    if (a1) {
        if ((DWORD)a1 != 997)
            result = (DWORD)(a1 + 24);
        }
        else {
            result = GetLastError();
	}
    TRACE("(%d, %d)\n", a1, result);
    return result;
}

// #1083: XGetOverlappedResult
INT WINAPI XGetOverlappedResult(void * p0, DWORD * pResult, DWORD bWait) {
    //FIXME("stub: %p %p %d\n",p0,pResult,bWait); Commented out because this is called every frame
    if (pResult)
        *pResult = 0;
    return ERROR_SUCCESS;
}

// #5001: XliveInput
INT WINAPI XliveInput(DWORD * p) {
    p[5] = 0;
    //FIXME("stub: %p\n", p);
    return 1; // do not handle window message calls
}

// #5002: XliveInput
INT WINAPI XLiveRender(void) {
    //FIXME("stub\n"); Commented out because this is called every frame
    return 0;
}

// #5003: XLiveUninitialize
INT WINAPI XLiveUninitialize(void) {
    return 0;
}

// #5005: XLiveOnCreateDevice
INT WINAPI XLiveOnCreateDevice(DWORD p0,DWORD p1) {
    FIXME("stub: (%d %d)\n",p0,p1);
    return 0;
}

// #5006: XLiveOnDestroyDevice
INT WINAPI XLiveOnDestroyDevice(void) {
    FIXME("stub:\n");
    return 0;
}

// #5007: XLiveOnResetDevice
INT WINAPI XLiveOnResetDevice(DWORD p0) {
    FIXME("stub: (%d)\n",p0);
    return 0;
}

// #5008: XHVCreateEngine
INT WINAPI XHVCreateEngine(DWORD p0, DWORD p1, void ** ppEngine)
{
    if ( ppEngine)
        *ppEngine = NULL;
    FIXME("stub: (%d, %d, %p)\n",p0,p1,ppEngine);
    return -1; // disable live voice
}

// #5010: XLiveRegisterDataSection
INT WINAPI XLiveRegisterDataSection (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("(%d, %d, %d)\n", w1, w2, w3);
    return 0;
}

// #5011: XLiveRegisterDataSection
INT WINAPI XLiveUnregisterDataSection (DWORD w1) {
    FIXME ("(%d)\n", w1);
    return 0;
}

// #5012: XLiveUpdateHashes
INT WINAPI XLiveUpdateHashes (DWORD w1, DWORD w2) {
    FIXME ("(%d, %d)\n", w1, w2);
    return 0;
}

// should be defined somewhere else, needed by 5016
typedef struct {
    DWORD   dwMagick;
    DWORD   dwSize;
    DWORD   __fill[2];
    BYTE    bData[4];
} FakeProtectedBuffer;

// #5016: XLivePBufferAllocate
DWORD WINAPI XLivePBufferAllocate (int size, FakeProtectedBuffer ** pBuffer) {
    *pBuffer = (FakeProtectedBuffer *)malloc (size+16);
    TRACE ("(%d, %p)\n", size, pBuffer);
    if (!*pBuffer) {
        return E_OUTOFMEMORY;
    }

    (*pBuffer)->dwMagick = 0xDEADDEAD;      // some arbitrary number
    (*pBuffer)->dwSize = size;
    return 0;
}

// #5017: XLivePBufferFree
DWORD WINAPI XLivePBufferFree(FakeProtectedBuffer * pBuffer) {
    FIXME("(%p)\n", pBuffer);
    if (pBuffer && pBuffer->dwMagick == 0xDEADDEAD)
        free (pBuffer);
    return 0;
}

// #5018: XLivePBufferGetByte
DWORD WINAPI XLivePBufferGetByte (FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * value) {
    TRACE ("(%p, %d, %p)\n", pBuffer, offset, value);
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !value || offset > pBuffer->dwSize)
        return 0;
    *value = pBuffer->bData[offset];
    return 0;
}

// #5019: XLivePBufferSetByte
DWORD WINAPI XLivePBufferSetByte(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE value) {
    TRACE ("(%p, %d, %d)\n", pBuffer, offset, value);
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || offset > pBuffer->dwSize)
        return 0;
    pBuffer->bData[offset] = value;
    return 0;
}

// #5020: XLivePBufferGetDWORD
DWORD WINAPI XLivePBufferGetDWORD (FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD * pdwValue) {
    TRACE ("(%p, %d, %p)\n", pBuffer, dwOffset, pdwValue);
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || dwOffset > pBuffer->dwSize-4 || !pdwValue)
        return 0;
    *pdwValue = *(DWORD *)(pBuffer->bData+dwOffset);
    return 0;
}

// #5021: XLivePBufferSetDWORD
DWORD WINAPI XLivePBufferSetDWORD (FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD dwValue ) {
    TRACE ("(%p, %d, %d)\n", pBuffer, dwOffset, dwValue);
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || dwOffset > pBuffer->dwSize-4)
        return 0;
    *(DWORD *)(pBuffer->bData+dwOffset) = dwValue;
    return 0;
}

// #5022: XLiveGetUpdateInformation
INT WINAPI XLiveGetUpdateInformation (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return -1; // no update
}

// #5023: XNetGetCurrentAdapter
INT WINAPI XNetGetCurrentAdapter (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 0;
}

// #5024: XLiveUpdateSystem
INT WINAPI XLiveUpdateSystem (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return -1; // no update
}

// #5025: XLiveGetLiveIdError
INT WINAPI XLiveGetLiveIdError (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}

// #5026: XLiveSetSponsorToken
DWORD WINAPI XLiveSetSponsorToken (LPCWSTR pwszToken, DWORD dwTitleId) {
    FIXME ("stub: (, 0x%08x)\n", dwTitleId);
    return S_OK;
}

// #5027: XLiveUninstallTitle
INT WINAPI XLiveUninstallTitle (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0; 
}

// #5028 XLiveLoadLibraryEx
INT WINAPI XLiveLoadLibraryEx(LPCWSTR lpLibFileName, int a2, DWORD dwFlags) {
    FIXME ("stub: (%p, %d, %d)\n", lpLibFileName, a2, dwFlags);
    return 0; 
}

// #5029 XLiveFreeLibrary
INT WINAPI XLiveFreeLibrary(HMODULE hLibModule) {
    FIXME ("stub: (%p)\n", hLibModule);
    return 0; 
}

// #5030: XLivePreTranslateMessage
INT WINAPI XLivePreTranslateMessage(DWORD w1) {
    //FIXME("stub: %d\n", unknown);
    return 0;
}

// #5031: XLiveSetDebugLevel
DWORD WINAPI XLiveSetDebugLevel (DWORD xdlLevel, DWORD * pxdlOldLevel) {
    FIXME ("stub: (%d, %p)\n", xdlLevel, pxdlOldLevel);
    return 0;
}

// #5032 XLiveVerifyArcadeLicense
INT WINAPI XLiveVerifyArcadeLicense(struct __SecureBufferHandleStruct *a1, int a2) {
    FIXME ("stub: (%p, %d)\n", a1, a2);
    return 0;
}

// #5034: XLiveProtectData
DWORD WINAPI XLiveProtectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE h) {
    FIXME("stub: %p %d %p %p(%d)\n",pInBuffer,dwInDataSize,pOutBuffer,pDataSize,*pDataSize);

    if ( *pDataSize == 0 ) {
        *pDataSize = dwInDataSize;
        return 0x8007007A; //Insufficent buffer
    }
    if ( !pOutBuffer ) {
        return 0x8007007A; //Insufficent buffer
    }
    if (*pDataSize >= dwInDataSize && pOutBuffer) {
        // HexDump(pInBuffer,dwInDataSize);
        memcpy (pOutBuffer, pInBuffer, dwInDataSize);
    }
    else {
        return 0x8007007A; //Insufficent buffer
    }
    return 0;
}

// #5035: XLiveUnprotectData
INT WINAPI XLiveUnprotectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE * ph) {

    if (!pDataSize || !ph)      // invalid parameter
        return E_FAIL;
    FIXME("stub: %p %d %p %p(%d) %p\n",pInBuffer,dwInDataSize, pOutBuffer,pDataSize,*pDataSize,ph);
    if (!pOutBuffer)
    {
        *pDataSize = dwInDataSize;
        return 0x8007007A;
    }
    *ph = (HANDLE)1;
    if (!pOutBuffer || *pDataSize < dwInDataSize) {
        *pDataSize = dwInDataSize;
        return 0x8007007A;
    }
    *pDataSize = dwInDataSize;
    // HexDump(pInBuffer,dwInDataSize);
    memcpy (pOutBuffer, pInBuffer, dwInDataSize);
    return 0;
}

// #5036: XLiveCreateProtectedDataContext
INT WINAPI XLiveCreateProtectedDataContext(DWORD * dwType, PHANDLE pHandle) {
    FIXME("stub: (%d, %p)\n",*dwType, pHandle);
    if (pHandle)
        *pHandle = (HANDLE)(++handlecounter);
    return 0;
}

// #5037: XLiveQueryProtectedDataInformation
INT WINAPI XLiveQueryProtectedDataInformation(HANDLE h, DWORD * p0) {
    FIXME("stub: %p\n",p0);
    return 0;
}

// #5038: XLiveCloseProtectedDataContext
INT WINAPI XLiveCloseProtectedDataContext(HANDLE h) {
    FIXME("stub:\n");
    return 0;
}

// #5039: XLiveVerifyDataFile
INT WINAPI XLiveVerifyDataFile(DWORD w1) {
    FIXME("stub: (%d)\n",w1);
    return 0;
}

// #5206 XShowMessagesUI
INT WINAPI XShowMessagesUI(int a1) {
    FIXME("stub: (%d)\n",a1);
    return 0;
}

// #5208 XShowGameInviteUI
INT WINAPI XShowGameInviteUI (unsigned int a2, const void *Src, int a4, int a5) {
    FIXME("stub: (%d, %p, %d, %d)\n",a2, Src, a4, a5);
    return 0;
}

// #5209 XShowMessageComposeUI
INT WINAPI XShowMessageComposeUI (int a1, const void *Src, int a3, char *a4) {
    FIXME("stub: (%d, %p, %d, %p)\n",a1, Src, a3, a4);
    return 0;
}

// #5210 XShowFriendRequestUI
INT WINAPI XShowFriendRequestUI (char a1, unsigned int a2, long int a3) {
    FIXME("stub: (%d, %d, %ld)\n",a1, a2, a3);
    return 0;
}

// #5212  XShowCustomPlayerListUI
INT WINAPI XShowCustomPlayerListUI(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12) {
    FIXME("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    return 0;
}

// #5214: XShowPlayerReviewUI
INT WINAPI XShowPlayerReviewUI (DWORD w1, DWORD w2, DWORD w3) {
    FIXME ("stub: (%d, %d, %d)\n", w1, w2, w3);
    return 0;
}
// #5215: XShowGuideUI
INT WINAPI XShowGuideUI (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 1;
}

// #5216: XShowKeyboardUI
INT WINAPI XShowKeyboardUI (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7, DWORD w8) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7, w8);
    return 0;
}

// #5230: XLocatorServerAdvertise
INT WINAPI XLocatorServerAdvertise(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, unsigned int a11, unsigned int a12, unsigned int a13, int a14, int a15) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    return 0;
}

// #5231: XLocatorServerUnAdvertise
INT WINAPI XLocatorServerUnAdvertise(int a1, int *a2) {
    FIXME ("stub: (%d, %p)\n", a1, a2);
    return 0;
}

// #5233: XLocatorGetServiceProperty
INT WINAPI XLocatorGetServiceProperty (int a3, unsigned int a4, int a5, int *a6) {
    FIXME ("stub: (%d, %d, %d, %p)\n", a3, a4, a5, a6);
    return 0;
}

// #5234: XLocatorCreateServerEnumerator
INT WINAPI XLocatorCreateServerEnumerator (int a3, unsigned int a4, unsigned int a5, int Src, unsigned int a7, int a8, unsigned int a9, int a10, int a11, int a12) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n", a3, a4, a5, Src, a7, a8, a9, a10, a11, a12);
    return 0;
}

// #5235: XLocatorCreateServerEnumeratorByIDs
INT WINAPI XLocatorCreateServerEnumeratorByIDs (int a3, unsigned int a4, unsigned int a5, int a6, unsigned int a7, const void *Src, int a9, int a10) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %p, %d, %d)\n", a3, a4, a5, a6, a7, Src, a9, a10);
    return 0;
}

// #5236: XLocatorServiceInitialize
INT WINAPI XLocatorServiceInitialize(DWORD *d1,DWORD *d2) {
// correct would be: XLocatorServiceInitialize(struct _XLOCATOR_INIT_INFO *a1, void **a2)
    FIXME("(%p, %p)\n",d1,d2);
    *d1=1;
    *d2=1;
    return 0;
}

// #5237: XLocatorServiceUnInitialize
INT WINAPI XLocatorServiceUnInitialize(void *a1) {
    FIXME ("stub: (%p)\n", a1);
    return 0;
}

// #5238: XLocatorCreateKey
INT WINAPI XLocatorCreateKey(int a1, int a2) {
    FIXME ("stub: (%d, %d)\n", a1, a2);
    return 0;
}

// #5250: XShowAchievementsUI
INT WINAPI XShowAchievementsUI(unsigned __int32 a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5251: XCloseHandle
DWORD WINAPI XCloseHandle(DWORD p1) {
    FIXME ("stub: (%d)\n",p1);
    if ( !p1 || p1 == -1 )
    {
        SetLastError(87);
    }
    return 0;
}

// #5252: XShowGamerCardUI
INT WINAPI XShowGamerCardUI(unsigned __int32 a1, __int64 a2, DWORD w1) {
// third param might be wrong
    FIXME ("stub: (%d, %lld, %d)\n", a1, a2, w1);
    return 0;
}

// #5254: XLiveCancelOverlapped
// another definition says this should be INT WINAPI XCancelOverlapped (DWORD)
INT WINAPI XLiveCancelOverlapped(void * pOver) {
    FIXME("stub: (%p)\n", pOver);
    return ERROR_SUCCESS;
}

// #5255: XEnumerateBack
INT WINAPI XEnumerateBack(int a1, int a2, int a3, int a4, int a5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5);
    return 0;
}

// #5256: XEnumerate
INT WINAPI XEnumerate (HANDLE hEnum, void * pvBuffer, DWORD cbBuffer, DWORD * pcItemsReturned, void * pOverlapped) {
    if (pcItemsReturned)
        *pcItemsReturned = 0;
    return 0;
}

// #5257: XLiveManageCredentials 
INT WINAPI XLiveManageCredentials(char *a1, const unsigned __int16 *a2, char a3, ULONG_PTR dwData) {
    FIXME ("stub: (%p, %p, %d)\n", a1, a2, a3);
    return 0;
}

// #5258: XLiveSignout
INT WINAPI XLiveSignout(ULONG_PTR a1) {
    FIXME ("stub:\n");
    return 0;
}

// #5259: XLiveSignin
INT WINAPI XLiveSignin(const unsigned __int16 *a1, unsigned __int16 *a2, char a3, ULONG_PTR a4) {
    FIXME ("stub: (%p, %p, %d)\n", a1, a2, a3);
    return 0;
}

// #5260: XShowSigninUI
INT WINAPI XShowSigninUI(DWORD cPanes,DWORD dwFlags){
    FIXME("Signing in user 0 (%d, %d)\n", cPanes, dwFlags);
    if ( dwFlags & XSSUI_FLAGS_LOCALSIGNINONLY )
        Xliveusers[0].signedin = eXUserSigninState_SignedInLocally;
    else
        Xliveusers[0].signedin = eXUserSigninState_SignedInToLive;
    return 0;
}

// #5261: XUserGetXUID
INT WINAPI XUserGetXUID(DWORD dwUserIndex, PXUID pXuid) {
    FIXME("stub: (%d, %p)\n", dwUserIndex, pXuid);
    *pXuid = Xliveusers[dwUserIndex].xuid;
    return ERROR_SUCCESS;
}

// #5262: XUserGetSigninState
XUSER_SIGNIN_STATE WINAPI XUserGetSigninState(DWORD dwUserIndex){
    TRACE("(%d)\n", dwUserIndex);
    if ( dwUserIndex > 2 ) {
        return eXUserSigninState_NotSignedIn;
    }
    TRACE(".signedin: %d\n", Xliveusers[dwUserIndex].signedin);
    return Xliveusers[dwUserIndex].signedin;
    // return 1; // eXUserSigninState_SignedInLocally
}

// #5263: XUserGetName
INT WINAPI XUserGetName(DWORD dwUserId, char * pBuffer, DWORD dwBufLen) {
    TRACE("(%d, %p, %d)\n", dwUserId, pBuffer, dwBufLen);
    if ( dwUserId > 2 ) {
        return ERROR_NO_SUCH_USER;
    }
    if ( !Xliveusers[dwUserId].signedin  ) {
        return ERROR_NO_SUCH_USER;
    }
    if (!pBuffer) {
        return 1;
    }
    lstrcpynA(pBuffer,Xliveusers[dwUserId].username,dwBufLen);
    return ERROR_SUCCESS;
}

// #5264: XUserAreUsersFriends
INT WINAPI XUserAreUsersFriends(DWORD dwUserIndex, DWORD * pXuids, DWORD dwXuidCount, DWORD * pResult, void * pOverlapped) {
    FIXME ("stub: (%d, %p, %d, %p, %p)\n", dwUserIndex, pXuids, dwXuidCount, pResult, pOverlapped);
    return ERROR_NOT_LOGGED_ON;
}

// #5265: XLiveUserCheckPrivilege
// Another definition says this should be INT WINAPI XUserCheckPrivilege (DWORD user, DWORD priv, PBOOL b)
DWORD WINAPI XLiveUserCheckPrivilege(DWORD dwUserIndex,DWORD PrivType, PBOOL pfResult ) {
    FIXME("stub: (%d, %d, %p(%d))\n",dwUserIndex,PrivType,pfResult,*pfResult);
    if ( PrivType != 254 && PrivType != 252 && PrivType != 251 && PrivType != 249
         && PrivType != 248 && PrivType != 247  && PrivType != 246 
         && PrivType != 245 && PrivType != 244 && PrivType != 243 
         && PrivType != 238 && PrivType != 235 && PrivType != 234 
         && PrivType != 226 ) {
         FIXME ("invalid PrivType %d", PrivType);
         return 87;
    }
    if (!pfResult) {
        FIXME ("pfResult must not be NULL");
        return 87;
    }
    *pfResult = FALSE;
    if ( PrivType == 254 && XLivepIsUserIndexValid(dwUserIndex,0,0) && Xliveusers[dwUserIndex].signedin == eXUserSigninState_SignedInToLive )
        *pfResult = TRUE;
    
    return ERROR_SUCCESS;
}

// #5266: XShowMessageBoxUI
INT WINAPI XShowMessageBoxUI(int a3, char *a4, char *a5, int a6, int a7, unsigned int a8, int a9, int a10, int a11) {
    FIXME ("stub: (%d, %p, %p, %d, %d, %d, %d, %d, %d)\n", a3, a4, a5, a6, a7, a8, a9, a10, a11);
    return 0;
}

// #5267: XUserGetSigninInfo
INT WINAPI XUserGetSigninInfo(DWORD dwUser, DWORD dwFlags, XUSER_SIGNIN_INFO * pInfo) {
    FIXME("stub: (%d, %d, %p)\n", dwUser, dwFlags, pInfo);
    if ( !pInfo || dwUser > 2 )
        return ERROR_NO_SUCH_USER;
    memset(pInfo,0,sizeof(XUSER_SIGNIN_INFO));
    if ( dwFlags & XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY )
        pInfo->xuid = Xliveusers[dwUser].xuid;
    else
        pInfo->xuid = Xliveusers[dwUser].xuid;
    pInfo->dwInfoFlags = 0;
    pInfo->UserSigninState = Xliveusers[dwUser].signedin;
    pInfo->dwGuestNumber = 0;
    pInfo->dwSponsorUserIndex = 0;
    lstrcpynA(pInfo->szUserName,Xliveusers[dwUser].username,15);
    return ERROR_SUCCESS;
}

// #5271: XShowPlayersUI
INT WINAPI XShowPlayersUI(int a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5273: XUserReadGamerpictureByKey
INT WINAPI XUserReadGamerpictureByKey (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6);
    return 0;
}

// #5274: XUserAwardGamerPicture
INT WINAPI XUserAwardGamerPicture(int a1, int a2, int a3, int *a4) {
    FIXME ("stub: (%d, %d, %d, %p)\n", a1, a2, a3, a4);
    return 0;
}

// #5275: XShowFriendsUI
INT WINAPI XShowFriendsUI (DWORD w1) {
    FIXME ("stub: (%d)\n", w1);
    return 0;
}

// #5277: XUserSetContext
void WINAPI XUserSetContext( DWORD dwUserIndex, DWORD dwContextId,DWORD dwContextValue) {
    if ( dwUserIndex > 2 ) {
        ERR("invalid user id: %d\n",dwUserIndex);
        return;
    }
    if ( dwContextId != X_CONTEXT_PRESENCE ) {
        FIXME("Setting contexts different from X_CONTEXT_PRESENCE is not supported yet!\n");
        return;
    }
    _XNotifyPresenceChanged_Event();
    Xliveusers[dwUserIndex].contextvalue = dwContextValue;
}

// #5278: XUserWriteAchievements
INT WINAPI XUserWriteAchievements (DWORD dwNumAchievements,
                                   const XUSER_ACHIEVEMENT *pAchievements,
                                   void *pOverlapped) {
    XUSER_ACHIEVEMENT * curr = pAchievements;
    int k = 0;
    HANDLE hFile;
    for ( k = 0; k < dwNumAchievements; k++ ) {
        char buf[512];
        sprintf(buf,"%sachievements\\%d",xlivedir,curr->dwUserIndex);
        if (!DirectoryExists(buf)) {
            if ( !CreateDirectoryA(buf,NULL)) {
                ERR("Cannot create directory %s\n",buf);
                return E_FAIL;
            }
        }
        sprintf(buf,"%sachievements\\%d\\%d.achievement",xlivedir,curr->dwUserIndex,curr->dwAchievementId);
        hFile = CreateFileA(buf,GENERIC_WRITE,0x0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if ( hFile == INVALID_HANDLE_VALUE ) {
            ERR("Cannot create file %s\n",buf);
            return E_FAIL;
        }
    }
    return 0;
}

// #5279: XUserReadAchievementPicture
INT WINAPI XUserReadAchievementPicture (int a2, int a3, int a4, int a5, int a6, int a7, int *a8) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %p)\n", a2, a3, a4, a5, a6, a7, a8);
    return 0;
}

// #5280: XUserCreateAchievementEnumerator
INT WINAPI XUserCreateAchievementEnumerator(DWORD dwTitleId, DWORD dwUserIndex, ULONGLONG xuid, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, DWORD * pcbBuffer, HANDLE * phEnum) {
    FIXME("stub: (%d, %d, %lld, %d, %d, %d, %p, %p)\n", dwTitleId, dwUserIndex, xuid, dwDetailFlags, dwStartingIndex, cItem, pcbBuffer, phEnum);
    if (pcbBuffer)
        *pcbBuffer = 0;
    if (phEnum)
        *phEnum = INVALID_HANDLE_VALUE;
    return 1;   // return error (otherwise, 0-size buffer will be allocated)
}

// #5281: XUserReadStats
INT WINAPI XUserReadStats (DWORD dwTitleId,
                           DWORD dwNumXuids,
                           const XUID *pXuids,
                           DWORD dwNumStatsSpecs,
                           const XUSER_STATS_SPEC *pSpecs,
                           PDWORD *pcbResults,
                           PXUSER_STATS_READ_RESULTS pResults,
                           void *pOverlapped) {
    FIXME ("stub: (%d, %d, %p, %d, %p, %p, %p, %p)\n", dwTitleId, dwNumXuids, pXuids, dwNumStatsSpecs, pSpecs, pcbResults, pResults, pOverlapped);
    if (pcbResults && (*pcbResults) < sizeof(DWORD)) {
        *pcbResults = sizeof(DWORD);
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if (pResults) {
        pResults->dwNumViews = 0;
    }
    return 0;
}

// #5282: XUserReadGamerPicture
INT WINAPI XUserReadGamerPicture(int a2, int a3, int a4, int a5, int a6, int *a7) {
    FIXME ("unk: (%d %d %d %d %d %p)\n",a2,a3,a4,a5,a6,a7);
    return 0;
}

// #5284: XUserCreateStatsEnumeratorByRank
DWORD WINAPI XUserCreateStatsEnumeratorByRank (DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, DWORD dwNuStatSpec, void * pSpecs, DWORD * pcbBuffer, PHANDLE phEnum) {
    FIXME ("stub: (%d, %d, %d, %d, %p, %p, %p)\n", dwTitleId, dwRankStart, dwNumRows, dwNuStatSpec, pSpecs, pcbBuffer, phEnum);
    if (pcbBuffer)
        *pcbBuffer = 0;
    *phEnum = INVALID_HANDLE_VALUE;
    return 1;
}

// #5285: XUserCreateStatsEnumeratorByRating
INT WINAPI XUserCreateStatsEnumeratorByRating (DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, DWORD dwNuStatSpec, void * pSpecs, DWORD * pcbBuffer, PHANDLE phEnum) {
    FIXME ("stub: (%d, %d, %d, %d, %p, %p, %p)\n", dwTitleId, dwRankStart, dwNumRows, dwNuStatSpec, pSpecs, pcbBuffer, phEnum);
    if (pcbBuffer)
        pcbBuffer = 0;
    *phEnum = INVALID_HANDLE_VALUE;
    return 1;
}

// #5286: XUserCreateStatsEnumeratorByXuid
DWORD WINAPI XUserCreateStatsEnumeratorByXuid (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD * pcbBuffer, PHANDLE phEnum) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %p, %p)\n", w1, w2, w3, w4, w5, w6, pcbBuffer, phEnum);
    if (pcbBuffer)
        pcbBuffer = 0;
    *phEnum = INVALID_HANDLE_VALUE;
    return 1;
}

// #5287: XUserResetStatsView
INT WINAPI XUserResetStatsView(int a2, int a3, int *a4) {
    FIXME ("stub: (%d, %d, %p)\n", a2, a3, a4);
    return 0;
}

// #5288: XUserGetProperty
INT WINAPI XUserGetProperty(int a1, int a2, int a3, int a4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", a1, a2, a3, a4);
    return 0;
}

// #5289: XUserGetContext
INT WINAPI XUserGetContext (int a2, int pContext, int *a4) {
    FIXME ("stub: (%d, %d, %p)\n", a2, pContext, a4);
    if (!pContext) {
        TRACE ("pContext must not be NULL.");
        return 87;
    }
    return 0;
}

// #5290: XUserGetReputationStars
DWORD WINAPI XUserGetReputationStars(float a3) {
    FIXME ("stub: (%f)\n", a3);
    return 0;
}

// #5291: XUserResetStatsViewAllUsers
INT WINAPI XUserResetStatsViewAllUsers (int a2, int *a3) {
    FIXME ("stub: (%d, %p)\n", a2, a3);
    return 0;
}

// #5292 XUserSetContextEx
INT WINAPI XUserSetContextEx (DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue, void * pOverlapped) {
    FIXME ("stub: (%d, %d, %d, %p)\n", dwUserIndex, dwContextId, dwContextValue, pOverlapped);
    XUserSetContext(dwUserIndex,dwContextId,dwContextValue);
    return 0;
}

// #5293: XUserSetPropertyEx
INT WINAPI XUserSetPropertyEx (DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, void * pvValue, void * pOverlapped) {
    // FIXME ("XUserSetPropertyEx (%d, 0x%x, ...)\n", dwUserIndex, dwPropertyId);
    char buf[512];
    HANDLE hFile;
    DWORD written;
    if (pOverlapped )
        FIXME("Overlapped ignored!\n");
    sprintf(buf,"%sproperties\\%08X",xlivedir,dwPropertyId);
    hFile = CreateFileA(buf,GENERIC_WRITE,0x0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if ( hFile == INVALID_HANDLE_VALUE ) {
        ERR("Cannot write property!\n");
        return ERROR_ACCESS_DENIED;
    }
    WriteFile(hFile,pvValue,cbValue,&written,NULL);
    if (written < cbValue ) {
        ERR("Cannot write property!\n");
        return ERROR_ACCESS_DENIED;
    }
    CloseHandle(hFile);
    return ERROR_SUCCESS;
}

// #5276: XUserSetProperty
void WINAPI XUserSetProperty(
    DWORD dwUserIndex,
    DWORD dwPropertyId,
    DWORD cbValue,
    const VOID *pvValue ) {
    XUserSetPropertyEx(dwUserIndex,dwPropertyId,cbValue,pvValue,0x0);
}

// #5294: XLivePBufferGetByteArray
DWORD WINAPI XLivePBufferGetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * destination, DWORD size) {
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !destination || offset+size > pBuffer->dwSize)
        return 0;
    memcpy (destination, pBuffer->bData+offset, size);
    return 0;
}

// #5295: XLivePBufferSetByteArray
DWORD WINAPI XLivePBufferSetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * source, DWORD size) {
    if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !source || offset+size > pBuffer->dwSize)
        return 0;
    memcpy (pBuffer->bData+offset, source, size);
    return 0;
}

// #5296: XLiveGetLocalOnlinePort
INT WINAPI XLiveGetLocalOnlinePort(int a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5297: XLiveInitializeEx
INT WINAPI XLiveInitializeEx(XLIVE_INITIALIZE_INFO * pXii, DWORD dwVersion) {
    char fullpathtogame[1024];
    HANDLE exeFile;
    DWORD dwTitleId;
    char buf[512];
    char * crcbuf;
    DWORD bytesread;
    DWORD crc32;

    FIXME("stub: %p %d\n", pXii, dwVersion);
    //TODO: Determine titelid correctly, for now we have to use crc32 of game executable
    GetModuleFileNameA(0,fullpathtogame,1023);
    exeFile = CreateFileA(fullpathtogame,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if ( exeFile == INVALID_HANDLE_VALUE ) {
        ERR("Cannot read executable");
        return E_FAIL;
    }
    crcbuf = fullpathtogame; //Will invalidate fullpathtogame , but it is no longer needed and that will save stack space
    bytesread = 1;
    crc32 = 0x0;
    while ( ReadFile(exeFile,crcbuf,1024,&bytesread,NULL) && bytesread ) {
        crc32 = XLIVEPRIVComputeCrc32(crcbuf,bytesread,crc32);
    }
    FIXME("Calculated titleId from crc32 : 0x%08x\n",crc32);
    curr_titleId = crc32;
    CloseHandle(exeFile);
    initialized = TRUE;
    //Try to create xlive directory
    if ( !CreateDirectoryA(xlivebasedir,NULL) ) {
        if ( GetLastError() != ERROR_ALREADY_EXISTS )
            ERR("Cannot create persistent storage dir!\n");
    }
    dwTitleId = GetTitleID();
    sprintf(xlivedir,"%s%08x\\",xlivebasedir,dwTitleId);
    if ( !CreateDirectoryA(xlivedir,NULL) ) {
        if ( GetLastError() != ERROR_ALREADY_EXISTS )
            ERR("Cannot create persistent storage dir!\n");
    }
    sprintf(buf,"%sachievements",xlivedir);
    if ( !CreateDirectoryA(buf,NULL) ) {
        if ( GetLastError() != ERROR_ALREADY_EXISTS )
            ERR("Cannot create achievement storage dir!\n");
    }
    sprintf(buf,"%sproperties",xlivedir);
    if ( !CreateDirectoryA(buf,NULL) ) {
        if ( GetLastError() != ERROR_ALREADY_EXISTS )
            ERR("Cannot create property storage dir!\n");
    }
    
    
    
    
    
    
    
    
    return 0;
}

// #5000: XLiveInitialize
INT WINAPI XLiveInitialize(XLIVE_INITIALIZE_INFO * pXii) {
    FIXME("stub: (%p)\n", pXii);
    XLiveInitializeEx(pXii,1);
    return 0;
}

// #5298: XLiveGetGuideKey
INT WINAPI XLiveGetGuideKey(int a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5299: XShowGuideKeyRemapUI
INT WINAPI XShowGuideKeyRemapUI(int a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5303: XStringVerify
DWORD WINAPI XStringVerify (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, WORD * pResult, DWORD w7) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %p, %d, )\n", w1, w2, w3, w4, w5, pResult, w7);
    *pResult = 0;
    return 0;
}

// #5304: XStorageUploadFromMemoryGetProgress
INT WINAPI XStorageUploadFromMemoryGetProgress(int a1, int a2, int a3, int a4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", a1, a2, a3, a4);
    return 0;
}

// #5305: XStorageUploadFromMemory
DWORD WINAPI XStorageUploadFromMemory (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5306: XStorageEnumerate
INT WINAPI XStorageEnumerate (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7);
    return 0;
}

// #5307: XStorageDownloadToMemoryGetProgress
INT WINAPI XStorageDownloadToMemoryGetProgress(int a1, int a2, int a3, int a4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", a1, a2, a3, a4);
    return 0;
}

// #5308: XStorageDelete
INT WINAPI XStorageDelete(int a1, int a2, int a3) {
    FIXME ("stub: (%d, %d, %d)\n", a1, a2, a3);
    return 0;
}

// #5309: XStorageBuildServerPathByXuid
INT WINAPI XStorageBuildServerPathByXuid(int a1, int a2, int a3, int a4, unsigned int a5, int a6, int a7, int a8) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5, a6, a7, a8);
    return 0;
}

// #5310: XOnlineStartup
INT WINAPI XOnlineStartup(void) {
    FIXME("stub:\n");
    return 0;
}

// #5311: XOnlineCleanup
INT WINAPI XOnlineCleanup(void) {
    FIXME("stub\n");
    return 0;
}

// #5312: XFriendsCreateEnumerator
INT WINAPI XFriendsCreateEnumerator(DWORD p0, DWORD p1, DWORD p2, DWORD p3, HANDLE * phEnum) {
    *phEnum = INVALID_HANDLE_VALUE;
    return 0;
}

// #5313: XPresenceInitialize
DWORD WINAPI XPresenceInitialize(int a1) {
    FIXME("stub: (%d)\n",a1);
    return 0;
}

// #5314: XUserMuteListQuery
INT WINAPI XUserMuteListQuery (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}



// #5331: XUserReadProfileSettings
INT WINAPI XUserReadProfileSettings(DWORD dwTitleId, DWORD dwUserIndex, DWORD dwNumSettingIds,
                                    DWORD * pdwSettingIds, DWORD * pcbResults, PXUSER_READ_PROFILE_SETTING_RESULT pResults, DWORD pOverlapped) {
    int i = 0;
    char path[1024];
    unsigned char * destptr;
    DWORD size;
    HANDLE propFile;
    DWORD bytesread;

    //FIXME("stub: %d %d %d %p %p %p %d\n",dwTitleId,dwUserIndex,dwNumSettingIds,pdwSettingIds,pcbResults,pResults,pOverlapped);
    /*if (*pcbResults < 1036) {
    	*pcbResults = 1036;	// TODO: make correct calculation by IDs.
    	return ERROR_INSUFFICIENT_BUFFER;
    }*/

    if (*pcbResults == 0 || !pResults) {
        *pcbResults = sizeof(DWORD)+sizeof(XUSER_PROFILE_SETTING)*dwNumSettingIds;
        return 0;
    }
    memset (pResults, 0, *pcbResults);
    for ( i = 0; i < dwNumSettingIds; i++ ) {
        DWORD id = pdwSettingIds[i];
        if ( XISSYSTEMPROPERTY(id) ) {
            FIXME("Setting ID: %d : System properties not supported , returning zeroed data\n",id);
            continue;
        }
        sprintf(path,"%sproperties\\%08X",xlivedir,id);
        propFile = CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if ( propFile == INVALID_HANDLE_VALUE ) {
            ERR("Setting ID: %d : Cannot load property\n",id);
            continue;
        }

// XUSER_DATA_TYPE_CONTEXT     ((BYTE)0)
// XUSER_DATA_TYPE_INT32       ((BYTE)1)
// XUSER_DATA_TYPE_INT64       ((BYTE)2)
// XUSER_DATA_TYPE_DOUBLE      ((BYTE)3)
// XUSER_DATA_TYPE_UNICODE     ((BYTE)4)
// XUSER_DATA_TYPE_FLOAT       ((BYTE)5)
// XUSER_DATA_TYPE_BINARY      ((BYTE)6)
// XUSER_DATA_TYPE_DATETIME    ((BYTE)7)
// XUSER_DATA_TYPE_NULL        ((BYTE)0xFF)
        pResults->pSettings[i].dwSettingId = id;
        pResults->pSettings[i].source = XSOURCE_TITLE; //Dunno if also system can have XSOURCE_TITLE or the opposite
        pResults->pSettings[i].user.dwUserIndex = dwUserIndex;
        pResults->pSettings[i].user.xuid = Xliveusers[dwUserIndex].xuid;
        pResults->pSettings[i].data.type = XPROPERTYTYPEFROMID(id);

        switch ( XPROPERTYTYPEFROMID(id) ) {
        case XUSER_DATA_TYPE_CONTEXT:
            FIXME("Context value not handled yet!\n");
            break;
        case XUSER_DATA_TYPE_INT32:
            ReadFile(propFile,&pResults->pSettings[i].data.nData,4,&bytesread,NULL);
            if ( bytesread != 4 ) {
                ERR("Not enough bytes on setting %08x\n",id);
            }
            break;
        case XUSER_DATA_TYPE_INT64:
            ReadFile(propFile,&pResults->pSettings[i].data.i64Data,8,&bytesread,NULL);
            if ( bytesread != 8 ) {
                ERR("Not enough bytes on setting %08x\n",id);
            }
            break;
        case XUSER_DATA_TYPE_DOUBLE:
            ReadFile(propFile,&pResults->pSettings[i].data.dblData,sizeof(double),&bytesread,NULL);
            if ( bytesread != sizeof(double) ) {
                ERR("Not enough bytes on setting %08x\n",id);
            }
            break;
        case XUSER_DATA_TYPE_DATETIME:
            ReadFile(propFile,&pResults->pSettings[i].data.ftData,sizeof(FILETIME),&bytesread,NULL);
            if ( bytesread != sizeof(FILETIME) ) {
                ERR("Not enough bytes on setting %08x\n",id);
            }
            break;
        case XUSER_DATA_TYPE_UNICODE:
            destptr = &pResults->pSettings[i].data.string.pwszData;
            GetFileSize(propFile,&size);
            pResults->pSettings[i].data.string.cbData = size;
            pResults->pSettings[i].data.string.pwszData = malloc(size);
            ReadFile(propFile,pResults->pSettings[i].data.string.pwszData,size,&bytesread,NULL);
            break;
        case XUSER_DATA_TYPE_BINARY:
            destptr = &pResults->pSettings[i].data.binary.pbData;
            GetFileSize(propFile,&size);
            pResults->pSettings[i].data.binary.cbData = size;
            pResults->pSettings[i].data.binary.pbData = malloc(size);
            ReadFile(propFile,pResults->pSettings[i].data.binary.pbData,size,&bytesread,NULL);
            break;

        }
        CloseHandle(propFile);
        if ( XPROPERTYTYPEFROMID(id) == XUSER_DATA_TYPE_CONTEXT ) {
            FIXME("Context value not handled yet!\n");
        }
    }
    return 0;
}

// #5332: XSessionEnd
INT WINAPI XSessionEnd (DWORD w1, DWORD w2) {
    FIXME ("stub: (%d, %d)\n", w1, w2);
    return 0;
}

// #5333: XSessionArbitrationRegister
DWORD WINAPI XSessionArbitrationRegister (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5, DWORD w6, DWORD w7) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5, w6, w7);
    return 0;
}

// #5334: XOnlineGetServiceInfo
INT WINAPI XOnlineGetServiceInfo(int a1, int a2) {
    FIXME ("stub: (%d, %d)\n", a1, a2);
    return 0;
}

// #5335 XTitleServerCreateEnumerator
INT WINAPI XTitleServerCreateEnumerator (LPCSTR pszServerInfo, DWORD cItem, DWORD * pcbBuffer, PHANDLE phEnum) {
    FIXME ("stub: (%p, %d, %p, %p)\n", pszServerInfo, cItem, pcbBuffer, phEnum);
    *phEnum = INVALID_HANDLE_VALUE;
    return 1;
}

// #5336: XSessionLeaveRemote
DWORD WINAPI XSessionLeaveRemote (DWORD w1, DWORD w2, DWORD w3, DWORD w4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", w1, w2, w3, w4);
    return 0;
}

// #5337 XUserWriteProfileSettings
INT WINAPI XUserWriteProfileSettings (DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4) {
    FIXME ("stub: (%d, %d, %d, %d)\n", dw1, dw2, dw3, dw4);
    return 0;
}

// #5338: XPresenceSubscribe
INT WINAPI XPresenceSubscribe(int a1, int a2, int a3) {
    FIXME ("stub: (%d, %d, %d)\n", a1, a2, a3);
    return 0;
}

// #5339: XUserReadProfileSettingsByXuid
DWORD WINAPI XUserReadProfileSettingsByXuid(
    DWORD dwTitleId,
    DWORD dwUserIndexRequester,
    DWORD dwNumFor,
    const XUID *pxuidFor,
    DWORD dwNumSettingIds,
    const DWORD *pdwSettingIds,
    DWORD *pcbResults,
    PXUSER_READ_PROFILE_SETTING_RESULT pResults,
    void * pXOverlapped ) {
    FIXME("partial stub");
    return XUserReadProfileSettings(dwTitleId,dwUserIndexRequester,dwNumSettingIds,pdwSettingIds,pcbResults,pResults,(DWORD)pXOverlapped);
}

// #5340: XPresenceCreateEnumerator
INT WINAPI XPresenceCreateEnumerator(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5, a6, a7);
    return 0;
}

// #5341: XPresenceUnsubscribe
INT WINAPI XPresenceUnsubscribe(int a1, int a2, int a3) {
    FIXME ("stub: (%d, %d, %d)\n", a1, a2, a3);
    return 0;
}

// #5342: XSessionModifySkill
INT WINAPI XSessionModifySkill(int a2, unsigned int a3, int a4, int *a5) {
    FIXME ("stub: (%d, %d, %d, %p)\n", a2, a3, a4, a5);
    return 0;
}

// #5343: XLiveCalculateSkill
DWORD WINAPI XLiveCalculateSkill (DWORD w1, DWORD w2, DWORD w3, DWORD w4, DWORD w5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", w1, w2, w3, w4, w5);
    return 0;
}

// #5344: XStorageBuildServerPath
DWORD WINAPI XStorageBuildServerPath (DWORD dwUserIndex, DWORD StorageFacility,
                                      void * pvStorageFacilityInfo, DWORD dwStorageFacilityInfoSize,
                                      void * pwszItemName, void * pwszServerPath, DWORD * pdwServerPathLength) {
    FIXME ("stub: ()\n");
    return 0;
}

// #5345: XStorageDownloadToMemory
DWORD WINAPI XStorageDownloadToMemory (DWORD dwUserIndex, DWORD w2,DWORD w3, DWORD w4, DWORD w5, DWORD w6, void * p1) {
    FIXME ("stub: ()\n");
    return 0;
}

// #5346: XUserEstimateRankForRating
INT WINAPI XUserEstimateRankForRating(int a1, int a2, int a3, int a4, int a5) {
    FIXME ("stub: (%d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5);
    return 0;
}

// #5347: XLiveProtectedLoadLibrary
INT WINAPI XLiveProtectedLoadLibrary(void *a2, int a3, void *lpLibFileName, DWORD dwFlags, int a6) {
    FIXME ("stub: (%p, %d, %p, %d, %d)\n", a2, a3, lpLibFileName, dwFlags, a6);
    return 0;
}

// #5348: XLiveProtectedCreateFile
INT WINAPI XLiveProtectedCreateFile (HANDLE hContentAccess, void * pvReserved, PCWSTR pszFilePath, 
    DWORD dwDesiredAccess, DWORD dwShareMode, SECURITY_ATTRIBUTES * pSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE * phModule) {

        HANDLE h = INVALID_HANDLE_VALUE; // 
         h=CreateFileW (pszFilePath, dwDesiredAccess, dwShareMode, pSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, NULL);
        if (phModule)
            *phModule = h;
        // return h != INVALID_HANDLE_VALUE ? S_OK : 1;    /* error code */
        return 1;
}

// #5349: XLiveProtectedVerifyFile
DWORD WINAPI XLiveProtectedVerifyFile (HANDLE hContentAccess, VOID * pvReserved, PCWSTR pszFilePath) {
    FIXME ("stub: ()\n");
    return 0;
}

// #5350: XLiveContentCreateAccessHandle
DWORD WINAPI XLiveContentCreateAccessHandle(DWORD dwTitleId, void * pContentInfo,
        DWORD dwLicenseInfoVersion, void * xebBuffer, DWORD dwOffset, HANDLE * phAccess, void * pOverlapped)
{
    FIXME("stub: ()\n");
    if (phAccess)
        *phAccess = INVALID_HANDLE_VALUE;
    return E_OUTOFMEMORY;
}

// #5351: XLiveContentInstallPackage
INT WINAPI XLiveContentInstallPackage(struct _XLIVE_CONTENT_INFO_V1 *pContentInfo, unsigned __int16 *szCabFilePath, struct _XLIVE_CONTENT_INSTALL_CALLBACK_PARAMS *pInstallCallbackParams) {
    FIXME ("stub: (%p, %p, %p)\n", pContentInfo, szCabFilePath, pInstallCallbackParams);
    if (!pContentInfo) {
        FIXME("Invalid pContentInfo.  Must not be NULL.");
        return -2147467261;
    }
    if (!szCabFilePath) {
       FIXME("Invalid szCabFilePath.  Must not be NULL.");
        return -2147467261;
    }
    if (*(DWORD *)pContentInfo != 1) {
        FIXME("Invalid pContentInfo->dwContentAPIVersion. Must be 1");
        return -2147467261;
    }
    if (!*(DWORD *)pContentInfo + 1) {
        FIXME("Invalid Invalid pContentInfo->dwTitleID. ");
        return -2147467261;
    }
    if ( (*(DWORD *)pContentInfo + 2) != 2) {
        FIXME("Invalid Invalid pContentInfo->dwContentType. Must be XCONTENTTYPE_MARKETPLACE (==2)");
        return -2147467261;
    }
    if (pInstallCallbackParams)  {
        if (*(DWORD *)pInstallCallbackParams !=12 ){
            FIXME("nvalid pInstallCallbackParams->cbSize. Must be sizeof(XLIVE_CONTENT_INSTALL_CALLBACK_PARAMS)");
        return -2147467261;
        }
        if (!*(DWORD *)pInstallCallbackParams +1 ){
            FIXME("Invalid pInstallCallbackParams->pInstallCallback.  Must not be NULL.");
        return -2147467261;
        }
    }
    // now it should call TranslateContentInfoV1ToInternal
    return 0;
}

// #5352: XLiveContentUninstall
DWORD WINAPI XLiveContentUninstall (void * pContentInfo, void * pxuidFor, void * pInstallCallbackParams) {
    FIXME ("stub: ()\n");
    return 0;
}

// #5354: XLiveContentVerifyInstalledPackage
INT WINAPI XLiveContentVerifyInstalledPackage(struct _XLIVE_CONTENT_INFO_V1 *a1, struct _XLIVE_CONTENT_INSTALL_CALLBACK_PARAMS *a2) {
    FIXME ("stub: (%p, %p)\n", a1, a2);
    return 0;
}

// #5355: XLiveContentGetPath
DWORD WINAPI XLiveContentGetPath(DWORD dwUserIndex, void * pContentInfo, wchar_t * pszPath, DWORD * pcchPath) {
    if (dwUserIndex) {
        TRACE("Invalid dwUserIndex, must always be 0");
        return -2147024809;
    }
    if (!pContentInfo) {
        TRACE("Invalid pContentInfo. Must not be NULL");
        return -2147024809;
    }
    // enforce the following tests
    if ( *pszPath > 0 ) {
        pcchPath[0] = 0x0;
    }
    if (!pcchPath) {
        TRACE("Invalid pcchPath. Must not be NULL");
        return -2147024809;
    }
    if (!pszPath && pcchPath) {
        TRACE("Invalid pcchPath. Must be 0 if pszPath is NULL");
        return -2147024809;
    }

    FIXME ("stub: ()\n");
    return 0;
}

// should be defined somewhere else, needed by 5356
INT FUNC001(DWORD * p1 , DWORD * p2)
{
    p2[0] = p1[1];
    p2[1] = p1[2];
    memcpy(&p2[2],&p1[3],0x14);
    return 0;
}

// #5356: XLiveContentGetDisplayName
DWORD WINAPI XLiveContentGetDisplayName(DWORD dwUserIndex,DWORD* pContentInfo, wchar_t * pszDisplayName, DWORD * pcchDisplayName) {
    int result;
    DWORD v6[0x14+2];

    FIXME ("stub: (%d, %p, %ls, %p)\n",dwUserIndex,pContentInfo,pszDisplayName,pcchDisplayName);
    FIXME ("%d, (%x), %p, (%x)\n",dwUserIndex,*pContentInfo,pszDisplayName,*pcchDisplayName);

    if (dwUserIndex) {
        TRACE("Invalid dwUserIndex, must always be 0");
        return -2147024809;
    }
    if (!pContentInfo) {
        TRACE("Invalid pContentInfo. Must not be NULL");
        return -2147024809;
    }
    if (!pcchDisplayName) {
        TRACE("Invalid pcchDisplayName. Must not be NULL");
        return -2147024809;
    }
    if (!pszDisplayName && pcchDisplayName) {
        TRACE("Invalid pcchDisplayName. Must be 0 if pszDisplayName is NULL");
        return -2147024809;
    }

    result = FUNC001(pContentInfo,v6);
    //8c4 access
    FIXME("result %i", result);
    return 0;
}

// #5357: XLiveContentGetThumbnail
INT WINAPI XLiveContentGetThumbnail(int a2, struct _XLIVE_CONTENT_INFO_V1 *a3, void *a4, int a5) {
    FIXME ("stub: (%d, %p, %p, %d)\n", a2, a3, a4, a5);
    return 0;
}

// #5358: XLiveContentInstallLicense
INT WINAPI XLiveContentInstallLicense(struct _XLIVE_CONTENT_INFO_V1 *a1, const WCHAR *a2, int a3) {
    FIXME ("stub: (%p, %p, %d)\n", a1, a2, a3);
    return 0;
}

// #5359: XLiveGetUPnPState
INT WINAPI XLiveGetUPnPState(int a1) {
    FIXME ("stub: (%d)\n", a1);
    return 0;
}

// #5360: XLiveContentCreateEnumerator
DWORD WINAPI XLiveContentCreateEnumerator (DWORD cItems, DWORD * pdwFlags/*seems to contain three dwords */, DWORD *pchBuffer, HANDLE * phContent) {
    FIXME("stub: %d %p %p %p\n",cItems,pdwFlags,pchBuffer,phContent);
    if ( !cItems || cItems > 0x64 )
    {
        ERR("cItems bigger than %d\n",0x64);
        return ERROR_INVALID_PARAMETER;
    }

    if ( pdwFlags)
    {
        DWORD pdwFlags_LO = pdwFlags[0];
        DWORD pdwFlags_HI = pdwFlags[1];
        DWORD userId = pdwFlags[2];
        if ( pdwFlags_LO == 1 /*unk*/ )
        {
            if ( pdwFlags_HI & XLIVE_CONTENT_FLAG_RETRIEVE_FOR_ALL_USERS && pdwFlags_HI & XLIVE_CONTENT_FLAG_RETRIEVE_BY_XUID )
            {
                ERR("Cannot retrieve at the same time for user and for all users\n");
            } else {
                if ( pdwFlags_HI & XLIVE_CONTENT_FLAG_RETRIEVE_FOR_ALL_USERS || pdwFlags_HI & XLIVE_CONTENT_FLAG_RETRIEVE_BY_XUID || XLivepIsUserIndexValid(userId,0,0) )
                {
                    if ( pdwFlags_HI & 2 || pdwFlags[7] == 2 )
                    {
                        if ( pchBuffer )
                        {
                            //TODO: XVerifySameFamily
                            //TODO: Do enumeration

                        }

                    }
                }
            }
        }


    }
    *phContent = INVALID_HANDLE_VALUE;
    return 0;
}

// #5361: XLiveContentRetrieveOffersByDate
DWORD WINAPI XLiveContentRetrieveOffersByDate (DWORD dwUserIndex,DWORD dwOffserInfoVersion,        SYSTEMTIME * pstStartDate, void * pOffserInfoArray, DWORD * pcOfferInfo, void * pOverlapped) {
    FIXME ("stub: ()\n");
    if (pcOfferInfo)
        *pcOfferInfo = 0;
    return 0;
}

// #5365: XShowMarketplaceUI
DWORD WINAPI XShowMarketplaceUI (DWORD dwUserIndex, DWORD dwEntryPoint, ULONGLONG dwOfferId, DWORD dwContentCategories) {
    FIXME ("stub: ()\n");
    return 1;
}

// #5367:
DWORD WINAPI XLIVE_5367 (HANDLE h, DWORD w1, DWORD w2, BYTE * b1, DWORD w3) {
    FIXME ("stub: (xx, %d, %d, %p, %d)\n", w1, w2, b1, w3);
    return 1;
}

// #5372
DWORD WINAPI XLIVE_5372 (HANDLE a1, DWORD a2, DWORD a3, DWORD a4, BYTE *b5, HANDLE h6) {
    FIXME("unk: (xx, %d, %d, %d, %p, xx)\n",a2,a3,a4,b5);
    return -1; // might be 1 also
}

// #5374
DWORD WINAPI XLIVE_5374 (void) {
    FIXME("unk:\n");
    return 65535;
}

// #5376
DWORD WINAPI XLIVE_5376 (HANDLE a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5, DWORD a6) {
    FIXME("unk: (xx, %d, %d, %d, %d, %d)\n",a2,a3,a4,a5,a6);
    return 65535;
}

