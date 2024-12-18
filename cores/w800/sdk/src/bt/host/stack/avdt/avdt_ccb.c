/******************************************************************************
 *
 *  Copyright (C) 2002-2012 Broadcom Corporation
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
 *  This module contains the channel control block state machine and
 *  functions which operate on the channel control block.
 *
 ******************************************************************************/

#include <string.h>
#include "../../stack/include/bt_types.h"
#include "../../include/bt_target.h"
#include "../../include/bt_utils.h"
#include "../../stack/include/avdt_api.h"
#include "../../stack/include/avdtc_api.h"
#include "../../stack/include/avdt_int.h"
#include "../../gki/common/gki.h"
#include "../../include/bt_common.h"
#include "../../stack/include/btu.h"

/*****************************************************************************
** state machine constants and types
*****************************************************************************/
#if AVDT_DEBUG == TRUE

/* verbose state strings for trace */
const char *const avdt_ccb_st_str[] = {
    "CCB_IDLE_ST",
    "CCB_OPENING_ST",
    "CCB_OPEN_ST",
    "CCB_CLOSING_ST"
};

/* verbose event strings for trace */
const char *const avdt_ccb_evt_str[] = {
    "API_DISCOVER_REQ_EVT",
    "API_GETCAP_REQ_EVT",
    "API_START_REQ_EVT",
    "API_SUSPEND_REQ_EVT",
    "API_DISCOVER_RSP_EVT",
    "API_GETCAP_RSP_EVT",
    "API_START_RSP_EVT",
    "API_SUSPEND_RSP_EVT",
    "API_CONNECT_REQ_EVT",
    "API_DISCONNECT_REQ_EVT",
    "MSG_DISCOVER_CMD_EVT",
    "MSG_GETCAP_CMD_EVT",
    "MSG_START_CMD_EVT",
    "MSG_SUSPEND_CMD_EVT",
    "MSG_DISCOVER_RSP_EVT",
    "MSG_GETCAP_RSP_EVT",
    "MSG_START_RSP_EVT",
    "MSG_SUSPEND_RSP_EVT",
    "RCVRSP_EVT",
    "SENDMSG_EVT",
    "RET_TOUT_EVT",
    "RSP_TOUT_EVT",
    "IDLE_TOUT_EVT",
    "UL_OPEN_EVT",
    "UL_CLOSE_EVT",
    "LL_OPEN_EVT",
    "LL_CLOSE_EVT",
    "LL_CONG_EVT"
};

#endif


/* action function list */
const tAVDT_CCB_ACTION avdt_ccb_action[] = {
    avdt_ccb_chan_open,
    avdt_ccb_chan_close,
    avdt_ccb_chk_close,
    avdt_ccb_hdl_discover_cmd,
    avdt_ccb_hdl_discover_rsp,
    avdt_ccb_hdl_getcap_cmd,
    avdt_ccb_hdl_getcap_rsp,
    avdt_ccb_hdl_start_cmd,
    avdt_ccb_hdl_start_rsp,
    avdt_ccb_hdl_suspend_cmd,
    avdt_ccb_hdl_suspend_rsp,
    avdt_ccb_snd_discover_cmd,
    avdt_ccb_snd_discover_rsp,
    avdt_ccb_snd_getcap_cmd,
    avdt_ccb_snd_getcap_rsp,
    avdt_ccb_snd_start_cmd,
    avdt_ccb_snd_start_rsp,
    avdt_ccb_snd_suspend_cmd,
    avdt_ccb_snd_suspend_rsp,
    avdt_ccb_clear_cmds,
    avdt_ccb_cmd_fail,
    avdt_ccb_free_cmd,
    avdt_ccb_cong_state,
    avdt_ccb_ret_cmd,
    avdt_ccb_snd_cmd,
    avdt_ccb_snd_msg,
    avdt_ccb_set_reconn,
    avdt_ccb_clr_reconn,
    avdt_ccb_chk_reconn,
    avdt_ccb_chk_timer,
    avdt_ccb_set_conn,
    avdt_ccb_set_disconn,
    avdt_ccb_do_disconn,
    avdt_ccb_ll_closed,
    avdt_ccb_ll_opened,
    avdt_ccb_dealloc
};

/* state table information */
#define AVDT_CCB_ACTIONS            2       /* number of actions */
#define AVDT_CCB_NEXT_STATE         2       /* position of next state */
#define AVDT_CCB_NUM_COLS           3       /* number of columns in state tables */

/* state table for idle state */
const uint8_t avdt_ccb_st_idle[][AVDT_CCB_NUM_COLS] = {
    /* Event                      Action 1                    Action 2                    Next state */
    /* API_DISCOVER_REQ_EVT */   {AVDT_CCB_SND_DISCOVER_CMD,  AVDT_CCB_CHAN_OPEN,         AVDT_CCB_OPENING_ST},
    /* API_GETCAP_REQ_EVT */     {AVDT_CCB_SND_GETCAP_CMD,    AVDT_CCB_CHAN_OPEN,         AVDT_CCB_OPENING_ST},
    /* API_START_REQ_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_SUSPEND_REQ_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_DISCOVER_RSP_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_GETCAP_RSP_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_START_RSP_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_SUSPEND_RSP_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* API_CONNECT_REQ_EVT */    {AVDT_CCB_SET_CONN,          AVDT_CCB_CHAN_OPEN,         AVDT_CCB_OPENING_ST},
    /* API_DISCONNECT_REQ_EVT */ {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_DISCOVER_CMD_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_GETCAP_CMD_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_START_CMD_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_SUSPEND_CMD_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_DISCOVER_RSP_EVT */   {AVDT_CCB_HDL_DISCOVER_RSP,  AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_GETCAP_RSP_EVT */     {AVDT_CCB_HDL_GETCAP_RSP,    AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_START_RSP_EVT */      {AVDT_CCB_HDL_START_RSP,     AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* MSG_SUSPEND_RSP_EVT */    {AVDT_CCB_HDL_SUSPEND_RSP,   AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* RCVRSP_EVT */             {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* SENDMSG_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* RET_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* RSP_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* IDLE_TOUT_EVT */          {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* UL_OPEN_EVT */            {AVDT_CCB_CHAN_OPEN,         AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* UL_CLOSE_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* LL_OPEN_EVT */            {AVDT_CCB_LL_OPENED,         AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* LL_CLOSE_EVT */           {AVDT_CCB_LL_CLOSED,         AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* LL_CONG_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST}
};

/* state table for opening state */
const uint8_t avdt_ccb_st_opening[][AVDT_CCB_NUM_COLS] = {
    /* Event                      Action 1                    Action 2                    Next state */
    /* API_DISCOVER_REQ_EVT */   {AVDT_CCB_SND_DISCOVER_CMD,  AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_GETCAP_REQ_EVT */     {AVDT_CCB_SND_GETCAP_CMD,    AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_START_REQ_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_SUSPEND_REQ_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_DISCOVER_RSP_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_GETCAP_RSP_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_START_RSP_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_SUSPEND_RSP_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_CONNECT_REQ_EVT */    {AVDT_CCB_SET_CONN,          AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* API_DISCONNECT_REQ_EVT */ {AVDT_CCB_SET_DISCONN,       AVDT_CCB_DO_DISCONN,        AVDT_CCB_CLOSING_ST},
    /* MSG_DISCOVER_CMD_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_GETCAP_CMD_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_START_CMD_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_SUSPEND_CMD_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_DISCOVER_RSP_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_GETCAP_RSP_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_START_RSP_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* MSG_SUSPEND_RSP_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* RCVRSP_EVT */             {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* SENDMSG_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* RET_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* RSP_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* IDLE_TOUT_EVT */          {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* UL_OPEN_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST},
    /* UL_CLOSE_EVT */           {AVDT_CCB_CLEAR_CMDS,        AVDT_CCB_CHAN_CLOSE,        AVDT_CCB_CLOSING_ST},
    /* LL_OPEN_EVT */            {AVDT_CCB_SND_CMD,           AVDT_CCB_LL_OPENED,         AVDT_CCB_OPEN_ST},
    /* LL_CLOSE_EVT */           {AVDT_CCB_LL_CLOSED,         AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* LL_CONG_EVT */            {AVDT_CCB_CONG_STATE,        AVDT_CCB_IGNORE,            AVDT_CCB_OPENING_ST}
};

/* state table for open state */
const uint8_t avdt_ccb_st_open[][AVDT_CCB_NUM_COLS] = {
    /* Event                      Action 1                    Action 2                    Next state */
    /* API_DISCOVER_REQ_EVT */   {AVDT_CCB_SND_DISCOVER_CMD,  AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_GETCAP_REQ_EVT */     {AVDT_CCB_SND_GETCAP_CMD,    AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_START_REQ_EVT */      {AVDT_CCB_SND_START_CMD,     AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_SUSPEND_REQ_EVT */    {AVDT_CCB_SND_SUSPEND_CMD,   AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_DISCOVER_RSP_EVT */   {AVDT_CCB_SND_DISCOVER_RSP,  AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_GETCAP_RSP_EVT */     {AVDT_CCB_SND_GETCAP_RSP,    AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_START_RSP_EVT */      {AVDT_CCB_SND_START_RSP,     AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_SUSPEND_RSP_EVT */    {AVDT_CCB_SND_SUSPEND_RSP,   AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* API_CONNECT_REQ_EVT */    {AVDT_CCB_SET_CONN,          AVDT_CCB_LL_OPENED,         AVDT_CCB_OPEN_ST},
    /* API_DISCONNECT_REQ_EVT */ {AVDT_CCB_SET_DISCONN,       AVDT_CCB_DO_DISCONN,        AVDT_CCB_CLOSING_ST},
    /* MSG_DISCOVER_CMD_EVT */   {AVDT_CCB_HDL_DISCOVER_CMD,  AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* MSG_GETCAP_CMD_EVT */     {AVDT_CCB_HDL_GETCAP_CMD,    AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* MSG_START_CMD_EVT */      {AVDT_CCB_HDL_START_CMD,     AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* MSG_SUSPEND_CMD_EVT */    {AVDT_CCB_HDL_SUSPEND_CMD,   AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* MSG_DISCOVER_RSP_EVT */   {AVDT_CCB_CHK_CLOSE,         AVDT_CCB_HDL_DISCOVER_RSP,  AVDT_CCB_OPEN_ST},
    /* MSG_GETCAP_RSP_EVT */     {AVDT_CCB_CHK_CLOSE,         AVDT_CCB_HDL_GETCAP_RSP,    AVDT_CCB_OPEN_ST},
    /* MSG_START_RSP_EVT */      {AVDT_CCB_HDL_START_RSP,     AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* MSG_SUSPEND_RSP_EVT */    {AVDT_CCB_HDL_SUSPEND_RSP,   AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* RCVRSP_EVT */             {AVDT_CCB_FREE_CMD,          AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* SENDMSG_EVT */            {AVDT_CCB_SND_MSG,           AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* RET_TOUT_EVT */           {AVDT_CCB_RET_CMD,           AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* RSP_TOUT_EVT */           {AVDT_CCB_CMD_FAIL,          AVDT_CCB_SND_CMD,           AVDT_CCB_OPEN_ST},
    /* IDLE_TOUT_EVT */          {AVDT_CCB_CLEAR_CMDS,        AVDT_CCB_CHAN_CLOSE,        AVDT_CCB_CLOSING_ST},
    /* UL_OPEN_EVT */            {AVDT_CCB_CHK_TIMER,         AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* UL_CLOSE_EVT */           {AVDT_CCB_CHK_CLOSE,         AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* LL_OPEN_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_OPEN_ST},
    /* LL_CLOSE_EVT */           {AVDT_CCB_LL_CLOSED,         AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* LL_CONG_EVT */            {AVDT_CCB_CONG_STATE,        AVDT_CCB_SND_MSG,           AVDT_CCB_OPEN_ST}
};

/* state table for closing state */
const uint8_t avdt_ccb_st_closing[][AVDT_CCB_NUM_COLS] = {
    /* Event                      Action 1                    Action 2                    Next state */
    /* API_DISCOVER_REQ_EVT */   {AVDT_CCB_SET_RECONN,        AVDT_CCB_SND_DISCOVER_CMD,  AVDT_CCB_CLOSING_ST},
    /* API_GETCAP_REQ_EVT */     {AVDT_CCB_SET_RECONN,        AVDT_CCB_SND_GETCAP_CMD,    AVDT_CCB_CLOSING_ST},
    /* API_START_REQ_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_SUSPEND_REQ_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_DISCOVER_RSP_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_GETCAP_RSP_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_START_RSP_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_SUSPEND_RSP_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* API_CONNECT_REQ_EVT */    {AVDT_CCB_SET_RECONN,        AVDT_CCB_SET_CONN,          AVDT_CCB_CLOSING_ST},
    /* API_DISCONNECT_REQ_EVT */ {AVDT_CCB_CLR_RECONN,        AVDT_CCB_SET_DISCONN,       AVDT_CCB_CLOSING_ST},
    /* MSG_DISCOVER_CMD_EVT */   {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_GETCAP_CMD_EVT */     {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_START_CMD_EVT */      {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_SUSPEND_CMD_EVT */    {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_DISCOVER_RSP_EVT */   {AVDT_CCB_HDL_DISCOVER_RSP,  AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_GETCAP_RSP_EVT */     {AVDT_CCB_HDL_GETCAP_RSP,    AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_START_RSP_EVT */      {AVDT_CCB_HDL_START_RSP,     AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* MSG_SUSPEND_RSP_EVT */    {AVDT_CCB_HDL_SUSPEND_RSP,   AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* RCVRSP_EVT */             {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* SENDMSG_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* RET_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* RSP_TOUT_EVT */           {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* IDLE_TOUT_EVT */          {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* UL_OPEN_EVT */            {AVDT_CCB_SET_RECONN,        AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* UL_CLOSE_EVT */           {AVDT_CCB_CLR_RECONN,        AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* LL_OPEN_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST},
    /* LL_CLOSE_EVT */           {AVDT_CCB_CHK_RECONN,        AVDT_CCB_IGNORE,            AVDT_CCB_IDLE_ST},
    /* LL_CONG_EVT */            {AVDT_CCB_IGNORE,            AVDT_CCB_IGNORE,            AVDT_CCB_CLOSING_ST}
};

/* type for state table */
typedef const uint8_t (*tAVDT_CCB_ST_TBL)[AVDT_CCB_NUM_COLS];

/* state table */
const tAVDT_CCB_ST_TBL avdt_ccb_st_tbl[] = {
    avdt_ccb_st_idle,
    avdt_ccb_st_opening,
    avdt_ccb_st_open,
    avdt_ccb_st_closing
};

/*******************************************************************************
**
** Function         avdt_ccb_init
**
** Description      Initialize channel control block module.
**
**
** Returns          Nothing.
**
*******************************************************************************/
void avdt_ccb_init(void)
{
    wm_memset(&avdt_cb.ccb[0], 0, sizeof(tAVDT_CCB) * AVDT_NUM_LINKS);
    avdt_cb.p_ccb_act = (tAVDT_CCB_ACTION *) avdt_ccb_action;
}

/*******************************************************************************
**
** Function         avdt_ccb_event
**
** Description      State machine event handling function for ccb
**
**
** Returns          Nothing.
**
*******************************************************************************/
void avdt_ccb_event(tAVDT_CCB *p_ccb, uint8_t event, tAVDT_CCB_EVT *p_data)
{
    tAVDT_CCB_ST_TBL    state_table;
    uint8_t               action;
    int                 i;
#if AVDT_DEBUG == TRUE
    AVDT_TRACE_EVENT("CCB ccb=%d event=%s state=%s", avdt_ccb_to_idx(p_ccb), avdt_ccb_evt_str[event],
                     avdt_ccb_st_str[p_ccb->state]);
#endif
    /* look up the state table for the current state */
    state_table = avdt_ccb_st_tbl[p_ccb->state];

    /* set next state */
    if(p_ccb->state != state_table[event][AVDT_CCB_NEXT_STATE]) {
        p_ccb->state = state_table[event][AVDT_CCB_NEXT_STATE];
    }

    /* execute action functions */
    for(i = 0; i < AVDT_CCB_ACTIONS; i++) {
        if((action = state_table[event][i]) != AVDT_CCB_IGNORE) {
            (*avdt_cb.p_ccb_act[action])(p_ccb, p_data);
        } else {
            break;
        }
    }
}


/*******************************************************************************
**
** Function         avdt_ccb_by_bd
**
** Description      This lookup function finds the ccb for a BD address.
**
**
** Returns          pointer to the ccb, or NULL if none found.
**
*******************************************************************************/
tAVDT_CCB *avdt_ccb_by_bd(BD_ADDR bd_addr)
{
    tAVDT_CCB   *p_ccb = &avdt_cb.ccb[0];
    int         i;

    for(i = 0; i < AVDT_NUM_LINKS; i++, p_ccb++) {
        /* if allocated ccb has matching ccb */
        if(p_ccb->allocated && (!memcmp(p_ccb->peer_addr, bd_addr, BD_ADDR_LEN))) {
            break;
        }
    }

    if(i == AVDT_NUM_LINKS) {
        /* if no ccb found */
        p_ccb = NULL;
        AVDT_TRACE_DEBUG("No ccb for addr %02x-%02x-%02x-%02x-%02x-%02x",
                         bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
    }

    return p_ccb;
}

/*******************************************************************************
**
** Function         avdt_ccb_alloc
**
** Description      Allocate a channel control block.
**
**
** Returns          pointer to the ccb, or NULL if none could be allocated.
**
*******************************************************************************/
tAVDT_CCB *avdt_ccb_alloc(BD_ADDR bd_addr)
{
    tAVDT_CCB   *p_ccb = &avdt_cb.ccb[0];
    int         i;

    for(i = 0; i < AVDT_NUM_LINKS; i++, p_ccb++) {
        if(!p_ccb->allocated) {
            p_ccb->allocated = TRUE;
            wm_memcpy(p_ccb->peer_addr, bd_addr, BD_ADDR_LEN);
            p_ccb->cmd_q = fixed_queue_new(SIZE_MAX);
            p_ccb->rsp_q = fixed_queue_new(SIZE_MAX);
#ifdef USE_ALARM
            p_ccb->idle_ccb_timer = alarm_new("avdt_ccb.idle_ccb_timer");
            p_ccb->ret_ccb_timer = alarm_new("avdt_ccb.ret_ccb_timer");
            p_ccb->rsp_ccb_timer = alarm_new("avdt_ccb.rsp_ccb_timer");
#endif
            AVDT_TRACE_DEBUG("avdt_ccb_alloc %d", i);
            break;
        }
    }

    if(i == AVDT_NUM_LINKS) {
        /* out of ccbs */
        p_ccb = NULL;
        AVDT_TRACE_WARNING("Out of ccbs");
    }

    return p_ccb;
}

/*******************************************************************************
**
** Function         avdt_ccb_dealloc
**
** Description      Deallocate a stream control block.
**
**
** Returns          void.
**
*******************************************************************************/
void avdt_ccb_dealloc(tAVDT_CCB *p_ccb, tAVDT_CCB_EVT *p_data)
{
    UNUSED(p_data);
    AVDT_TRACE_DEBUG("avdt_ccb_dealloc %d", avdt_ccb_to_idx(p_ccb));
#ifdef USE_ALARM
    alarm_free(p_ccb->idle_ccb_timer);
    alarm_free(p_ccb->ret_ccb_timer);
    alarm_free(p_ccb->rsp_ccb_timer);
#else
    btu_stop_timer(&p_ccb->idle_ccb_timer);
    btu_stop_timer(&p_ccb->ret_ccb_timer);
    btu_stop_timer(&p_ccb->rsp_ccb_timer);
#endif
    fixed_queue_free(p_ccb->cmd_q, NULL);
    fixed_queue_free(p_ccb->rsp_q, NULL);
    wm_memset(p_ccb, 0, sizeof(tAVDT_CCB));
}

/*******************************************************************************
**
** Function         avdt_ccb_to_idx
**
** Description      Given a pointer to an ccb, return its index.
**
**
** Returns          Index of ccb.
**
*******************************************************************************/
uint8_t avdt_ccb_to_idx(tAVDT_CCB *p_ccb)
{
    /* use array arithmetic to determine index */
    return (uint8_t)(p_ccb - avdt_cb.ccb);
}

/*******************************************************************************
**
** Function         avdt_ccb_by_idx
**
** Description      Return ccb pointer based on ccb index.
**
**
** Returns          pointer to the ccb, or NULL if none found.
**
*******************************************************************************/
tAVDT_CCB *avdt_ccb_by_idx(uint8_t idx)
{
    tAVDT_CCB   *p_ccb;

    /* verify index */
    if(idx < AVDT_NUM_LINKS) {
        p_ccb = &avdt_cb.ccb[idx];
    } else {
        p_ccb = NULL;
        AVDT_TRACE_WARNING("No ccb for idx %d", idx);
    }

    return p_ccb;
}

