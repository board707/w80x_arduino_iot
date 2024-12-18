/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
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
 *  This file contains functions for the Bluetooth Device Manager
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "../../stack/include/bt_types.h"
#include "../../gki/common/gki.h"
#include "../../include/bt_common.h"
#include "../../stack/include/hcimsgs.h"
#include "../../stack/include/btu.h"
#include "../../stack/include/btm_api.h"
#include "../../stack/include/btm_int.h"
#include "../../stack/include/hcidefs.h"
#include "../../stack/include/l2c_api.h"
#include "../../osi/include/compat.h"

static tBTM_SEC_DEV_REC *btm_find_oldest_dev(void);

/*******************************************************************************
**
** Function         BTM_SecAddDevice
**
** Description      Add/modify device.  This function will be normally called
**                  during host startup to restore all required information
**                  stored in the NVRAM.
**
** Parameters:      bd_addr          - BD address of the peer
**                  dev_class        - Device Class
**                  bd_name          - Name of the peer device.  NULL if unknown.
**                  features         - Remote device's features (up to 3 pages). NULL if not known
**                  trusted_mask     - Bitwise OR of services that do not
**                                     require authorization. (array of uint32_t)
**                  link_key         - Connection link key. NULL if unknown.
**
** Returns          TRUE if added OK, else FALSE
**
*******************************************************************************/
uint8_t BTM_SecAddDevice(BD_ADDR bd_addr, DEV_CLASS dev_class, BD_NAME bd_name,
                         uint8_t *features, uint32_t trusted_mask[],
                         LINK_KEY link_key, uint8_t key_type, tBTM_IO_CAP io_cap,
                         uint8_t pin_length)
{
    tBTM_SEC_DEV_REC  *p_dev_rec;
    int               i, j;
    uint8_t           found = FALSE;
    BTM_TRACE_API("%s: link key type:%x", __func__, key_type);
    p_dev_rec = btm_find_dev(bd_addr);

    if(!p_dev_rec) {
        if(list_length(btm_cb.sec_dev_rec) > BTM_SEC_MAX_DEVICE_RECORDS) {
            BTM_TRACE_DEBUG("%s: Max devices reached!", __func__);
            return FALSE;
        }

        BTM_TRACE_DEBUG("%s: allocate a new dev rec", __func__);
        p_dev_rec = GKI_getbuf(sizeof(tBTM_SEC_DEV_REC));
        list_append(btm_cb.sec_dev_rec, p_dev_rec);
        wm_memcpy(p_dev_rec->bd_addr, bd_addr, BD_ADDR_LEN);
        p_dev_rec->hci_handle = BTM_GetHCIConnHandle(bd_addr, BT_TRANSPORT_BR_EDR);
#if BLE_INCLUDED == TRUE
        /* use default value for background connection params */
        /* update conn params, use default value for background connection params */
        wm_memset(&p_dev_rec->conn_params, 0xff, sizeof(tBTM_LE_CONN_PRAMS));
#endif
    }

    p_dev_rec->bond_type = BOND_TYPE_UNKNOWN;           /* Default value */
    p_dev_rec->timestamp = btm_cb.dev_rec_count++;

    if(dev_class) {
        wm_memcpy(p_dev_rec->dev_class, dev_class, DEV_CLASS_LEN);
    }

    wm_memset(p_dev_rec->sec_bd_name, 0, sizeof(tBTM_BD_NAME));

    if(bd_name && bd_name[0]) {
        p_dev_rec->sec_flags |= BTM_SEC_NAME_KNOWN;
        strlcpy((char *)p_dev_rec->sec_bd_name,
                (char *)bd_name, BTM_MAX_REM_BD_NAME_LEN);
    }

    p_dev_rec->num_read_pages = 0;

    if(features) {
        wm_memcpy(p_dev_rec->features, features, sizeof(p_dev_rec->features));

        for(i = HCI_EXT_FEATURES_PAGE_MAX; i >= 0; i--) {
            for(j = 0; j < HCI_FEATURE_BYTES_PER_PAGE; j++) {
                if(p_dev_rec->features[i][j] != 0) {
                    found = TRUE;
                    break;
                }
            }

            if(found) {
                p_dev_rec->num_read_pages = i + 1;
                break;
            }
        }
    } else {
        wm_memset(p_dev_rec->features, 0, sizeof(p_dev_rec->features));
    }

    BTM_SEC_COPY_TRUSTED_DEVICE(trusted_mask, p_dev_rec->trusted_mask);

    if(link_key) {
        BTM_TRACE_EVENT("%s: BDA: %02x:%02x:%02x:%02x:%02x:%02x", __func__,
                        bd_addr[0], bd_addr[1], bd_addr[2],
                        bd_addr[3], bd_addr[4], bd_addr[5]);
        p_dev_rec->sec_flags |= BTM_SEC_LINK_KEY_KNOWN;
        wm_memcpy(p_dev_rec->link_key, link_key, LINK_KEY_LEN);
        p_dev_rec->link_key_type = key_type;
        p_dev_rec->pin_code_length = pin_length;

        if(pin_length >= 16 ||
                key_type == BTM_LKEY_TYPE_AUTH_COMB ||
                key_type == BTM_LKEY_TYPE_AUTH_COMB_P_256) {
            // Set the flag if the link key was made by using either a 16 digit
            // pin or MITM.
            p_dev_rec->sec_flags |= BTM_SEC_16_DIGIT_PIN_AUTHED | BTM_SEC_LINK_KEY_AUTHED;
        }
    }

#if defined(BTIF_MIXED_MODE_INCLUDED) && (BTIF_MIXED_MODE_INCLUDED == TRUE)

    if(key_type  < BTM_MAX_PRE_SM4_LKEY_TYPE) {
        p_dev_rec->sm4 = BTM_SM4_KNOWN;
    } else {
        p_dev_rec->sm4 = BTM_SM4_TRUE;
    }

#endif
    p_dev_rec->rmt_io_caps = io_cap;
    p_dev_rec->device_type |= BT_DEVICE_TYPE_BREDR;
    return(TRUE);
}


/*******************************************************************************
**
** Function         BTM_SecDeleteDevice
**
** Description      Free resources associated with the device.
**
** Parameters:      bd_addr          - BD address of the peer
**
** Returns          TRUE if removed OK, FALSE if not found or ACL link is active
**
*******************************************************************************/
uint8_t BTM_SecDeleteDevice(BD_ADDR bd_addr)
{
    tBTM_SEC_DEV_REC *p_dev_rec;

    if(BTM_IsAclConnectionUp(bd_addr, BT_TRANSPORT_LE) ||
            BTM_IsAclConnectionUp(bd_addr, BT_TRANSPORT_BR_EDR)) {
        BTM_TRACE_WARNING("%s FAILED: Cannot Delete when connection is active", __func__);
        return FALSE;
    }

    if((p_dev_rec = btm_find_dev(bd_addr)) != NULL) {
        btm_sec_free_dev(p_dev_rec);
        /* Tell controller to get rid of the link key, if it has one stored */
        BTM_DeleteStoredLinkKey(p_dev_rec->bd_addr, NULL);
    }

    return TRUE;
}

/*******************************************************************************
**
** Function         BTM_SecReadDevName
**
** Description      Looks for the device name in the security database for the
**                  specified BD address.
**
** Returns          Pointer to the name or NULL
**
*******************************************************************************/
char *BTM_SecReadDevName(BD_ADDR bd_addr)
{
    char *p_name = NULL;
    tBTM_SEC_DEV_REC *p_srec;

    if((p_srec = btm_find_dev(bd_addr)) != NULL) {
        p_name = (char *)p_srec->sec_bd_name;
    }

    return(p_name);
}

uint8_t is_bd_addr_equal(void *data, void *context)
{
    tBTM_SEC_DEV_REC *p_dev_rec = data;
    BD_ADDR *bd_addr = context;

    if(!memcmp(p_dev_rec->bd_addr, bd_addr, BD_ADDR_LEN)) {
        return false;
    }

    return true;
}

/*******************************************************************************
**
** Function         btm_sec_alloc_dev
**
** Description      Look for the record in the device database for the record
**                  with specified address
**
** Returns          Pointer to the record or NULL
**
*******************************************************************************/
tBTM_SEC_DEV_REC *btm_sec_alloc_dev(BD_ADDR bd_addr)
{
    tBTM_SEC_DEV_REC *p_dev_rec = NULL;
    tBTM_INQ_INFO    *p_inq_info;
    BTM_TRACE_EVENT("btm_sec_alloc_dev");

    if(list_length(btm_cb.sec_dev_rec) > BTM_SEC_MAX_DEVICE_RECORDS) {
        p_dev_rec = btm_find_oldest_dev();
    } else {
        BTM_TRACE_DEBUG("allocate a new dev rec");
        p_dev_rec = GKI_getbuf(sizeof(tBTM_SEC_DEV_REC));
        list_append(btm_cb.sec_dev_rec, p_dev_rec);
    }

    p_dev_rec->bond_type = BOND_TYPE_UNKNOWN;           /* Default value */
    p_dev_rec->sec_flags = BTM_SEC_IN_USE;

    /* Check with the BT manager if details about remote device are known */
    /* outgoing connection */
    if((p_inq_info = BTM_InqDbRead(bd_addr)) != NULL) {
        wm_memcpy(p_dev_rec->dev_class, p_inq_info->results.dev_class, DEV_CLASS_LEN);
#if BLE_INCLUDED == TRUE
        p_dev_rec->device_type = p_inq_info->results.device_type;
        p_dev_rec->ble.ble_addr_type = p_inq_info->results.ble_addr_type;
#endif
    } else if(!memcmp(bd_addr, btm_cb.connecting_bda, BD_ADDR_LEN)) {
        wm_memcpy(p_dev_rec->dev_class, btm_cb.connecting_dc, DEV_CLASS_LEN);
    }

#if BLE_INCLUDED == TRUE
    /* update conn params, use default value for background connection params */
    wm_memset(&p_dev_rec->conn_params, 0xff, sizeof(tBTM_LE_CONN_PRAMS));
#endif
    wm_memcpy(p_dev_rec->bd_addr, bd_addr, BD_ADDR_LEN);
#if BLE_INCLUDED == TRUE
    p_dev_rec->ble_hci_handle = BTM_GetHCIConnHandle(bd_addr, BT_TRANSPORT_LE);
#endif
    p_dev_rec->hci_handle = BTM_GetHCIConnHandle(bd_addr, BT_TRANSPORT_BR_EDR);
    p_dev_rec->timestamp = btm_cb.dev_rec_count++;
    return(p_dev_rec);
}


/*******************************************************************************
**
** Function         btm_sec_free_dev
**
** Description      Mark device record as not used
**
*******************************************************************************/
void btm_sec_free_dev(tBTM_SEC_DEV_REC *p_dev_rec)
{
    p_dev_rec->bond_type = BOND_TYPE_UNKNOWN;
    p_dev_rec->sec_flags = 0;
#if BLE_INCLUDED == TRUE
    /* Clear out any saved BLE keys */
    btm_sec_clear_ble_keys(p_dev_rec);
    /* clear the ble block */
    wm_memset(&p_dev_rec->ble, 0, sizeof(tBTM_SEC_BLE));
#endif
}

/*******************************************************************************
**
** Function         btm_dev_support_switch
**
** Description      This function is called by the L2CAP to check if remote
**                  device supports role switch
**
** Parameters:      bd_addr       - Address of the peer device
**
** Returns          TRUE if device is known and role switch is supported
**
*******************************************************************************/
uint8_t btm_dev_support_switch(BD_ADDR bd_addr)
{
    tBTM_SEC_DEV_REC  *p_dev_rec;
    uint8_t   xx;
    uint8_t feature_empty = TRUE;
#if BTM_SCO_INCLUDED == TRUE

    /* Role switch is not allowed if a SCO is up */
    if(btm_is_sco_active_by_bdaddr(bd_addr)) {
        return(FALSE);
    }

#endif
    p_dev_rec = btm_find_dev(bd_addr);

    //if (p_dev_rec && controller_get_interface()->supports_master_slave_role_switch())
    if(p_dev_rec && HCI_SWITCH_SUPPORTED(btm_cb.devcb.local_lmp_features[HCI_EXT_FEATURES_PAGE_0])) {
        if(HCI_SWITCH_SUPPORTED(p_dev_rec->features[HCI_EXT_FEATURES_PAGE_0])) {
            BTM_TRACE_DEBUG("btm_dev_support_switch return TRUE (feature found)");
            return (TRUE);
        }

        /* If the feature field is all zero, we never received them */
        for(xx = 0 ; xx < BD_FEATURES_LEN ; xx++) {
            if(p_dev_rec->features[HCI_EXT_FEATURES_PAGE_0][xx] != 0x00) {
                feature_empty = FALSE; /* at least one is != 0 */
                break;
            }
        }

        /* If we don't know peer's capabilities, assume it supports Role-switch */
        if(feature_empty) {
            BTM_TRACE_DEBUG("btm_dev_support_switch return TRUE (feature empty)");
            return (TRUE);
        }
    }

    BTM_TRACE_DEBUG("btm_dev_support_switch return FALSE");
    return(FALSE);
}

uint8_t is_handle_equal(void *data, void *context)
{
    tBTM_SEC_DEV_REC *p_dev_rec = data;
    uint16_t *handle = context;

    if(p_dev_rec->hci_handle == *handle
#if BLE_INCLUDED == TRUE
            || p_dev_rec->ble_hci_handle == *handle
#endif
      ) {
        return false;
    }

    return true;
}

/*******************************************************************************
**
** Function         btm_find_dev_by_handle
**
** Description      Look for the record in the device database for the record
**                  with specified handle
**
** Returns          Pointer to the record or NULL
**
*******************************************************************************/
tBTM_SEC_DEV_REC *btm_find_dev_by_handle(uint16_t handle)
{
    list_node_t *n = list_foreach(btm_cb.sec_dev_rec, is_handle_equal, &handle);

    if(n) {
        return list_node(n);
    }

    return NULL;
}

uint8_t is_address_equal(void *data, void *context)
{
    tBTM_SEC_DEV_REC *p_dev_rec = data;
    BD_ADDR *bd_addr = context;

    if(!memcmp(p_dev_rec->bd_addr, *bd_addr, BD_ADDR_LEN)) {
        return false;
    }

#if BLE_INCLUDED == TRUE

    // If a LE random address is looking for device record
    if(!memcmp(p_dev_rec->ble.pseudo_addr, *bd_addr, BD_ADDR_LEN)) {
        return false;
    }

    if(btm_ble_addr_resolvable(*bd_addr, p_dev_rec)) {
        return false;
    }

#endif
    return true;
}

/*******************************************************************************
**
** Function         btm_find_dev
**
** Description      Look for the record in the device database for the record
**                  with specified BD address
**
** Returns          Pointer to the record or NULL
**
*******************************************************************************/
tBTM_SEC_DEV_REC *btm_find_dev(BD_ADDR bd_addr)
{
    if(!bd_addr) {
        return NULL;
    }

    list_node_t *n = list_foreach(btm_cb.sec_dev_rec, is_address_equal, bd_addr);

    if(n) {
        return list_node(n);
    }

    return NULL;
}

/*******************************************************************************
**
** Function         btm_consolidate_dev
5**
** Description      combine security records if identified as same peer
**
** Returns          none
**
*******************************************************************************/
void btm_consolidate_dev(tBTM_SEC_DEV_REC *p_target_rec)
{
#if BLE_INCLUDED == TRUE
    tBTM_SEC_DEV_REC temp_rec = *p_target_rec;
    BTM_TRACE_DEBUG("%s", __func__);
    list_node_t *end = list_end(btm_cb.sec_dev_rec);

    for(list_node_t *node = list_begin(btm_cb.sec_dev_rec); node != end; node = list_next(node)) {
        tBTM_SEC_DEV_REC *p_dev_rec = list_node(node);

        if(p_target_rec == p_dev_rec) {
            continue;
        }

        if(!memcmp(p_dev_rec->bd_addr, p_target_rec->bd_addr, BD_ADDR_LEN)) {
            wm_memcpy(p_target_rec, p_dev_rec, sizeof(tBTM_SEC_DEV_REC));
            p_target_rec->ble = temp_rec.ble;
            p_target_rec->ble_hci_handle = temp_rec.ble_hci_handle;
            p_target_rec->enc_key_size = temp_rec.enc_key_size;
            p_target_rec->conn_params = temp_rec.conn_params;
            p_target_rec->device_type |= temp_rec.device_type;
            p_target_rec->sec_flags |= temp_rec.sec_flags;
            p_target_rec->new_encryption_key_is_p256 = temp_rec.new_encryption_key_is_p256;
            p_target_rec->no_smp_on_br = temp_rec.no_smp_on_br;
            p_target_rec->bond_type = temp_rec.bond_type;
            /* remove the combined record */
            list_remove(btm_cb.sec_dev_rec, p_dev_rec);
            p_dev_rec->bond_type = BOND_TYPE_UNKNOWN;
            break;
        }

        /* an RPA device entry is a duplicate of the target record */
        if(btm_ble_addr_resolvable(p_dev_rec->bd_addr, p_target_rec)) {
            if(memcmp(p_target_rec->ble.pseudo_addr, p_dev_rec->bd_addr, BD_ADDR_LEN) == 0) {
                p_target_rec->ble.ble_addr_type = p_dev_rec->ble.ble_addr_type;
                p_target_rec->device_type |= p_dev_rec->device_type;
                /* remove the combined record */
                list_remove(btm_cb.sec_dev_rec, p_dev_rec);
                p_dev_rec->bond_type = BOND_TYPE_UNKNOWN;
            }

            break;
        }
    }

#endif
}

/*******************************************************************************
**
** Function         btm_find_or_alloc_dev
**
** Description      Look for the record in the device database for the record
**                  with specified BD address
**
** Returns          Pointer to the record or NULL
**
*******************************************************************************/
tBTM_SEC_DEV_REC *btm_find_or_alloc_dev(BD_ADDR bd_addr)
{
    tBTM_SEC_DEV_REC *p_dev_rec;
    BTM_TRACE_EVENT("btm_find_or_alloc_dev");

    if((p_dev_rec = btm_find_dev(bd_addr)) == NULL) {
        /* Allocate a new device record or reuse the oldest one */
        p_dev_rec = btm_sec_alloc_dev(bd_addr);
    }

    return(p_dev_rec);
}

/*******************************************************************************
**
** Function         btm_find_oldest_dev
**
** Description      Locates the oldest device in use. It first looks for
**                  the oldest non-paired device.  If all devices are paired it
**                  deletes the oldest paired device.
**
** Returns          Pointer to the record or NULL
**
*******************************************************************************/
tBTM_SEC_DEV_REC *btm_find_oldest_dev(void)
{
    tBTM_SEC_DEV_REC *p_oldest = NULL;
    uint32_t       ot = 0xFFFFFFFF;
    tBTM_SEC_DEV_REC *p_oldest_paired = NULL;
    uint32_t       ot_paired = 0xFFFFFFFF;
    /* First look for the non-paired devices for the oldest entry */
    list_node_t *end = list_end(btm_cb.sec_dev_rec);

    for(list_node_t *node = list_begin(btm_cb.sec_dev_rec); node != end; node = list_next(node)) {
        tBTM_SEC_DEV_REC *p_dev_rec = list_node(node);

        /* Device is not paired */
        if((p_dev_rec->sec_flags & (BTM_SEC_LINK_KEY_KNOWN | BTM_SEC_LE_LINK_KEY_KNOWN)) == 0) {
            if(p_dev_rec->timestamp < ot) {
                p_oldest = p_dev_rec;
                ot       = p_dev_rec->timestamp;
            }

            continue;
        }

        if(p_dev_rec->timestamp < ot_paired) {
            p_oldest_paired = p_dev_rec;
            ot_paired       = p_dev_rec->timestamp;
        }
    }

    /* if non-paired device return oldest */
    if(ot != 0xFFFFFFFF) {
        return(p_oldest);
    }

    /* only paired devices present, return oldest */
    return p_oldest_paired;
}

/*******************************************************************************
**
** Function         btm_get_bond_type_dev
**
** Description      Get the bond type for a device in the device database
**                  with specified BD address
**
** Returns          The device bond type if known, otherwise BOND_TYPE_UNKNOWN
**
*******************************************************************************/
tBTM_BOND_TYPE btm_get_bond_type_dev(BD_ADDR bd_addr)
{
    tBTM_SEC_DEV_REC *p_dev_rec = btm_find_dev(bd_addr);

    if(p_dev_rec == NULL) {
        return BOND_TYPE_UNKNOWN;
    }

    return p_dev_rec->bond_type;
}

/*******************************************************************************
**
** Function         btm_set_bond_type_dev
**
** Description      Set the bond type for a device in the device database
**                  with specified BD address
**
** Returns          TRUE on success, otherwise FALSE
**
*******************************************************************************/
uint8_t btm_set_bond_type_dev(BD_ADDR bd_addr, tBTM_BOND_TYPE bond_type)
{
    tBTM_SEC_DEV_REC *p_dev_rec = btm_find_dev(bd_addr);

    if(p_dev_rec == NULL) {
        return FALSE;
    }

    p_dev_rec->bond_type = bond_type;
    return TRUE;
}
