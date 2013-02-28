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
VOID WineXliveSessionCopy(WINEXLIVESESSION * dst, WINEXLIVESESSION * src )
{
    memcpy(dst,src,sizeof(WINEXLIVESESSION));
    if ( src->xlive_session.cContexts > 0 && src->xlive_session.pContexts )
    {
        dst->xlive_session.pContexts = (PXUSER_CONTEXT)malloc(src->xlive_session.cContexts*sizeof(XUSER_CONTEXT));
        memcpy(dst->xlive_session.pContexts,src->xlive_session.pContexts,src->xlive_session.cContexts*sizeof(XUSER_CONTEXT));
    }else{
        dst->xlive_session.pContexts = 0x0;
    }
    if ( src->xlive_session.cProperties > 0 && src->xlive_session.pProperties )
    {
        dst->xlive_session.pProperties= (PXUSER_PROPERTY)malloc(src->xlive_session.cProperties*sizeof(XUSER_PROPERTY));
        memcpy(dst->xlive_session.pProperties,src->xlive_session.pProperties,src->xlive_session.cProperties*sizeof(XUSER_PROPERTY));
    }else{
        dst->xlive_session.pProperties = 0x0;
    }
}
VOID WineXliveSessionSubFree(WINEXLIVESESSION * s)
{
    if ( s->xlive_session.pContexts )
        free(s->xlive_session.pContexts);
    if ( s->xlive_session.pProperties )
        free(s->xlive_session.pProperties);
}


typedef struct {
    WINEXLIVESESSION * m_sessions;
    DWORD session_count;
    HANDLE announceThread;
    
} CSessionMgr;
/// @param xs Pointer to WINEXLIVESESSION , function will duplicate it so caller can free it just after
/// @return Index of the session on the list
DWORD CSessionMgr_addSession(CSessionMgr* I, WINEXLIVESESSION* xs)
{
    if ( !I->m_sessions )
        I->m_sessions = malloc(sizeof(WINEXLIVESESSION)*(I->session_count+1));
    else
        I->m_sessions = realloc(I->m_sessions,sizeof(WINEXLIVESESSION)*(I->session_count+1));
    WineXliveSessionCopy(&(I->m_sessions[I->session_count]),xs);
    I->session_count++;
}
/// @brief Remove a session
/// @param id Index on the list of the session
void CSessionMgr_removeSession(CSessionMgr* I, DWORD id)
{
    if ( id >= I->session_count )
    {
        ERR("Invalid session id\n");
        return;
    }
    //Free the removed session
    WineXliveSessionSubFree(&I->m_sessions[id]);
    //Shift left elements , no need to reallocate internal pointers
    int index = 0;
    for ( index = id; index < I->session_count-1; index++ )
    {
        memcpy(&I->m_sessions[index],&I->m_sessions[index+1],sizeof(WINEXLIVESESSION));
    }
    I->session_count--;
    I->m_sessions = realloc(I->m_sessions,sizeof(WINEXLIVESESSION)*(I->session_count));
}
void CSessionMgr_announceThreadMain(CSessionMgr* I)
{
    FIXME("Announce thread started (I=%p)\n",I);
    while ( 1 )
    {
        
        
        Sleep(500);
    }
}
void CSessionMgr_CSessionMgr(CSessionMgr* I)
{
    I->m_sessions = 0x0;
    I->session_count = 0;
    FIXME("Starting announce thread...\n");
    I->announceThread = CreateThread(NULL,0,&CSessionMgr_announceThreadMain,I,0x0,NULL);
}

CSessionMgr * sMgr = 0x0; 
void MatchMakingStartup()
{
    FIXME("trace always\n");
    sMgr = (CSessionMgr*)malloc(sizeof(CSessionMgr));
    CSessionMgr_CSessionMgr(sMgr);
    
}
// #5300: XSessionCreate
INT WINAPI XSessionCreate (DWORD dwFlags,
         DWORD dwUserIndex,
         DWORD dwMaxPublicSlots,
         DWORD dwMaxPrivateSlots,
         ULONGLONG *pqwSessionNonce,
         PXSESSION_INFO pSessionInfo,
         PXOVERLAPPED pXOverlapped,
         HANDLE *ph) {
    FIXME ("stub: (%d, %d, %d, %d, %p, %p, %p, %p)\n", dwFlags,dwUserIndex,dwMaxPublicSlots,dwMaxPrivateSlots,pqwSessionNonce,pSessionInfo,pXOverlapped,ph);
    WINEXLIVESESSION s;
    s.dwFlags = dwFlags;
    s.xlive_session.dwOpenPublicSlots = dwMaxPublicSlots;
    s.xlive_session.dwOpenPrivateSlots = dwMaxPrivateSlots;
    s.xlive_session.dwFilledPrivateSlots = 0;
    s.xlive_session.dwFilledPublicSlots = 0;
    memcpy(&(s.xlive_session.info),pSessionInfo,sizeof(XSESSION_INFO));
    s.xlive_session.pProperties = NULL;
    s.xlive_session.pContexts = NULL;
    s.xlive_session.cContexts = 0;
    s.xlive_session.cProperties = 0;
    *ph = CSessionMgr_addSession(sMgr,&s);
    return ERROR_SUCCESS;
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
INT WINAPI XSessionSearch( DWORD dwProcedureIndex,
         DWORD dwUserIndex,
         DWORD dwNumResults,
         WORD wNumProperties,
         WORD wNumContexts,
         PXUSER_PROPERTY pSearchProperties,
         PXUSER_CONTEXT pSearchContexts,
         DWORD *pcbResultsBuffer,
         PXSESSION_SEARCHRESULT_HEADER pSearchResults,
         PXOVERLAPPED pXOverlapped) {
    FIXME ("stub: (%d, %d, %d, %d, %d, %p, %p, %p, %p, %p)\n",dwUserIndex,dwNumResults,wNumProperties,wNumContexts,pSearchProperties,pSearchContexts,pcbResultsBuffer,pSearchResults,pXOverlapped);
    if ( !pSearchResults || *pcbResultsBuffer < sizeof(PXSESSION_SEARCHRESULT_HEADER))
    {
        *pcbResultsBuffer = sizeof(PXSESSION_SEARCHRESULT_HEADER);
        return ERROR_INSUFFICIENT_BUFFER;
    }
    return ERROR_SUCCESS;
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