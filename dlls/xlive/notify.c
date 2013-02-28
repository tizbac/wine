#include "config.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winsock2.h"
#include "wine/debug.h"
#include <wine/server_protocol.h>
#include <stdlib.h>
#include "xlivestructs.h"
WINE_DEFAULT_DEBUG_CHANNEL(xlive_notify);
typedef struct {
    int type;
    char param[128];
} XLiveEvent;
typedef struct {
    unsigned long long areas;
    int eventcount;
    XLiveEvent events[128];

} XLiveListener;

XLiveListener * xlive_g_listeners = 0x0;
int xlive_g_listener_count = 0;
void _XListenerAddEvent_PDW(XLiveListener *l, int type, DWORD param )
{
    if ( l->eventcount >= 128 )
    {
        ERR("Too many unprocessed events!\n");
        return;
    }
    memcpy(l->events[l->eventcount].param,&param,sizeof(param));
    l->events[l->eventcount].type = type;
    
    FIXME("trace: new event: %d %d(%p)\n",type,param,l->events[l->eventcount].param);
    l->eventcount++;
}
void _XListenerAddEvent_VOID(XLiveListener *l, int type)
{
    if ( l->eventcount >= 128 )
    {
        ERR("Too many unprocessed events!\n");
        return;
    }
    l->events[l->eventcount].type = type;
    FIXME("trace: new event: %d (%p)\n",type,l->events[l->eventcount].param);
    l->eventcount++;
}

void _XNotifySigninChanged_Event(void) {
    int i;
    for ( i = 0; i < xlive_g_listener_count; i++ )
    {
        if ( xlive_g_listeners[i].areas & XNOTIFY_SYSTEM )
        {
            _XListenerAddEvent_PDW(&xlive_g_listeners[i],XN_SYS_SIGNINCHANGED,1);
            
        }
        
    }
}
void _XNotifyPresenceChanged_Event(void) {
    
    int i;
    for ( i = 0; i < xlive_g_listener_count; i++ )
    {
        if ( xlive_g_listeners[i].areas & XNOTIFY_LIVE )
        {
            _XListenerAddEvent_VOID(&xlive_g_listeners[i],XN_LIVE_PRESENCE_CHANGED);
            
        }
        
    }
}


// #5270: XNotifyCreateListener
//TODO: WaitForSingleObject support, atm only polling is supported
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

BOOL _XNotifyRemoveEvent(XLiveListener *l,int index,  DWORD * pdwId, void ** pParam) {
    int after;
    int k;
    XLiveEvent * ev;

    if ( index >= l->eventcount )
        return 0;
    *pdwId = l->events[index].type;
    *pParam = l->events[index].param;
    after = l->eventcount-(index+1);
    //Shift the event list to the left
    for ( ev = &l->events[index],k=0;k < after; k++) {
        XLiveEvent * next = ++ev;
        memcpy(ev,next,sizeof(XLiveEvent));
        //WARNING: High chance of crap code, coded at 2:25 AM
    }
    l->eventcount--;
    FIXME("trace: event delivered: %d %p\n",*pdwId,*pParam);
    return 1;
}
// #651: XNotifyGetNext
BOOL WINAPI XNotifyGetNext(HANDLE hNotificationListener, DWORD dwMsgFilter, DWORD * pdwId, void ** pParam) {
    TRACE("(%d %d %p %p)\n",hNotificationListener,dwMsgFilter,pdwId,pParam);
    int index = (int)(hNotificationListener-1);
    if ( !hNotificationListener )
        return 0;
    if ( hNotificationListener > xlive_g_listener_count )
    {
        ERR("Invalid handle");
        return 0;
    }
    if ( !dwMsgFilter && xlive_g_listeners[index].eventcount )
    {
        return _XNotifyRemoveEvent(&xlive_g_listeners[index],0,pdwId,pParam);
    }
    if ( xlive_g_listeners[index].eventcount && dwMsgFilter )
    {
        int i;
        for ( i = 0; i < xlive_g_listeners[index].eventcount; i++ )
        {
            if ( xlive_g_listeners[index].events[i].type == dwMsgFilter )
            {
                return _XNotifyRemoveEvent(&xlive_g_listeners[index],i,pdwId,pParam);
            }
            
        }
    }
    return 0;
}

// #652: XNotifyPositionUI
INT WINAPI XNotifyPositionUI(DWORD dwPosition) {
    FIXME("stub: (%d)\n", dwPosition);
        return 0;
}

// #653: XNotifyDelayUI
INT WINAPI XNotifyDelayUI(long delay) {
    FIXME("stub: (%ld)\n", delay);
    return 0;
}
