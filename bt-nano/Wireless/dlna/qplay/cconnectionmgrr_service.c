/************************************************************
*
*    CyberLink for C
*
*    Copyright (C) Satoshi Konno 2009
*
*    File: cconnectionmgr_service.c
*
*    Revision:
*        2009/06/11
*        - first release.
*
************************************************************/
#include "BspConfig.h"
#ifdef __WIFI_UPNP_C__
#define NOT_INCLUDE_OTHER

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cybergarage/upnp/std/av/cmediarenderer.h>
#include <cybergarage/upnp/std/av/ccontent.h>
#include <cybergarage/upnp/std/av/cresource.h>
#include "dlna.h"

//#pragma arm section code = "upnpCode", rodata = "upnpCode", rwdata = "upnpdata", zidata = "upnpdata"


/****************************************
* Service Description (Connection Manager)
****************************************/

#if defined(__arm__) && defined(__ARMCC_VERSION)
__attribute__((used, section("CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION"))) static char CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION[] =
#elif defined(__arm__) && defined(__GNUC__)
static char __attribute__((used, section("__CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION"))) CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION[] =

#else

#endif

"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\n"
"<specVersion>\n"
"<major>1</major>\n"
"<minor>0</minor>\n"
"</specVersion>\n"
"<actionList>\n"
"<action>\n"
"<name>GetProtocolInfo</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>Source</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>SourceProtocolInfo</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>Sink</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>SinkProtocolInfo</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>ConnectionIDs</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"</actionList>\n"
"<serviceStateTable>\n"
"<stateVariable sendEvents=\"yes\">\n"
"<name>SourceProtocolInfo</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"yes\">\n"
"<name>SinkProtocolInfo</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"yes\">\n"
"<name>CurrentConnectionIDs</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"</serviceStateTable>\n"
"</scpd>\n";


/****************************************
* cg_upnpav_dmr_conmgr_actionreceived
****************************************/

BOOL cg_upnpav_dmr_conmgr_actionreceived(CgUpnpAction *action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *actionName;
    CgUpnpArgument *arg;
    CgString *protocolInfos;
    CgUpnpAvProtocolInfo *protocolInfo;

    actionName = (char *)cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    /* GetProtocolInfo*/
    if (cg_streq(actionName, CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_PROTOCOL_INFO)) {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINK);
        if (!arg)
            return FALSE;
        protocolInfos = cg_string_new();
        for (protocolInfo = cg_upnpav_dmr_getprotocolinfos(dmr); protocolInfo; protocolInfo = cg_upnpav_protocolinfo_next(protocolInfo)) {
            if (0 < cg_string_length(protocolInfos))
                cg_string_addvalue(protocolInfos, ",");
            cg_string_addvalue(protocolInfos, cg_upnpav_protocolinfo_getstring(protocolInfo));
        }
        cg_upnp_argument_setvalue(arg, cg_string_getvalue(protocolInfos));
        cg_string_delete(protocolInfos);
        return TRUE;
    }

    return FALSE;
}

/****************************************
 * cg_upnpav_dmr_conmgr_queryreceived
 ****************************************/

BOOL cg_upnpav_dmr_conmgr_queryreceived(CgUpnpStateVariable *statVar)
{
    return FALSE;
}

/****************************************
* cg_upnpav_dmr_conmgr_init
****************************************/
BOOL cg_upnpav_dmr_conmgr_init(CgUpnpAvRenderer *dmr)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAction *action;

    dev = cg_upnpav_dmr_getdevice(dmr);
    if (!dev)
        return FALSE;

    service = cg_upnp_device_getservicebytype(dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
    if (!service)
        return FALSE;

#if 1
    char *xml_buf = NULL;
    unsigned long xml_len= 0;
    if(Cg_Get_Xml(XML_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION, &xml_buf, &xml_len) == -1)
    {
       rk_printf("xml get dmr connectionmanager fail\n");
       return NULL;
    }
    if (cg_upnp_service_parsedescription(service, xml_buf, xml_len) == FALSE)
    {
        free(xml_buf);
        return FALSE;
    }
    free(xml_buf);
#else
    if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION)) == FALSE)
        return FALSE;
#endif
    cg_upnp_service_setuserdata(service, dmr);
    for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
        cg_upnp_action_setuserdata(action, dmr);

    return TRUE;
}


/****************************************
 * cg_upnpav_dmr_setsinkprotocolinfo
 ****************************************/

void cg_upnpav_dmr_setsinkprotocolinfo(CgUpnpAvRenderer *dmr, char *value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO);
    cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getsinkprotocolinfo
 ****************************************/

char *cg_upnpav_dmr_getsinkprotocolinfo(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINKPROTOCOLINFO);
    return cg_upnp_statevariable_getvalue(stateVar);
}

/****************************************
 * cg_upnpav_dmr_setsourceprotocolinfo
 ****************************************/

void cg_upnpav_dmr_setsourceprotocolinfo(CgUpnpAvRenderer *dmr, char *value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCEPROTOCOLINFO);
    cg_upnp_statevariable_setvalue(stateVar, value);
}

/****************************************
 * cg_upnpav_dmr_getsourceprotocolinfo
 ****************************************/

char *cg_upnpav_dmr_getsourceprotocolinfo(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SOURCEPROTOCOLINFO);
    return cg_upnp_statevariable_getvalue(stateVar);
}
//#pragma arm section code


//增加qqplayer认证服务
#ifdef _QPLAY_ENABLE

#if defined(__arm__) && defined(__ARMCC_VERSION)
__attribute__((used, section("CG_QPLAY_SERVICE_DESCRIPTION"))) static char CG_QPLAY_SERVICE_DESCRIPTION[] =
#elif defined(__arm__) && defined(__GNUC__)
static char __attribute__((used, section("__CG_QPLAY_SERVICE_DESCRIPTION")))CG_QPLAY_SERVICE_DESCRIPTION[] =
#else

#endif

"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<scpd xmlns=\"urn:schemas-tencent-com:service-1-0\">\n"
"<specVersion>\n"
"<major>1</major>\n"
"<minor>0</minor>\n"
"</specVersion>\n"
"<actionList>\n"
"<action>\n"
"<name>SetNetwork</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>SSID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_SSID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>Key</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Key</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>AuthAlgo</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_AuthAlgo</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>CipherAlgo</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_CipherAlgo</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>QPlayAuth</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>Seed</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Seed</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>Code</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Code</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>MID</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_MID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>DID</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_DID</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>InsertTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>RemoveTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfTracks</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>GetTracksInfo</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfTracks</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>SetTracksInfo</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NextIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NextIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>GetTracksCount</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>NrTracks</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"<action>\n"
"<name>GetMaxTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>MaxTracks</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"</actionList>\n"
"<serviceStateTable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_SSID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Key</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_AuthAlgo</name>\n"
"<dataType>string</dataType>\n"
"<allowedValueList>\n"
"<allowedValue>open</allowedValue>\n"
"<allowedValue>shared</allowedValue>\n"
"<allowedValue>WPA</allowedValue>\n"
"<allowedValue>WPAPSK</allowedValue>\n"
"<allowedValue>WPA2</allowedValue>\n"
"<allowedValue>WPA2PSK</allowedValue>\n"
"</allowedValueList>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_CipherAlgo</name>\n"
"<dataType>string</dataType>\n"
"<allowedValueList>\n"
"<allowedValue>none</allowedValue>\n"
"<allowedValue>WEP</allowedValue>\n"
"<allowedValue>TKIP</allowedValue>\n"
"<allowedValue>AES</allowedValue>\n"
"</allowedValueList>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Seed</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Code</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_MID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_DID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_QueueID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_StartingIndex</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_NextIndex</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_NumberOfTracks</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_TracksMetaData</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"</serviceStateTable>\n"
"</scpd>\n";


#if 0
static char CG_QPLAYSERVER_DATA[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
"<s:Body>\n"
"<u:QPlayAuthResponse xmlns:u=\"urn:schemas-tencent-com:service:QPlay:2\">\n"
"<Code>641c7318aa0ba180741bae4bd7dc875d</Code>\n"
"<MID>62900062</MID>\n"
"<DID>LLSS-A001</DID>\n"
"</u:QPlayAuthResponse>\n"
"</s:Body>\n"
"</s:Envelope>\n"
;
#endif


BOOL rk_qplay_init(CgUpnpAvRenderer *dmr)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAction *action;

    dev = cg_upnpav_dmr_getdevice(dmr);
    if (!dev)
        return FALSE;

    service = cg_upnp_device_getservicebytype(dev, RK_QPLAY_SERVICE_TYPE);
    if (!service)
    {
        rk_printf("no qplay server");
        return FALSE;
    }

#if 1
    char *xml_buf = NULL;
    unsigned long xml_len= 0;
    if(Cg_Get_Xml(XML_DMR_QPLAY_SERVER_DESCRIPTION, &xml_buf, &xml_len) == -1)
    {
       rk_printf("xml get dmr qplay fail\n");
       return NULL;
    }
    if (cg_upnp_service_parsedescription(service, xml_buf, xml_len) == FALSE)
    {
        free(xml_buf);
        return FALSE;
    }
    free(xml_buf);
#else
    if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION)) == FALSE)
        return FALSE;
#endif
    cg_upnp_service_setuserdata(service, dmr);
    for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
        cg_upnp_action_setuserdata(action, dmr);
	player_init_qplay_list();
   return TRUE;
}
#endif
#endif


