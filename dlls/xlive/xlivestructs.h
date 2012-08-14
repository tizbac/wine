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

typedef struct _XUSER_READ_PROFILE_SETTING_RESULT {
    DWORD dwSettingsLen;
    XUSER_PROFILE_SETTING *pSettings;
} XUSER_READ_PROFILE_SETTING_RESULT, *PXUSER_READ_PROFILE_SETTING_RESULT;


typedef struct {
    XUSER_SIGNIN_STATE signedin;
    CHAR username[XUSER_NAME_SIZE];
    ULONGLONG xuid;
    DWORD contextvalue;
} WINEXLIVEUSER;
