/******************************************************************************
 *
 *  Copyright (C) 2014 The Android Open Source Project
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
 *  This file contains compile-time configurable constants for advanced
 *  audio
 *
 ******************************************************************************/
#include "../../include/bt_target.h"
#if defined(BTA_JV_INCLUDED) && (BTA_JV_INCLUDED == TRUE)
#include "bt_common.h"
#include "bta_api.h"
#include "bta_jv_api.h"

#ifndef BTA_JV_SDP_DB_SIZE
#define BTA_JV_SDP_DB_SIZE          1200
#endif

#ifndef BTA_JV_SDP_RAW_DATA_SIZE
#define BTA_JV_SDP_RAW_DATA_SIZE    800
#endif

/* The platform may choose to use dynamic memory for these data buffers.
 * p_bta_jv_cfg->p_sdp_db must be allocated/stay allocated
 * between BTA_JvEnable and BTA_JvDisable
 * p_bta_jv_cfg->p_sdp_raw_data can be allocated before calling BTA_JvStartDiscovery
 * it can be de-allocated after the last call to access the database */
static uint8_t bta_jv_sdp_raw_data[BTA_JV_SDP_RAW_DATA_SIZE];
static uint8_t __attribute__((aligned(4))) bta_jv_sdp_db_data[BTA_JV_SDP_DB_SIZE];

/* JV configuration structure */
const tBTA_JV_CFG bta_jv_cfg = {
    BTA_JV_SDP_RAW_DATA_SIZE,   /* The size of p_sdp_raw_data */
    BTA_JV_SDP_DB_SIZE,         /* The size of p_sdp_db_data */
    bta_jv_sdp_raw_data,        /* The data buffer to keep raw data */
    (tSDP_DISCOVERY_DB *)bta_jv_sdp_db_data   /* The data buffer to keep SDP database */
};

tBTA_JV_CFG *p_bta_jv_cfg = (tBTA_JV_CFG *) &bta_jv_cfg;
#endif

