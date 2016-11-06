/*
 * Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**
 * @brief BLE Heart Rate Collector application main file.
 *
 * This file contains the source code for a sample heart rate collector.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_db_discovery.h"
#include "softdevice_handler.h"
#include "app_util.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "peer_manager.h"
#include "ble_hrs_c.h"
#include "ble_bas_c.h"
#include "app_util.h"
#include "app_timer.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "fstorage.h"
#include "ble_conn_state.h"
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "app_pwm.h"
#include "nrf_drv_pwm.h"
#include "app_fifo.h"


#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE        GATT_MTU_SIZE_DEFAULT               /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define CENTRAL_LINK_COUNT          0                                   /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT       0                                   /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define STRING_BUFFER_LEN           50
#define BOND_DELETE_ALL_BUTTON_ID   0                                   /**< Button used for deleting all bonded centrals during startup. */

#define APP_TIMER_PRESCALER         0                                   /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE     2                                   /**< Size of timer operation queues. */

#define SEC_PARAM_BOND              1                                   /**< Perform bonding. */
#define SEC_PARAM_MITM              0                                   /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC              0                                   /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS          0                                   /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES   BLE_GAP_IO_CAPS_NONE                /**< No I/O capabilities. */
#define SEC_PARAM_OOB               0                                   /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE      7                                   /**< Minimum encryption key size in octets. */
#define SEC_PARAM_MAX_KEY_SIZE      16                                  /**< Maximum encryption key size in octets. */

#define SCAN_INTERVAL               0x0050                              /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                 0x004F                              /**< Determines scan window in units of 0.625 millisecond. */

#define MIN_CONNECTION_INTERVAL     MSEC_TO_UNITS(7.5, UNIT_1_25_MS)    /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL     MSEC_TO_UNITS(30, UNIT_1_25_MS)     /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY               0                                   /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT         MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Determines supervision time-out in units of 10 millisecond. */

#define IBEACON_HEADER				0x1AFF4C000215
#define TARGET_UUID_0				0xFAE37D16A7F65896
#define TARGET_UUID_1				0xA83768345875DBEB

#define TARGET_MAJOR				9
#define TARGET_MINOR				16

#define DISAPPEAR_THRESHOLD			20
#define DELAY_FIFO_LENGTH			10

/**@brief Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)


/**@brief Variable length data encapsulation in terms of length and pointer to data */
typedef struct
{
    uint8_t  * p_data;      /**< Pointer to data. */
    uint16_t   data_len;    /**< Length of data. */
} data_t;

/**@brief iBeacon data type */
typedef struct
{
    uint8_t   flag_len;
    uint8_t * flags;
    uint8_t   header_len;
    uint8_t * header_data;
    uint8_t   uuid_len;
    uint8_t * uuid;
    uint32_t  major;
    uint32_t  minor;
    int8_t    tx_pwr;
} ibcn_data_t;

static bool               		m_memory_access_in_progress;  	/**< Flag to keep track of ongoing operations on persistent memory. */
static ble_gap_scan_params_t 	m_scan_param;   				/** @brief Scan parameters requested for scanning and connection. */
static volatile uint8_t			attempts;
static volatile bool			turned_on;
//static volatile app_fifo_t		delay;
static volatile uint32_t		delay[DELAY_FIFO_LENGTH];
static volatile uint32_t		total;
static volatile uint32_t		actual_delay;
//static volatile bool			ready_flag;

static void scan_start(void);

//static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);
APP_PWM_INSTANCE(PWM0,0);  // Create the instance "PWM0" using TIMER0.


/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


//void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
//{
//    ready_flag = true;
//}

/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t ibcn_adv_report_parse(data_t * advdata, ibcn_data_t * ibcn_data)
{
    int		  index = 0;
    uint8_t * p_data;
    uint64_t  tmp;

    p_data = advdata->p_data;

    ibcn_data->flag_len = p_data[index];
    ibcn_data->flags = &p_data[index + 1];

    index += ibcn_data->flag_len + 1;

    ibcn_data->header_len = 6;
    ibcn_data->header_data = &p_data[index];

    index += ibcn_data->header_len;

    ibcn_data->uuid_len = 16;
    ibcn_data->uuid = &p_data[index];

    index += ibcn_data->uuid_len;

    ibcn_data->major = (p_data[index] << 8) | (p_data[index + 1]);
    ibcn_data->minor = (p_data[index + 2] << 8) | (p_data[index + 3]);
    ibcn_data->tx_pwr = (int8_t)p_data[index + 4];

    tmp = IBEACON_HEADER;
    for (index = ibcn_data->header_len - 1; index >= 0; index--) {
    	if (ibcn_data->header_data[index] != (tmp & 0xFF))
    		return NRF_ERROR_NOT_FOUND;
    	tmp >>= 8;
    }

    tmp = TARGET_UUID_1;
	for (index = ibcn_data->uuid_len - 1; index >= (ibcn_data->uuid_len / 2); index--) {
		if (ibcn_data->uuid[index] != (tmp & 0xFF))
			return NRF_ERROR_NOT_FOUND;
		tmp >>= 8;
	}

	tmp = TARGET_UUID_0;
	for (index = (ibcn_data->uuid_len / 2) - 1; index >= 0; index--) {
		if (ibcn_data->uuid[index] != (tmp & 0xFF))
			return NRF_ERROR_NOT_FOUND;
		tmp >>= 8;
	}

    return NRF_SUCCESS;
}


static uint32_t ibcn_find_broadcaster(ibcn_data_t * ibcn_data, uint16_t target_major, uint16_t target_minor) {
	if (ibcn_data->major != target_major)
	    	return NRF_ERROR_NOT_FOUND;

	if (ibcn_data->minor != target_minor)
		return NRF_ERROR_NOT_FOUND;

	return NRF_SUCCESS;
}


/* Borrowed from David G Young (http://stackoverflow.com/questions/20416218/understanding-ibeacon-distancing) */
static double calculateAccuracy(int8_t txPower, int8_t rssi) {
	double accuracy;

	if (rssi == 0) {
		return -1.0; // if we cannot determine accuracy, return -1.
	}

	double ratio = rssi*1.0/txPower;
	if (ratio < 1.0) {
		return pow(ratio,10);
	} else {
		accuracy = (0.89976)*pow(ratio,7.7095) + 0.111;
		return accuracy;
  	}
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
//static void sleep_mode_enter(void)
//{
//    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
//    APP_ERROR_CHECK(err_code);
//
//    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);
//
//    // Go to system-off mode (this function will not return; wakeup will cause a reset).
//    err_code = sd_power_system_off();
//    APP_ERROR_CHECK(err_code);
//}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    //uint32_t                err_code;
    const ble_gap_evt_t   * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
        	//SEGGER_RTT_WriteString(0, "something found!\n\r");

            data_t 		adv_data;
            ibcn_data_t ibcn_data;
            int8_t		rssi;
            double		distance;
            uint32_t	current_delay;
            int8_t		i;

            // Initialize advertisement report for parsing.
            adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
            adv_data.data_len = p_gap_evt->params.adv_report.dlen;
            rssi = p_gap_evt->params.adv_report.rssi;

            if (ibcn_adv_report_parse(&adv_data, &ibcn_data) == NRF_SUCCESS && ibcn_find_broadcaster(&ibcn_data, TARGET_MAJOR, TARGET_MINOR) == NRF_SUCCESS) {

            	if (!turned_on) {
					//LEDS_ON(BSP_LED_1_MASK);
					turned_on = true;
					//SEGGER_RTT_WriteString(0, "phone found!\n\r");
            	}
            	attempts = 0;
            	total = 0;

            	distance = calculateAccuracy(ibcn_data.tx_pwr, rssi);
            	current_delay = 50 + (uint32_t)(distance * 950);

            	for (i = 0; i < DELAY_FIFO_LENGTH - 1; i++) {
            		delay[i] = delay[i + 1];
            		total += delay[0];
            	}
            	delay[DELAY_FIFO_LENGTH - 1] = current_delay;
            	total += current_delay;

            	actual_delay = (total + actual_delay) / DELAY_FIFO_LENGTH + 1;

            } else if (attempts < DISAPPEAR_THRESHOLD) {

            	attempts++;

            } else if (turned_on) {

            	//LEDS_OFF(BSP_LED_1_MASK);
            	turned_on = false;
            	//SEGGER_RTT_WriteString(0, "phone lost!\n\r");

            }

        }break; // BLE_GAP_EVT_ADV_REPORT

        default:
            break;
    }
}


/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch (sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
            /* fall through */
        case NRF_EVT_FLASH_OPERATION_ERROR:

            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                scan_start();
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *  been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    // Modules which depend on ble_conn_state, like Peer Manager,
    // should have their callbacks invoked after ble_conn_state's.
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    fs_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event) {}


/**@brief Function to start scanning.
 */
static void scan_start(void)
{
    uint32_t flash_busy;

    // If there is any pending write to flash, defer scanning until it completes.
    (void) fs_queued_op_count_get(&flash_busy);

    if (flash_busy != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    ret_code_t ret;

    m_scan_param.active   = 0;
    m_scan_param.interval = SCAN_INTERVAL;
    m_scan_param.window   = SCAN_WINDOW;
	m_scan_param.timeout  = 0x0000; // No timeout.

	#if (NRF_SD_BLE_API_VERSION == 2)
		m_scan_param.selective   = 0;
		m_scan_param.p_whitelist = NULL;
	#endif

	#if (NRF_SD_BLE_API_VERSION == 3)
		m_scan_param.use_whitelist  = 0;
		m_scan_param.adv_dir_report = 0;
	#endif

    NRF_LOG_DEBUG("Starting scan.\r\n");

    (void) sd_ble_gap_scan_stop();

    	ret = 0;
        ret = sd_ble_gap_scan_start(&m_scan_param);
        // It is okay to ignore this error since we are stopping the scan anyway.
        if (ret != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(ret);
        }
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for the Power manager.
 */
//static void power_manage(void)
//{
//    uint32_t err_code = sd_app_evt_wait();
//    APP_ERROR_CHECK(err_code);
//}


int main(void)
{
    bool		erase_bonds;
    //uint32_t 	err_code;
    //uint32_t	delay_init[DELAY_FIFO_LENGTH];
    int8_t		i;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
    buttons_leds_init(&erase_bonds);
    log_init();
    ble_stack_init();
    if (erase_bonds == true)
    {
        NRF_LOG_INFO("Bonds erased!\r\n");
    }

    for (i = 0; i < DELAY_FIFO_LENGTH; i++)
    	delay[i] = 500;
    //app_fifo_init(&delay, delay_init, DELAY_FIFO_LENGTH);

//    /* 1-channel PWM, 200Hz, output on DK LED pin. */
//    app_pwm_config_t pwm0_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, BSP_LED_0);
//
//    /* Initialize and enable PWM. */
//	err_code = app_pwm_init(&PWM0,&pwm0_cfg,pwm_ready_callback);
//	APP_ERROR_CHECK(err_code);
//	app_pwm_enable(&PWM0);

    attempts = DISAPPEAR_THRESHOLD;
    //total = 500 * DELAY_FIFO_LENGTH;
    turned_on = false;
    //actual_delay = 500;

    // Start scanning for peripherals and initiate connection
    // with devices that advertise Heart Rate UUID.
    NRF_LOG_INFO("Heart rate collector example\r\n");
    scan_start();

    for (;;)
    {
    	if (turned_on) {
			LEDS_INVERT(BSP_LED_3_MASK);
			nrf_delay_ms(actual_delay);
    	} else {
    		LEDS_OFF(BSP_LED_3_MASK);
    	}
//        if (NRF_LOG_PROCESS() == false)
//        {
//            power_manage();
//        }
    }
}
