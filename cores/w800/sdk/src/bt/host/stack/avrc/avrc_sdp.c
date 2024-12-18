/******************************************************************************
 *
 *  Copyright (C) 2003-2013 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  AVRCP SDP related functions
 *
 ******************************************************************************/
#include <string.h>

#include "../../include/bt_common.h"
#include "../../stack/include/avrc_api.h"
#include "../../stack/include/avrc_int.h"

#ifndef SDP_AVRCP_1_4
#define SDP_AVRCP_1_4      TRUE
#endif

#ifndef SDP_AVCTP_1_4
#define SDP_AVCTP_1_4      TRUE
#endif

/*****************************************************************************
**  Global data
*****************************************************************************/
#if AVRC_DYNAMIC_MEMORY == FALSE
tAVRC_CB avrc_cb;
#endif

/******************************************************************************
**
** Function         avrc_sdp_cback
**
** Description      This is the SDP callback function used by A2D_FindService.
**                  This function will be executed by SDP when the service
**                  search is completed.  If the search is successful, it
**                  finds the first record in the database that matches the
**                  UUID of the search.  Then retrieves various parameters
**                  from the record.  When it is finished it calls the
**                  application callback function.
**
** Returns          Nothing.
**
******************************************************************************/
static void avrc_sdp_cback(uint16_t status)
{
    AVRC_TRACE_API("avrc_sdp_cback status: %d", status);
    /* reset service_uuid, so can start another find service */
    avrc_cb.service_uuid = 0;
    /* return info from sdp record in app callback function */
    (*avrc_cb.p_cback)(status);
    return;
}

/******************************************************************************
**
** Function         AVRC_FindService
**
** Description      This function is called by the application to perform service
**                  discovery and retrieve AVRCP SDP record information from a
**                  peer device.  Information is returned for the first service
**                  record found on the server that matches the service UUID.
**                  The callback function will be executed when service discovery
**                  is complete.  There can only be one outstanding call to
**                  AVRC_FindService() at a time; the application must wait for
**                  the callback before it makes another call to the function.
**                  The application is responsible for allocating memory for the
**                  discovery database.  It is recommended that the size of the
**                  discovery database be at least 300 bytes.  The application
**                  can deallocate the memory after the callback function has
**                  executed.
**
**                  Input Parameters:
**                      service_uuid: Indicates TG(UUID_SERVCLASS_AV_REM_CTRL_TARGET)
**                                           or CT(UUID_SERVCLASS_AV_REMOTE_CONTROL)
**
**                      bd_addr:  BD address of the peer device.
**
**                      p_db:  SDP discovery database parameters.
**
**                      p_cback:  Pointer to the callback function.
**
**                  Output Parameters:
**                      None.
**
** Returns          AVRC_SUCCESS if successful.
**                  AVRC_BAD_PARAMS if discovery database parameters are invalid.
**                  AVRC_NO_RESOURCES if there are not enough resources to
**                                    perform the service search.
**
******************************************************************************/
uint16_t AVRC_FindService(uint16_t service_uuid, BD_ADDR bd_addr,
                          tAVRC_SDP_DB_PARAMS *p_db, tAVRC_FIND_CBACK *p_cback)
{
    tSDP_UUID   uuid_list;
    uint8_t     result = TRUE;
    uint16_t      a2d_attr_list[] = {ATTR_ID_SERVICE_CLASS_ID_LIST, /* update AVRC_NUM_ATTR, if changed */
                                     ATTR_ID_PROTOCOL_DESC_LIST,
                                     ATTR_ID_BT_PROFILE_DESC_LIST,
                                     ATTR_ID_SERVICE_NAME,
                                     ATTR_ID_SUPPORTED_FEATURES,
                                     ATTR_ID_PROVIDER_NAME
                                    };
    AVRC_TRACE_API("AVRC_FindService uuid: %x", service_uuid);

    if((service_uuid != UUID_SERVCLASS_AV_REM_CTRL_TARGET
            && service_uuid != UUID_SERVCLASS_AV_REMOTE_CONTROL) ||
            p_db == NULL || p_db->p_db == NULL || p_cback == NULL) {
        return AVRC_BAD_PARAM;
    }

    /* check if it is busy */
    if(avrc_cb.service_uuid == UUID_SERVCLASS_AV_REM_CTRL_TARGET ||
            avrc_cb.service_uuid == UUID_SERVCLASS_AV_REMOTE_CONTROL) {
        return AVRC_NO_RESOURCES;
    }

    /* set up discovery database */
    uuid_list.len = LEN_UUID_16;
    uuid_list.uu.uuid16 = service_uuid;

    if(p_db->p_attrs == NULL || p_db->num_attr == 0) {
        p_db->p_attrs  = a2d_attr_list;
        p_db->num_attr = AVRC_NUM_ATTR;
    }

    result = SDP_InitDiscoveryDb(p_db->p_db, p_db->db_len, 1, &uuid_list, p_db->num_attr,
                                 p_db->p_attrs);

    if(result == TRUE) {
        /* store service_uuid and discovery db pointer */
        avrc_cb.p_db = p_db->p_db;
        avrc_cb.service_uuid = service_uuid;
        avrc_cb.p_cback = p_cback;
        /* perform service search */
        result = SDP_ServiceSearchAttributeRequest(bd_addr, p_db->p_db, avrc_sdp_cback);
    }

    return (result ? AVRC_SUCCESS : AVRC_FAIL);
}

/******************************************************************************
**
** Function         AVRC_AddRecord
**
** Description      This function is called to build an AVRCP SDP record.
**                  Prior to calling this function the application must
**                  call SDP_CreateRecord() to create an SDP record.
**
**                  Input Parameters:
**                      service_uuid:  Indicates TG(UUID_SERVCLASS_AV_REM_CTRL_TARGET)
**                                            or CT(UUID_SERVCLASS_AV_REMOTE_CONTROL)
**
**                      p_service_name:  Pointer to a null-terminated character
**                      string containing the service name.
**                      If service name is not used set this to NULL.
**
**                      p_provider_name:  Pointer to a null-terminated character
**                      string containing the provider name.
**                      If provider name is not used set this to NULL.
**
**                      categories:  Supported categories.
**
**                      sdp_handle:  SDP handle returned by SDP_CreateRecord().
**
**                      browse_supported:  browse support info.
**
**                      profile_version:  profile version of avrcp record.
**
**                  Output Parameters:
**                      None.
**
** Returns          AVRC_SUCCESS if successful.
**                  AVRC_NO_RESOURCES if not enough resources to build the SDP record.
**
******************************************************************************/
uint16_t AVRC_AddRecord(uint16_t service_uuid, char *p_service_name,
                        char *p_provider_name, uint16_t categories, uint32_t sdp_handle,
                        uint8_t browse_supported, uint16_t profile_version)
{
    uint16_t      browse_list[1];
    uint8_t     result = TRUE;
    uint8_t       temp[8];
    uint8_t       *p;
    uint16_t      count = 1;
    uint8_t       index = 0;
    uint16_t      class_list[2];
    AVRC_TRACE_API("AVRC_AddRecord uuid: %x", service_uuid);

    if(service_uuid != UUID_SERVCLASS_AV_REM_CTRL_TARGET
            && service_uuid != UUID_SERVCLASS_AV_REMOTE_CONTROL) {
        return AVRC_BAD_PARAM;
    }

    /* add service class id list */
    class_list[0] = service_uuid;

    if((service_uuid == UUID_SERVCLASS_AV_REMOTE_CONTROL) && (profile_version > AVRC_REV_1_3)) {
        class_list[1] = UUID_SERVCLASS_AV_REM_CTRL_CONTROL;
        count = 2;
    }

    result &= SDP_AddServiceClassIdList(sdp_handle, count, class_list);
    /* add protocol descriptor list   */
    tSDP_PROTOCOL_ELEM  avrc_proto_desc_list [AVRC_NUM_PROTO_ELEMS];
    avrc_proto_desc_list[0].num_params = 1;
    avrc_proto_desc_list[0].protocol_uuid = UUID_PROTOCOL_L2CAP;
    avrc_proto_desc_list[0].params[0] = AVCT_PSM;
    avrc_proto_desc_list[0].params[1] = 0;

    for(index = 1; index < AVRC_NUM_PROTO_ELEMS; index++) {
        avrc_proto_desc_list[index].num_params = 1;
        avrc_proto_desc_list[index].protocol_uuid = UUID_PROTOCOL_AVCTP;
        avrc_proto_desc_list[index].params[0] = AVCT_REV_1_4;
        avrc_proto_desc_list[index].params[1] = 0;
    }

    result &= SDP_AddProtocolList(sdp_handle, AVRC_NUM_PROTO_ELEMS,
                                  (tSDP_PROTOCOL_ELEM *)avrc_proto_desc_list);

    /* additional protocal descriptor, required only for version > 1.3    */
    if((profile_version > AVRC_REV_1_3) && (browse_supported)) {
        tSDP_PROTO_LIST_ELEM  avrc_add_proto_desc_list;
        avrc_add_proto_desc_list.num_elems = 2;
        avrc_add_proto_desc_list.list_elem[0].num_params = 1;
        avrc_add_proto_desc_list.list_elem[0].protocol_uuid = UUID_PROTOCOL_L2CAP;
        avrc_add_proto_desc_list.list_elem[0].params[0] = AVCT_BR_PSM;
        avrc_add_proto_desc_list.list_elem[0].params[1] = 0;
        avrc_add_proto_desc_list.list_elem[1].num_params = 1;
        avrc_add_proto_desc_list.list_elem[1].protocol_uuid = UUID_PROTOCOL_AVCTP;
        avrc_add_proto_desc_list.list_elem[1].params[0] = AVCT_REV_1_4;
        avrc_add_proto_desc_list.list_elem[1].params[1] = 0;
        result &= SDP_AddAdditionProtoLists(sdp_handle, 1,
                                            (tSDP_PROTO_LIST_ELEM *)&avrc_add_proto_desc_list);
    }

    /* add profile descriptor list   */
    result &= SDP_AddProfileDescriptorList(sdp_handle, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                                           profile_version);
    /* add supported categories */
    p = temp;
    UINT16_TO_BE_STREAM(p, categories);
    result &= SDP_AddAttribute(sdp_handle, ATTR_ID_SUPPORTED_FEATURES, UINT_DESC_TYPE,
                               (uint32_t)2, (uint8_t *)temp);

    /* add provider name */
    if(p_provider_name != NULL) {
        result &= SDP_AddAttribute(sdp_handle, ATTR_ID_PROVIDER_NAME, TEXT_STR_DESC_TYPE,
                                   (uint32_t)(strlen(p_provider_name) + 1), (uint8_t *) p_provider_name);
    }

    /* add service name */
    if(p_service_name != NULL) {
        result &= SDP_AddAttribute(sdp_handle, ATTR_ID_SERVICE_NAME, TEXT_STR_DESC_TYPE,
                                   (uint32_t)(strlen(p_service_name) + 1), (uint8_t *) p_service_name);
    }

    /* add browse group list */
    browse_list[0] = UUID_SERVCLASS_PUBLIC_BROWSE_GROUP;
    result &= SDP_AddUuidSequence(sdp_handle, ATTR_ID_BROWSE_GROUP_LIST, 1, browse_list);
    return (result ? AVRC_SUCCESS : AVRC_FAIL);
}



/******************************************************************************
**
** Function         AVRC_SetTraceLevel
**
** Description      Sets the trace level for AVRC. If 0xff is passed, the
**                  current trace level is returned.
**
**                  Input Parameters:
**                      new_level:  The level to set the AVRC tracing to:
**                      0xff-returns the current setting.
**                      0-turns off tracing.
**                      >= 1-Errors.
**                      >= 2-Warnings.
**                      >= 3-APIs.
**                      >= 4-Events.
**                      >= 5-Debug.
**
** Returns          The new trace level or current trace level if
**                  the input parameter is 0xff.
**
******************************************************************************/
uint8_t AVRC_SetTraceLevel(uint8_t new_level)
{
    if(new_level != 0xFF) {
        avrc_cb.trace_level = new_level;
    }

    return (avrc_cb.trace_level);
}

/*******************************************************************************
**
** Function         AVRC_Init
**
** Description      This function is called at stack startup to allocate the
**                  control block (if using dynamic memory), and initializes the
**                  control block and tracing level.
**
** Returns          void
**
*******************************************************************************/
void AVRC_Init(void)
{
    wm_memset(&avrc_cb, 0, sizeof(tAVRC_CB));
#if defined(AVRC_INITIAL_TRACE_LEVEL)
    avrc_cb.trace_level  = AVRC_INITIAL_TRACE_LEVEL;
#else
    avrc_cb.trace_level  = BT_TRACE_LEVEL_NONE;
#endif
}

