#include "config.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winsock2.h"
#include "wine/debug.h"
#include <wine/server_protocol.h>
#include "stdlib.h"
#include "xlivestructs.h"
WINE_DEFAULT_DEBUG_CHANNEL(xlive_notify);
typedef struct {
    int type;
    void * param;
    int paramsize;
} XLiveEvent;
typedef struct {
    unsigned long long areas;
    int eventcount;
    XLiveEvent events[128];

} XLiveListener;

XLiveListener * xlive_g_listeners = 0x0;
int xlive_g_listener_count = 0;
INT WINAPI XNotifyCreateListener(unsigned long long qwAreas)
{
    FIXME("trace");
    if ( !xlive_g_listeners )
    {
        xlive_g_listeners = malloc(sizeof(XLiveListener));
    }else{
        xlive_g_listeners = realloc(xlive_g_listeners,sizeof(XLiveListener)*(xlive_g_listener_count+1));
    }
    xlive_g_listeners[xlive_g_listener_count].areas = qwAreas;
    xlive_g_listeners[xlive_g_listener_count].eventcount = 0;
    xlive_g_listener_count++;
    return xlive_g_listener_count;
}

BOOL _XNotifyRemoveEvent(XLiveListener *l,int index,  DWORD * pdwId, void * pParam)
{
    if ( index >= l->eventcount )
        return 0;
    *pdwId = l->events[index].type;
    *pParam = l->events[index].param;
    int after = l->eventcount-(index+1);
    //Shift the event list to the left
    int k;
    for ( XLiveEvent * ev = &l->events[index], k = 0;k < after; k++)
    {
        XLiveEvent * next = ++ev;
        memcpy(ev,next,sizeof(XLiveEvent));
        //WARNING: High chance of crap code, coded at 2:25 AM
    }
    l->eventcount--;
    return 1;
}

BOOL WINAPI XNotifyGetNext(HANDLE hNotificationListener, DWORD dwMsgFilter, DWORD * pdwId, void * pParam)
{
    if ( !hNotificationListener )
        return 0;
    if ( hNotificationListener > xlive_g_listener_count )
    {
        ERR("Invalid handle");
        return 0;
    }
    if ( !dwMsgFilter && xlive_g_listeners[hNotificationListener-1].eventcount )
    {
        return _XNotifyRemoveEvent(&xlive_g_listeners[hNotificationListener-1],0,pdwId,pParam);
    }
    //TODO: Filter
    return 0;
}