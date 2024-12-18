/******************************************************************************
 *
 *  Copyright (C) 2004-2012 Broadcom Corporation
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
 *  This file contains the audio gateway functions controlling the RFCOMM
 *  connections.
 *
 ******************************************************************************/

#include <string.h>
#include "../../include/bt_target.h"
#if defined(BTA_HFP_HSP_AG_INCLUDED) && (BTA_HFP_HSP_AG_INCLUDED == TRUE)
#include "bta_api.h"
#include "bta_sys.h"
#include "bta_ag_api.h"
#include "bta_ag_int.h"
#include "bta_ag_co.h"
#include "btm_api.h"
#include "port_api.h"
#include "rfcdefs.h"
#include "bt_common.h"
#include "gki.h"
#include "utl.h"

/* Event mask for RfCOMM port callback */
#define BTA_AG_PORT_EV_MASK         PORT_EV_RXCHAR

/* each scb has its own rfcomm callbacks */
void bta_ag_port_cback_1(uint32_t code, uint16_t port_handle);
void bta_ag_port_cback_2(uint32_t code, uint16_t port_handle);
void bta_ag_port_cback_3(uint32_t code, uint16_t port_handle);

void bta_ag_mgmt_cback_1(uint32_t code, uint16_t port_handle);
void bta_ag_mgmt_cback_2(uint32_t code, uint16_t port_handle);
void bta_ag_mgmt_cback_3(uint32_t code, uint16_t port_handle);

int bta_ag_data_cback_1(uint16_t port_handle, void *p_data, uint16_t len);
int bta_ag_data_cback_2(uint16_t port_handle, void *p_data, uint16_t len);
int bta_ag_data_cback_3(uint16_t port_handle, void *p_data, uint16_t len);

/* rfcomm callback function tables */
typedef tPORT_CALLBACK *tBTA_AG_PORT_CBACK;
const tBTA_AG_PORT_CBACK bta_ag_port_cback_tbl[] = {
    bta_ag_port_cback_1,
    bta_ag_port_cback_2,
    bta_ag_port_cback_3
};

const tBTA_AG_PORT_CBACK bta_ag_mgmt_cback_tbl[] = {
    bta_ag_mgmt_cback_1,
    bta_ag_mgmt_cback_2,
    bta_ag_mgmt_cback_3
};

typedef tPORT_DATA_CALLBACK *tBTA_AG_DATA_CBACK;
const tBTA_AG_DATA_CBACK bta_ag_data_cback_tbl[] = {
    bta_ag_data_cback_1,
    bta_ag_data_cback_2,
    bta_ag_data_cback_3
};

/*******************************************************************************
**
** Function         bta_ag_port_cback
**
** Description      RFCOMM Port callback
**
**
** Returns          void
**
*******************************************************************************/
static void bta_ag_port_cback(uint32_t code, uint16_t port_handle, uint16_t handle)
{
    tBTA_AG_SCB *p_scb;
    UNUSED(code);

    if((p_scb = bta_ag_scb_by_idx(handle)) != NULL) {
        /* ignore port events for port handles other than connected handle */
        if(port_handle != p_scb->conn_handle) {
            APPL_TRACE_DEBUG("ag_port_cback ignoring handle:%d conn_handle = %d other handle = %d",
                             port_handle, p_scb->conn_handle, handle);
            return;
        }

        BT_HDR *p_buf = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR));
        p_buf->event = BTA_AG_RFC_DATA_EVT;
        p_buf->layer_specific = handle;
        bta_sys_sendmsg(p_buf);
    }
}

/*******************************************************************************
**
** Function         bta_ag_mgmt_cback
**
** Description      RFCOMM management callback
**
**
** Returns          void
**
*******************************************************************************/
static void bta_ag_mgmt_cback(uint32_t code, uint16_t port_handle, uint16_t handle)
{
    tBTA_AG_SCB     *p_scb;
    uint16_t          event;
    uint8_t           i;
    uint8_t         found_handle = FALSE;
    APPL_TRACE_DEBUG("ag_mgmt_cback : code = %d, port_handle = %d, handle = %d",
                     code, port_handle, handle);

    if((p_scb = bta_ag_scb_by_idx(handle)) != NULL) {
        /* ignore close event for port handles other than connected handle */
        if((code != PORT_SUCCESS) && (port_handle != p_scb->conn_handle)) {
            APPL_TRACE_DEBUG("ag_mgmt_cback ignoring handle:%d", port_handle);
            return;
        }

        if(code == PORT_SUCCESS) {
            if(p_scb->conn_handle) {    /* Outgoing connection */
                if(port_handle == p_scb->conn_handle) {
                    found_handle = TRUE;
                }
            } else {                    /* Incoming connection */
                for(i = 0; i < BTA_AG_NUM_IDX; i++) {
                    if(port_handle == p_scb->serv_handle[i]) {
                        found_handle = TRUE;
                    }
                }
            }

            if(!found_handle) {
                APPL_TRACE_ERROR("bta_ag_mgmt_cback: PORT_SUCCESS, ignoring handle = %d", port_handle);
                return;
            }

            event = BTA_AG_RFC_OPEN_EVT;
        }
        /* distinguish server close events */
        else if(port_handle == p_scb->conn_handle) {
            event = BTA_AG_RFC_CLOSE_EVT;
        } else {
            event = BTA_AG_RFC_SRV_CLOSE_EVT;
        }

        tBTA_AG_RFC *p_buf = (tBTA_AG_RFC *)GKI_getbuf(sizeof(tBTA_AG_RFC));
        p_buf->hdr.event = event;
        p_buf->hdr.layer_specific = handle;
        p_buf->port_handle = port_handle;
        bta_sys_sendmsg(p_buf);
    }
}

/*******************************************************************************
**
** Function         bta_ag_data_cback
**
** Description      RFCOMM data callback
**
**
** Returns          void
**
*******************************************************************************/
static int bta_ag_data_cback(uint16_t port_handle, void *p_data, uint16_t len, uint16_t handle)
{
    UNUSED(port_handle);
    /* call data call-out directly */
    bta_ag_co_tx_write(handle, (uint8_t *) p_data, len);
    return 0;
}

/*******************************************************************************
**
** Function         bta_ag_port_cback_1 to 3
**                  bta_ag_mgmt_cback_1 to 3
**
** Description      RFCOMM callback functions.  This is an easy way to
**                  distinguish scb from the callback.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_mgmt_cback_1(uint32_t code, uint16_t handle)
{
    bta_ag_mgmt_cback(code, handle, 1);
}
void bta_ag_mgmt_cback_2(uint32_t code, uint16_t handle)
{
    bta_ag_mgmt_cback(code, handle, 2);
}
void bta_ag_mgmt_cback_3(uint32_t code, uint16_t handle)
{
    bta_ag_mgmt_cback(code, handle, 3);
}
void bta_ag_port_cback_1(uint32_t code, uint16_t handle)
{
    bta_ag_port_cback(code, handle, 1);
}
void bta_ag_port_cback_2(uint32_t code, uint16_t handle)
{
    bta_ag_port_cback(code, handle, 2);
}
void bta_ag_port_cback_3(uint32_t code, uint16_t handle)
{
    bta_ag_port_cback(code, handle, 3);
}

/*******************************************************************************
**
** Function         bta_ag_data_cback_1 to 3
**
** Description      RFCOMM data callback functions.  This is an easy way to
**                  distinguish scb from the callback.
**
**
** Returns          void
**
*******************************************************************************/
int bta_ag_data_cback_1(uint16_t port_handle, void *p_data, uint16_t len)
{
    return bta_ag_data_cback(port_handle, p_data, len, 1);
}
int bta_ag_data_cback_2(uint16_t port_handle, void *p_data, uint16_t len)
{
    return bta_ag_data_cback(port_handle, p_data, len, 2);
}
int bta_ag_data_cback_3(uint16_t port_handle, void *p_data, uint16_t len)
{
    return bta_ag_data_cback(port_handle, p_data, len, 3);
}

/*******************************************************************************
**
** Function         bta_ag_setup_port
**
** Description      Setup RFCOMM port for use by AG.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_setup_port(tBTA_AG_SCB *p_scb, uint16_t handle)
{
    uint16_t i = bta_ag_scb_to_idx(p_scb) - 1;

    /* set up data callback if using pass through mode */
    if(bta_ag_cb.parse_mode == BTA_AG_PASS_THROUGH) {
        PORT_SetDataCallback(handle, bta_ag_data_cback_tbl[i]);
    }

    PORT_SetEventMask(handle, BTA_AG_PORT_EV_MASK);
    PORT_SetEventCallback(handle, bta_ag_port_cback_tbl[i]);
}

/*******************************************************************************
**
** Function         bta_ag_start_servers
**
** Description      Setup RFCOMM servers for use by AG.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_start_servers(tBTA_AG_SCB *p_scb, tBTA_SERVICE_MASK services)
{
    int i;
    int bta_ag_port_status;
    services >>= BTA_HSP_SERVICE_ID;

    for(i = 0; i < BTA_AG_NUM_IDX && services != 0; i++, services >>= 1) {
        /* if service is set in mask */
        if(services & 1) {
            BTM_SetSecurityLevel(FALSE, "", bta_ag_sec_id[i], p_scb->serv_sec_mask,
                                 BT_PSM_RFCOMM, BTM_SEC_PROTO_RFCOMM, bta_ag_cb.profile[i].scn);
            bta_ag_port_status =  RFCOMM_CreateConnection(bta_ag_uuid[i], bta_ag_cb.profile[i].scn,
                                  TRUE, BTA_AG_MTU, (uint8_t *) bd_addr_any, &(p_scb->serv_handle[i]),
                                  bta_ag_mgmt_cback_tbl[bta_ag_scb_to_idx(p_scb) - 1]);

            if(bta_ag_port_status  == PORT_SUCCESS) {
                bta_ag_setup_port(p_scb, p_scb->serv_handle[i]);
            } else {
                /* TODO: CR#137125 to handle to error properly */
                APPL_TRACE_DEBUG("bta_ag_start_servers: RFCOMM_CreateConnection returned error:%d",
                                 bta_ag_port_status);
            }
        }
    }
}

/*******************************************************************************
**
** Function         bta_ag_close_servers
**
** Description      Close RFCOMM servers port for use by AG.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_close_servers(tBTA_AG_SCB *p_scb, tBTA_SERVICE_MASK services)
{
    int i;
    services >>= BTA_HSP_SERVICE_ID;

    for(i = 0; i < BTA_AG_NUM_IDX && services != 0; i++, services >>= 1) {
        /* if service is set in mask */
        if(services & 1) {
            RFCOMM_RemoveServer(p_scb->serv_handle[i]);
            p_scb->serv_handle[i] = 0;
        }
    }
}

/*******************************************************************************
**
** Function         bta_ag_is_server_closed
**
** Description      Returns TRUE if all servers are closed.
**
**
** Returns          TRUE if all servers are closed, FALSE otherwise
**
*******************************************************************************/
uint8_t bta_ag_is_server_closed(tBTA_AG_SCB *p_scb)
{
    uint8_t xx;
    uint8_t is_closed = TRUE;

    for(xx = 0; xx < BTA_AG_NUM_IDX; xx++) {
        if(p_scb->serv_handle[xx] != 0) {
            is_closed = FALSE;
        }
    }

    return is_closed;
}

/*******************************************************************************
**
** Function         bta_ag_rfc_do_open
**
** Description      Open an RFCOMM connection to the peer device.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_rfc_do_open(tBTA_AG_SCB *p_scb, tBTA_AG_DATA *p_data)
{
    BTM_SetSecurityLevel(TRUE, "", bta_ag_sec_id[p_scb->conn_service],
                         p_scb->cli_sec_mask, BT_PSM_RFCOMM, BTM_SEC_PROTO_RFCOMM, p_scb->peer_scn);

    if(RFCOMM_CreateConnection(bta_ag_uuid[p_scb->conn_service], p_scb->peer_scn,
                               FALSE, BTA_AG_MTU, p_scb->peer_addr, &(p_scb->conn_handle),
                               bta_ag_mgmt_cback_tbl[bta_ag_scb_to_idx(p_scb) - 1]) == PORT_SUCCESS) {
        bta_ag_setup_port(p_scb, p_scb->conn_handle);
        APPL_TRACE_DEBUG("bta_ag_rfc_do_open : conn_handle = %d", p_scb->conn_handle);
    }
    /* RFCOMM create connection failed; send ourselves RFCOMM close event */
    else {
        bta_ag_sm_execute(p_scb, BTA_AG_RFC_CLOSE_EVT, p_data);
    }
}

/*******************************************************************************
**
** Function         bta_ag_rfc_do_close
**
** Description      Close RFCOMM connection.
**
**
** Returns          void
**
*******************************************************************************/
void bta_ag_rfc_do_close(tBTA_AG_SCB *p_scb, tBTA_AG_DATA *p_data)
{
    UNUSED(p_data);

    if(p_scb->conn_handle) {
        RFCOMM_RemoveConnection(p_scb->conn_handle);
    } else {
        /* Close API was called while AG is in Opening state.               */
        /* Need to trigger the state machine to send callback to the app    */
        /* and move back to INIT state.                                     */
        tBTA_AG_RFC *p_buf = (tBTA_AG_RFC *)GKI_getbuf(sizeof(tBTA_AG_RFC));
        p_buf->hdr.event = BTA_AG_RFC_CLOSE_EVT;
        p_buf->hdr.layer_specific = bta_ag_scb_to_idx(p_scb);
        bta_sys_sendmsg(p_buf);
        /* Cancel SDP if it had been started. */
        /*
        if(p_scb->p_disc_db)
        {
            (void)SDP_CancelServiceSearch (p_scb->p_disc_db);
        }
        */
    }
}
#endif
