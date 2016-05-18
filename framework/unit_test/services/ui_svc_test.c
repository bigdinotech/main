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

#include "util/compiler.h"
#include "util/cunit_test.h"

#include "os/os.h"
#include "cfw/cfw.h"
#include "services/ui_service/ui_service.h"
#include "infra/log.h"
#include "infra/panic.h"

#include "services/service_tests.h"
#include "pwrbtn_service_test.h"
#include "machine.h"
#include "drivers/data_type.h"
#include "lib/led/soc_led.h"

#define PANIC_FAIL_1 1
#define PANIC_FAIL_2 2

#define UISVC_MAIN_CLIENT_NAME "MAIN"

#ifdef TEST_UI_SVC

#ifdef CONFIG_LED
#define TEST_UI_SVC_LED
#endif

#ifdef CONFIG_HAPTIC
#define TEST_UI_SVC_VIBR
#endif

#ifdef CONFIG_PWRBTN_GPIO
#define TEST_UI_SVC_PWRBTN
#endif

#define TEST_UI_DEFAULT  1

#define TST_NOMINAL_NOTIF_SINGLE_1         1
#define TST_NOMINAL_NOTIF_SINGLE_2         2
#define TST_NOMINAL_NOTIF_SINGLE_3         3
#define TST_NOMINAL_NOTIF_SINGLE_4         4
#define TST_NOMINAL_NOTIF_SINGLE_5         5
#define TST_NOMINAL_NOTIF_SINGLE_6         6
#define TST_NOMINAL_NOTIF_DOUBLE           7
#define TST_TIME_MODIFIED_NOTIF_SINGLE_1   8
#define TST_TIME_MODIFIED_NOTIF_SINGLE_2   9
#define TST_TIME_MODIFIED_NOTIF_SINGLE_3   10
#define TST_TIME_MODIFIED_NOTIF_SINGLE_4   11
#define TST_TIME_MODIFIED_NOTIF_DOUBLE     12
#define TST_EVT_MODIFIED_NOTIF_DOUBLE      13

typedef enum test_id_common {
	TEST_EVT_FOR_SVC = 0,
#if defined(TEST_UI_SVC_PWRBTN)
	TEST_EVT_FOR_DRV,
	TEST_BTN_NOTIFICATION,
	TEST_BTN_TIME_MODIFIED,
	TEST_BTN_EVT_ALLOWED_MODIFIED,
#endif
#if defined(TEST_UI_SVC_VIBR)
	TEST_PLAY_SQUARE_X2_VIBR,
	TEST_PLAY_SPECIAL_VIBR,
	TEST_VIBR_DRV_ERROR,
	TEST_VIBR_BUSY_ERROR,
#endif
#if defined(TEST_UI_SVC_LED)
	TEST_PLAY_BLINK_X1_LED,
	TEST_PLAY_BLINK_X2_LED,
	TEST_PLAY_WAVE_X2_LED,
	TEST_PLAY_BLINK_X3_LED,
	TEST_LED_DRV_ERROR,
	TEST_LED_BUSY_ERROR,
#endif
	TEST_SET_BAD_EVT,
	NB_TEST
}test_id_common;

static cfw_service_conn_t *ui_service_conn = NULL;
static cfw_client_t __maybe_unused *ui_svc;
static uint8_t svc_evt_test;
#if defined(TEST_UI_SVC_PWRBTN)
static uint8_t btn_notif_idx = 0;
#endif
T_TIMER test_timer = NULL;
static int test_type;
int8_t ui_svc_init_done = 0;
static void test_callback(void);
void test_play_blink_x1_led(void);
void test_play_special_vibr(void);

/* Used to count the number of response sent by the test busy error */
uint8_t g_nb_returned_rsp;

/* Test events settings
 *
 * @details Test events setting path: between test app, framework, ui service,
 *          down to the point where the driver receives the events.
 * @return  none.
 */
void test_set_enabled_event_for_drv(void)
{
	int8_t ret = 0;

#if defined(TEST_UI_SVC_PWRBTN)
	/* Test events enabling. Here, given ones have been arbitrary chosen and */
	/* are just for testing.                                                 */
	/* Chosen one is for power button.                                       */
	ret = ui_service_set_enabled_events(
		ui_service_conn,
		UI_PWRBTN_SINGLE_EVT_IDX |
		UI_PWRBTN_DOUBLE_EVT_IDX,
		UI_PWRBTN_SINGLE_EVT_IDX |
		UI_PWRBTN_DOUBLE_EVT_IDX,
		NULL);
#endif

	if (ret)
		cu_print("TEST: Cannot set event. Error [%d]\n", ret);
}


/* Test bad events settings
 *
 * @details Test events setting path: between test app, framework, ui service,
 *          down to the point where the driver receives the events.
 * @return   none.
 */
void test_set_event_error(void)
{
	int8_t ret;

	/* Test bad events mask. Passed mask is arbitray chosen. */
	ret = ui_service_set_enabled_events(ui_service_conn,
					    0xffff,
					    0x00,
					    NULL);

	CU_ASSERT("Bad event with invalid param set", ret == UI_INVALID_PARAM);
	if (ret)
		cu_print("TEST: Cannot set event. Error [%d]\n", ret);
}

/* Test busy error case.
 * @details In this test two requests are sent at the same time to the UI SVC.
 *          While treating the first one, the service should indicate that it
 *          is busy.
 * @return  none.
 */
void test_led_busy_error(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);

#if defined(CONFIG_LED_MULTICOLOR)
	/* Arbitrary values set just for testing. */
	pattern->rgb[0].r = 0x1F;
	pattern->rgb[0].g = 0x1F;
	pattern->rgb[0].b = 0x00;
	pattern->intensity = 0x10;
#else
	pattern->intensity = 50;
#endif
	pattern->duration[0].duration_on = 5000;

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED2, LED_BLINK_X1, pattern,
				    PATTERN_OVERRIDE,
				    NULL);
	/* free pattern */
	bfree(pattern);

	/* And try to send immediately a new request, so that the service */
	/* will return a busy error. */
	test_play_blink_x1_led();
}

/* Test error propagation from driver.
 *
 * @details Test that when a driver returns an error, it is correctly
 *        transfered.
 * @return   none.
 */
void test_led_drv_error(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);

	/* Arbitrary values set just for testing. */
	/* Here they are chosen so that driver will return an error. */
#if defined(CONFIG_LED_MULTICOLOR)
	/* Here, expected error is -1 (I2C_WRITE_ERROR) */
	pattern->rgb[0].g = 0xFF;
	pattern->rgb[0].b = 0xFF;
	pattern->rgb[0].r = 0xFF;
	pattern->intensity = 0xFF;
#else
	/* Here, expected error is 1 (DRV_RC_FAIL) */
	pattern->intensity = 200;
#endif
	pattern->duration[0].duration_on = 1;
	pattern->duration[0].duration_off = 1;
	pattern->duration[0].duration_on = 1;
	pattern->duration[0].duration_off = 1;
	pattern->repetition_count = 1;

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED1,
				    LED_BLINK_X2,
				    pattern,
				    PATTERN_OVERRIDE,
				    NULL);
	/* free pattern */
	bfree(pattern);
}


/* Test error propagation from driver.
 *
 * @details Test that when a driver returns an error, it is correctly
 *        transfered.
 * @return   none.
 */
void test_haptic_drv_error(void)
{
	vibration_u *pattern;
	/* Use bad number for pattern type =>
	 * expected response is 1 (DRV_RC_FAIL) */
	int8_t type_pattern = 3;

	pattern = balloc(sizeof *pattern, NULL);

	ui_service_play_vibr_pattern(ui_service_conn,
				     type_pattern,
				     pattern,
				     NULL);

	/* free pattern */
	bfree(pattern);
}

/* Test busy error case.
 * @details In this test two requests are sent at the same time to the UI SVC.
 *          While treating the first one, the service should indicate that
 *          the driver is busy
 * @return  none.
 */
void test_haptic_busy_error(void)
{
	vibration_u *pattern;

	pattern = balloc(sizeof *pattern, NULL);

	pattern->square_x2.amplitude = 255;
	pattern->square_x2.duration_on_1 = 1000;
	pattern->square_x2.duration_off_1 = 500;
	pattern->square_x2.duration_on_2 = 1000;
	pattern->square_x2.duration_off_2 = 500;
	pattern->square_x2.repetition_count = 2;

	ui_service_play_vibr_pattern(ui_service_conn,
				     VIBRATION_SQUARE_X2,
				     pattern,
				     NULL);

	/* free pattern */
	bfree(pattern);

	/* Send the next request: return expected UI_BUSY */
	test_play_special_vibr();
}


/* Test events settings for service.
 *
 * @details Test events setting path: between test app, framework, ui service,
 *          down to the point where the ui service opens the new service and
 *          registers given events.
 * @return  none.
 */
static void handle_message(struct cfw_message *msg, void *param)
{
	cu_print("TEST: handle_message id: %d \n", CFW_MESSAGE_ID(msg));

	/* For the pupose of test. No need to analyze received message.
	 * We have just registered a fake service for the test. */
	switch (CFW_MESSAGE_ID(msg)) {
	default:
		break;
	}
	cfw_msg_free(msg);
}

void test_service_connected(conn_handle_t *instance)
{
	cu_print("TEST handle_message: test_service_connected");
}

void test_service_disconnected(conn_handle_t *instance)
{
	cu_print("TEST handle_message: test_service_disconnected");
}

static service_t test_service = {
	.service_id = TEST_SVC_ID,
	.client_connected = test_service_connected,
	.client_disconnected = test_service_disconnected,
};

void test_set_enabled_event_for_svc(void)
{
	int8_t ret;

	/* This part of the test is made for testing event enabling
	 * for services, and to check that the ui service will correctly
	 * open needed service. */

	/* Register service. */
	cfw_register_service(
		get_test_queue(), &test_service, handle_message, NULL);

	/* Arbitrary chosen event. */
	ret = ui_service_set_enabled_events(ui_service_conn,
					    UI_TEST_EVT_IDX,
					    UI_TEST_EVT_IDX,
					    NULL);

	if (ret)
		cu_print("TEST: Cannot set event. Error [%d]\n", ret);
}

/* Test switching on LED.
 *
 * @details Test LED path: between test app, framework, ui service,
 *          down to the point where the LED is ON.
 * @return   none.
 */
void test_play_blink_x1_led(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);
#if defined(CONFIG_LED_MULTICOLOR)
	/* Arbitrary values set just for testing. */
	pattern->rgb[0].r = 0x1F;
	pattern->rgb[0].g = 0x1F;
	pattern->rgb[0].b = 0x00;
	pattern->intensity = 0x10;
#else
	pattern->intensity = 33;
#endif
	pattern->duration[0].duration_on = 3000;

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED1,
				    LED_BLINK_X1,
				    pattern,
				    PATTERN_OVERRIDE,
				    NULL);

	bfree(pattern);
}

/* Test blinking LED with pattern blink_x2.
 *
 * @details Test LED path: between test app, framework, ui service,
 *          down to the point where the LED is ON.
 * @return   none.
 */
void test_play_blink_x2_led(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);

#if defined(CONFIG_LED_MULTICOLOR)
	/* Arbitrary values set just for testing. */
	pattern->rgb[0].g = 0x1F;
	pattern->rgb[0].b = 0x1F;
	pattern->rgb[0].r = 0x00;
	pattern->rgb[1].g = 0x00;
	pattern->rgb[1].b = 0x3F;
	pattern->rgb[1].r = 0x7F;
	pattern->intensity = 0x0A;
#else
	pattern->intensity = 2;
#endif
	pattern->duration[0].duration_on = 3000;
	pattern->duration[0].duration_off = 1000;
	pattern->duration[1].duration_on = 1000;
	pattern->duration[1].duration_off = 1000;
	pattern->repetition_count = 2;

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED2,
				    LED_BLINK_X2,
				    pattern,
				    PATTERN_OVERRIDE,
				    NULL);

	bfree(pattern);
}


/* Test blinking LED with wave_x2 pattern.
 *
 * @details Test LED path: between test app, framework, ui service,
 *          down to the point where the LED is ON.
 * @return   none.
 */
void test_play_wave_led(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);

#if defined(CONFIG_LED_MULTICOLOR)
	/* Arbitrary values set just for testing. */
	pattern->rgb[0].g = 0x1F;
	pattern->rgb[0].b = 0x1F;
	pattern->rgb[0].r = 0x00;
	pattern->intensity = 0x0A;
#else
	pattern->intensity = 100;
#endif
	pattern->duration[0].duration_on = 6000;
	pattern->duration[0].duration_off = 1000;
	pattern->duration[1].duration_on = 3000;
	pattern->duration[1].duration_off = 1000;
	pattern->repetition_count = 2;

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED1,
				    LED_WAVE_X2,
				    pattern,
				    PATTERN_OVERRIDE,
				    NULL);

	bfree(pattern);
}

/* Test blinking LED with blink_X3 pattern.
 *
 * @details Test LED path: between test app, framework, ui service,
 *          down to the point where the LED is ON.
 * @return  none.
 */
void test_play_blink_x3_led(void)
{
	led_s *pattern;

	pattern = balloc(sizeof *pattern, NULL);

#if defined(CONFIG_LED_MULTICOLOR)
	/* Arbitrary values set just for testing. */
	pattern->rgb[0].g = 0x1F;
	pattern->rgb[0].b = 0x1F;
	pattern->rgb[0].r = 0x00;
	pattern->rgb[1].g = 0x00;
	pattern->rgb[1].b = 0x1F;
	pattern->rgb[1].r = 0xFF;
	pattern->rgb[2].g = 0x3F;
	pattern->rgb[2].b = 0x1F;
	pattern->rgb[2].r = 0x3F;
	pattern->intensity = 0x0A;
	pattern->duration[0].duration_on = 3000;
	pattern->duration[0].duration_off = 1000;
	pattern->duration[1].duration_on = 3000;
	pattern->duration[1].duration_off = 1000;
	pattern->duration[2].duration_on = 3000;
	pattern->duration[2].duration_off = 1000;
	pattern->repetition_count = 2;
#else
	/* This pattern is not supported */
#endif

	ui_service_play_led_pattern(ui_service_conn,
				    UI_LED2,
				    LED_BLINK_X3,
				    pattern,
				    PATTERN_OVERRIDE,
				    NULL);

	bfree(pattern);
}


/* Vibration test with pattern square_x2.
 *
 * @details Test DRV2605 haptic driver path: between test app, framework, ui service,
 *          down to the point where the vibration is ON.
 * @return   none.
 */
void test_play_square_x2_vibr(void)
{
	vibration_u *pattern;

	pattern = balloc(sizeof *pattern, NULL);

	pattern->square_x2.amplitude = 127;
	pattern->square_x2.duration_on_1 = 3000;
	pattern->square_x2.duration_off_1 = 1000;
	pattern->square_x2.duration_on_2 = 2000;
	pattern->square_x2.duration_off_2 = 1000;
	pattern->square_x2.repetition_count = 2;

	ui_service_play_vibr_pattern(ui_service_conn,
				     VIBRATION_SQUARE_X2,
				     pattern,
				     NULL);

	/* free pattern */
	bfree(pattern);
}


/* Vibration test with pattern special effect.
 *
 * @details Test DRV2605 haptic driver path: between test app, framework, ui service,
 *          down to the point where the vibration is ON.
 * @return   none.
 */
void test_play_special_vibr(void)
{
	vibration_u *pattern;

	pattern = balloc(sizeof *pattern, NULL);

	pattern->special_effect.effect_1 = 47;
	pattern->special_effect.duration_off_1 = 2000;
	pattern->special_effect.effect_2 = 48;
	pattern->special_effect.duration_off_2 = 2000;
	pattern->special_effect.effect_3 = 49;
	pattern->special_effect.duration_off_3 = 2000;
	pattern->special_effect.effect_4 = 50;
	pattern->special_effect.duration_off_4 = 2000;
	pattern->special_effect.effect_5 = 51;

	ui_service_play_vibr_pattern(ui_service_conn,
				     VIBRATION_SPECIAL_EFFECTS,
				     pattern,
				     NULL);

	/* free pattern */
	bfree(pattern);
}


#if defined(TEST_UI_SVC_PWRBTN)
/*
 * Function to set the button time range
 */
void test_set_btn_time()
{
	struct time_setting *new_time = NULL;
	int ret;

	new_time = balloc(sizeof(struct time_setting), NULL);
	new_time->min_range = 500;
	new_time->max_range = 3000;
	new_time->double_press_gap = 500;
	new_time->double_press_step = 1000;

	ret = ui_service_set_pwrbtn_time(ui_service_conn, new_time, NULL);

	if (ret)
		cu_print("TEST: Cannot set button time. Error [%d]\n", ret);
	bfree(new_time);
}
#endif


/* Function perform a test.
 *
 * @details When this function is called, a new test is executed:
 *        set events to driver,
 *        set event for new service,
 *        ....
 * NB: don't forget to change svc_evt_test value before calling this function
 * to play an other test.
 * @return   none.
 */
void test_callback()
{
	switch (svc_evt_test) {
#if defined(TEST_UI_SVC_PWRBTN)
	case TEST_EVT_FOR_DRV:
		cu_print("TEST: TEST_EVT_FOR_DRV\n");
		test_set_enabled_event_for_drv();
		break;
	case TEST_BTN_NOTIFICATION:
		cu_print("TEST: TEST_BTN_NOTIFICATION\n");
		/* launch press simulation */
		press_simulation(SS_GPIO_SERVICE_ID, PWRBTN_TST_AON_INPUT_PIN,
				 PWRBTN_TST_SS_OUTPUT_PIN);
		break;
	case TEST_BTN_TIME_MODIFIED:
		cu_print("TEST: TEST_BTN_TIME_MODIFED\n");
		/* set new time : min range = 3s =>
		 * only press duration < 3s and double press will be notified
		 * by boutton module */
		test_set_btn_time();
		/* launch press simulation */
		press_simulation(SS_GPIO_SERVICE_ID, PWRBTN_TST_AON_INPUT_PIN,
				 PWRBTN_TST_SS_OUTPUT_PIN);
		break;
	case TEST_BTN_EVT_ALLOWED_MODIFIED:
		cu_print("TEST: TEST_BTN_EVT_ALLOWED_MODIFIED\n");
		/* Set the double press as the only event allowed =>
		 * only double press will be notified */
		ui_service_set_enabled_events(
			ui_service_conn,
			UI_PWRBTN_SINGLE_EVT_IDX |
			UI_PWRBTN_DOUBLE_EVT_IDX,
			UI_PWRBTN_DOUBLE_EVT_IDX,
			NULL);
		/* launch press simulation */
		press_simulation(SS_GPIO_SERVICE_ID, PWRBTN_TST_AON_INPUT_PIN,
				 PWRBTN_TST_SS_OUTPUT_PIN);
		break;
#endif
	case TEST_EVT_FOR_SVC:
		cu_print("TEST: TEST_EVT_FOR_SVC\n");
		test_set_enabled_event_for_svc();
		break;
#if defined(TEST_UI_SVC_LED)
	case TEST_PLAY_BLINK_X1_LED:
		cu_print("TEST: TEST_PLAY_BLINK_X1_LED\n");
		test_play_blink_x1_led();
		break;
	case TEST_PLAY_BLINK_X2_LED:
		cu_print("TEST: TEST_PLAY_BLINK_X2_LED\n");
		test_play_blink_x2_led();
		break;
	case TEST_PLAY_WAVE_X2_LED:
		cu_print("TEST: TEST_PLAY_WAVE_X2_LED\n");
		test_play_wave_led();
		break;
	case TEST_PLAY_BLINK_X3_LED:
		cu_print("TEST: TEST_PLAY_BLINK_X3_LED\n");
		test_play_blink_x3_led();
		break;
	case TEST_LED_BUSY_ERROR:
		cu_print("TEST: TEST_LED_BUSY_ERROR\n");
		test_led_busy_error();
		break;
	case TEST_LED_DRV_ERROR:
		cu_print("TEST: TEST_LED_DRV_ERROR\n");
		test_led_drv_error();
		break;
#endif
#if defined(TEST_UI_SVC_VIBR)
	case TEST_PLAY_SQUARE_X2_VIBR:
		cu_print("TEST: TEST_PLAY_SQUARE_X2_VIBR\n");
		test_play_square_x2_vibr();
		break;
	case TEST_PLAY_SPECIAL_VIBR:
		cu_print("TEST: TEST_PLAY_SPECIAL_VIBR\n");
		test_play_special_vibr();
		break;
	case TEST_VIBR_BUSY_ERROR:
		cu_print("TEST: TEST_VIBR_BUSY_ERROR\n");
		test_haptic_busy_error();
		break;
	case TEST_VIBR_DRV_ERROR:
		cu_print("TEST: TEST_VIBR_DRV_ERROR\n");
		test_haptic_drv_error();
		break;
#endif
	case TEST_SET_BAD_EVT:
		cu_print("TEST: TEST_SET_BAD_EVT\n");
		test_set_event_error();
		/* This test doesn't send a response => increase test number end call test_callback */
		svc_evt_test++;
		test_callback();
		break;
	default:
		cu_print(
			"TEST: no more test to play -- end of UI_SERVICE Test Suite\n");
		break;
	}
}


/* Function to handle messages from framework.
 *
 * @param[in]  msg   Message from framework.
 * @param[in]  param Additional data. NULL if not used.
 * @return   none.
 */
void ui_svc_handle_message(struct cfw_message *msg, void *param)
{
	uint32_t ui_avail_events;
	uint32_t ui_avail_notifs;
	int client_events[5];
	int ui_status;
	uint8_t nb_events;
	cfw_open_conn_rsp_msg_t *cnf = NULL;
	uint32_t ui_events;
	uint32_t gest_id;

#if defined(TEST_UI_SVC_PWRBTN)
	union ui_service_drv_evt *ui_evt;
#endif

	gest_id = 0;
	ui_events = 0;

	cu_print("TEST: received msg: %d src: %d dst: %d",
		 CFW_MESSAGE_ID(msg),
		 CFW_MESSAGE_SRC(msg),
		 CFW_MESSAGE_DST(msg));

	switch (CFW_MESSAGE_ID(msg)) {
	case MSG_ID_CFW_OPEN_SERVICE_RSP: {
		cnf = (cfw_open_conn_rsp_msg_t *)msg;
		ui_service_conn = cnf->service_conn;
		if (ui_service_conn == NULL) panic(PANIC_FAIL_2);

		nb_events = 0;
		client_events[nb_events++] = MSG_ID_UI_SERVICE_BTN_SINGLE_EVT;
		client_events[nb_events++] = MSG_ID_UI_SERVICE_BTN_DOUBLE_EVT;

		/* Test event registration. */
		cfw_register_events(ui_service_conn, client_events, nb_events,
				    msg->priv);
		cu_print("TEST: service id %d opened", UI_SVC_SERVICE_ID);
	}
	break;
	case MSG_ID_CFW_CLOSE_SERVICE_RSP:
		cu_print("TEST: ui service has been closed\n");
		break;
	case MSG_ID_UI_SERVICE_GET_AVAILABLE_FEATURES_RSP: {
		ui_status =
			((ui_service_get_available_features_rsp_t *)msg)->
			status;
		ui_avail_events =
			((ui_service_get_available_features_rsp_t *)msg)->
			events;
		ui_avail_notifs =
			((ui_service_get_available_features_rsp_t *)msg)->
			notifications;

		cu_print(
			"TEST: get feat status:%d  events:0x%x notifications:0x%x",
			ui_status,
			ui_avail_events,
			ui_avail_notifs);
	}
	break;
	case MSG_ID_UI_SERVICE_GET_ENABLED_EVENT_RSP: {
		ui_status = ((ui_service_get_events_rsp_t *)msg)->status;
		if (!ui_status)
			ui_events =
				((ui_service_get_events_rsp_t *)msg)->events;

		cu_print("TEST: get evt status:%d config events:0x%x\n",
			 ui_status,
			 ui_events);
	}
	break;
	case MSG_ID_CFW_REGISTER_EVT_RSP:
		ui_svc_init_done = 1;
		cu_print(
			"TEST: events registered => start UI SERVICE Test Suite");

		/* Ready for first test. */
		svc_evt_test = TEST_EVT_FOR_SVC;

		test_callback();
		break;
	case MSG_ID_UI_SERVICE_SET_ENABLED_EVENT_RSP:
		ui_status = ((ui_service_set_events_rsp_t *)msg)->status;
		cu_print("TEST: set evt status: %d\n", ui_status);
		CU_ASSERT("Event setting failed", ui_status == UI_SUCCESS);
		/* Increase test number for next call except for pwr btn test */
#if defined(TEST_UI_SVC_PWRBTN)
		if (svc_evt_test != TEST_BTN_EVT_ALLOWED_MODIFIED) {
#endif
		svc_evt_test++;
		test_callback();
#if defined(TEST_UI_SVC_PWRBTN)
	}
#endif
		break;
#if defined(TEST_UI_SVC_LED)
	case MSG_ID_UI_SERVICE_PLAY_LED_PATTERN_RSP:
		ui_status = ((ui_play_led_pattern_rsp_t *)msg)->status;
		cu_print("TEST: led status: %d\n", ui_status, DRV_RC_OK);
		if (svc_evt_test == TEST_PLAY_BLINK_X1_LED ||
		    svc_evt_test == TEST_PLAY_BLINK_X2_LED ||
		    svc_evt_test == TEST_PLAY_WAVE_X2_LED) {
			CU_ASSERT("led pattern status KO",
				  ui_status == DRV_RC_OK);
		} else if (svc_evt_test == TEST_LED_DRV_ERROR) {
			CU_ASSERT("led driver status OK",
				  ui_status == DRV_RC_FAIL);
		} else if (svc_evt_test == TEST_LED_BUSY_ERROR) {
			CU_ASSERT("led driver status not busy",
				  ui_status == UI_BUSY);
		} else if (svc_evt_test == TEST_PLAY_BLINK_X3_LED) {
#if defined(CONFIG_LED_MULTICOLOR)
			CU_ASSERT("led driver status not Ok",
				  ui_status == DRV_RC_OK);
#else
			CU_ASSERT("led driver status pattern blink_x3 accepted",
				  ui_status == DRV_RC_MODE_NOT_SUPPORTED);
#endif
		}
		/* Increase test number for next call */
		svc_evt_test++;
		test_callback();
		break;
#endif
#if defined(TEST_UI_SVC_VIBR)
	case MSG_ID_UI_SERVICE_PLAY_VIBR_PATTERN_RSP:
		ui_status = ((ui_service_play_vibr_pattern_rsp_t *)msg)->status;
		cu_print("TEST: DRV2605 haptic driver status: %d\n", ui_status,
			 DRV_RC_OK);
		if (svc_evt_test == TEST_PLAY_SQUARE_X2_VIBR ||
		    svc_evt_test == TEST_PLAY_SPECIAL_VIBR) {
			CU_ASSERT("vibr pattern status not OK",
				  ui_status == DRV_RC_OK);
			svc_evt_test++;
			test_callback();
		} else if (svc_evt_test == TEST_VIBR_DRV_ERROR) {
			CU_ASSERT("haptic driver status not FAIL",
				  ui_status == DRV_RC_FAIL);
			svc_evt_test++;
			test_callback();
		} else if ((svc_evt_test == TEST_VIBR_BUSY_ERROR) &&
			   (g_nb_returned_rsp == 0)) {
			CU_ASSERT("haptic driver status not BUSY",
				  ui_status == UI_BUSY);
			svc_evt_test++;
			test_callback();
			g_nb_returned_rsp++;
		}
		/* check the second response of the test busy error, this response occur during led test */
		else if (g_nb_returned_rsp == 1) {
			CU_ASSERT("haptic driver status not OK",
				  ui_status == DRV_RC_OK);
			g_nb_returned_rsp++;
		}
		break;
#endif
#ifdef TEST_UI_SVC_PWRBTN
	case MSG_ID_UI_SERVICE_BTN_SINGLE_EVT:
		btn_notif_idx++;
		ui_evt = (union ui_service_drv_evt *)msg;
		cu_print(
			"TEST: power button single evt received (duration: %d) \n",
			ui_evt->btn_evt.duration);
		if (svc_evt_test == TEST_BTN_NOTIFICATION) {
			switch (btn_notif_idx) {
			case TST_NOMINAL_NOTIF_SINGLE_1:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			case TST_NOMINAL_NOTIF_SINGLE_2:
				CU_ASSERT(
					"Wrong duration for button notification (expected 2500ms)",
					(ui_evt->btn_evt.duration > 2500) &&
					(ui_evt->btn_evt.duration < 2600));
				break;
			case TST_NOMINAL_NOTIF_SINGLE_3:
				CU_ASSERT(
					"Wrong duration for button notification (expected 4000ms)",
					(ui_evt->btn_evt.duration > 4000) &&
					(ui_evt->btn_evt.duration < 4100));
				break;
			case TST_NOMINAL_NOTIF_SINGLE_4:
				CU_ASSERT(
					"Wrong duration for button notification (expected 7000ms)",
					(ui_evt->btn_evt.duration > 7000) &&
					(ui_evt->btn_evt.duration < 7100));
				break;
			case TST_NOMINAL_NOTIF_SINGLE_5:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			case TST_NOMINAL_NOTIF_SINGLE_6:
				CU_ASSERT(
					"Wrong duration for button notification (expected 2700ms)",
					(ui_evt->btn_evt.duration > 2700) &&
					(ui_evt->btn_evt.duration < 2800));
				break;
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		} else if (svc_evt_test == TEST_BTN_TIME_MODIFIED) {
			switch (btn_notif_idx) {
			case TST_TIME_MODIFIED_NOTIF_SINGLE_1:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			case TST_TIME_MODIFIED_NOTIF_SINGLE_2:
				CU_ASSERT(
					"Wrong duration for button notification (expected 2500ms)",
					(ui_evt->btn_evt.duration > 2500) &&
					(ui_evt->btn_evt.duration < 2600));
				break;
			case TST_TIME_MODIFIED_NOTIF_SINGLE_3:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			case TST_TIME_MODIFIED_NOTIF_SINGLE_4:
				CU_ASSERT(
					"Wrong duration for button notification (expected 2700ms)",
					(ui_evt->btn_evt.duration > 2700) &&
					(ui_evt->btn_evt.duration < 2800));
				break;
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		} else if (svc_evt_test == TEST_BTN_EVT_ALLOWED_MODIFIED) {
			switch (btn_notif_idx) {
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		}
		break;
	case MSG_ID_UI_SERVICE_BTN_DOUBLE_EVT:
		btn_notif_idx++;
		ui_evt = (union ui_service_drv_evt *)msg;
		cu_print(
			"TEST: power button double evt received (duration: %d)\n",
			ui_evt->btn_evt.duration);
		if (svc_evt_test == TEST_BTN_NOTIFICATION) {
			switch (btn_notif_idx) {
			case TST_NOMINAL_NOTIF_DOUBLE:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		} else if (svc_evt_test == TEST_BTN_TIME_MODIFIED) {
			switch (btn_notif_idx) {
			case TST_TIME_MODIFIED_NOTIF_DOUBLE:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		} else if (svc_evt_test == TEST_BTN_EVT_ALLOWED_MODIFIED) {
			switch (btn_notif_idx) {
			case TST_EVT_MODIFIED_NOTIF_DOUBLE:
				CU_ASSERT(
					"Wrong duration for button notification (expected 700ms)",
					(ui_evt->btn_evt.duration > 700) &&
					(ui_evt->btn_evt.duration < 800));
				break;
			default:
				CU_ASSERT("Wrong notification received", 0);
			}
		}
		svc_evt_test++;
		test_callback();
		break;
#endif
	default:
		break;
	}

	cfw_msg_free(msg);
}

/* Initialize ui service tests.
 *
 * @details This function is start point of UI_SERVICE test suite
 * @return   none.
 */
void ui_svc_test(void)
{
	svc_evt_test = 0;
	uint64_t timeout = 0;
	g_nb_returned_rsp = 0;

	test_type = TEST_UI_DEFAULT;

	cu_print(
		"##############################################################\n");
	cu_print(
		"# Purpose of user notification test :                        #\n");
	cu_print(
		"#     Test ui_service registration                           #\n");
	cu_print(
		"#     Test ui_service event setting                          #\n");
	cu_print(
		"#     Test to play vibration pattern                         #\n");
	cu_print(
		"#     Test to play led pattern                               #\n");
	cu_print(
		"#     Test some robustness case                              #\n");
	cu_print(
		"##############################################################\n");

	ui_svc = cfw_client_init(
		get_test_queue(), ui_svc_handle_message, UISVC_MAIN_CLIENT_NAME);
	if (ui_svc == NULL) panic(PANIC_FAIL_1);
	CU_ASSERT("UI_SVC_ID register failed ",
		  cfw_service_registered(UI_SVC_SERVICE_ID) != -1);

	cu_print("UI_SVC_ID opening service \n");
	cfw_open_service_conn(ui_svc, UI_SVC_SERVICE_ID,
			      (void *)UI_SVC_SERVICE_ID);
	SRV_WAIT((ui_svc_init_done != 1), 3000);
	CU_ASSERT("UI_SERVICE INIT FAILED !", ui_svc_init_done == 1);
	/* Loop until the end of test suite */
	cu_print("Waiting to perform %d test...", NB_TEST);
	while ((svc_evt_test < NB_TEST) && (++timeout <= 0xFFFFFF)) {
		/* dispatch message queue */
		queue_process_message(get_test_queue());
	}
}
#endif /* TEST_UI_SVC */
