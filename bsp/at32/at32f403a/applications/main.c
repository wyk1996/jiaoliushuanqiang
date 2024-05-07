/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file
 * @note
 * @brief
 *
 * @author
 * @date
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <time.h>
#include <board.h>
#include <fal.h>
#include <string.h>
#include "user_lib.h"
#include "easyflash.h"
#include "user_lib.h"
#include "ch_port.h"
#include "mfrc522.h"
#include "w25qxx.h"
#include "4GMain.h"
#include "cmsis_armcc.h"
extern _m1_card_info m1_card_info;

SYSTEM_RTCTIME gs_SysTime;

CP56TIME2A_T gsCP56Time;

/* 用于获取RTC毫秒 */
int32_t gsi_RTC_Counts;

uint32_t gui_RTC_millisecond;

//extern S_DEVICE_CFG gs_DevCfg;
//extern S_APP_CHARGE gs_AppCharge;
extern long list_mem(void);
extern int wdt_sample();
/* UNIQUE_ID[31: 0] */
uint32_t Unique_ID1;
/* UNIQUE_ID[63:32] */
uint32_t Unique_ID2;
/* UNIQUE_ID[95:63] */
uint32_t Unique_ID3;



uint32_t test = 0;
/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */


//当前存储  0x8000 000     大小是 4B000=307200(300k)       boot程序(100K)  地址0x804B000  ---0x807 D000
SYSTEM_RTCTIME gs_SysTime;
pFunction Jump_To_LoadCode;
uint32_t JumpAddress;
#define LoadCodeAddress    0x804B000    //0x804B000    // 跳转到烧写代码的位置，通过这个位置执行程序，将应用代码从FLASH里面导出来烧写到应用程序区域
//跳转到烧写代码部分
void  JumpToProgramCode(void)
{
    __set_FAULTMASK(1);	//关闭中断,确保跳转过程中 不会进入中断,导致跳转失败
    __disable_irq();  // 可以使用这个函数 关闭总中断
    //rt_hw_interrupt_disable();  //只是屏蔽了全局【中断请求】，配置使能的中断，依旧可以【中断】，只是，中断处理函数ISR，暂不执行。

    /* 关闭滴答定时器，复位到默认值 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    /* 关闭所有中断，清除所有中断挂起标志 */
    uint8_t i=0;
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    JumpAddress = *(__IO uint32_t*) (LoadCodeAddress + 4);
    Jump_To_LoadCode = (pFunction) JumpAddress;
    //初始化用户程序的堆栈指针
    __set_MSP(*(__IO uint32_t*) LoadCodeAddress);
    Jump_To_LoadCode();
}


int main(void)
{
    rt_device_t device;

    struct tm *Net_time;

    time_t time_now = 0;

    uint32_t timeout = rt_tick_get();

    uint32_t mem_timeout = rt_tick_get();

    device = rt_device_find("rtc");

    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }

    RT_ASSERT(fal_init() > 0);

    rt_kprintf("The current version of APP firmware is %s\n", FIRMWARE_VERSION);

//    if(rt_memcmp((const char *)&(item_info_node->uc_SN[2]),(const char *)ucDev_Sn,6)==0)

    /* 获取MCU唯一ID */
    Unique_ID1 = *(uint32_t *)(0x1FFFF7E8);
    Unique_ID1 = SW32(Unique_ID1);
    Unique_ID2 = *(uint32_t *)(0x1FFFF7EC);
    Unique_ID2 = SW32(Unique_ID2);
    Unique_ID3 = *(uint32_t *)(0x1FFFF7F0);
    Unique_ID3 = SW32(Unique_ID3);

    rt_kprintf("MCU Id:%08x %08x %08x\n",Unique_ID1,Unique_ID2,Unique_ID3);

    memcpy(&m1_card_info.MCUID[0],&Unique_ID1, 4);
    memcpy(&m1_card_info.MCUID[4],&Unique_ID2, 4);
    memcpy(&m1_card_info.MCUID[8],&Unique_ID3, 4);

#if(doguser)
    wdt_sample(); //看门狗程序20s   喂狗函数在空闲线程里面
#endif

    while(1)
    {
        /* 每隔1000个滴答(1000ms),更新一下gs_SysTime,使其保持为最新值 */
        if(rt_tick_get()>=(100+timeout))
        {
            timeout = rt_tick_get();
            rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time_now);

            //time_now = time_now + 8*60*60;
            Net_time = localtime(&time_now);
            gs_SysTime.ucYear = Net_time->tm_year;
            gs_SysTime.ucMonth = (Net_time->tm_mon)+1;
            gs_SysTime.ucDay = Net_time->tm_mday;
            gs_SysTime.ucWeek = Net_time->tm_wday;
            gs_SysTime.ucHour = Net_time->tm_hour;
            gs_SysTime.ucMin = Net_time->tm_min;
            gs_SysTime.ucSec = Net_time->tm_sec;

            //list_thread();  //查看线程栈使用率
            localtime_to_cp56time((time_now+28800), &gsCP56Time);

//			rt_kprintf("NTP Time:%04d-%02d-%02d-%02d %02d:%02d:%02d\r\n",(Net_time->tm_year)+1900,\
//                    (Net_time->tm_mon)+1, Net_time->tm_mday, Net_time->tm_wday, Net_time->tm_hour,Net_time->tm_min,gs_SysTime.ucSec);
        }

//        /* 每隔30秒,查询一下内存信息 */
//		if(rt_tick_get()>=(30000+mem_timeout))
//		{
//			mem_timeout = rt_tick_get();
//			list_mem();
//		}

        rt_thread_mdelay(100);
    }
}
