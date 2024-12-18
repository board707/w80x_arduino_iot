/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BEACON_H__
#define __BEACON_H__

#include "../../../../porting/w800/include/os/os_mbuf.h"

void bt_mesh_beacon_enable(void);
void bt_mesh_beacon_disable(void);

void bt_mesh_beacon_ivu_initiator(bool enable);

void bt_mesh_beacon_recv(const bt_addr_le_t *addr, struct os_mbuf *buf);

void bt_mesh_beacon_create(struct bt_mesh_subnet *sub,
                           struct os_mbuf *buf);

void bt_mesh_beacon_init(void);
void bt_mesh_beacon_deinit(void);

#endif
