/*
 * Copyright (c) 2015, Intel Corporation. All rights reserved.
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

#ifndef __IPC_REQUESTS_H__
#define __IPC_REQUESTS_H__


/**
 * @defgroup ipc_messages IPC messages
 * Message identifiers passed in the IPC layer.
 *
 * <table>
 * <tr><th><b>Include file</b><td><tt> \#include "infra/ipc_requests.h"</tt>
 * </table>
 *
 * @ingroup ipc
 * @{
 */

/**
 * The frame is a message.
 */
#define IPC_MSG_TYPE_MESSAGE 0x1
/**
 * Request to free a message.
 */
#define IPC_MSG_TYPE_FREE    0x2
/**
 * Set the MessageBox as synchronized.
 */
#define IPC_MSG_TYPE_SYNC    0x3

/**
 * Allocate a port.
 *
 * This request is always flowing from a slave to the master.
 */
#define IPC_REQUEST_ALLOC_PORT         0x10

/**
 * Message for test commands engine.
 */
#define IPC_REQUEST_REG_TCMD_ENGINE    0x13
/**
 * Notify a panic (for log dump).
 */
#define IPC_PANIC_NOTIFICATION         0x15
/**
 * Message for power management.
 */
#define IPC_REQUEST_POWER_MANAGEMENT   0x16
/**
 * Send the log of a slave to the master (for log aggregation).
 */
#define IPC_REQUEST_LOGGER             0x17
/**
 * Message is for power management (deep sleep).
 */
#define IPC_REQUEST_INFRA_PM           0x18
/**
 * Write to an arbitrary location with mask
 * param1: data
 * param2: mask
 * ptr: address to write
 */
#define IPC_WRITE_MASK                 0x19
/** @} */
#endif
