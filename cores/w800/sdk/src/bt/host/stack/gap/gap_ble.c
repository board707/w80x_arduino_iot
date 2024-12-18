/******************************************************************************
 *
 *  Copyright (C) 2009-2013 Broadcom Corporation
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
#include "../../include/bt_target.h"

#if (defined BLE_INCLUDED && BLE_INCLUDED == TRUE)

#include "bt_utils.h"
#include <string.h>
#include "gap_int.h"
#include "gap_api.h"
#include "gattdefs.h"
#include "gatt_api.h"
#include "gatt_int.h"
#include "btm_int.h"
#include "hcimsgs.h"

#define GAP_CHAR_ICON_SIZE          2
#define GAP_CHAR_DEV_NAME_SIZE      248
#define GAP_MAX_NUM_INC_SVR       0
#define GAP_MAX_ATTR_NUM          (2 * GAP_MAX_CHAR_NUM + GAP_MAX_NUM_INC_SVR + 1)
#define GAP_MAX_CHAR_VALUE_SIZE   (30 + GAP_CHAR_DEV_NAME_SIZE)


#ifndef GAP_ATTR_DB_SIZE
#define GAP_ATTR_DB_SIZE      GATT_DB_MEM_SIZE(GAP_MAX_NUM_INC_SVR, GAP_MAX_CHAR_NUM, GAP_MAX_CHAR_VALUE_SIZE)
#endif

static void gap_ble_s_attr_request_cback(uint16_t conn_id, uint32_t trans_id,
        tGATTS_REQ_TYPE op_code, tGATTS_DATA *p_data);

/* client connection callback */
static void  gap_ble_c_connect_cback(tGATT_IF gatt_if, BD_ADDR bda, uint16_t conn_id,
                                     uint8_t connected,
                                     tGATT_DISCONN_REASON reason, tGATT_TRANSPORT transport);
static void  gap_ble_c_cmpl_cback(uint16_t conn_id, tGATTC_OPTYPE op, tGATT_STATUS status,
                                  tGATT_CL_COMPLETE *p_data);

static tGATT_CBACK gap_cback = {
    gap_ble_c_connect_cback,
    gap_ble_c_cmpl_cback,
    NULL,
    NULL,
    gap_ble_s_attr_request_cback,
    NULL,
    NULL
};



/*******************************************************************************
**
** Function         gap_find_clcb_by_bd_addr
**
** Description      The function searches all LCB with macthing bd address
**
** Returns          total number of clcb found.
**
*******************************************************************************/
tGAP_CLCB *gap_find_clcb_by_bd_addr(BD_ADDR bda)
{
    uint8_t i_clcb;
    tGAP_CLCB    *p_clcb = NULL;

    for(i_clcb = 0, p_clcb = gap_cb.clcb; i_clcb < GAP_MAX_CL; i_clcb++, p_clcb++) {
        if(p_clcb->in_use && !memcmp(p_clcb->bda, bda, BD_ADDR_LEN)) {
            return p_clcb;
        }
    }

    return NULL;
}

/*******************************************************************************
**
** Function         gap_ble_find_clcb_by_conn_id
**
** Description      The function searches all LCB with macthing connection ID
**
** Returns          total number of clcb found.
**
*******************************************************************************/
tGAP_CLCB *gap_ble_find_clcb_by_conn_id(uint16_t conn_id)
{
    uint8_t i_clcb;
    tGAP_CLCB    *p_clcb = NULL;

    for(i_clcb = 0, p_clcb = gap_cb.clcb; i_clcb < GAP_MAX_CL; i_clcb++, p_clcb++) {
        if(p_clcb->in_use && p_clcb->connected && p_clcb->conn_id == conn_id) {
            return p_clcb;
        }
    }

    return p_clcb;
}

/*******************************************************************************
**
** Function         gap_clcb_alloc
**
** Description      The function allocates a GAP  connection link control block
**
** Returns           NULL if not found. Otherwise pointer to the connection link block.
**
*******************************************************************************/
tGAP_CLCB *gap_clcb_alloc(BD_ADDR bda)
{
    uint8_t         i_clcb = 0;
    tGAP_CLCB    *p_clcb = NULL;

    for(i_clcb = 0, p_clcb = gap_cb.clcb; i_clcb < GAP_MAX_CL; i_clcb++, p_clcb++) {
        if(!p_clcb->in_use) {
            fixed_queue_free(p_clcb->pending_req_q, NULL);
            wm_memset(p_clcb, 0, sizeof(tGAP_CLCB));
            p_clcb->in_use = TRUE;
            wm_memcpy(p_clcb->bda, bda, BD_ADDR_LEN);
            p_clcb->pending_req_q = fixed_queue_new(SIZE_MAX);
            break;
        }
    }

    return p_clcb;
}

/*******************************************************************************
**
** Function         gap_ble_dealloc_clcb
**
** Description      The function clean up the pending request queue in GAP
**
** Returns          none
**
*******************************************************************************/
void gap_ble_dealloc_clcb(tGAP_CLCB *p_clcb)
{
    tGAP_BLE_REQ    *p_q;

    while((p_q = (tGAP_BLE_REQ *)fixed_queue_try_dequeue(p_clcb->pending_req_q)) != NULL) {
        /* send callback to all pending requests if being removed*/
        if(p_q->p_cback != NULL) {
            (*p_q->p_cback)(FALSE, p_clcb->bda, 0, NULL);
        }

        GKI_freebuf(p_q);
    }

    fixed_queue_free(p_clcb->pending_req_q, NULL);
    wm_memset(p_clcb, 0, sizeof(tGAP_CLCB));
}

/*******************************************************************************
**
** Function         gap_ble_enqueue_request
**
** Description      The function enqueue a GAP client request
**
** Returns           TRUE is successul; FALSE otherwise
**
*******************************************************************************/
uint8_t gap_ble_enqueue_request(tGAP_CLCB *p_clcb, uint16_t uuid, tGAP_BLE_CMPL_CBACK *p_cback)
{
    tGAP_BLE_REQ *p_q = (tGAP_BLE_REQ *)GKI_getbuf(sizeof(tGAP_BLE_REQ));
    p_q->p_cback = p_cback;
    p_q->uuid = uuid;
    fixed_queue_enqueue(p_clcb->pending_req_q, p_q);
    return TRUE;
}

/*******************************************************************************
**
** Function         gap_ble_dequeue_request
**
** Description      The function dequeue a GAP client request if any
**
** Returns           TRUE is successul; FALSE otherwise
**
*******************************************************************************/
uint8_t gap_ble_dequeue_request(tGAP_CLCB *p_clcb, uint16_t *p_uuid, tGAP_BLE_CMPL_CBACK **p_cback)
{
    tGAP_BLE_REQ *p_q = (tGAP_BLE_REQ *)fixed_queue_try_dequeue(p_clcb->pending_req_q);;

    if(p_q != NULL) {
        *p_cback    = p_q->p_cback;
        *p_uuid     = p_q->uuid;
        GKI_freebuf(p_q);
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
**   GAP Attributes Database Request callback
*******************************************************************************/
tGATT_STATUS gap_read_attr_value(uint16_t handle, tGATT_VALUE *p_value, uint8_t is_long)
{
    tGAP_ATTR   *p_db_attr = gap_cb.gatt_attr;
    uint8_t       *p = p_value->value, i;
    uint16_t      offset = p_value->offset;
    uint8_t       *p_dev_name = NULL;

    for(i = 0; i < GAP_MAX_CHAR_NUM; i ++, p_db_attr ++) {
        if(handle == p_db_attr->handle) {
            if(p_db_attr->uuid != GATT_UUID_GAP_DEVICE_NAME &&
                    is_long == TRUE) {
                return GATT_NOT_LONG;
            }

            switch(p_db_attr->uuid) {
                case GATT_UUID_GAP_DEVICE_NAME:
                    BTM_ReadLocalDeviceName((char **)&p_dev_name);

                    if(strlen((char *)p_dev_name) > GATT_MAX_ATTR_LEN) {
                        p_value->len = GATT_MAX_ATTR_LEN;
                    } else {
                        p_value->len = (uint16_t)strlen((char *)p_dev_name);
                    }

                    if(offset > p_value->len) {
                        return GATT_INVALID_OFFSET;
                    } else {
                        p_value->len -= offset;
                        p_dev_name += offset;
                        ARRAY_TO_STREAM(p, p_dev_name, p_value->len);
                        GAP_TRACE_EVENT("GATT_UUID_GAP_DEVICE_NAME len=0x%04x", p_value->len);
                    }

                    break;

                case GATT_UUID_GAP_ICON:
                    UINT16_TO_STREAM(p, p_db_attr->attr_value.icon);
                    p_value->len = 2;
                    break;

                case GATT_UUID_GAP_PREF_CONN_PARAM:
                    UINT16_TO_STREAM(p, p_db_attr->attr_value.conn_param.int_min);   /* int_min */
                    UINT16_TO_STREAM(p, p_db_attr->attr_value.conn_param.int_max);   /* int_max */
                    UINT16_TO_STREAM(p, p_db_attr->attr_value.conn_param.latency);   /* latency */
                    UINT16_TO_STREAM(p, p_db_attr->attr_value.conn_param.sp_tout);   /* sp_tout */
                    p_value->len = 8;
                    break;

                /* address resolution */
                case GATT_UUID_GAP_CENTRAL_ADDR_RESOL:
                    UINT8_TO_STREAM(p, p_db_attr->attr_value.addr_resolution);
                    p_value->len = 1;
                    break;
            }

            return GATT_SUCCESS;
        }
    }

    return GATT_NOT_FOUND;
}

/*******************************************************************************
**   GAP Attributes Database Read/Read Blob Request process
*******************************************************************************/
tGATT_STATUS gap_proc_read(tGATTS_REQ_TYPE type, tGATT_READ_REQ *p_data, tGATTS_RSP *p_rsp)
{
    tGATT_STATUS    status = GATT_NO_RESOURCES;
    UNUSED(type);

    if(p_data->is_long) {
        p_rsp->attr_value.offset = p_data->offset;
    }

    p_rsp->attr_value.handle = p_data->handle;
    status = gap_read_attr_value(p_data->handle, &p_rsp->attr_value, p_data->is_long);
    return status;
}

/******************************************************************************
**
** Function         gap_proc_write_req
**
** Description      GAP ATT server process a write request.
**
** Returns          void.
**
*******************************************************************************/
uint8_t gap_proc_write_req(tGATTS_REQ_TYPE type, tGATT_WRITE_REQ *p_data)
{
    tGAP_ATTR   *p_db_attr = gap_cb.gatt_attr;
    uint8_t   i;
    UNUSED(type);

    for(i = 0; i < GAP_MAX_CHAR_NUM; i ++, p_db_attr ++) {
        if(p_data-> handle == p_db_attr->handle) {
            return GATT_WRITE_NOT_PERMIT;
        }
    }

    return GATT_NOT_FOUND;
}

/******************************************************************************
**
** Function         gap_ble_s_attr_request_cback
**
** Description      GAP ATT server attribute access request callback.
**
** Returns          void.
**
*******************************************************************************/
void gap_ble_s_attr_request_cback(uint16_t conn_id, uint32_t trans_id,
                                  tGATTS_REQ_TYPE type, tGATTS_DATA *p_data)
{
    uint8_t       status = GATT_INVALID_PDU;
    tGATTS_RSP  rsp_msg;
    uint8_t     ignore = FALSE;
    GAP_TRACE_EVENT("gap_ble_s_attr_request_cback : recv type (0x%02x)", type);
    wm_memset(&rsp_msg, 0, sizeof(tGATTS_RSP));

    switch(type) {
        case GATTS_REQ_TYPE_READ:
            status = gap_proc_read(type, &p_data->read_req, &rsp_msg);
            break;

        case GATTS_REQ_TYPE_WRITE:
            if(!p_data->write_req.need_rsp) {
                ignore = TRUE;
            }

            status = gap_proc_write_req(type, &p_data->write_req);
            break;

        case GATTS_REQ_TYPE_WRITE_EXEC:
            ignore = TRUE;
            GAP_TRACE_EVENT("Ignore GATTS_REQ_TYPE_WRITE_EXEC");
            break;

        case GATTS_REQ_TYPE_MTU:
            GAP_TRACE_EVENT("Get MTU exchange new mtu size: %d", p_data->mtu);
            ignore = TRUE;
            break;

        default:
            GAP_TRACE_EVENT("Unknown/unexpected LE GAP ATT request: 0x%02x", type);
            break;
    }

    if(!ignore) {
        GATTS_SendRsp(conn_id, trans_id, status, &rsp_msg);
    }
}

/*******************************************************************************
**
** Function         btm_ble_att_db_init
**
** Description      GAP ATT database initalization.
**
** Returns          void.
**
*******************************************************************************/
void gap_attr_db_init(void)
{
    tBT_UUID        app_uuid = {LEN_UUID_128, {0}};
    tBT_UUID        uuid     = {LEN_UUID_16, {UUID_SERVCLASS_GAP_SERVER}};
    uint16_t          service_handle;
    tGAP_ATTR       *p_db_attr = &gap_cb.gatt_attr[0];
    tGATT_STATUS    status;
    /* Fill our internal UUID with a fixed pattern 0x82 */
    wm_memset(&app_uuid.uu.uuid128, 0x82, LEN_UUID_128);
    wm_memset(gap_cb.gatt_attr, 0, sizeof(tGAP_ATTR) *GAP_MAX_CHAR_NUM);
    gap_cb.gatt_if = GATT_Register(&app_uuid, &gap_cback);
    GATT_StartIf(gap_cb.gatt_if);
    /* Create a GAP service */
    service_handle = GATTS_CreateService(gap_cb.gatt_if, &uuid, 0, GAP_MAX_ATTR_NUM, TRUE);
    GAP_TRACE_EVENT("gap_attr_db_init service_handle = %d", service_handle);
    /* add Device Name Characteristic
    */
    uuid.len = LEN_UUID_16;
    uuid.uu.uuid16 = p_db_attr->uuid = GATT_UUID_GAP_DEVICE_NAME;
    p_db_attr->handle = GATTS_AddCharacteristic(service_handle, &uuid, GATT_PERM_READ,
                        GATT_CHAR_PROP_BIT_READ);
    p_db_attr ++;
    /* add Icon characteristic
    */
    uuid.uu.uuid16   = p_db_attr->uuid = GATT_UUID_GAP_ICON;
    p_db_attr->handle = GATTS_AddCharacteristic(service_handle,
                        &uuid,
                        GATT_PERM_READ,
                        GATT_CHAR_PROP_BIT_READ);
    p_db_attr ++;
#if BTM_PERIPHERAL_ENABLED == TRUE       /* Only needed for peripheral testing */
    /* add preferred connection parameter characteristic
    */
    uuid.uu.uuid16 = p_db_attr->uuid = GATT_UUID_GAP_PREF_CONN_PARAM;
    p_db_attr->attr_value.conn_param.int_max = GAP_PREFER_CONN_INT_MAX; /* 6 */
    p_db_attr->attr_value.conn_param.int_min = GAP_PREFER_CONN_INT_MIN; /* 0 */
    p_db_attr->attr_value.conn_param.latency = GAP_PREFER_CONN_LATENCY; /* 0 */
    p_db_attr->attr_value.conn_param.sp_tout = GAP_PREFER_CONN_SP_TOUT; /* 2000 */
    p_db_attr->handle = GATTS_AddCharacteristic(service_handle,
                        &uuid,
                        GATT_PERM_READ,
                        GATT_CHAR_PROP_BIT_READ);
    p_db_attr ++;
#endif
    /* add Central address resolution Characteristic */
    uuid.len = LEN_UUID_16;
    uuid.uu.uuid16 = p_db_attr->uuid = GATT_UUID_GAP_CENTRAL_ADDR_RESOL;
    p_db_attr->handle = GATTS_AddCharacteristic(service_handle, &uuid,
                        GATT_PERM_READ, GATT_CHAR_PROP_BIT_READ);
    p_db_attr->attr_value.addr_resolution = 0;
    p_db_attr++;
    /* start service now */
    wm_memset(&app_uuid.uu.uuid128, 0x81, LEN_UUID_128);
    status = GATTS_StartService(gap_cb.gatt_if, service_handle, GAP_TRANSPORT_SUPPORTED);
    GAP_TRACE_EVENT("GAP App gatt_if: %d  s_hdl = %d start_status=%d",
                    gap_cb.gatt_if, service_handle, status);
}

/*******************************************************************************
**
** Function         GAP_BleAttrDBUpdate
**
** Description      GAP ATT database update.
**
** Returns          void.
**
*******************************************************************************/
void GAP_BleAttrDBUpdate(uint16_t attr_uuid, tGAP_BLE_ATTR_VALUE *p_value)
{
    tGAP_ATTR  *p_db_attr = gap_cb.gatt_attr;
    uint8_t       i = 0;
    GAP_TRACE_EVENT("GAP_BleAttrDBUpdate attr_uuid=0x%04x", attr_uuid);

    for(i = 0; i < GAP_MAX_CHAR_NUM; i ++, p_db_attr ++) {
        if(p_db_attr->uuid == attr_uuid) {
            GAP_TRACE_EVENT("Found attr_uuid=0x%04x", attr_uuid);

            switch(attr_uuid) {
                case GATT_UUID_GAP_ICON:
                    p_db_attr->attr_value.icon  =  p_value->icon;
                    break;

                case GATT_UUID_GAP_PREF_CONN_PARAM:
                    wm_memcpy((void *)&p_db_attr->attr_value.conn_param,
                              (const void *)&p_value->conn_param, sizeof(tGAP_BLE_PREF_PARAM));
                    break;

                case GATT_UUID_GAP_DEVICE_NAME:
                    BTM_SetLocalDeviceName((char *)p_value->p_dev_name);
                    break;

                case GATT_UUID_GAP_CENTRAL_ADDR_RESOL:
                    p_db_attr->attr_value.addr_resolution = p_value->addr_resolution;
                    break;
            }

            break;
        }
    }

    return;
}

/*******************************************************************************
**
** Function         gap_ble_send_cl_read_request
**
** Description      utility function to send a read request for a GAP charactersitic
**
** Returns          TRUE if read started, else FALSE if GAP is busy
**
*******************************************************************************/
uint8_t gap_ble_send_cl_read_request(tGAP_CLCB *p_clcb)
{
    tGATT_READ_PARAM        param;
    uint16_t                  uuid = 0;
    uint8_t                 started = FALSE;

    if(gap_ble_dequeue_request(p_clcb, &uuid, &p_clcb->p_cback)) {
        wm_memset(&param, 0, sizeof(tGATT_READ_PARAM));
        param.service.uuid.len       = LEN_UUID_16;
        param.service.uuid.uu.uuid16 = uuid;
        param.service.s_handle       = 1;
        param.service.e_handle       = 0xFFFF;
        param.service.auth_req       = 0;

        if(GATTC_Read(p_clcb->conn_id, GATT_READ_BY_TYPE, &param) == GATT_SUCCESS) {
            p_clcb->cl_op_uuid = uuid;
            started = TRUE;
        }
    }

    return started;
}

/*******************************************************************************
**
** Function         gap_ble_cl_op_cmpl
**
** Description      GAP client operation complete callback
**
** Returns          void
**
*******************************************************************************/
void gap_ble_cl_op_cmpl(tGAP_CLCB *p_clcb, uint8_t status, uint16_t len, uint8_t *p_name)
{
    tGAP_BLE_CMPL_CBACK *p_cback = p_clcb->p_cback;
    uint16_t                  op = p_clcb->cl_op_uuid;
    GAP_TRACE_EVENT("gap_ble_cl_op_cmpl status: %d", status);
    p_clcb->cl_op_uuid = 0;
    p_clcb->p_cback = NULL;

    if(p_cback && op) {
        GAP_TRACE_EVENT("calling gap_ble_cl_op_cmpl");
        (* p_cback)(status, p_clcb->bda, len, (char *)p_name);
    }

    /* if no further activity is requested in callback, drop the link */
    if(p_clcb->connected) {
        if(!gap_ble_send_cl_read_request(p_clcb)) {
            GATT_Disconnect(p_clcb->conn_id);
            gap_ble_dealloc_clcb(p_clcb);
        }
    }
}

/*******************************************************************************
**
** Function         gap_ble_c_connect_cback
**
** Description      Client connection callback.
**
** Returns          void
**
*******************************************************************************/
static void gap_ble_c_connect_cback(tGATT_IF gatt_if, BD_ADDR bda, uint16_t conn_id,
                                    uint8_t connected, tGATT_DISCONN_REASON reason,
                                    tGATT_TRANSPORT transport)
{
    tGAP_CLCB   *p_clcb = gap_find_clcb_by_bd_addr(bda);
    UNUSED(gatt_if);
    UNUSED(transport);

    if(p_clcb != NULL) {
        if(connected) {
            p_clcb->conn_id = conn_id;
            p_clcb->connected = TRUE;
            /* start operation is pending */
            gap_ble_send_cl_read_request(p_clcb);
        } else {
            p_clcb->connected = FALSE;
            gap_ble_cl_op_cmpl(p_clcb, FALSE, 0, NULL);
            /* clean up clcb */
            gap_ble_dealloc_clcb(p_clcb);
        }
    }
}

/*******************************************************************************
**
** Function         gap_ble_c_cmpl_cback
**
** Description      Client operation complete callback.
**
** Returns          void
**
*******************************************************************************/
static void gap_ble_c_cmpl_cback(uint16_t conn_id, tGATTC_OPTYPE op, tGATT_STATUS status,
                                 tGATT_CL_COMPLETE *p_data)

{
    tGAP_CLCB   *p_clcb = gap_ble_find_clcb_by_conn_id(conn_id);
    uint16_t      op_type;
    uint16_t      min, max, latency, tout;
    uint16_t      len;
    uint8_t       *pp;

    if(p_clcb == NULL) {
        return;
    }

    op_type = p_clcb->cl_op_uuid;
    GAP_TRACE_EVENT("gap_ble_c_cmpl_cback() - op_code: 0x%02x  status: 0x%02x  read_type: 0x%04x", op,
                    status, op_type);

    /* Currently we only issue read commands */
    if(op != GATTC_OPTYPE_READ) {
        return;
    }

    if(status != GATT_SUCCESS) {
        gap_ble_cl_op_cmpl(p_clcb, FALSE, 0, NULL);
        return;
    }

    pp = p_data->att_value.value;

    switch(op_type) {
        case GATT_UUID_GAP_PREF_CONN_PARAM:
            GAP_TRACE_EVENT("GATT_UUID_GAP_PREF_CONN_PARAM");
            /* Extract the peripheral preferred connection parameters and save them */
            STREAM_TO_UINT16(min, pp);
            STREAM_TO_UINT16(max, pp);
            STREAM_TO_UINT16(latency, pp);
            STREAM_TO_UINT16(tout, pp);
            BTM_BleSetPrefConnParams(p_clcb->bda, min, max, latency, tout);
            /* release the connection here */
            gap_ble_cl_op_cmpl(p_clcb, TRUE, 0, NULL);
            break;

        case GATT_UUID_GAP_DEVICE_NAME:
            GAP_TRACE_EVENT("GATT_UUID_GAP_DEVICE_NAME");
            len = (uint16_t)strlen((char *)pp);

            if(len > GAP_CHAR_DEV_NAME_SIZE) {
                len = GAP_CHAR_DEV_NAME_SIZE;
            }

            gap_ble_cl_op_cmpl(p_clcb, TRUE, len, pp);
            break;

        case GATT_UUID_GAP_CENTRAL_ADDR_RESOL:
            gap_ble_cl_op_cmpl(p_clcb, TRUE, 1, pp);
            break;
    }
}


/*******************************************************************************
**
** Function         gap_ble_accept_cl_operation
**
** Description      Start a process to read peer address resolution capability
**
** Returns          TRUE if request accepted
**
*******************************************************************************/
uint8_t gap_ble_accept_cl_operation(BD_ADDR peer_bda, uint16_t uuid, tGAP_BLE_CMPL_CBACK *p_cback)
{
    tGAP_CLCB *p_clcb;
    uint8_t started = FALSE;

    if(p_cback == NULL && uuid != GATT_UUID_GAP_PREF_CONN_PARAM) {
        return(started);
    }

    if((p_clcb = gap_find_clcb_by_bd_addr(peer_bda)) == NULL) {
        if((p_clcb = gap_clcb_alloc(peer_bda)) == NULL) {
            GAP_TRACE_ERROR("gap_ble_accept_cl_operation max connection reached");
            return started;
        }
    }

    GAP_TRACE_EVENT("%s() - BDA: %08x%04x  cl_op_uuid: 0x%04x",
                    __FUNCTION__,
                    (peer_bda[0] << 24) + (peer_bda[1] << 16) + (peer_bda[2] << 8) + peer_bda[3],
                    (peer_bda[4] << 8) + peer_bda[5], uuid);

    if(GATT_GetConnIdIfConnected(gap_cb.gatt_if, peer_bda, &p_clcb->conn_id, BT_TRANSPORT_LE)) {
        p_clcb->connected = TRUE;
    }

    /* hold the link here */
    if(!GATT_Connect(gap_cb.gatt_if, p_clcb->bda, TRUE, BT_TRANSPORT_LE)) {
        return started;
    }

    /* enqueue the request */
    gap_ble_enqueue_request(p_clcb, uuid, p_cback);

    if(p_clcb->connected && p_clcb->cl_op_uuid == 0) {
        started = gap_ble_send_cl_read_request(p_clcb);
    } else { /* wait for connection up or pending operation to finish */
        started = TRUE;
    }

    return started;
}
/*******************************************************************************
**
** Function         GAP_BleReadPeerPrefConnParams
**
** Description      Start a process to read a connected peripheral's preferred
**                  connection parameters
**
** Returns          TRUE if read started, else FALSE if GAP is busy
**
*******************************************************************************/
uint8_t GAP_BleReadPeerPrefConnParams(BD_ADDR peer_bda)
{
    return gap_ble_accept_cl_operation(peer_bda, GATT_UUID_GAP_PREF_CONN_PARAM, NULL);
}

/*******************************************************************************
**
** Function         GAP_BleReadPeerDevName
**
** Description      Start a process to read a connected peripheral's device name.
**
** Returns          TRUE if request accepted
**
*******************************************************************************/
uint8_t GAP_BleReadPeerDevName(BD_ADDR peer_bda, tGAP_BLE_CMPL_CBACK *p_cback)
{
    return gap_ble_accept_cl_operation(peer_bda, GATT_UUID_GAP_DEVICE_NAME, p_cback);
}

/*******************************************************************************
**
** Function         GAP_BleReadPeerAddressResolutionCap
**
** Description      Start a process to read peer address resolution capability
**
** Returns          TRUE if request accepted
**
*******************************************************************************/
uint8_t GAP_BleReadPeerAddressResolutionCap(BD_ADDR peer_bda, tGAP_BLE_CMPL_CBACK *p_cback)
{
    return gap_ble_accept_cl_operation(peer_bda, GATT_UUID_GAP_CENTRAL_ADDR_RESOL, p_cback);
}

/*******************************************************************************
**
** Function         GAP_BleCancelReadPeerDevName
**
** Description      Cancel reading a peripheral's device name.
**
** Returns          TRUE if request accepted
**
*******************************************************************************/
uint8_t GAP_BleCancelReadPeerDevName(BD_ADDR peer_bda)
{
    tGAP_CLCB *p_clcb = gap_find_clcb_by_bd_addr(peer_bda);
    GAP_TRACE_EVENT("GAP_BleCancelReadPeerDevName() - BDA: %08x%04x  cl_op_uuid: 0x%04x",
                    (peer_bda[0] << 24) + (peer_bda[1] << 16) + (peer_bda[2] << 8) + peer_bda[3],
                    (peer_bda[4] << 8) + peer_bda[5], (p_clcb == NULL) ? 0 : p_clcb->cl_op_uuid);

    if(p_clcb == NULL) {
        GAP_TRACE_ERROR("Cannot cancel current op is not get dev name");
        return FALSE;
    }

    if(!p_clcb->connected) {
        if(!GATT_CancelConnect(gap_cb.gatt_if, peer_bda, TRUE)) {
            GAP_TRACE_ERROR("Cannot cancel where No connection id");
            return FALSE;
        }
    }

    gap_ble_cl_op_cmpl(p_clcb, FALSE, 0, NULL);
    return(TRUE);
}

#endif  /* BLE_INCLUDED */





