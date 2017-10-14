/**
 * Copyright (c) 2012 - 2017, Nordic Semiconductor ASA

 */
#include "s132config.h"

// #if 0
// /**@brief String literals for the iOS notification categories. used then printing to UART. */


// static const char * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
// {
//     "Other",
//     "Incoming Call",
//     "Missed Call",
//     "Voice Mail",
//     "Social",
//     "Schedule",
//     "Email",
//     "News",
//     "Health And Fitness",
//     "Business And Finance",
//     "Location",
//     "Entertainment"
// };

// /**@brief String literals for the iOS notification event types. Used then printing to UART. */
// static const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
// {
//     "Added",
//     "Modified",
//     "Removed"
// };


// /**@brief String literals for the iOS notification attribute types. Used when printing to UART. */
// static const char * lit_attrid[BLE_ANCS_NB_OF_NOTIF_ATTR] =
// {
//     "App Identifier",
//     "Title",
//     "Subtitle",
//     "Message",
//     "Message Size",
//     "Date",
//     "Positive Action Label",
//     "Negative Action Label"
// };


// /**@brief String literals for the iOS notification attribute types. Used When printing to UART. */
// static const char * lit_appid[BLE_ANCS_NB_OF_APP_ATTR] =
// {
//     "Display Name"
// };
// #endif

static ble_ancs_c_t       m_ancs_c;                                     /**< Structure used to identify the Apple Notification Service Client. */
static ble_db_discovery_t m_ble_db_discovery;                           /**< Structure used to identify the DB Discovery module. */
static nrf_ble_gatt_t     m_gatt;                                       /**< GATT module instance. */
//static pm_peer_id_t       m_peer_id;                                    /**< Device reference handle to the current bonded central. */
static uint16_t           m_cur_conn_handle = BLE_CONN_HANDLE_INVALID;  /**< Handle of the current connection. */


static ble_ancs_c_evt_notif_t m_notification_latest;                    /**< Local copy to keep track of the newest arriving notifications. */
//static ble_ancs_c_attr_t      m_notif_attr_latest;                      /**< Local copy of the newest notification attribute. */
//static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;               /**< Local copy of the newest app attribute. */

static uint8_t m_attr_appid[ATTR_DATA_SIZE+40];                            /**< Buffer to store attribute data. */

static uint8_t m_attr_title[ATTR_DATA_SIZE+40];                            /**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[ATTR_DATA_SIZE+20];                         /**< Buffer to store attribute data. */

// static uint8_t m_attr_message[ATTR_DATA_SIZE];                          /**< Buffer to store attribute data. */
// static uint8_t m_attr_message_size[ATTR_DATA_SIZE];                     /**< Buffer to store attribute data. */
// static uint8_t m_attr_date[ATTR_DATA_SIZE];                             /**< Buffer to store attribute data. */
// static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */
// static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */

// static uint8_t m_attr_disp_name[ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */

static void delete_bonds(void);


//==========================
#include "ble_nus.h"
//
//uint32_t ble_nus_string_send(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length);

ble_nus_t                        m_nus;
/*ble接收数据回调*/
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length);



static void services_nus_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}
//====================


void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xdeadefff, line_num, p_file_name);
}


/**@brief Fetch the list of peer manager peer IDs.
 *
 * @param[inout] p_peers   The buffer where to store the list of peer IDs.
 * @param[inout] p_size    In: The size of the @p p_peers buffer.
 *                         Out: The number of peers copied in the buffer.
 */
//static void peer_list_get(pm_peer_id_t * p_peers, uint32_t * p_size)
//{
//    pm_peer_id_t peer_id;
//    uint32_t     peers_to_copy;

//    peers_to_copy = (*p_size < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) ?
//                     *p_size : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

//    peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
//    *p_size = 0;

//    while ((peer_id != PM_PEER_ID_INVALID) && (peers_to_copy--))
//    {
//        p_peers[(*p_size)++] = peer_id;
//        peer_id = pm_next_peer_id_get(peer_id);
//    }
//}



/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t ret;

    switch (p_evt->evt_id)
    {
    case PM_EVT_BONDED_PEER_CONNECTED:
    {
        NRF_LOG_DEBUG("Connected to previously bonded device\r\n");
//            m_peer_id = p_evt->peer_id;
    }
    break; // PM_EVT_BONDED_PEER_CONNECTED

    case PM_EVT_CONN_SEC_SUCCEEDED:
    {
        NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.\r\n",
                     ble_conn_state_role(p_evt->conn_handle),
                     p_evt->conn_handle,
                     p_evt->params.conn_sec_succeeded.procedure);

//            m_peer_id = p_evt->peer_id;

        // Note: You should check on what kind of white list policy your application should use.
        if (p_evt->params.conn_sec_succeeded.procedure == PM_LINK_SECURED_PROCEDURE_BONDING)
        {
            NRF_LOG_DEBUG("New Bond, add the peer to the whitelist if possible\r\n");
//                NRF_LOG_DEBUG("\tm_whitelist_peer_cnt %d, MAX_PEERS_WLIST %d\r\n",
//                               m_whitelist_peer_cnt + 1,
//                               BLE_GAP_WHITELIST_ADDR_MAX_COUNT);

//                if (m_whitelist_peer_cnt < BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
//                {
//                    // Bonded to a new peer, add it to the whitelist.
//                    m_whitelist_peers[m_whitelist_peer_cnt++] = m_peer_id;
//                    m_is_wl_changed = true;
//                }
        }
        // Discover peer's services.
        memset(&m_ble_db_discovery, 0x00, sizeof(m_ble_db_discovery));
        ret  = ble_db_discovery_start(&m_ble_db_discovery, p_evt->conn_handle);
        APP_ERROR_CHECK(ret);
    }
    break;

    case PM_EVT_CONN_SEC_FAILED:
    {
        /* Often, when securing fails, it shouldn't be restarted, for security reasons.
         * Other times, it can be restarted directly.
         * Sometimes it can be restarted, but only after changing some Security Parameters.
         * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
         * Sometimes it is impossible, to secure the link, or the peer device does not support it.
         * How to handle this error is highly application dependent. */
    } break;

    case PM_EVT_CONN_SEC_CONFIG_REQ:
    {
        // Reject pairing request from an already bonded peer.
        pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
    }
    break;

    case PM_EVT_STORAGE_FULL:
    {
        // Run garbage collection on the flash.
        ret = fds_gc();
        if (ret == FDS_ERR_BUSY || ret == FDS_ERR_NO_SPACE_IN_QUEUES)
        {
            // Retry.
        }
        else
        {
            APP_ERROR_CHECK(ret);
        }
    }
    break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
    {
//            advertising_start(false);
    } break;

    case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
    {
        // The local database has likely changed, send service changed indications.
        pm_local_database_has_changed();
    }
    break;

    case PM_EVT_PEER_DATA_UPDATE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
    }
    break;

    case PM_EVT_PEER_DELETE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
    }
    break;

    case PM_EVT_PEERS_DELETE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
    }
    break;

    case PM_EVT_ERROR_UNEXPECTED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
    }
    break;

    case PM_EVT_CONN_SEC_START:
    case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
    case PM_EVT_PEER_DELETE_SUCCEEDED:
    case PM_EVT_LOCAL_DB_CACHE_APPLIED:
    case PM_EVT_SERVICE_CHANGED_IND_SENT:
    case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
    default:
        break;
    }
}


/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
//static void sec_req_timeout_handler(void * p_context)
//{
//    ret_code_t           ret;
//    pm_conn_sec_status_t status;

//    if (m_cur_conn_handle != BLE_CONN_HANDLE_INVALID)
//    {
//        ret = pm_conn_sec_status_get(m_cur_conn_handle, &status);
//        APP_ERROR_CHECK(ret);

//        // If the link is still not secured by the peer, initiate security procedure.
//        if (!status.encrypted)
//        {
//            ret = pm_conn_secure(m_cur_conn_handle, false);
//            if (ret != NRF_ERROR_INVALID_STATE)
//            {
//                APP_ERROR_CHECK(ret);
//            }
//        }
//    }
//}




void AskPair(void)
{

    ret_code_t           ret;
    pm_conn_sec_status_t status;

    if (m_cur_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        ret = pm_conn_sec_status_get(m_cur_conn_handle, &status);
        APP_ERROR_CHECK(ret);
        LOG("pair1");

        // If the link is still not secured by the peer, initiate security procedure.
        if (!status.encrypted)
        {
            delete_bonds();
            LOG("pair2");
            ret = pm_conn_secure(m_cur_conn_handle, true);
            if (ret != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(ret);
            }
        }
    }
}


/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    ret_code_t ret;

    delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("Notifications Enabled.\r\n");
}


static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{


    LOG("\r\n\r\nEvent:%d 0-add 1modify 2 remove\r\n", p_notif->evt_id);
    LOG("Category ID:%d\r\n",p_notif->category_id);
    LOG("Category Cnt:%u\r\n", (unsigned int) p_notif->category_count);
    LOG("UID:%u\r\n", (unsigned int) p_notif->notif_uid);
    LOG("pre=%d\r\n",p_notif->evt_flags.pre_existing);



    if( (p_notif->evt_id==BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)&&
//      ((p_notif->category_id==BLE_ANCS_CATEGORY_ID_INCOMING_CALL)||
//      (p_notif->category_id==BLE_ANCS_CATEGORY_ID_SOCIAL))&&
            (p_notif->evt_flags.pre_existing == 0) )
    {
//        category_id=p_notif->category_id;
        if(NRF_SUCCESS==nrf_ble_ancs_c_request_attrs(&m_ancs_c, p_notif))
        {

            LOG("ask attrs\r\n");

        }
    }
    else if( (p_notif->category_id==BLE_ANCS_CATEGORY_ID_MISSED_CALL)||((p_notif->category_id==BLE_ANCS_CATEGORY_ID_INCOMING_CALL)&&(BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED==p_notif->evt_id)))
    {
        //LcdOnTime=1;/*电话挂机返回主界面*/
    }
}


/*返回最大数据长度为16*/
uint8_t FormatMsg(uint8_t*msgin,uint8_t msglen,uint8_t* msgout)
{
    uint8_t outlen=0,i;
//	id:1 hex:4c697570656e67

//id:1 hex:e894a1e5be84e794b1


//??159-1948-1246??
//id:1 hex:e280ad3135392d313934382d31323436e280ac

//	??+86 159 1948 1246
//id:1 hex:e2808e2b38362031353920313934382031323436

    for(i=0; i<msglen; i++)
    {
        if(msgin[i]<0x80)
        {
            if((msgin[i]<0x7b)&&(msgin[i]>0x2f))
                msgout[outlen++]=msgin[i];
        } else if((msgin[i]==0xe2)&&\
                  (msgin[i+1]==0x80)&&\
                  (msgin[i+2]==0xad)
                 )
        {
            i+=2;
        }
        else if((msgin[i]==0xe2)&&\
                (msgin[i+1]==0x80)&&\
                (msgin[i+2]==0x8e)
               )
        {
            i+=2;
        } else
        {
            break;
        }
        if(outlen>=16)
        {
            break;
        }
    }

    return outlen;

}
#include "protocol.h"
#include "in_flash_manage.h"
/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS notification attribute.
 */
static void notif_attr_print(ble_ancs_c_attr_t * p_attr)//,ble_ancs_c_attr_list_t * ancs_attr_list
{
    static MSG_TYPE type;
    uint8_t *p;
    uint16_t len;
    uint16_t msglen;
    uint8_t msgbuf[16];

    //	uint16_t gbk[64];
    //	bool Isname;

// 		p_attr_data

    p=p_attr->p_attr_data;
    len=p_attr->attr_len;

// #ifdef PRINTF_ANCS
// 		uint8_t buf_printf[100];
// 		UsartTx(p,len);
// 		UsartTx(buf_printf,sprintf((char*)buf_printf,"\r\nid:%d len=%d hex:", p_attr->attr_id,len));
// 		usart_send_buf();
// 		UsartPrintfHex(p,len);
// 		UsartTx("\r\n",2);
// 		usart_send_buf();
// #endif

    if(len>BLE_ANCS_ATTR_DATA_MAX)
        return;
    //Delay_100us(10);
    if(p_attr->attr_id==BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
    {
        type=NONE;

        if((len == 21)&&(0==memcmp("com.apple.mobilephone",p,len))&&\
                (StuEeprom.StuPara.ancs_notice&ANCS_PHONE))
        {
            type=NEW_CALL;
        } else if((len == 19)&&(0==memcmp("com.apple.MobileSMS",p,len))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_MSG))
        {
            type=NEW_MSG;
        } else if((len == 15)&&(0==memcmp("com.tencent.xin",p,len))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_WEICHAT))
        {
            type=WEICHAR;
        } else if((len == 15)&&(0==memcmp("com.tencent.mqq",p,len))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_QQ))
        {
            type=QQ;
        } else if((len == 21)&&(0==memcmp("net.whatsapp.WhatsApp",p,len))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_WHATSAPP))
        {
            type=WHATSAPP;
        }
        else if((len>=13)&&(0==memcmp("com.facebook.",p,13))&&\
                (StuEeprom.StuPara.ancs_notice&ANCS_FACEBOOK))
        {
            type=FACEBOOK;
        }		else if((len==20)&&(0==memcmp("com.atebits.Tweetie2",p,len))&&\
                        (StuEeprom.StuPara.ancs_notice&ANCS_TWITTER))
        {
            //com.atebits.Tweetie2
            type=TWITTER;
        } else if((len>=10)&&(0==memcmp("com.skype.",p,10))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_SKYPE))
        {
            //com.skype.tomskype
            type=SKYPE;
        } else if((len>=23)&&(0==memcmp("com.toyopagroup.picaboo",p,23))&&\
                  (StuEeprom.StuPara.ancs_notice&ANCS_SNAPCHAT))
        {
            type=SNAPCHAT;
        }
        //com.apple.wallet




    } else if((p_attr->attr_id==BLE_ANCS_NOTIF_ATTR_ID_TITLE)&&(type!=NONE))
    {   /*?a??êy?Y*/
        //		msglen=len;
        msglen=0;
        if(len)
        {
            msglen=FormatMsg(p,len,msgbuf);
            //msglen=IosMsgFormat(p,len,(uint16_t*)gbk,&Isname);
        }

        if((type==WEICHAR)&&(0==msglen))
        {
            msglen=6;
            memcpy(msgbuf,"WeChat",msglen);
        }
        if((type==WHATSAPP)&&(0==msglen))
        {
            msglen=8;
            memcpy(msgbuf,"WhatsApp",msglen);
        }
        if((type==FACEBOOK)&&(0==msglen))
        {
            msglen=8;
            memcpy(msgbuf,"Facebook",msglen);
        }
        if((type==TWITTER)&&(0==msglen))
        {
            msglen=7;
            memcpy(msgbuf,"Twitter",msglen);
        }
        if((type==SKYPE)&&(0==msglen))
        {
            msglen=5;
            memcpy(msgbuf,"Skype",msglen);
        }
        if((type==SNAPCHAT)&&(0==msglen))
        {
            msglen=8;
            memcpy(msgbuf,"SnapChat",msglen);
        }
        SendMsg(type,msglen,msgbuf);
        //		if(type==ANCS_TYPE_SMS)
        //		{
        //			SendMsg(type,msglen,msgbuf);
        //			//SendSms(true,Isname,msglen,(uint8_t*)gbk,1,1,5);
        //		}else if(type==ANCS_TYPE_PHONE)
        //		{
        //			SendMsg(NEW_CALL,msglen,msgbuf);
        //			//SendSms(false,Isname,msglen,(uint8_t*)gbk,1,1,10);
        //		}
    }
}

/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS App attribute.
 */
static void app_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
        //  NRF_LOG_INFO("%s: %s\r\n", (uint32_t)lit_appid[p_attr->attr_id], (uint32_t)p_attr->p_attr_data);
    }
    else if (p_attr->attr_len == 0)
    {
        //  NRF_LOG_INFO("%s: (N/A)\r\n", (uint32_t) lit_appid[p_attr->attr_id]);
    }
}


/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
    case BLE_ANCS_NP_UNKNOWN_COMMAND:
        NRF_LOG_INFO("Error: Command ID was not recognized by the Notification Provider. \r\n");
        break;

    case BLE_ANCS_NP_INVALID_COMMAND:
        NRF_LOG_INFO("Error: Command failed to be parsed on the Notification Provider. \r\n");
        break;

    case BLE_ANCS_NP_INVALID_PARAMETER:
        NRF_LOG_INFO("Error: Parameter does not refer to an existing object on the Notification Provider. \r\n");
        break;

    case BLE_ANCS_NP_ACTION_FAILED:
        NRF_LOG_INFO("Error: Perform Notification Action Failed on the Notification Provider. \r\n");
        break;

    default:
        break;
    }
}
///**@brief Function for initializing the timer module.
// */
//static void timers_init(void)
//{
//    ret_code_t ret;

//    ret = app_timer_init();
//    APP_ERROR_CHECK(ret);

//    // Create security request timer.
//    ret = app_timer_create(&m_sec_req_timer_id,
//                           APP_TIMER_MODE_SINGLE_SHOT,
//                           sec_req_timeout_handler);
//    APP_ERROR_CHECK(ret);
//}


/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    ret_code_t ret = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
    case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
        NRF_LOG_DEBUG("Apple Notification Center Service discovered on the server.\r\n");
        ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
        APP_ERROR_CHECK(ret);
        apple_notification_setup();
        break;

    case BLE_ANCS_C_EVT_NOTIF:
        m_notification_latest = p_evt->notif;
        notif_print(&m_notification_latest);
        break;
    case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
        notif_attr_print(&p_evt->attr);//, p_evt->ancs_attr_list
        break;
    case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
        NRF_LOG_DEBUG("Apple Notification Center Service not discovered on the server.\r\n");
        break;

    case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
        app_attr_print(&p_evt->attr);

        break;
    case BLE_ANCS_C_EVT_NP_ERROR:
        err_code_print(p_evt->err_code_np);
        break;
    default:
        // No implementation needed.
        break;
    }
}


/**@brief Function for initializing GAP connection parameters.
 *
 * @details Use this function to set up all necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(bool quick_flg)
{
    ret_code_t              ret;
    ble_gap_conn_params_t   gap_conn_params;


    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    if(quick_flg==true)
    {
        gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL_QUICK;
        gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL_QUICK;
    }
    else
    {
        gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
        gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    }
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;


    ret = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(ret);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t ret = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(ret);
}


/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             ret;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    ret = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(ret);
}


/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    static uint8_t first=0;

    ble_gap_sec_params_t sec_param;
    ret_code_t           ret;

    if(first!=0) return ;
    first=1;
    ret = pm_init();
    APP_ERROR_CHECK(ret);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    ret = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(ret);

    ret = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(ret);
}


/**
 * @brief Delete all data stored for all peers
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!\r\n");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
//static void sleep_mode_enter(void)
//{
//    uint32_t ret = bsp_indication_set(BSP_INDICATE_IDLE);
//    APP_ERROR_CHECK(ret);

//    // Prepare wakeup buttons.
//    ret = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(ret);

//    // Go to system-off mode (this function will not return; wakeup will cause a reset).
//    ret = sd_power_system_off();
//    APP_ERROR_CHECK(ret);
//}


/**@brief Function for handling advertising events.
 *
 * @details This function is called for advertising events that are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t ret;

//     LOG("evt:%x\r\n",ble_adv_evt);
    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        NRF_LOG_INFO("Fast advertising\r\n");
//            ret = bsp_indication_set(BSP_INDICATE_ADVERTISING);
//            APP_ERROR_CHECK(ret);
        break;

    case BLE_ADV_EVT_SLOW:
        NRF_LOG_INFO("Slow advertising\r\n");
//            ret = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
//            APP_ERROR_CHECK(ret);
        break;

    case BLE_ADV_EVT_FAST_WHITELIST:
        NRF_LOG_INFO("Fast advertising with Whitelist\r\n");
//            ret = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
//            APP_ERROR_CHECK(ret);
        break;

    case BLE_ADV_EVT_IDLE:
        StartAdv(true);
//            sleep_mode_enter();
        break;

    case BLE_ADV_EVT_WHITELIST_REQUEST:
    {
        ble_gap_addr_t whitelist_addrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        ble_gap_irk_t  whitelist_irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        uint32_t       addr_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
        uint32_t       irk_cnt  = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

        ret = pm_whitelist_get(whitelist_addrs, &addr_cnt, whitelist_irks, &irk_cnt);
        APP_ERROR_CHECK(ret);
        NRF_LOG_DEBUG("pm_whitelist_get returns %d addr in whitelist and %d irk whitelist\r\n",
                      addr_cnt,
                      irk_cnt);

        // Apply the whitelist.
        ret = ble_advertising_whitelist_reply(whitelist_addrs,
                                              addr_cnt,
                                              whitelist_irks,
                                              irk_cnt);
        APP_ERROR_CHECK(ret);
    }
    break;

    default:
        break;
    }


}


/**@brief Function for handling the application's BLE stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    ret_code_t ret = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected.\r\n");
//            ret = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(ret);

        m_cur_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//            ret               = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
//            APP_ERROR_CHECK(ret);
        break; // BLE_GAP_EVT_CONNECTED

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected.\r\n");
        m_cur_conn_handle = BLE_CONN_HANDLE_INVALID;
//            ret               = app_timer_stop(m_sec_req_timer_id);
//            APP_ERROR_CHECK(ret);

        if (p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle)
        {
            m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
        }
//            if (m_is_wl_changed)
//            {
//                // The whitelist has been modified, update it in the Peer Manager.
//                ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
//                APP_ERROR_CHECK(ret);

//                ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
//                if (ret != NRF_ERROR_NOT_SUPPORTED)
//                {
//                    APP_ERROR_CHECK(ret);
//                }

//                m_is_wl_changed = false;
//            }
        break; // BLE_GAP_EVT_DISCONNECTED

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        NRF_LOG_DEBUG("GATT Client Timeout.\r\n");
        ret = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(ret);
        break; // BLE_GATTC_EVT_TIMEOUT

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        NRF_LOG_DEBUG("GATT Server Timeout.\r\n");
        ret = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(ret);
        break; // BLE_GATTS_EVT_TIMEOUT

    default:
        // No implementation needed.
        break;
    }
    APP_ERROR_CHECK(ret);
}


///**@brief Function for handling events from the BSP module.
// *
// * @param[in] event  Event generated by button press.
// */
//static void bsp_event_handler(bsp_event_t event)
//{
//    ret_code_t ret;

//    switch (event)
//    {
//        case BSP_EVENT_SLEEP:
////            sleep_mode_enter();
//            break;

//        case BSP_EVENT_DISCONNECT:
//            ret = sd_ble_gap_disconnect(m_cur_conn_handle,
//                                        BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            if (ret != NRF_ERROR_INVALID_STATE)
//            {
//                APP_ERROR_CHECK(ret);
//            }
//            break;

//        case BSP_EVENT_WHITELIST_OFF:
//            if (m_ancs_c.conn_handle == BLE_CONN_HANDLE_INVALID)
//            {
//                ret = ble_advertising_restart_without_whitelist();
//                if (ret != NRF_ERROR_INVALID_STATE)
//                {
//                    APP_ERROR_CHECK(ret);
//                }
//            }
//            break;

//        case BSP_EVENT_KEY_0:
//            ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &m_notification_latest);
//            APP_ERROR_CHECK(ret);
//            break;

//        case BSP_EVENT_KEY_1:
//            if(m_notif_attr_app_id_latest.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER
//                && m_notif_attr_app_id_latest.attr_len != 0)
//            {
//                NRF_LOG_INFO("Request for %s: \r\n", (uint32_t)m_notif_attr_app_id_latest.p_attr_data);
//                ret = nrf_ble_ancs_c_app_attr_request(&m_ancs_c,
//                                                      m_notif_attr_app_id_latest.p_attr_data,
//                                                      m_notif_attr_app_id_latest.attr_len);
//                APP_ERROR_CHECK(ret);
//            }
//            break;

//        case BSP_EVENT_KEY_2:
//            if(m_notification_latest.evt_flags.positive_action == true)
//            {
//                NRF_LOG_INFO("Performing Positive Action.\r\n");
//                ret = nrf_ancs_perform_notif_action(&m_ancs_c,
//                                                    m_notification_latest.notif_uid,
//                                                    ACTION_ID_POSITIVE);
//                APP_ERROR_CHECK(ret);
//            }
//            break;

//        case BSP_EVENT_KEY_3:
//            if(m_notification_latest.evt_flags.negative_action == true)
//            {
//                NRF_LOG_INFO("Performing Negative Action.\r\n");
//                ret = nrf_ancs_perform_notif_action(&m_ancs_c,
//                                                    m_notification_latest.notif_uid,
//                                                    ACTION_ID_NEGATIVE);
//                APP_ERROR_CHECK(ret);
//            }
//            break;

//        default:
//            break;
//    }
//}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    /** The Connection state module has to be fed BLE events in order to function correctly
     * Remember to call ble_conn_state_on_ble_evt before calling any ble_conns_state_* functions. */
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_ancs_c_on_ble_evt(&m_ancs_c, p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
//    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the system event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}



static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                       .rc_ctiv       = 0,                                \
                                       .rc_temp_ctiv  = 0,                                \
                                       .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM
                                      };

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = softdevice_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Overwrite some of the default configurations for the BLE stack.
    ble_cfg_t ble_cfg;

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = softdevice_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Apple Notification Center Service.
 */
static void services_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ret_code_t        ret;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  m_attr_appid,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
//                                       BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
//                                       m_attr_disp_name,
//                                       sizeof(m_attr_disp_name));
//     APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  m_attr_title,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                   BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
//                                   m_attr_message,
//                                   ATTR_DATA_SIZE);
//     APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                  m_attr_subtitle,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                   BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
//                                   m_attr_message_size,
//                                   ATTR_DATA_SIZE);
//     APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                   BLE_ANCS_NOTIF_ATTR_ID_DATE,
//                                   m_attr_date,
//                                   ATTR_DATA_SIZE);
//     APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                   BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
//                                   m_attr_posaction,
//                                   ATTR_DATA_SIZE);
//     APP_ERROR_CHECK(ret);

//     ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                   BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
//                                   m_attr_negaction,
//                                   ATTR_DATA_SIZE);
//     APP_ERROR_CHECK(ret);

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    ret = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(ret);
}





/**@brief Function for initializing the advertising functionality.
 */
static void advertising_init(uint8_t *name,uint8_t len,uint8_t *menu,uint8_t menulen)
{
    ble_adv_modes_config_t options;
    ret_code_t             ret;
    ble_advdata_t          advdata;
    ble_gap_conn_sec_mode_t sec_mode;
    static ble_uuid_t                 adv_uuids= {BLE_UUID_NUS_SERVICE,BLE_UUID_TYPE_BLE};

    memset(&advdata, 0, sizeof(advdata));

    advdata.flags                    = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    advdata.uuids_complete.uuid_cnt=1;
    advdata.uuids_complete.p_uuids=&adv_uuids;

    //09
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    ret = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)name,len);
    APP_ERROR_CHECK(ret);
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;

    //ff
    ble_advdata_manuf_data_t adv_manu;
    adv_manu.company_identifier=((uint16_t)menu[1]<<8)|menu[0];
    adv_manu.data.p_data = &menu[2];
    adv_manu.data.size = menulen-2;
    advdata.p_manuf_specific_data = &adv_manu;


    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled      = true;
    options.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;
    options.ble_adv_slow_enabled      = true;
    options.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;

    ret = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(ret);
}

static uint8_t nrf52_mac[6];
void get_mac(uint8_t *mac)
{
    memcpy(mac,nrf52_mac,6);
}
bool is_ble_tx_enable(void)
{
    return m_nus.is_notification_enabled;
}
void set_addr(ble_gap_addr_t addr)
{
    uint8_t i;
    for(i=0; i<6; i++)
        nrf52_mac[i]=addr.addr[5-i];
}
extern const uint8_t version[8];
void StartAdv(bool startfast)
{
    uint32_t err_code;
    uint8_t buf[9];
    m_nus.is_notification_enabled=false;
    m_cur_conn_handle = BLE_CONN_HANDLE_INVALID;
    get_mac(buf);
    buf[6]=version[0];
    buf[7]=version[1];
    buf[8]=version[2];
    advertising_init("h703",4,buf,9);
    if(true==startfast)
    {
        err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    }
    else
    {
        err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);
    }
    APP_ERROR_CHECK(err_code);
}
static void db_discovery_init(void)
{
    ret_code_t ret = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(ret);
}


void close_all_gpio(void)
{
    uint8_t i;
    for(i = 2; i< 32 ; i++)
    {
        nrf_gpio_close(i);
    }
	
	
	nrf_gpio_cfg_output(KEY_IC_POWER_PIN);
	nrf_gpio_pin_clear(KEY_IC_POWER_PIN);
	nrf_gpio_cfg_output(KEY_PIN);
	nrf_gpio_pin_clear(KEY_PIN);


}



uint8_t ble_enable_flg=0;
bool erase_bound=false;
void power_manage(void)
{
    if(ble_enable_flg)
    {
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        __WFE();
        __SEV();
        __WFE();
    }
}



void conn_params_negotiation(void);
void ChangeToHightSpeed(bool hight_speed)
{

    gap_params_init(hight_speed);
    conn_params_init();
    conn_params_negotiation();

}


void EraseBond(void)
{
    erase_bound=true;

}


void s132_config_enable(bool en_ble)
{
    uint32_t err_code;
    ble_gap_addr_t addr;
    if(false!=en_ble)
    {
        ble_stack_init();
        gap_params_init(true);
        gatt_init();
        db_discovery_init();
        services_init();
        services_nus_init();
        conn_params_init();
        peer_manager_init();
        if(true==erase_bound)
            delete_bonds();

        sd_ble_gap_addr_get(&addr);
        set_addr(addr);
        erase_bound=false;
        StartAdv(true);
        ble_enable_flg = 1;
//         err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
//         APP_ERROR_CHECK(err_code);
    }
    else
    {
        ble_enable_flg=0;
        err_code=softdevice_handler_sd_disable();
        APP_ERROR_CHECK(err_code);
        ble_conn_params_stop();
    }


}



/*ble 发送数据接口*/
void BleWriteData(uint8_t * p_string, uint16_t length)
{

//  uint8_t  *TxBufDataOut(uint8_t *len)
//bool TxBleDataIn(uint8_t *buf,uint8_t len)


    uint32_t i;
    for(i=0; i<4; i++)
    {

        if(m_nus.is_notification_enabled!=true) return ;
        uint32_t err_code = ble_nus_string_send(&m_nus, p_string, length);
        if (err_code == NRF_SUCCESS)
        {
            delay_ms(20);
            return ;
        }
        delay_ms(200);
    }


}


bool send_ble_data(uint8_t * p_string, uint16_t length)
{
    uint32_t err_code = ble_nus_string_send(&m_nus, p_string, length);

    if (err_code == NRF_SUCCESS)
    {
        return true;
    }
    return false;
}
bool IsBleConnect(void)
{
    if(m_cur_conn_handle == BLE_CONN_HANDLE_INVALID)
        return false;
    else
        return true;
}
/*ble接收数据回调*/
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    BleQueueIn(p_data,(uint8_t)length);
//     uint16_t i;
//     LOG("BLE");
//     for(i=0; i<length; i++)
//     {
//         LOG("%02x",p_data[i]);
//     }
//     LOG("\r\n");
//     if(p_data[0]==0x39)
//     {
//         AskPair();
//     }  if(p_data[0]==0x38)
//     {
//        app_error_handler(0,0,0);
//     }
//


}



