#ifndef _BLE_CONFIG_H__
#define _BLE_CONFIG_H__

#include "nrf_gpio.h"
#include "stdint.h"
#include "stdbool.h"
#include "softdevice_handler.h"
#include "string.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "utility.h"
#include "nrf_ble_ancs_c.h"
#include "ble_db_discovery.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "peer_manager.h"
#include "app_timer.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "fds.h"
#include "fstorage.h"
#include "utility.h"
#include "app_timer.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_log.h"
#include "ble_fifo.h"


#define ATTR_DATA_SIZE                 BLE_ANCS_ATTR_DATA_MAX                      /**< Allocated size for attribute data. */
#define MESSAGE_BUFFER_SIZE            18                                          /**< Size of buffer holding optional messages in notifications. */
#define SEC_PARAM_BOND                 1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                 0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                 0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS             0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES      BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                  0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE         7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE         16                                          /**< Maximum encryption key size. */




#define APP_ADV_FAST_INTERVAL          1600                                          /**< The advertising interval (in units of 0.625 ms). The default value corresponds to 25 ms. */
#define APP_ADV_FAST_TIMEOUT           60                                         /**< The advertising time-out in units of seconds. */

#define APP_ADV_SLOW_INTERVAL          3200                                        /**< Slow advertising interval (in units of 0.625 ms). The default value corresponds to 2 seconds. */
#define APP_ADV_SLOW_TIMEOUT           180                                         /**< The advertising time-out in units of seconds. */

#define MIN_CONN_INTERVAL              MSEC_TO_UNITS(300, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL              MSEC_TO_UNITS(500, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                  0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT               MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory time-out (4 seconds). */

#define MIN_CONN_INTERVAL_QUICK              MSEC_TO_UNITS(7.5, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL_QUICK              MSEC_TO_UNITS(20, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */



#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(10000)  /**< Time from initiating an event (connect or start of notification) to the first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT   3                                           /**< Number of attempts before giving up the connection parameter negotiation. */



#define CONN_CFG_TAG 0


bool is_ble_tx_enable(void);
void s132_config_enable(bool en_ble);
void power_manage(void);
void StartAdv(bool startfast);
void get_mac(uint8_t *mac);
void close_all_gpio(void);
bool IsBleConnect(void);
void BleWriteData(uint8_t * p_string, uint16_t length);
bool send_ble_data(uint8_t * p_string, uint16_t length);

void ChangeToHightSpeed(bool hight_speed);
void AskPair(void);
void EraseBond(void);
#endif






