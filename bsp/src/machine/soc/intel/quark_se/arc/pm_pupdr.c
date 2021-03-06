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

#include <stdlib.h>

#include "machine/soc/intel/quark_se/pm_pupdr.h"

#include "infra/log.h"
#include "infra/device.h"
#include "infra/ipc.h"
#include "infra/port.h"
#include "infra/time.h"
#include "infra/panic.h"
#include "machine.h"
#include <drivers/system_timer.h>
#include <sys_clock.h>

#include "os/os.h"

#define PANIC_ARC_NOT_RESUMED 0xdeadbeef /*!< Panic error code when ARC did not resumed */
#define PM_SUSPEND_DEFAULT    0          /*!< Default value for suspend/shutdown functions */
#define MSG_ID_PM_SLEEP       0xff99

#define PER_SEC_32K           32768

// extern ASM functions to halt ARC
extern void halt_arc(void);

// OS init functions
extern void _arc_v2_irq_unit_init(void);
extern void _firq_stack_setup(void);

#ifdef CONFIG_DEEPSLEEP
// Context dump memory location
uint32_t _deep_sleep_registers[32];

// extern ASM functions to handle low level context save/restore
extern void deep_sleep_resume_hook(void);
extern void deep_sleep_dump_context(void);

// must return 0 for OK
int pm_core_deepsleep()
{
	uint32_t start;

	start = get_uptime_32k();

	// The Quark core wanted the ARC to go in deepsleep, but in the meantime the
	// actual next wakeup time was reset to a smaller value: abort the deepsleep
	// As we don't acknowledge the pm request to Quark, the system won't go into
	// deepsleep
	if (shared_data->arc_next_wakeup_valid) {
		if (shared_data->arc_next_wakeup <
		    shared_data->soc_next_wakeup) {
			return -1;
		}
	}

	// Suspend ARC devices
	int ret = suspend_devices(PM_SUSPENDED);
	if (ret != 0) {
		return ret;
	}

	shared_data->arc_start = (uint32_t)deep_sleep_resume_hook;

	// Send Acknoledge to QRK
	PM_ACK_SET_OK(shared_data->pm_request);
	// Dump context, ack QRK suspend req and suspend ARC core
	deep_sleep_dump_context();

	// ARC resumes here
	uint32_t icache_config = (
		CACHE_DIRECT | /* direct mapping (one-way assoc.) */
		CACHE_ENABLE   /* i-cache enabled */
		);
	uint32_t aux_irq_ctrl_value = (
		ARC_V2_AUX_IRQ_CTRL_LOOP_REGS | /* save lp_xxx registers */
		ARC_V2_AUX_IRQ_CTRL_BLINK |     /* save blink */
		ARC_V2_AUX_IRQ_CTRL_14_REGS     /* save r0 -> r13 (caller-saved) */
		);
	// Init instruction cache and IRQs
	WRITE_ARC_REG(icache_config, ARC_V2_IC_CTRL);
	WRITE_ARC_REG(aux_irq_ctrl_value, ARC_V2_AUX_IRQ_CTRL);
	// Init firq stack register
	_firq_stack_setup();
	// Init interrupt unit device driver
	_arc_v2_irq_unit_init();

	if (PM_GET_REQUEST(shared_data->pm_request) != PM_RESUME_REQUEST) {
		// QRK did not requested a resume, panic
		panic(PANIC_ARC_NOT_RESUMED);
	}

	// Resume ARC devices: this function succeeds or panics
	resume_devices();

	// Send resume ACK to QRK
	PM_ACK_SET_OK(shared_data->pm_request);
	uint32_t time = get_uptime_32k();

	// Elapsed ticks is rounded to upper tick by adding PER_SEC_32K/2
	_sys_idle_elapsed_ticks =
		((uint64_t)(time -
			    start) * sys_clock_ticks_per_sec + PER_SEC_32K / 2)
		/ PER_SEC_32K;
	extern uint32_t accumulated_cycle_count;
	extern uint32_t cycles_per_tick;
	accumulated_cycle_count += _sys_idle_elapsed_ticks * cycles_per_tick;
	_sys_clock_tick_announce();
#ifndef CONFIG_TICKLESS_IDLE
	/* Make sure that next time the timer fires, it really increments of
	 * only 1 tick.
	 * When tickless idle from Zephyr will be activated on ARC, this code can
	 * go away. */
	_sys_idle_elapsed_ticks = 1;
#endif
	// Restore MBX interrupt
	extern void mbxIsr(void *param);
	irq_connect_dynamic(SOC_MBOX_INTERRUPT, ISR_DEFAULT_PRIO, mbxIsr, NULL,
			    0);
	irq_enable(SOC_MBOX_INTERRUPT);
	// TODO: move to a MBX driver when implemented
	// Restore ARC OS timer
	_sys_clock_driver_init(NULL);

	return ret;
}
#endif

int pm_notification_cb(uint8_t cpu_id, int req, int param)
{
	pm_shutdown_request(req, param);
	return 0;
}

/* return 0 if sleep, X if not */
int _idle_hook(int flags)
{
	// handle shutdown
	if (pm_is_shutdown_allowed()) {
		pm_shutdown();
		/* We should not reach this line */
	}
#ifdef CONFIG_DEEPSLEEP
	if (PM_GET_REQUEST(shared_data->pm_request) == PM_SUSPEND_REQUEST &&
	    !PM_IS_ACK_OK(shared_data->pm_request) &&
	    !PM_IS_ACK_ERROR(shared_data->pm_request)) {
		if (PM_GET_STATE(shared_data->pm_request) == PM_SUSPENDED &&
		    pm_is_deepsleep_allowed() &&
		    !pm_core_deepsleep()) {
			// Deepsleep OK
			return 0;
		}
		// Cannot go in low power mode
		PM_ACK_SET_ERROR(shared_data->pm_request);
	}
#endif
	return 1;
}

void pm_core_shutdown(enum pupdr_request req, int param)
{
	/* Notify QRK that ARC is down */
	shared_data->arc_ready = 0;
	halt_arc();
}

void pm_core_specific_ack_error()
{
	PM_ACK_SET_ERROR(shared_data->pm_request);
}

#ifdef CONFIG_DEEPSLEEP
__weak bool pm_is_core_deepsleep_allowed()
{
	return true;
}
#endif

__weak bool pm_is_core_shutdown_allowed()
{
	return true;
}
