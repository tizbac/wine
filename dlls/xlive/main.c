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
#include <wine/server_protocol.h>
#include "crc32.c"
#include "xlivestructs.h"

WINE_DEFAULT_DEBUG_CHANNEL(xlive);

const char xlivebasedir[] = "C:\\xlive\\"; //TODO: Make it tied to system 
char xlivedir[512];
DWORD curr_titleId;
BOOL initialized = FALSE;
WINEXLIVEUSER Xliveusers[3];
DWORD handlecounter = 0;
INT WINAPI XUserReadProfileSettings(DWORD dwTitleId, DWORD dwUserIndex, DWORD dwNumSettingIds,
                    DWORD * pdwSettingIds, DWORD * pcbResults, PXUSER_READ_PROFILE_SETTING_RESULT pResults, DWORD pOverlapped);
BOOL DirectoryExists(const char * szPath)
{
  DWORD dwAttrib = GetFileAttributesA(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
DWORD GetTitleID()
{
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
    for ( i = 0; i < len; i++ )
    {
        sprintf(&buf[strlen(buf)],"%02X",(unsigned int)data[i]);
    }
    sprintf(&buf[strlen(buf)],"\n");
    FIXME("%s",buf);
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
    
    for ( i = 0; i < 3; i++)
    {
        memset(&Xliveusers[i],0,sizeof(WINEXLIVEUSER));
        sprintf(Xliveusers[i].username,"Wineplayer%d",i);
        Xliveusers[i].xuid = 0x0000100010001000L * (i+1);
        if ( i == 0 )
            Xliveusers[i].signedin = eXUserSigninState_SignedInLocally;
        else
            Xliveusers[i].signedin = eXUserSigninState_NotSignedIn;
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


INT WINAPI XliveInput(DWORD * p)
{
    p[5] = 0;
    //FIXME("stub: %p\n", p);
    return 1;
}

INT WINAPI XLIVE_5002(void)
{
    //FIXME("stub\n"); Commented out because this is called every frame
    return 0;
}

INT WINAPI XLivePreTranslateMessage(DWORD unknown)
{
    //FIXME("stub: %d\n", unknown);
    return 0;
}

DWORD WINAPI XUserReadProfileSettingsByXuid(
         DWORD dwTitleId,
         DWORD dwUserIndexRequester,
         DWORD dwNumFor,
         const XUID *pxuidFor,
         DWORD dwNumSettingIds,
         const DWORD *pdwSettingIds,
         DWORD *pcbResults,
         PXUSER_READ_PROFILE_SETTING_RESULT pResults,
         void * pXOverlapped
)
{
    FIXME("partial stub");
    return XUserReadProfileSettings(dwTitleId,dwUserIndexRequester,dwNumSettingIds,pdwSettingIds,pcbResults,pResults,(DWORD)pXOverlapped);
}

// #5372
DWORD WINAPI xlive_5372 (HANDLE a1, DWORD a2, DWORD a3, DWORD a4, BYTE *a5, HANDLE a6)
{
    FIXME("stub: %d %d %d %d %p %d\n",a1,a2,a3,a4,a5,a6);
    return 1;
}
INT WINAPI XUserGetName(DWORD dwUserId, char * pBuffer, DWORD dwBufLen)
{
    FIXME("stub: %d %p %d\n", dwUserId, pBuffer, dwBufLen);
    if ( dwUserId > 2 )
        return ERROR_NO_SUCH_USER;
    if ( !Xliveusers[dwUserId].signedin  )
    {
        return ERROR_NO_SUCH_USER;
    }
    if (!pBuffer)
        return 1;
    lstrcpynA(pBuffer,Xliveusers[dwUserId].username,dwBufLen);
    return ERROR_SUCCESS;
}


INT WINAPI XUserGetSigninInfo(DWORD dwUser, DWORD dwFlags, XUSER_SIGNIN_INFO * pInfo)
{
    FIXME("stub: %d %d %p\n", dwUser, dwFlags, pInfo);
    if ( !pInfo || dwUser > 2 )
        return ERROR_NO_SUCH_USER;
    memset(pInfo,0,sizeof(XUSER_SIGNIN_INFO));
    if ( dwFlags & XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY )
        pInfo->xuid = INVALID_XUID;
    else
        pInfo->xuid = Xliveusers[dwUser].xuid;
    pInfo->dwInfoFlags = 0;
    pInfo->UserSigninState = eXUserSigninState_SignedInLocally;
    pInfo->dwGuestNumber = 0;
    pInfo->dwSponsorUserIndex = 0;
    lstrcpynA(pInfo->szUserName,Xliveusers[dwUser].username,15);
    return ERROR_SUCCESS;
}

INT WINAPI XNotifyCreateListener(DWORD unknown, DWORD unknown2)
{
    FIXME("stub: %d %d\n", unknown, unknown2);
    return 1;
}

INT WINAPI XLiveInitializeEx(XLIVE_INITIALIZE_INFO * pXii, DWORD dwVersion)
{
    
    FIXME("stub: %p %d\n", pXii, dwVersion);
    
    
    //TODO: Determine titelid correctly, for now we have to use crc32 of game executable
    char fullpathtogame[1024];
    GetModuleFileNameA(0,fullpathtogame,1023);
    HANDLE exeFile = CreateFileA(fullpathtogame,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if ( exeFile == INVALID_HANDLE_VALUE )
    {
        ERR("Cannot read executable");
        return E_FAIL;
    }
    char * crcbuf = fullpathtogame; //Will invalidate fullpathtogame , but it is no longer needed and that will save stack space
    DWORD bytesread = 1;
    DWORD crc32 = 0x0;
    while ( ReadFile(exeFile,crcbuf,1024,&bytesread,NULL) && bytesread )
    {
        crc32 = XLIVEPRIVComputeCrc32(crcbuf,bytesread,crc32);
    }
    FIXME("Calculated titleId from crc32 : 0x%08x\n",crc32);
    curr_titleId = crc32;
    CloseHandle(exeFile);
    initialized = TRUE;
    //Try to create xlive directory
    if ( !CreateDirectoryA(xlivebasedir,NULL) )
    {
      if ( GetLastError() != ERROR_ALREADY_EXISTS )
        ERR("Cannot create persistent storage dir!\n");
    }
    DWORD dwTitleId = GetTitleID();
    sprintf(xlivedir,"%s%08x\\",xlivebasedir,dwTitleId);
    if ( !CreateDirectoryA(xlivedir,NULL) )
    {
      if ( GetLastError() != ERROR_ALREADY_EXISTS )
        ERR("Cannot create persistent storage dir!\n");
    }
    char buf[512];
    sprintf(buf,"%sachievements",xlivedir);
    if ( !CreateDirectoryA(buf,NULL) )
    {
      if ( GetLastError() != ERROR_ALREADY_EXISTS )
        ERR("Cannot create achievement storage dir!\n");
    }
    sprintf(buf,"%sproperties",xlivedir);
    if ( !CreateDirectoryA(buf,NULL) )
    {
      if ( GetLastError() != ERROR_ALREADY_EXISTS )
        ERR("Cannot create property storage dir!\n");
    }
    
    
    return 0;
}

INT WINAPI XLiveInitialize(XLIVE_INITIALIZE_INFO * pXii)
{
    FIXME("stub: %p\n", pXii);
    XLiveInitializeEx(pXii,1);
    return 0;
}

INT WINAPI XNetStartup(XNetStartupParams * p)
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
XUSER_SIGNIN_STATE WINAPI XUserGetSigninState(DWORD dwUserIndex)
{
    if ( dwUserIndex > 2 )
        return eXUserSigninState_NotSignedIn;
    return Xliveusers[dwUserIndex].signedin;
}
INT WINAPI XUserGetXUID(DWORD dwUserIndex, PXUID pXuid)
{

    return Xliveusers[dwUserIndex].xuid;
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
void WINAPI XUserSetContext( DWORD dwUserIndex, DWORD dwContextId,DWORD dwContextValue)
{
  if ( dwUserIndex > 2 )
  {
      ERR("invalid user id: %d\n",dwUserIndex);
      return;
  }
  if ( dwContextId != X_CONTEXT_PRESENCE )
  {
      FIXME("Setting contexts different from X_CONTEXT_PRESENCE is not supported yet!\n");
      return;
  }
  
  Xliveusers[dwUserIndex].contextvalue = dwContextValue;

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
  FIXME("stub: type=%d\n",*dwType);
  if (pHandle)
      *pHandle = (HANDLE)(++handlecounter);
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
  
  if ( *pDataSize == 0 )
  {
    *pDataSize = dwInDataSize;
    return 0x8007007A; //Insufficent buffer
  }
  if ( !pOutBuffer )
      return 0x8007007A; //Insufficent buffer
  if (*pDataSize >= dwInDataSize && pOutBuffer)
  {
     HexDump(pInBuffer,dwInDataSize);
     memcpy (pOutBuffer, pInBuffer, dwInDataSize);
  }
  else
      return 0x8007007A; //Insufficent buffer
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
					DWORD * pdwSettingIds, DWORD * pcbResults, PXUSER_READ_PROFILE_SETTING_RESULT pResults, DWORD pOverlapped)
{
    int i = 0;
    char path[1024];
    unsigned char * destptr;
    DWORD size;
    //FIXME("stub: %d %d %d %p %p %p %d\n",dwTitleId,dwUserIndex,dwNumSettingIds,pdwSettingIds,pcbResults,pResults,pOverlapped);
	/*if (*pcbResults < 1036) {
		*pcbResults = 1036;	// TODO: make correct calculation by IDs.
		return ERROR_INSUFFICIENT_BUFFER;
	}*/
    
    if (*pcbResults == 0 || !pResults)
    {
        *pcbResults = sizeof(DWORD)+sizeof(XUSER_PROFILE_SETTING)*dwNumSettingIds;
        return 0;
    }
    memset (pResults, 0, *pcbResults);
    for ( i = 0; i < dwNumSettingIds; i++ )
    {
        DWORD id = pdwSettingIds[i];
        if ( XISSYSTEMPROPERTY(id) )
        {
            FIXME("Setting ID: %d : System properties not supported , returning zeroed data\n",id);
            continue;
        }
        sprintf(path,"%sproperties\\%08X",xlivedir,id);
        HANDLE propFile = CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if ( propFile == INVALID_HANDLE_VALUE )
        {
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

        DWORD bytesread;
        switch ( XPROPERTYTYPEFROMID(id) )
        {
            case XUSER_DATA_TYPE_CONTEXT:
                FIXME("Context value not handled yet!\n");
                break;
            case XUSER_DATA_TYPE_INT32:
                ReadFile(propFile,&pResults->pSettings[i].data.nData,4,&bytesread,NULL);
                if ( bytesread != 4 )
                {
                    ERR("Not enough bytes on setting %08x\n",id);
                }
                break;
            case XUSER_DATA_TYPE_INT64:
                ReadFile(propFile,&pResults->pSettings[i].data.i64Data,8,&bytesread,NULL);
                if ( bytesread != 8 )
                {
                    ERR("Not enough bytes on setting %08x\n",id);
                }
                break;    
            case XUSER_DATA_TYPE_DOUBLE:
                ReadFile(propFile,&pResults->pSettings[i].data.dblData,sizeof(double),&bytesread,NULL);
                if ( bytesread != sizeof(double) )
                {
                    ERR("Not enough bytes on setting %08x\n",id);
                }
                break;
            case XUSER_DATA_TYPE_DATETIME:
                ReadFile(propFile,&pResults->pSettings[i].data.ftData,sizeof(FILETIME),&bytesread,NULL);
                if ( bytesread != sizeof(FILETIME) )
                {
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
        if ( XPROPERTYTYPEFROMID(id) == XUSER_DATA_TYPE_CONTEXT )
        {
            FIXME("Context value not handled yet!\n");
        }
        
        
        
    }
    
    
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
    
    if (!pDataSize || !ph)      // invalid parameter
                return E_FAIL;
    FIXME("stub: %d %d %p %p(%d) %p\n",pInBuffer,dwInDataSize, pOutBuffer,pDataSize,*pDataSize,ph);
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
    HexDump(pInBuffer,dwInDataSize);
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
INT WINAPI XUserWriteAchievements (DWORD dwNumAchievements,
         CONST XUSER_ACHIEVEMENT *pAchievements,
         void *pOverlapped)
{
    XUSER_ACHIEVEMENT * curr = pAchievements;
    int k = 0;
    for ( k = 0; k < dwNumAchievements; k++ )
    {
        char buf[512];
        sprintf(buf,"%sachievements\\%d",xlivedir,curr->dwUserIndex);
        if (!DirectoryExists(buf))
        {
            if ( !CreateDirectoryA(buf,NULL))
            {
                ERR("Cannot create directory %s\n",buf);
                return E_FAIL;
            }
        }
        sprintf(buf,"%sachievements\\%d\\%d.achievement",xlivedir,curr->dwUserIndex,curr->dwAchievementId);
        HANDLE hFile = CreateFileA(buf,GENERIC_WRITE,0x0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            ERR("Cannot create file %s\n",buf);
            return E_FAIL;
        }
        
    }
    return 0;
}
INT WINAPI XUserCreateAchievementEnumerator(DWORD dwTitleId, DWORD dwUserIndex, ULONGLONG xuid, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, DWORD * pcbBuffer, HANDLE * phEnum)
{
    FIXME("stub\n");
    if (pcbBuffer)
        *pcbBuffer = 0;
    if (phEnum)
            *phEnum = INVALID_HANDLE_VALUE;
    return 1;   // return error (otherwise, 0-size buffer will be allocated)
}
INT WINAPI XUserReadStats (DWORD dwTitleId,
                           DWORD dwNumXuids,
                           CONST XUID *pXuids,
                           DWORD dwNumStatsSpecs,
                           CONST XUSER_STATS_SPEC *pSpecs,
                           PDWORD *pcbResults,
                           PXUSER_STATS_READ_RESULTS pResults,
                           void *pOverlapped)
{
    FIXME("stub\n");
    if (pcbResults)
    {
        *pcbResults = sizeof(DWORD);
        return 0;
    }
    if (pResults)
    {
            pResults->dwNumViews = 0;
    }
    return 0;
}

typedef struct{
        DWORD   dwMagick;       
        DWORD   dwSize;
        DWORD   __fill[2];
        BYTE    bData[4];
} FakeProtectedBuffer;
DWORD WINAPI XLivePBufferAllocate (int size, FakeProtectedBuffer ** pBuffer) {
    *pBuffer = (FakeProtectedBuffer *)malloc (size+16);
    if (!*pBuffer) {
            return E_OUTOFMEMORY;
    }

    (*pBuffer)->dwMagick = 0xDEADDEAD;      // some arbitrary number
    (*pBuffer)->dwSize = size;
    return 0;
}
DWORD WINAPI XLivePBufferFree(FakeProtectedBuffer * pBuffer)
{
    FIXME("stub\n");
    if (pBuffer && pBuffer->dwMagick == 0xDEADDEAD)
        free (pBuffer);
    return 0;
}
DWORD WINAPI XLivePBufferSetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * source, DWORD size) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !source || offset < 0 || offset+size > pBuffer->dwSize)
                return 0;
        memcpy (pBuffer->bData+offset, source, size);
        return 0;
}
DWORD WINAPI XLivePBufferGetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * destination, DWORD size) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !destination || offset < 0 || offset+size > pBuffer->dwSize)
                return 0;
        memcpy (destination, pBuffer->bData+offset, size);
        return 0;
}
DWORD WINAPI XLivePBufferSetByte(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE value) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || offset < 0 || offset > pBuffer->dwSize)
                return 0;
        pBuffer->bData[offset] = value;
        return 0;
}
DWORD WINAPI XLivePBufferGetByte (FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * value) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || !value || offset < 0 || offset > pBuffer->dwSize)
                return 0;
        *value = pBuffer->bData[offset];
        return 0;
}
DWORD WINAPI XLivePBufferGetDWORD (FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD * pdwValue) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || dwOffset < 0 || dwOffset > pBuffer->dwSize-4 || !pdwValue)
                return 0;
        *pdwValue = *(DWORD *)(pBuffer->bData+dwOffset);
        return 0;
}
DWORD WINAPI XLivePBufferSetDWORD (FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD dwValue ) {
        if (!pBuffer || pBuffer->dwMagick != 0xDEADDEAD || dwOffset < 0 || dwOffset > pBuffer->dwSize-4)
                return 0;
        *(DWORD *)(pBuffer->bData+dwOffset) = dwValue;
        return 0;
}

DWORD WINAPI XLiveContentCreateEnumerator (DWORD a1, void * a2, DWORD *pchBuffer, HANDLE * phContent) {
  FIXME("stub: %d %p %p %p\n",a1,a2,pchBuffer,phContent);
	if (phContent)
		*phContent = INVALID_HANDLE_VALUE;
	return 1;
}
DWORD WINAPI XLIVE_5313(DWORD dw1)
{
  FIXME("Unknown , dw1=%d\n",dw1);
  return 0;
}

DWORD WINAPI XLiveSetDebugLevel (DWORD xdlLevel, DWORD * pxdlOldLevel) { 
	FIXME ("XLiveSetDebugLevel (%d)\n", xdlLevel);
	return 0;
}
INT FUNC001(DWORD * p1 , DWORD * p2)
{
  p2[0] = p1[1];
  p2[1] = p1[2];
  memcpy(&p2[2],&p1[3],0x14);
  return 0;
}
DWORD WINAPI XLIVE_5356(DWORD p1,DWORD* p2,DWORD p3,DWORD *p4 /*some buffer size*/)
{
  FIXME ("UNK %d %p %d %p \n",p1,p2,p3,p4);
  FIXME ("%p(%x) %p(%x)\n",p2,*p2,p4,*p4);
  if ( *p4 == 0 )
  {
    *p4 = 0x10;//totally unk
    return -2147024774;
  }
  //if ( p1 || !p2 || !p4 || !p3 && !p4 )
    return -2147024809;
  DWORD v6[0x14+2];
  int result = FUNC001(p2,v6);
  //8c4 access
	return 0;
}
DWORD WINAPI XCloseHandle(DWORD p1)
{
  FIXME ("%d\n",p1);
  if ( !p1 || p1 == -1 )
  {
    SetLastError(87);
  }
  return 0;
}
DWORD WINAPI XLIVE_5355(DWORD errDesc , DWORD p2 , char* p3 , DWORD *p4) // maybe some func to get extended error str?
{
  FIXME("stub\n");
  if ( *p4 > 0 )
    p3[0] = 0x0;
    
  return 0;
}
DWORD WINAPI XLiveContentCreateAccessHandle(DWORD dwTitleId, void * pContentInfo, 
	DWORD dwLicenseInfoVersion, void * xebBuffer, DWORD dwOffset, HANDLE * phAccess, void * pOverlapped)
{
  FIXME("stub\n");
  if (phAccess)
		*phAccess = INVALID_HANDLE_VALUE;
  return E_OUTOFMEMORY;	

}

/* #5293: XUserSetPropertyEx*/
INT WINAPI XUserSetPropertyEx (DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, void * pvValue, void * pOverlapped) { 
       // FIXME ("XUserSetPropertyEx (%d, 0x%x, ...)\n", dwUserIndex, dwPropertyId);
    char buf[512];
    if (pOverlapped )
        FIXME("Overlapped ignored!\n");
    sprintf(buf,"%sproperties\\%08X",xlivedir,dwPropertyId);
    HANDLE hFile = CreateFileA(buf,GENERIC_WRITE,0x0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        ERR("Cannot write property!\n");
        return ERROR_ACCESS_DENIED;
    }
    DWORD written;
    WriteFile(hFile,pvValue,cbValue,&written,NULL);
    if (written < cbValue )
    {
        ERR("Cannot write property!\n");
        return ERROR_ACCESS_DENIED;
    }
    CloseHandle(hFile);
    return ERROR_SUCCESS;
}

DWORD WINAPI XLIVE_76(DWORD a1)
{
    FIXME("unk(%d), returning 0\n",a1);
    return 0;
}
DWORD WINAPI XLIVE_78(DWORD a1,DWORD a2,DWORD a3)
{
    FIXME("unk %d %d %d, returning 0\n",a1,a2,a3);
    return 0;

}

void WINAPI XUserSetProperty(
         DWORD dwUserIndex,
         DWORD dwPropertyId,
         DWORD cbValue,
         CONST VOID *pvValue
)
{
    XUserSetPropertyEx(dwUserIndex,dwPropertyId,cbValue,pvValue,0x0);
}
