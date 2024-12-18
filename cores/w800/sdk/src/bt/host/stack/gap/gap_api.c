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

#include <string.h>

#include "../../include/bt_target.h"
#include "../../include/bt_utils.h"
#include "../../stack/include/gap_int.h"

tGAP_CB  gap_cb;

/*******************************************************************************
**
** Function         GAP_SetTraceLevel
**
** Description      This function sets the trace level for GAP.  If called with
**                  a value of 0xFF, it simply returns the current trace level.
**
** Returns          The new or current trace level
**
*******************************************************************************/
uint8_t GAP_SetTraceLevel(uint8_t new_level)
{
    if(new_level != 0xFF) {
        gap_cb.trace_level = new_level;
    }

    return (gap_cb.trace_level);
}

/*******************************************************************************
**
** Function         GAP_Init
**
** Description      Initializes the control blocks used by GAP.
**
**                  This routine should not be called except once per
**                      stack invocation.
**
** Returns          Nothing
**
*******************************************************************************/
void GAP_Init(void)
{
    wm_memset(&gap_cb, 0, sizeof(tGAP_CB));
#if defined(GAP_INITIAL_TRACE_LEVEL)
    gap_cb.trace_level = GAP_INITIAL_TRACE_LEVEL;
#else
    gap_cb.trace_level = BT_TRACE_LEVEL_NONE;    /* No traces */
#endif
#if GAP_CONN_INCLUDED == TRUE
    gap_conn_init();
#endif
#if BLE_INCLUDED == TRUE
    gap_attr_db_init();
#endif
}

