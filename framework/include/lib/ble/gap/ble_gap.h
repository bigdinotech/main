/*
 * Copyright (c) 2015-2016, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BLE_GAP_H_
#define BLE_GAP_H_

#include <stdint.h>
// for ssize_t
#include <sys/types.h>

/**
 * @defgroup ble_gap GAP BLE service
 *
 * <table>
 * <tr><th><b>Include file</b><td><tt> \#include "lib/ble/gap/ble_gap.h"</tt>
 * <tr><th><b>Source path</b> <td><tt>framework/src/lib/ble/gap</tt>
 * <tr><th><b>Config flag</b> <td><tt>BLE_GAP_LIB</tt>
 * </table>
 *
 * @ingroup ble_services
 * @{
 */

/* Forward declarations */
struct bt_conn;
struct bt_gatt_attr;

/**
 * Register GAP service.
 *
 * The functions on_gap_xxx must be implemented by the user of this
 * library to handle events.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int ble_gap_init(void);

/**
 * Implements the GAP device name read.
 *
 * @param conn Connection
 * @param attr Attribute
 * @param buf Buffer to fill for the response
 * @param len Length of the read request (buffer is appropriate)
 * @param offset Offset of the read request
 * @return The status of the request handling
 *
 * @note This function has a default implementation which can be overridden by
 * a local implementation.  Nevertheless it should comply with the GATT function
 * requirements.
 */
ssize_t on_gap_rd_device_name(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr, void *buf,
			      uint16_t len,
			      uint16_t offset);

/**
 * Implements the GAP appearance read.
 *
 * @param conn Connection
 * @param attr Attribute
 * @param buf Buffer to fill for the response (if NULL, the function should only return the length)
 * @param len Length of the read request (buffer is appropriate)
 * @param offset Offset of the read request
 * @return The length of the attribute value if positive otherwise the negative error code
 *
 * @note This function has a default implementation which can be overridden by
 * a local implementation.  Nevertheless it should comply with the GATT function
 * requirements.
 */
ssize_t on_gap_rd_appearance(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len,
			     uint16_t offset);

/** Format of the PPCP characteristic value */
struct gap_ppcp {
	uint16_t interval_min;
	uint16_t interval_max;
	uint16_t slave_latency;
	uint16_t link_sup_to;
};

/**
 * Implements the GAP PPCP read.
 *
 * @param conn Connection
 * @param attr Attribute
 * @param buf Buffer to fill for the response (if NULL, the function should only return the length)
 * @param len Length of the read request (buffer is appropriate)
 * @param offset Offset of the read request
 * @return The length of the attribute value if positive otherwise the negative error code
 *
 * @note This function has a default implementation which can be overridden by
 * a local implementation.  Nevertheless it should comply with the GATT function
 * requirements.
 */
ssize_t on_gap_rd_ppcp(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len,
		       uint16_t offset);

/**
 * @}
 */
#endif /* BLE_GAP_H_ */
