#include "ancs51822.h"
#include "utility.h"

#include "bsp.h"

static uint8_t m_ancs_uuid_type;
/**<Store ANCS UUID. */

static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt);

//static const char * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
//{
// "Other",
// "Incoming Call",
// "Missed Call",
// "Voice Mail",
// "Social",
// "Schedule",
// "Email",
// "News",
// "Health And Fitness",
// "Business And Finance",
// "Location",
// "Entertainment"
//};
//static const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
//{
// "Added",
// "Modified",
// "Removed"
//};
//static const char * lit_attrid[BLE_ANCS_NB_OF_ATTRS] =
//{
// "App Identifier",
// "Title",
// "Subtitle",
// "Message",
// "Message Size",
// "Date",
// "Positive Action Label",
// "Negative Action Label"
//};

ble_ancs_c_t m_ancs_c;
/**< Structureused to identify the Apple Notification Service Client. */
static ble_ancs_c_evt_notif_t m_notification_latest; /**< Local copy

to keep track of the newest arriving notifications. */
static uint8_t m_attr_ident[BLE_ANCS_ATTR_DATA_MAX+40];
static uint8_t m_attr_title[BLE_ANCS_ATTR_DATA_MAX+40];

/**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[BLE_ANCS_ATTR_DATA_MAX+20];

/**< Buffer to store attribute data. */
//static uint8_t m_attr_message[BLE_ANCS_ATTR_DATA_MAX+20];

///**< Buffer to store attribute data. */
//static uint8_t m_attr_message_size[BLE_ANCS_ATTR_DATA_MAX+20];

///**< Buffer to store attribute data. */
//static uint8_t m_attr_date[BLE_ANCS_ATTR_DATA_MAX+20];

///**< Buffer to store attribute data. */
//static uint8_t m_attr_posaction[BLE_ANCS_ATTR_DATA_MAX+20];

///**< Buffer to store attribute data. */
//static uint8_t m_attr_negaction[BLE_ANCS_ATTR_DATA_MAX+20];

/**< Buffer to store attribute data. */
/**@brief Function for setting up GATTC notifications from the Notification Provider.
* *
@details This function is called when a successful connection has been established.
*/

//void Delay_100us(unsigned int  iTime)
//{
//	unsigned int i,j;
//	for(i=0;i<iTime;i++)
//	{
//		for (j=0;j<330;j++);
//	}
//}


//uint8_t start_apple_notifacation_setup=0;
void apple_notification_setup(void)
{

    uint32_t err_code;
    delay_ms(100);
//	Delay_100us(2000); // Delay because we cannot add a CCCD to close to starting encryption.iOS specific.
//	if(start_apple_notifacation_setup!=0xaa) return;
//  start_apple_notifacation_setup=0;
    err_code = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);
    err_code = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);


//	#ifdef PRINTF_ANCS
//	uint8_t buf_printf[100];
//	UsartTx(buf_printf,sprintf((char*)buf_printf,"Notifications Enabled.\n\r"));
//	#endif

}



ble_ancs_c_category_id_values_t category_id;
/**@brief Function for printing an iOS notification.
* *
@param[in] p_notif Pointer to the iOS notification.
*/
static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{

#ifdef PRINTF_ANCS
	uint8_t buf_printf[100];
    UsartTx(buf_printf,sprintf((char*)buf_printf,"\r\n\r\nEvent:%d 0-add 1modify 2 remove\r\n", p_notif->evt_id));
    UsartTx(buf_printf,sprintf((char*)buf_printf,"Category ID:%d\r\n",p_notif->category_id));
    UsartTx(buf_printf,sprintf((char*)buf_printf,"Category Cnt:%u\r\n", (unsigned int) p_notif->category_count));
    UsartTx(buf_printf,sprintf((char*)buf_printf,"UID:%u\r\n", (unsigned int) p_notif->notif_uid));
    UsartTx(buf_printf,sprintf((char*)buf_printf,"pre=%d\r\n",p_notif->evt_flags.pre_existing));
#endif


    if( (p_notif->evt_id==BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)&&
//		((p_notif->category_id==BLE_ANCS_CATEGORY_ID_INCOMING_CALL)||
//		(p_notif->category_id==BLE_ANCS_CATEGORY_ID_SOCIAL))&&
            (p_notif->evt_flags.pre_existing == 0) )
    {
        category_id=p_notif->category_id;
        if(NRF_SUCCESS==ble_ancs_c_request_attrs(p_notif))
				{
			#ifdef PRINTF_ANCS
			UsartTx(buf_printf,sprintf((char*)buf_printf,"ask attrs\r\n"));
			#endif
				}
    } else if( (p_notif->category_id==BLE_ANCS_CATEGORY_ID_MISSED_CALL)||((p_notif->category_id==BLE_ANCS_CATEGORY_ID_INCOMING_CALL)&&(BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED==p_notif->evt_id)))
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
* *
@param[in] p_attr Pointer to an iOS notification attribute.
* @param[in] ancs_attr_list Pointer to a list of attributes. Each entry in the list stores
a pointer to its attribute data, which is to be printed.
*/
static void notif_attr_print(ble_ancs_c_evt_notif_attr_t * p_attr,ble_ancs_c_attr_list_t * ancs_attr_list)
{
    static MSG_TYPE type;
    uint8_t *p;
    uint16_t len;
    uint16_t msglen;
    uint8_t msgbuf[16];

//	uint16_t gbk[64];
//	bool Isname;

    p=ancs_attr_list[p_attr->attr_id].p_attr_data;
    len=p_attr->attr_len;

#ifdef PRINTF_ANCS
	uint8_t buf_printf[100];
    UsartTx(p,len);
    UsartTx(buf_printf,sprintf((char*)buf_printf,"\r\nid:%d len=%d hex:", p_attr->attr_id,len));
    usart_send_buf();
    UsartPrintfHex(p,len);
    UsartTx("\r\n",2);
    usart_send_buf();
#endif

    if(len>BLE_ANCS_ATTR_DATA_MAX)
        return;
    //Delay_100us(10);
    if(p_attr->attr_id==BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
    {
        type=NONE;

        if((len == 21)&&(0==memcmp("com.apple.mobilephone",p,len))&&\
                (StuBandParaHistoryData.ancs_notice&ANCS_PHONE))
        {
            type=NEW_CALL;
        } else if((len == 19)&&(0==memcmp("com.apple.MobileSMS",p,len))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_MSG))
        {
            type=NEW_MSG;
        } else if((len == 15)&&(0==memcmp("com.tencent.xin",p,len))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_WEICHAT))
        {
            type=WEICHAR;
        } else if((len == 15)&&(0==memcmp("com.tencent.mqq",p,len))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_QQ))
        {
            type=QQ;
        } else if((len == 21)&&(0==memcmp("net.whatsapp.WhatsApp",p,len))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_WHATSAPP))
        {
            type=WHATSAPP;
        }
        else if((len>=13)&&(0==memcmp("com.facebook.",p,13))&&\
                (StuBandParaHistoryData.ancs_notice&ANCS_FACEBOOK))
        {
            type=FACEBOOK;
        }		else if((len==20)&&(0==memcmp("com.atebits.Tweetie2",p,len))&&\
                        (StuBandParaHistoryData.ancs_notice&ANCS_TWITTER))
        {
            //com.atebits.Tweetie2
            type=TWITTER;
        } else if((len>=10)&&(0==memcmp("com.skype.",p,10))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_SKYPE))
        {
            //com.skype.tomskype
            type=SKYPE;
        } else if((len>=23)&&(0==memcmp("com.toyopagroup.picaboo",p,23))&&\
                  (StuBandParaHistoryData.ancs_notice&ANCS_SNAPCHAT))
        {
            type=SNAPCHAT;
        }
        //com.apple.wallet




    } else if((p_attr->attr_id==BLE_ANCS_NOTIF_ATTR_ID_TITLE)&&(type!=NONE))
    {   /*解析数据*/
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
/**@brief Function for handling the Apple Notification Service client errors.
* *
@param[in] nrf_error Error code containing information about what went wrong.
*/
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}
/**@brief Function for initializing the Apple Notification Center Service.
*/
void service_init_ancs(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ble_uuid_t service_uuid;
    uint32_t err_code;

    err_code = sd_ble_uuid_vs_add(&ble_ancs_base_uuid128, &m_ancs_uuid_type);
    APP_ERROR_CHECK(err_code);
    err_code = sd_ble_uuid_vs_add(&ble_ancs_cp_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);
    err_code = sd_ble_uuid_vs_add(&ble_ancs_ns_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);
    err_code = sd_ble_uuid_vs_add(&ble_ancs_ds_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);
    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER, m_attr_ident,
                                   BLE_ANCS_ATTR_DATA_MAX);
    APP_ERROR_CHECK(err_code);
    err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_TITLE, m_attr_title,
                                   BLE_ANCS_ATTR_DATA_MAX);
    APP_ERROR_CHECK(err_code);
	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
	m_attr_subtitle,BLE_ANCS_ATTR_DATA_MAX);
	APP_ERROR_CHECK(err_code);
//	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_MESSAGE, m_attr_message,
//	BLE_ANCS_ATTR_DATA_MAX);
//	APP_ERROR_CHECK(err_code);
//	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
//	m_attr_message_size, BLE_ANCS_ATTR_DATA_MAX);
//	APP_ERROR_CHECK(err_code);
//	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_DATE, m_attr_date,
//	BLE_ANCS_ATTR_DATA_MAX);
//	APP_ERROR_CHECK(err_code);
//	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,m_attr_posaction,BLE_ANCS_ATTR_DATA_MAX);
//	APP_ERROR_CHECK(err_code);
//	err_code = ble_ancs_c_attr_add(BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,m_attr_negaction,BLE_ANCS_ATTR_DATA_MAX);
//	APP_ERROR_CHECK(err_code);

    ancs_init_obj.evt_handler = on_ancs_c_evt;
//	on_ancs_c_evt(0);
//	ancs_init_obj.evt_handler(0);
    ancs_init_obj.error_handler = apple_notification_error_handler;
    err_code = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for handling the Apple Notification Service client.
* *
@details This function is called for all events in the Apple Notification client that
* are passed to the application.
* *
@param[in] p_evt Event received from the Apple Notification Service client.
*/
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    // uint32_t err_code = NRF_SUCCESS;
    switch (p_evt->evt_type)
    {
    case BLE_ANCS_C_EVT_DISCOVER_COMPLETE:
        apple_notification_setup();
//		start_apple_notifacation_setup=0xaa;
        break;
    case BLE_ANCS_C_EVT_NOTIF:
        m_notification_latest = p_evt->notif;
        notif_print(&m_notification_latest);
        break;
    case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
        notif_attr_print(&p_evt->attr, p_evt->ancs_attr_list);
        break;
    case BLE_ANCS_C_EVT_DISCOVER_FAILED:
        // ANCS not found.
#if 0
        if (s110_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            err_code = sd_ble_gap_disconnect(s110_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        }
#endif
        break;
    default:
        // No implementation needed.
        break;
    }
}

