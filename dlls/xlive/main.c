/*
 * Copyright 2010 Austin English
 * Copyright 2012 Tiziano Bacocco
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

struct XUSER_READ_PROFILE_SETTINGS {
	DWORD	dwLength;
	BYTE *	pSettings;
};
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

INT WINAPI XNotifyGetNext(HANDLE hNotification, DWORD dwMsgFilter, DWORD * pdwId, void * pParam)
{
   // FIXME("stub: %d %d %p %p\n", hNotification, dwMsgFilter,  pdwId, pParam);
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
    //FIXME("stub\n"); Commented out because this is called every frame
    return 0;
}

INT WINAPI XLivePreTranslateMessage(DWORD unknown)
{
    FIXME("stub: %d\n", unknown);
    return 0;
}

INT WINAPI XUserGetName(DWORD dwUserId, char * pBuffer, DWORD dwBufLen)
{
    FIXME("stub: %d %p %d\n", dwUserId, pBuffer, dwBufLen);
    char playername[] = "Player";
    if (!pBuffer)
        return 1;
    lstrcpynA(pBuffer,playername,dwBufLen);
    return ERROR_SUCCESS;
}

typedef struct {
   DWORD        xuidL;
   DWORD        xuidH;
   DWORD    dwInfoFlags;
   DWORD        UserSigninState;
   DWORD    dwGuestNumber;
   DWORD    dwSponsorUserIndex;
   CHAR     szUserName[16];
} XUSER_SIGNIN_INFO;

INT WINAPI XUserGetSigninInfo(DWORD dwUser, DWORD dwFlags, XUSER_SIGNIN_INFO * pInfo)
{
    FIXME("stub: %d %d %p\n", dwUser, dwFlags, pInfo);
    memset(pInfo,0,sizeof(XUSER_SIGNIN_INFO));
    pInfo->xuidL = pInfo->xuidH = dwFlags != 1 ? (dwUser+1)*0x10001000 : 0;
   // if (dwFlags != 1) {
                pInfo->dwInfoFlags = 0;
                pInfo->UserSigninState = 1;
                lstrcpynA(pInfo->szUserName,"WinePlayer",15);
                return 0;
   // }
    return 1;
}

INT WINAPI XNotifyCreateListener(DWORD unknown, DWORD unknown2)
{
    FIXME("stub: %d %d\n", unknown, unknown2);
    return 1;
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
    //FIXME("stub\n"); Commented out because this is called every frame
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
INT WINAPI XUserSetContext(DWORD p0, DWORD p1, DWORD p2)
{
  FIXME("stub: %d %d %d\n",p0,p1,p2);
  return 0;
}
INT WINAPI XGetOverlappedResult(void * p0, DWORD * pResult, DWORD bWait)
{
  //FIXME("stub: %p %p %d\n",p0,pResult,bWait); Commented out because this is called every frame
  if (pResult)
		*pResult = 0;
  return ERROR_SUCCESS;

}
INT WINAPI XLiveOnResetDevice(DWORD p0)
{
  FIXME("stub: %d\n",p0);
  return 0;
}
INT WINAPI XLiveOnCreateDevice(DWORD p0,DWORD p1)
{
  FIXME("stub: %d %d\n",p0,p1);
  return 0;
}
INT WINAPI XLiveUninitialize()
{
FIXME("stub\n");
  return 0;
}
INT WINAPI XNetXnAddrToMachineId( void * addr1, void * pMachId)
{
  FIXME("stub: %p %p\n",addr1,pMachId);
  return 0;
}
DWORD WINAPI XLiveUserCheckPrivilege(DWORD uIndex,DWORD PrivType, PBOOL pfResult )
{
  FIXME("stub: %d %d %p\n",uIndex,PrivType,pfResult);
  *pfResult = FALSE;
  return ERROR_SUCCESS;
}
INT WINAPI NetDll_XNetQosServiceLookup(DWORD flags, WSAEVENT hEvent, void ** ppxnqos)
{
	//FIXME("stub\n"); Commented out because this is called every frame
	return 0;
}
INT WINAPI XLiveCreateProtectedDataContext(DWORD * dwType, PHANDLE pHandle)
{
  FIXME("stub\n");
  if (pHandle)
        *pHandle = (HANDLE)1;
  return 0;
}
INT WINAPI XLiveCloseProtectedDataContext(HANDLE h)
{
  FIXME("stub\n");
  return 0;
}
DWORD WINAPI XLiveProtectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE h)
{
  FIXME("stub: %p %d %p %p(%d) %d\n",pInBuffer,dwInDataSize,pOutBuffer,pDataSize,*pDataSize,h);
  *pDataSize = dwInDataSize;
  if ( !pOutBuffer )
      return 0x8007007A; //Insufficent buffer
  if (*pDataSize >= dwInDataSize && pOutBuffer)
            memcpy (pOutBuffer, pInBuffer, dwInDataSize);
    return 0;
}
INT WINAPI XLiveCancelOverlapped(void * pOver)
{
  FIXME("stub\n");
  return ERROR_SUCCESS;
}
INT WINAPI XLIVE_5310(void)
{
    FIXME("stub\n");
    return 0;
}
INT WINAPI XUserReadProfileSettings(DWORD dwTitleId, DWORD dwUserIndex, DWORD dwNumSettingIds, 
					DWORD * pdwSettingIds, DWORD * pcbResults, struct XUSER_READ_PROFILE_SETTINGS * pResults, DWORD pOverlapped)
{
    FIXME("stub: %d %d %d %p %p %p %d\n",dwTitleId,dwUserIndex,dwNumSettingIds,pdwSettingIds,pcbResults,pResults,pOverlapped);
	/*if (*pcbResults < 1036) {
		*pcbResults = 1036;	// TODO: make correct calculation by IDs.
		return ERROR_INSUFFICIENT_BUFFER;
	}*/
if (!pResults)
	return 0;
	memset (pResults, 0, *pcbResults);
	pResults->dwLength = *pcbResults-sizeof (struct XUSER_READ_PROFILE_SETTINGS);
	pResults->pSettings = (BYTE *)pResults+sizeof (struct XUSER_READ_PROFILE_SETTINGS);
    return 0;
}

INT WINAPI XFriendsCreateEnumerator(DWORD p0, DWORD p1, DWORD p2, DWORD p3, HANDLE * phEnum)
{
	*phEnum = INVALID_HANDLE_VALUE;
	return 0; 

}
INT WINAPI XEnumerate (HANDLE hEnum, void * pvBuffer, DWORD cbBuffer, DWORD * pcItemsReturned, void * pOverlapped)
{
	if (pcItemsReturned)
		*pcItemsReturned = 0;
	return 0;
}
INT WINAPI XGetOverlappedExtendedError (void * p0)
{
	return 0;
}
INT WINAPI XLiveUnprotectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE * ph)
{
    FIXME("stub\n");
    if (!pDataSize || !ph)      // invalid parameter
                return E_FAIL;
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
    memcpy (pOutBuffer, pInBuffer, dwInDataSize);
    return 0;
}
void WINAPI XWSACleanup(void)
{
    FIXME("stub\n");
}
INT WINAPI XOnlineCleanup(void)
{
    FIXME("stub\n");
    return 0;
}
INT WINAPI XLiveQueryProtectedDataInformation(HANDLE h, DWORD * p0)
{
    FIXME("stub: %u %p\n",h,p0);
    return 0;
}
INT WINAPI XUserWriteAchievements (DWORD p0, DWORD p1, DWORD p2)
{
    FIXME("stub\n");
    return 0;
}
INT WINAPI XUserCreateAchievementEnumerator(DWORD dwTitleId, DWORD dwUserIndex, DWORD xuidL, DWORD xuidHi, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, DWORD * pcbBuffer, HANDLE * phEnum)
{
    FIXME("stub\n");
    if (pcbBuffer)
        *pcbBuffer = 0;
    if (phEnum)
            *phEnum = INVALID_HANDLE_VALUE;
    return 1;   // return error (otherwise, 0-size buffer will be allocated)
}
INT WINAPI XUserReadStats (DWORD p0, DWORD p1, DWORD p2, DWORD p3, DWORD p4 , DWORD * pcbResults, DWORD * pResults, void * p7)
{
    FIXME("stub\n");
    if (pcbResults)     
            *pcbResults = 4;
    if (pResults)
            *pResults = 0;
    return 0;
}