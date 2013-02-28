#include <windows.h>
#define XUSER_NAME_SIZE                 16
#define XUSER_INFO_FLAG_LIVE_ENABLED    0x00000001
#define XUSER_INFO_FLAG_GUEST           0x00000002
#define XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY      0x00000001
#define XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY     0x00000002
#define INVALID_XUID                    ((XUID) 0)
#define X_PROPERTY_TYPE_MASK            0xF0000000
#define X_PROPERTY_SCOPE_MASK           0x00008000
#define X_PROPERTY_ID_MASK              0x00007FFF
#define XPROPERTYID(global, type, id)   ((global ? X_PROPERTY_SCOPE_MASK : 0) | ((type << 28) & X_PROPERTY_TYPE_MASK) | (id & X_PROPERTY_ID_MASK))
#define XCONTEXTID(global, id)          XPROPERTYID(global, XUSER_DATA_TYPE_CONTEXT, id)
#define XPROPERTYTYPEFROMID(id)         ((id >> 28) & 0xf)
#define XISSYSTEMPROPERTY(id)           (id & X_PROPERTY_SCOPE_MASK)
#define X_CONTEXT_PRESENCE              XCONTEXTID(1, 0x1)
#define X_CONTEXT_GAME_TYPE             XCONTEXTID(1, 0xA)
#define X_CONTEXT_GAME_MODE             XCONTEXTID(1, 0xB)
#define XUSER_DATA_TYPE_CONTEXT     ((BYTE)0)
#define XUSER_DATA_TYPE_INT32       ((BYTE)1)
#define XUSER_DATA_TYPE_INT64       ((BYTE)2)
#define XUSER_DATA_TYPE_DOUBLE      ((BYTE)3)
#define XUSER_DATA_TYPE_UNICODE     ((BYTE)4)
#define XUSER_DATA_TYPE_FLOAT       ((BYTE)5)
#define XUSER_DATA_TYPE_BINARY      ((BYTE)6)
#define XUSER_DATA_TYPE_DATETIME    ((BYTE)7)
#define XUSER_DATA_TYPE_NULL        ((BYTE)0xFF)

#define XNOTIFY_SYSTEM                  (0x00000001)
#define XNOTIFY_LIVE                    (0x00000002)
#define XNOTIFY_FRIENDS                 (0x00000004)
#define XNOTIFY_CUSTOM                  (0x00000008)
#define XNOTIFY_XMP                     (0x00000020)
#define XNOTIFY_MSGR                    (0x00000040)
#define XNOTIFY_PARTY                   (0x00000080)
#define XNOTIFY_ALL                     (XNOTIFY_SYSTEM | XNOTIFY_LIVE | XNOTIFY_FRIENDS | XNOTIFY_CUSTOM | XNOTIFY_XMP | XNOTIFY_MSGR | XNOTIFY_PARTY)
#define XSSUI_FLAGS_LOCALSIGNINONLY                 0x00000001
#define XSSUI_FLAGS_SHOWONLYONLINEENABLED           0x00000002
#define XSSUI_FLAGS_ALLOW_SIGNOUT                   0x00000004
#define XSSUI_FLAGS_DISALLOW_PLAYAS                 0x00000010
#define XSSUI_FLAGS_ADDUSER                         0x00010000
#define XSSUI_FLAGS_ENABLE_GUEST                    0x00080000
#define XSSUI_FLAGS_CONVERTOFFLINETOGUEST           0x00400000
#define XSSUI_FLAGS_DISALLOW_GUEST                  0x01000000



#define XNID(Version, Area, Index)      (DWORD)( (WORD)(Area) << 25 | (WORD)(Version) << 16 | (WORD)(Index))
#define XNID_VERSION(msgid)             (((msgid) >> 16) & 0x1FF)
#define XNID_AREA(msgid)                (((msgid) >> 25) & 0x3F)
#define XNID_INDEX(msgid)               ((msgid) & 0xFFFF)

#define _XNAREA_SYSTEM                  (0)
#define _XNAREA_LIVE                    (1)
#define _XNAREA_FRIENDS                 (2)
#define _XNAREA_CUSTOM                  (3)
#define _XNAREA_XMP                     (5)
#define _XNAREA_MSGR                    (6)
#define _XNAREA_PARTY                   (7)


#define XN_SYS_FIRST                    XNID(0, _XNAREA_SYSTEM, 0x0001)
#define XN_SYS_UI                       XNID(0, _XNAREA_SYSTEM, 0x0009)
#define XN_SYS_SIGNINCHANGED            XNID(0, _XNAREA_SYSTEM, 0x000a)
#define XN_SYS_STORAGEDEVICESCHANGED    XNID(0, _XNAREA_SYSTEM, 0x000b)
#define XN_SYS_PROFILESETTINGCHANGED    XNID(0, _XNAREA_SYSTEM, 0x000e)
#define XN_SYS_MUTELISTCHANGED          XNID(0, _XNAREA_SYSTEM, 0x0011)
#define XN_SYS_INPUTDEVICESCHANGED      XNID(0, _XNAREA_SYSTEM, 0x0012)
#define XN_SYS_INPUTDEVICECONFIGCHANGED XNID(1, _XNAREA_SYSTEM, 0x0013)
#define XN_SYS_PLAYTIMERNOTICE          XNID(3, _XNAREA_SYSTEM, 0x0015)
#define XN_SYS_AVATARCHANGED            XNID(4, _XNAREA_SYSTEM, 0x0017)
#define XN_SYS_NUIHARDWARESTATUSCHANGED XNID(6, _XNAREA_SYSTEM, 0x0019)
#define XN_SYS_NUIPAUSE                 XNID(6, _XNAREA_SYSTEM, 0x001a)
#define XN_SYS_NUIUIAPPROACH            XNID(6, _XNAREA_SYSTEM, 0x001b)
#define XN_SYS_DEVICEREMAP              XNID(6, _XNAREA_SYSTEM, 0x001c)
#define XN_SYS_NUIBINDINGCHANGED        XNID(6, _XNAREA_SYSTEM, 0x001d)
#define XN_SYS_AUDIOLATENCYCHANGED      XNID(8, _XNAREA_SYSTEM, 0x001e)
#define XN_SYS_NUICHATBINDINGCHANGED    XNID(8, _XNAREA_SYSTEM, 0x001f)
#define XN_SYS_INPUTACTIVITYCHANGED     XNID(9, _XNAREA_SYSTEM, 0x0020)
#define XN_SYS_LAST                     XNID(0, _XNAREA_SYSTEM, 0x0023)
//TODO: Ensure these two below are correct
#define XLIVE_CONTENT_FLAG_RETRIEVE_FOR_ALL_USERS 0x1
#define XLIVE_CONTENT_FLAG_RETRIEVE_BY_XUID 0x8 



#define XNET_OPTID_STARTUP_PARAMS                   1
#define XNET_OPTID_NIC_XMIT_BYTES                   2
#define XNET_OPTID_NIC_XMIT_FRAMES                  3
#define XNET_OPTID_NIC_RECV_BYTES                   4
#define XNET_OPTID_NIC_RECV_FRAMES                  5
#define XNET_OPTID_CALLER_XMIT_BYTES                6
#define XNET_OPTID_CALLER_XMIT_FRAMES               7
#define XNET_OPTID_CALLER_RECV_BYTES                8
#define XNET_OPTID_CALLER_RECV_FRAMES               9

#define XNET_XNQOSINFO_COMPLETE         0x01    // Qos has finished processing this entry
#define XNET_XNQOSINFO_TARGET_CONTACTED 0x02    // Target host was successfully contacted
#define XNET_XNQOSINFO_TARGET_DISABLED  0x04    // Target host has disabled its Qos listener
#define XNET_XNQOSINFO_DATA_RECEIVED    0x08    // Target host supplied Qos data
#define XNET_XNQOSINFO_PARTIAL_COMPLETE 0x10    // Qos has unfinished estimates for this entry

#define XN_LIVE_FIRST                   XNID(0, _XNAREA_LIVE, 0x0001)
#define XN_LIVE_CONNECTIONCHANGED       XNID(0, _XNAREA_LIVE, 0x0001)
#define XN_LIVE_INVITE_ACCEPTED         XNID(0, _XNAREA_LIVE, 0x0002)
#define XN_LIVE_LINK_STATE_CHANGED      XNID(0, _XNAREA_LIVE, 0x0003)
#define XN_LIVE_CONTENT_INSTALLED       XNID(0, _XNAREA_LIVE, 0x0007)
#define XN_LIVE_MEMBERSHIP_PURCHASED    XNID(0, _XNAREA_LIVE, 0x0008)
#define XN_LIVE_VOICECHAT_AWAY          XNID(0, _XNAREA_LIVE, 0x0009)
#define XN_LIVE_PRESENCE_CHANGED        XNID(0, _XNAREA_LIVE, 0x000A)
#define XN_LIVE_LAST                    XNID(XNID_CURRENTVERSION+1, _XNAREA_LIVE, 0x0014)

typedef struct {
    BYTE bFlags;
    BYTE bReserved;
    WORD cProbesXmit;
    BYTE cProbesRecv;
    WORD cbData;
    BYTE *pbData;
    WORD wRttMinInMsecs;
    WORD wRttMedInMsecs;
    DWORD dwUpBitsPerSec;
    DWORD dwDnBitsPerSec;
} XNQOSINFO ;
typedef struct {
    UINT cxnqos;
    UINT cxnqosPending;
    XNQOSINFO axnqosinfo[1];
} XNQOS;

typedef enum 
{
    XONLINE_NAT_OPEN = 1,
    XONLINE_NAT_MODERATE,
    XONLINE_NAT_STRICT
} XONLINE_NAT_TYPE;
struct XUSER_READ_PROFILE_SETTINGS {
	DWORD	dwLength;
	BYTE *	pSettings;
};
typedef ULONGLONG                       XUID;
typedef XUID                            *PXUID;
typedef struct {
    DWORD dwId;
    LPWSTR pwszLabel;
    LPWSTR pwszDescription;
    LPWSTR pwszUnachieved;
    DWORD dwImageId;
    DWORD dwCred;
    FILETIME ftAchieved;
    DWORD dwFlags;
} XACHIEVEMENT_DETAILS, *PXACHIEVEMENT_DETAILS;

typedef struct {
    DWORD dwUserIndex;
    DWORD dwAchievementId;
} XUSER_ACHIEVEMENT, *PXUSER_ACHIEVEMENT;

typedef struct {
    BYTE        cfgSizeOfStruct;
    BYTE        cfgFlags;
    BYTE        cfgSockMaxDgramSockets;
    BYTE        cfgSockMaxStreamSockets;
    BYTE        cfgSockDefaultRecvBufsizeInK;
    BYTE        cfgSockDefaultSendBufsizeInK;
    BYTE        cfgKeyRegMax;
    BYTE        cfgSecRegMax;
    BYTE        cfgQosDataLimitDiv4;
    BYTE        cfgQosProbeTimeoutInSeconds;
    BYTE        cfgQosProbeRetries;
    BYTE        cfgQosSrvMaxSimultaneousResponses;
    BYTE        cfgQosPairWaitTimeInSeconds;
} XNetStartupParams;

typedef struct {
    DWORD dw1; //28 on tron evolution
    DWORD dw2;
    DWORD dw3;
} XLIVE_INITIALIZE_INFO;

typedef enum _XUSER_SIGNIN_STATE
{
    eXUserSigninState_NotSignedIn = 0x0 ,
    eXUserSigninState_SignedInLocally = 0x1,
    eXUserSigninState_SignedInToLive =0x2 
} XUSER_SIGNIN_STATE;

typedef struct _XUSER_SIGNIN_INFO {
    XUID xuid;
    DWORD dwInfoFlags;
    XUSER_SIGNIN_STATE UserSigninState;
    DWORD dwGuestNumber;
    DWORD dwSponsorUserIndex;
    CHAR szUserName[XUSER_NAME_SIZE];
} XUSER_SIGNIN_INFO, *PXUSER_SIGNIN_INFO;

#define X_PROPERTY_RANK                 XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x1)
#define X_PROPERTY_GAMERNAME            XPROPERTYID(1, XUSER_DATA_TYPE_UNICODE, 0x2)
#define X_PROPERTY_SESSION_ID           XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x3)
#define X_PROPERTY_GAMER_ZONE           XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x101)
#define X_PROPERTY_GAMER_COUNTRY        XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x102)
#define X_PROPERTY_GAMER_LANGUAGE       XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x103)
#define X_PROPERTY_GAMER_RATING         XPROPERTYID(1, XUSER_DATA_TYPE_FLOAT,   0x104)
#define X_PROPERTY_GAMER_MU             XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0x105)
#define X_PROPERTY_GAMER_SIGMA          XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0x106)
#define X_PROPERTY_GAMER_PUID           XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x107)
#define X_PROPERTY_AFFILIATE_SCORE      XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x108)
#define X_PROPERTY_GAMER_HOSTNAME       XPROPERTYID(1, XUSER_DATA_TYPE_UNICODE, 0x109)
#define X_PROPERTY_SESSION_SCORE        XPROPERTYID(1, XUSER_DATA_TYPE_FLOAT,   0x10A)
#define XPROPERTYTYPEFROMID(id)         ((id >> 28) & 0xf)
typedef struct  {
    BYTE type;
    union {
        LONG nData;
        LONGLONG i64Data;
        double dblData;
        struct {
            DWORD cbData;
            LPWSTR pwszData;
        }string;
        float fData;
        struct {
            DWORD cbData;
            PBYTE pbData;
        }binary;
        FILETIME ftData;
    };
} XUSER_DATA, *PXUSER_DATA;

typedef enum _XUSER_PROFILE_SOURCE
{
    XSOURCE_NO_VALUE = 0,
    XSOURCE_DEFAULT,
    XSOURCE_TITLE,
    XSOURCE_PERMISSION_DENIED
} XUSER_PROFILE_SOURCE;

typedef struct _XUSER_PROFILE_SETTING {
    XUSER_PROFILE_SOURCE source;
    union {
        DWORD dwUserIndex;
        XUID xuid;
    }user;
    DWORD dwSettingId;
    XUSER_DATA data;
} XUSER_PROFILE_SETTING, *PXUSER_PROFILE_SETTING;
#define XUSER_STATS_ATTRS_IN_SPEC       64
typedef struct _XUSER_READ_PROFILE_SETTING_RESULT {
    DWORD dwSettingsLen;
    XUSER_PROFILE_SETTING *pSettings;
} XUSER_READ_PROFILE_SETTING_RESULT, *PXUSER_READ_PROFILE_SETTING_RESULT;
typedef struct _XUSER_STATS_COLUMN {
    WORD wColumnId;
    XUSER_DATA Value;
} XUSER_STATS_COLUMN, *PXUSER_STATS_COLUMN;
typedef struct _XUSER_STATS_ROW {
    XUID xuid;
    DWORD dwRank;
    LONGLONG i64Rating;
    CHAR szGamertag[XUSER_NAME_SIZE];
    DWORD dwNumColumns;
    PXUSER_STATS_COLUMN pColumns;
} XUSER_STATS_ROW, *PXUSER_STATS_ROW;

typedef struct _XUSER_STATS_VIEW {
    DWORD dwViewId;
    DWORD dwTotalViewRows;
    DWORD dwNumRows;
    PXUSER_STATS_ROW pRows;
} XUSER_STATS_VIEW, *PXUSER_STATS_VIEW;

typedef struct _XUSER_STATS_READ_RESULTS {
    DWORD dwNumViews;
    PXUSER_STATS_VIEW pViews;
} XUSER_STATS_READ_RESULTS, *PXUSER_STATS_READ_RESULTS;

typedef struct _XUSER_STATS_SPEC {
    DWORD dwViewId;
    DWORD dwNumColumnIds;
    WORD rgwColumnIds[XUSER_STATS_ATTRS_IN_SPEC];
} XUSER_STATS_SPEC, *PXUSER_STATS_SPEC;
typedef struct {
    XUSER_SIGNIN_STATE signedin;
    CHAR username[XUSER_NAME_SIZE];
    ULONGLONG xuid;
    DWORD contextvalue;
} WINEXLIVEUSER;


typedef struct  {
    DWORD dwPropertyId;
    XUSER_DATA value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;
typedef struct  {
    DWORD dwContextId;
    DWORD dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;
typedef VOID (*PXOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,void* /*TODO: WTF */ pOverlapped);
typedef struct _XOVERLAPPED {
    ULONG_PTR InternalLow;
    ULONG_PTR InternalHigh;
    ULONG_PTR InternalContext;
    HANDLE hEvent;
    PXOVERLAPPED_COMPLETION_ROUTINE pCompletionRoutine;
    DWORD_PTR dwCompletionContext;
    DWORD dwExtendedError;
} XOVERLAPPED, *PXOVERLAPPED;
typedef struct {
    IN_ADDR ina;
    IN_ADDR inaOnline;
    WORD wPortOnline;
    BYTE abEnet[6];
    BYTE abOnline[20];
} XNADDR;
typedef struct {
    BYTE        ab[16];                         // xbox to xbox key exchange key
} XNKEY;
typedef struct {
    BYTE        ab[8];                          // xbox to xbox key identifier
} XNKID;
typedef struct {
    XNKID sessionID;
    XNADDR hostAddress;
    XNKEY keyExchangeKey;
} XSESSION_INFO, *PXSESSION_INFO;
typedef struct {
    XSESSION_INFO info;
    DWORD dwOpenPublicSlots;
    DWORD dwOpenPrivateSlots;
    DWORD dwFilledPublicSlots;
    DWORD dwFilledPrivateSlots;
    DWORD cProperties;
    DWORD cContexts;
    PXUSER_PROPERTY pProperties;
    PXUSER_CONTEXT pContexts;
} XSESSION_SEARCHRESULT, *PXSESSION_SEARCHRESULT;
typedef struct {
    DWORD dwSearchResults;
    XSESSION_SEARCHRESULT *pResults;
} XSESSION_SEARCHRESULT_HEADER, *PXSESSION_SEARCHRESULT_HEADER;

typedef struct {
    XSESSION_SEARCHRESULT xlive_session;
    unsigned long long last_announce_ms;
    BOOL isCreatedLocal;
    DWORD dwFlags;
} WINEXLIVESESSION;