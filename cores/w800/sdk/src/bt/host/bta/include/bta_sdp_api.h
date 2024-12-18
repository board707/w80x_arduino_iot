/******************************************************************************
 *
 *  Copyright (C) 2015 The Android Open Source Project
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
 *  This is the public interface file for the BTA SDP I/F
 *
 ******************************************************************************/
#ifndef BTA_SDP_API_H
#define BTA_SDP_API_H

#include "../../include/bt_sdp.h"
#include "../../include/bt_target.h"
#include "../../stack/include/bt_types.h"
#include "../include/bta_api.h"
#include "../../stack/include/btm_api.h"

/* status values */
#define BTA_SDP_SUCCESS                  0            /* Successful operation. */
#define BTA_SDP_FAILURE                  1            /* Generic failure. */
#define BTA_SDP_BUSY                     2            /* Temporarily can not handle this request. */

typedef uint8_t tBTA_SDP_STATUS;

/* SDP I/F callback events */
/* events received by tBTA_SDP_DM_CBACK */
#define BTA_SDP_ENABLE_EVT               0  /* SDP service i/f enabled*/
#define BTA_SDP_SEARCH_EVT               1  /* SDP Service started */
#define BTA_SDP_SEARCH_COMP_EVT          2  /* SDP search complete */
#define BTA_SDP_CREATE_RECORD_USER_EVT   3  /* SDP search complete */
#define BTA_SDP_REMOVE_RECORD_USER_EVT   4  /* SDP search complete */
#define BTA_SDP_MAX_EVT                  5  /* max number of SDP events */

#define BTA_SDP_MAX_RECORDS 15

typedef uint16_t tBTA_SDP_EVT;

/* data associated with BTA_SDP_DISCOVERY_COMP_EVT */
typedef struct {
    tBTA_SDP_STATUS      status;
    BD_ADDR              remote_addr;
    tBT_UUID             uuid;
    int                  record_count;
    bluetooth_sdp_record records[BTA_SDP_MAX_RECORDS];
} tBTA_SDP_SEARCH_COMP;

typedef union {
    tBTA_SDP_STATUS              status;            /* BTA_SDP_SEARCH_EVT */
    tBTA_SDP_SEARCH_COMP         sdp_search_comp;   /* BTA_SDP_SEARCH_COMP_EVT */
} tBTA_SDP;

/* SDP DM Interface callback */
typedef void (tBTA_SDP_DM_CBACK)(tBTA_SDP_EVT event, tBTA_SDP *p_data, void *user_data);

/* MCE configuration structure */
typedef struct {
    uint16_t  sdp_db_size;            /* The size of p_sdp_db */
    tSDP_DISCOVERY_DB   *p_sdp_db;  /* The data buffer to keep SDP database */
} tBTA_SDP_CFG;

#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
**
** Function         BTA_SdpEnable
**
** Description      Enable the SDP I/F service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_SDP_ENABLE_EVT. This function must
**                  be called before other functions in the MCE API are
**                  called.
**
** Returns          BTA_SDP_SUCCESS if successful.
**                  BTA_SDP_FAIL if internal failure.
**
*******************************************************************************/
extern tBTA_SDP_STATUS BTA_SdpEnable(tBTA_SDP_DM_CBACK *p_cback);

/*******************************************************************************
**
** Function         BTA_SdpSearch
**
** Description      Start a search for sdp records for a specific BD_ADDR with a
**                  specific profile uuid.
**                  When the search operation is completed, the callback function
**                  will be called with a BTA_SDP_SEARCH_EVT.
** Returns          BTA_SDP_SUCCESS if successful.
**                  BTA_SDP_FAIL if internal failure.
**
*******************************************************************************/
extern tBTA_SDP_STATUS BTA_SdpSearch(BD_ADDR bd_addr, tSDP_UUID *uuid);

/*******************************************************************************
**
** Function         BTA_SdpCreateRecordByUser
**
** Description      This function is used to request a callback to create a SDP
**                  record. The registered callback will be called with event
**                  BTA_SDP_CREATE_RECORD_USER_EVT.
**
** Returns          BTA_SDP_SUCCESS, if the request is being processed.
**                  BTA_SDP_FAILURE, otherwise.
**
*******************************************************************************/
extern tBTA_SDP_STATUS BTA_SdpCreateRecordByUser(void *user_data);

/*******************************************************************************
**
** Function         BTA_SdpRemoveRecordByUser
**
** Description      This function is used to request a callback to remove a SDP
**                  record. The registered callback will be called with event
**                  BTA_SDP_REMOVE_RECORD_USER_EVT.
**
** Returns          BTA_SDP_SUCCESS, if the request is being processed.
**                  BTA_SDP_FAILURE, otherwise.
**
*******************************************************************************/
extern tBTA_SDP_STATUS BTA_SdpRemoveRecordByUser(void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* BTA_SDP_API_H */
