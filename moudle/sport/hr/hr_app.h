#ifndef HR_APP_H___
#define HR_APP_H___
#include "stdint.h"
#include "stdbool.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "SoftwareRTC.h"
#include "in_flash_manage.h"
typedef enum {stop=0,hr_play,get_hr} HR_STATUE;



#define GET_BP  2
#define TIME_OUT_BP 1
typedef struct
{
    uint16_t get_unit;//��ȡ���ʺ󣬼�����ʵ�ʱ�� ����SAMP_FREQ����λ������
    uint16_t work_time;//����ģ�鹤����ʱ��msһ����λ
    volatile uint8_t hr;//��ȡ�������ʣ��������һ�ε�����
    uint8_t hr_fielt;//�����˲��������
    uint8_t wearFlg;//�����־��Ϊ���ʾ���
    HR_STATUE hr_statues;// ����ģ�鹤����״̬
    bool auto_detect_hr;
	uint8_t bpend;
	uint8_t bph;
	uint8_t bpl;
} STU_HR;


void InitHeatRate(void);
/*��ȡ����ģ���״̬  ֹͣ���������У���ȡ��������*/
uint8_t GetHrState(STU_HR *stuhr);

/*
start_flg Ϊ�濪ʼ���,����Ϊ��
*/
void StartHeatRate(uint8_t start_flg);


bool HrCheckWear(uint16_t *hr_detect);
bool ReadWearFlg(void);


void TaskAutoDetectHeartRate(uint8_t inter_min10,RTC_UTCTimeStruct tm,bool wear);



void WriteHeatRate(uint32_t timesec);



void NoticeCloseHr(void);


void task_check_wear(void);
uint16_t em7028_start_tect(bool startflg,uint8_t detect_sec);
void save_hr_or_redetect(void);




void gsesor_in(int16_t x,int16_t y,int16_t z );


void TaskCheckWearAndTestHr(void);

#define AUTO_TEST_TIME_OUT  (30000)

extern uint8_t hr_ok;
extern bool is_bp_detect;
extern uint8_t start_bp;
extern STU_HR StuHr;
#endif






