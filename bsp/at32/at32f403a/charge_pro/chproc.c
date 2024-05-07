/*******************************************************************************
 * @file chproc.c
 * @note
 * @brief 充电处理
 *
 * @author
 * @date     2021-05-02
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
#include <string.h>
#include "drv_gpio.h"
#include "chtask.h"
#include "rn8209d.h"
#include "common.h"
#include "dwin_com_pro.h"

#include "mfrc522.h"
#include "dlt645_port.h"
#include "YKCFrame.h"
#include "4GMain.h"


#define CH_ENABLE_PIN1              GET_PIN(C,0)           /*  B枪火线 */       // GET_PIN(C,8) //98//PC8 
#define CH_ENABLE_PIN2              GET_PIN(C,1)           /*  B枪零线 */      // GET_PIN(C,9) //99//PC9 

#define CH_ENABLE_PIN3              GET_PIN(C,2)          /*  A枪零线 */        // GET_PIN(C,8) //98//PC8 
#define CH_ENABLE_PIN4              GET_PIN(C,8)          /*  A枪火线 */       // GET_PIN(C,9) //99//PC9 

#define BEIJI_TIMESTAMP_OFFSET      (8 * 60 * 60)
#define CH_DAT_SEC                  (24 * 60 * 60)
#define CH_HALF_HOUR_SEC            (60 * 60 / 2)
//extern CH_TASK_T stChTcb;
typedef struct
{
    uint8_t       ucRunFlg;
    uint32_t      uiTime;
    rt_timer_t    stChTime;
} CH_RELAY_T;

extern RATE_T	           stChRate;                 /* 充电费率  */
extern RATE_T	           stChRateA;                 /* 充电费率  */
extern RATE_T	           stChRateB;                 /* 充电费率  */
extern CH_TASK_T stChTcb[GUN_MAX];
extern SYSTEM_RTCTIME gs_SysTime;

extern _dlt645_info dlt645_info[GUN_MAX];			  //电表信息
extern _m1_card_info  AB_GUNcard_info[GUN_MAX];
uint8_t electricity_err(_GUN_NUM gun,CH_TASK_T *pstChTcb);


void cp_pwm_full(void);
void cp_pwm9_full(void);
void cp1_pwm_ch_puls(void);
void cp2_pwm_ch_puls(void);
void cp1_pwm_ch_half(void);
void cp2_pwm_ch_half(void);


/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void  ch_ctl_init(void)
{
    rt_pin_mode(CH_ENABLE_PIN2, GPIO_MODE_OUTPUT);
    rt_pin_mode(CH_ENABLE_PIN1, GPIO_MODE_OUTPUT);
    rt_pin_mode(CH_ENABLE_PIN3, GPIO_MODE_OUTPUT);
    rt_pin_mode(CH_ENABLE_PIN4, GPIO_MODE_OUTPUT);

}

/**
 * @brief 充电控制使能
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
//void ch_ctl_enable(void)
//{
//    rt_pin_write(CH_ENABLE_PIN2, PIN_HIGH );
//    rt_thread_mdelay(100);
//    rt_pin_write(CH_ENABLE_PIN1, PIN_HIGH);
//}

void ch_ctl_enable(_GUN_NUM gun)
{
    if(GUN_A==gun)
    {
        rt_pin_write(CH_ENABLE_PIN3, PIN_HIGH );
        rt_thread_mdelay(100);
        rt_pin_write(CH_ENABLE_PIN4, PIN_HIGH);
    } else if(GUN_B==gun)
    {
        rt_pin_write(CH_ENABLE_PIN2, PIN_HIGH );
        rt_thread_mdelay(100);
        rt_pin_write(CH_ENABLE_PIN1, PIN_HIGH);
    }
}


/**
 * @brief 充电控制禁止
 * @param[in]
 * @param[out]
 * @return
 * @note
 */

//void ch_ctl_disable(void)
//{
//    // rt_enter_critical();

//    rt_pin_write(CH_ENABLE_PIN2, PIN_LOW);
//    rt_thread_mdelay(100);
//    rt_pin_write(CH_ENABLE_PIN1, PIN_LOW);
//    //rt_exit_critical();
//}


void ch_ctl_disable(_GUN_NUM gun)
{
    if(GUN_A==gun)
    {
        rt_pin_write(CH_ENABLE_PIN3, PIN_LOW );
        rt_thread_mdelay(100);
        rt_pin_write(CH_ENABLE_PIN4, PIN_LOW);
    } else if(GUN_B==gun)
    {
        rt_pin_write(CH_ENABLE_PIN2, PIN_LOW );
        rt_thread_mdelay(100);
        rt_pin_write(CH_ENABLE_PIN1, PIN_LOW);
    }
}


/**
 * @brief 跳转到新状态
 * @param[in] pstChTcb 指向充电任务的数据
 *            ucNewStat 要跳到的新状态
 * @param[out]
 * @return
 * @note
 */
void ch_jump_new_stat(CH_TASK_T *pstChTcb,uint8_t ucNewStat)
{
    pstChTcb->ucState = ucNewStat;
    pstChTcb->uiJumpTick = rt_tick_get();
}

/**
 * @brief 是否故障
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]  返回1时，故障发生
 * @return
 * @note
 */
uint8_t ch_is_fault(FAULT_T *pstFault)
{
    if(0 == pstFault->uiFaultFlg)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief 是否插枪
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_insert(GUN_T *pstGunTcb)
{
    if(CP_6V == pstGunTcb->ucCpStat || CP_9V == pstGunTcb->ucCpStat)      /* 插枪 */
    {
        pstGunTcb->ucCount = 0;
        return 1;
    }

    if(++pstGunTcb->ucCount > 5)                                          /* 连续5次检测到未插枪 */
    {
        pstGunTcb->ucCount = 0;
        return 0;
    }

    return 1;
}

/**
 * @brief cp 是否off
 * @param[in] pstGunTcb 指向枪
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_cp_off(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    static uint32_t curtick[2] = {0},lasttick[2] = {0};   //AB枪两个时间差
    curtick[gun] = rt_tick_get();
    if((CP_6V == pstChTcb->stGunStat.ucCpStat) || (CP_9V == pstChTcb->stGunStat.ucCpStat))    /* cp 充电中未断开 */
    {
        lasttick[gun] = curtick[gun];
        pstChTcb->stGunStat.ucCount_6v = 0;
        //ch_ctl_enable(gun);

        return 0;
    }

    if((++pstChTcb->stGunStat.ucCount_6v > 5) && ((curtick[gun] - lasttick[gun]) >  CM_TIME_2_SEC))      /* cp 连续5次off   且 连续2s*/
    {
        lasttick[gun] = curtick[gun];
        pstChTcb->stGunStat.ucCount_6v = 0;
        return 1;
    }

    return 0;
}


/**
 * @brief 是否过流
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_over_current(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        pstChTcb->stOverCurr.uiTick  = rt_tick_get();    /* 不停获取当前时间 */
        return 0;
    }

    if(DisSysConfigInfo.energymeter == 0)
    {
        if(dlt645_info[gun].out_cur < 35.0)
        {
            pstChTcb->stOverCurr.ucFlg = CH_DEF_FLG_DIS;       /* 没超过最大电流 */
            return 0;
        }
    }
    else
    {
        if(pstChTcb->stRn8209.usCurrent < CH_CURR_MAX)
        {
            pstChTcb->stOverCurr.ucFlg = CH_DEF_FLG_DIS;       /* 没超过最大电流 */
            return 0;
        }
    }

    if(CH_DEF_FLG_DIS == pstChTcb->stOverCurr.ucFlg)       /*  第一次发现过流 */
    {
        pstChTcb->stOverCurr.ucFlg   = CH_DEF_FLG_EN;      /* 置标志 */
        pstChTcb->stOverCurr.uiTick  = rt_tick_get();      /* 获取当前时间 */
        return 0;
    }


    if(((rt_tick_get() > pstChTcb->stOverCurr.uiTick) ? ((rt_tick_get() - pstChTcb->stOverCurr.uiTick)) : \
            ((0xFFFFFFFF - pstChTcb->stOverCurr.uiTick) + rt_tick_get())) < CH_TIME_15_SEC)
    {
        return 0;
    }


//    if(rt_tick_get() - pstChTcb->stOverCurr.uiTick < CH_TIME_15_SEC)
//    {
//        return 0;
//    }

    return 1;
}

/**
 * @brief 是否过压
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_over_volt(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        pstChTcb->stOverVolt.uiTick  = rt_tick_get();    /* 不停获取当前时间 */
        return 0;
    }


    if(DisSysConfigInfo.energymeter == 0)
    {
        if(dlt645_info[gun].out_cur < 253.0)
        {
            pstChTcb->stOverVolt.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }
    else
    {
        if(pstChTcb->stRn8209.usVolt < CH_VOLT_MAX)
        {
            pstChTcb->stOverVolt.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }


    if(CH_DEF_FLG_DIS == pstChTcb->stOverVolt.ucFlg)           /* 第一次发现过流 */
    {

        pstChTcb->stOverVolt.ucFlg   = CH_DEF_FLG_EN;          /* 置标志 */
        pstChTcb->stOverVolt.uiTick  = rt_tick_get();          /* 获取当前时间 */
        return 0;
    }

    if(((rt_tick_get() > pstChTcb->stOverVolt.uiTick) ? ((rt_tick_get() - pstChTcb->stOverVolt.uiTick)) : \
            ((0xFFFFFFFF - pstChTcb->stOverVolt.uiTick) + rt_tick_get())) < CH_TIME_15_SEC)
    {
        return 0;
    }

//    if(rt_tick_get() - pstChTcb->stOverVolt.uiTick < CH_TIME_15_SEC)
//    {
//       return 0;
//    }

    return 1;
}

/**
 * @brief 是否欠压
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_under_volt(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        pstChTcb->stUnderVolt.uiTick  = rt_tick_get();    /* 不停获取当前时间 */
        return 0;
    }

    if(DisSysConfigInfo.energymeter == 0)
    {
        if(dlt645_info[gun].out_vol >= 187.0)
        {   /* 没有欠压 */
            pstChTcb->stUnderVolt.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }
    else
    {
        if(pstChTcb->stRn8209.usVolt >= CH_VOLT_MIN)
        {   /* 没有欠压 */
            pstChTcb->stUnderVolt.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }

    if(CH_DEF_FLG_DIS == pstChTcb->stUnderVolt.ucFlg)
    {
        pstChTcb->stUnderVolt.ucFlg   = CH_DEF_FLG_EN;     /* 置标志 */
        pstChTcb->stUnderVolt.uiTick  = rt_tick_get();     /* 获取当前时间 */
        return 0;
    }

    if(((rt_tick_get() > pstChTcb->stUnderVolt.uiTick) ? ((rt_tick_get() - pstChTcb->stUnderVolt.uiTick)) : \
            ((0xFFFFFFFF - pstChTcb->stUnderVolt.uiTick) + rt_tick_get())) < CH_TIME_15_SEC)
    {
        return 0;   //===当前的时间减去第一次没有欠压的时间，例:第一次欠压后，你再启动第二次后，还会立马欠压，不会等到30s，因为时间无赋值。
    }


//    if(rt_tick_get() - pstChTcb->stUnderVolt.uiTick < CH_TIME_15_SEC)
//    {
//        return 0;
//    }

    return 1;
}



/**
 * @brief 是否持续9V状态
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_under_cp9v(CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        pstChTcb->stCP9Vstate.uiTick  = rt_tick_get();     /* 获取当前时间 */
        return 0;
    }

    if(CP_6V == pstChTcb->stGunStat.ucCpStat)
    {
        pstChTcb->stCP9Vstate.ucFlg = CH_DEF_FLG_DIS;
        return 0;
    }


    if(CH_DEF_FLG_DIS == pstChTcb->stCP9Vstate.ucFlg)
    {
        pstChTcb->stCP9Vstate.ucFlg   = CH_DEF_FLG_EN;     /* 置标志 */
        pstChTcb->stCP9Vstate.uiTick  = rt_tick_get();     /* 获取当前时间 */
        return 0;
    }


    if(((rt_tick_get() > pstChTcb->stCP9Vstate.uiTick) ? ((rt_tick_get() - pstChTcb->stCP9Vstate.uiTick)) : \
            ((0xFFFFFFFF - pstChTcb->stCP9Vstate.uiTick) + rt_tick_get())) < (CH_TIME_60_SEC * 30))
    {
        return 0;    //9V状态  持续30分钟，返回
    }

    return 1;
}













/**
 * @brief 充电待机处理
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_standy_proc(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(ch_is_fault(&pstChTcb->stIOFault) || ch_is_over_current(gun,pstChTcb) || ch_is_over_volt(gun,pstChTcb) || ch_is_under_volt(gun,pstChTcb) || electricity_err(gun,pstChTcb))
    {
        ch_jump_new_stat(pstChTcb,CHARGER_FAULT);
        return 1;
    }

    if(CP_6V == pstChTcb->stGunStat.ucCpStat || CP_9V == pstChTcb->stGunStat.ucCpStat )
    {
        pstChTcb->ucRunState = PT_STATE_INSERT_GUN_NOCHARGING;
        ch_jump_new_stat(pstChTcb,INSERT);
        return 1;
    }

    return 0;
}

/**
 * @brief 是否开始充电
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_start(CH_TASK_T *pstChTcb)
{
//按照24小时制来判断启动条件 【1】 模式5  【2】当前小时 = 预约小时  特别注意：获取时间不能这样写，这样插枪后不停的来获取，会影响记录存入的时间
////    pstChTcb->stCh.uiChStartTime = gs_SysTime;
//    if((pstChTcb->stChCtl.ucChMode == 5)&(pstChTcb->stCh.uiChStartTime.ucHour == pstChTcb->stChCtl.uiStopParam))
//    {
//        DispControl.CurSysState = DIP_STATE_NORMAL;  //预约时间到后，页面自动跳转到当前的状态（省略了一个预充电的枚举）
//        return 1;
//    }

    //用下面的方式来书写
    if((CH_START_CTL == pstChTcb->stChCtl.ucCtl)&(pstChTcb->stChCtl.ucChMode == 5))  //只有刷卡后下发启动命令和启动预约才可以启动
    {
        pstChTcb->stCh.uiChStartTime = gs_SysTime; //获取当前的时间
        if(pstChTcb->stCh.uiChStartTime.ucHour == pstChTcb->stChCtl.uiStopParam)
        {
            DispControl.CurSysState = DIP_STATE_NORMAL;  //预约时间到后，页面自动跳转到当前的状态（省略了一个预充电的枚举）
            return 1;
        }
        return 0;
    }

    if((CH_START_CTL == pstChTcb->stChCtl.ucCtl)&(pstChTcb->stChCtl.ucChMode != 5))  /* 其他的模式下发后，直接开始*/
    {
        return 1;
    }

    return 0;
}

/**
 * @brief 开始充电函数
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
void ch_start_func(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    uint8_t    ucState = 0;
    uint32_t   uiE = 0;

    memset(&pstChTcb->stCh,0,sizeof(CH_T));    //第一次开始启动时，电量、消费金额开始和结束时间初始值清空0
    pstChTcb->stLowCurr.uiTick  = rt_tick_get();
    if(DisSysConfigInfo.energymeter == 0)
    {
        dlt645_info[gun].out_vol = 0;
        dlt645_info[gun].out_cur = 0;   //上电后，先清空一下外部电表的初始值，前5s页面显示是0, 如果不清，会显示上一笔订单的最后显示
        //为什么645读出的电量不清零，主要是5s后读不出电量，然后又是在0.5度内就会有问题。如果0.5外就没有问题
        //memset(&dlt645_info[gun],0,sizeof(dlt645_info[gun]));
        uiE  = dlt645_info[gun].cur_hwh * 1000;  //小数点3位
    }
    else
    {
        uiE  = pstChTcb->stRn8209.uiE;
    }

    pstChTcb->stCh.uiDChQ   = uiE;   //当前的电表读数（外部电表先赋值一下，等充电后5s后再赋值一下，才可以更新数据）

    pstChTcb->stCh.uiChStartTime = gs_SysTime;
    pstChTcb->stCh.timestart = time(RT_NULL);

    //===小鹤独享双枪7kw
#if(show_XY_name == show_XH)
    if((stChTcb[GUN_A].ucState == CHARGING) || (stChTcb[GUN_B].ucState == CHARGING))
    {
        cp1_pwm_ch_half();
        cp2_pwm_ch_half();
    }
    else
    {
        if(gun == GUN_A)
        {
            cp1_pwm_ch_puls();
        }
        else
        {
            cp2_pwm_ch_puls();
        }
    }
#else
    if(gun == GUN_A)
    {
        cp1_pwm_ch_puls();
    }
    else
    {
        cp2_pwm_ch_puls();
    }
#endif


    if(CP_6V == pstChTcb->stGunStat.ucCpStat)
    {
        ucState = PRECHARGE;
        ch_ctl_enable(gun);
    }
    else
    {
        ucState = WAIT_CAR_READY;
    }

    ch_jump_new_stat(pstChTcb,ucState);
}

/**
 * @brief 插枪处理
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_insert_proc(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(ch_is_fault(&pstChTcb->stIOFault) || ch_is_over_current(gun,pstChTcb) || ch_is_over_volt(gun,pstChTcb) || ch_is_under_volt(gun,pstChTcb) || electricity_err(gun,pstChTcb))
    {
        ch_jump_new_stat(pstChTcb,CHARGER_FAULT);
        return 1;
    }

    if(0 == ch_is_insert(&pstChTcb->stGunStat))         //连续5次检测都未为插枪，才认为为为插枪
    {
        pstChTcb->ucRunState = PT_STATE_IDLE;              /* 插枪未充电 */
        ch_jump_new_stat(pstChTcb,STANDBY);
//        mode5();  //等于预约模式时，拔枪后自动写入记录
        //stChTcb.ucState = CH_TO_DIP_STOP;
        DispControl.CurSysState = DIP_STATE_NORMAL;  //只要检测到拔枪  就会清空一下状态
        memset(&pstChTcb->stChCtl,0,sizeof(pstChTcb->stChCtl));  //拔枪后，启停结构体清零
        return 1;
    }

    if(ch_is_start(pstChTcb))                               /* 开始充电 */
    {
        ch_start_func(gun,pstChTcb);
        //网络状态下增加一下条数
        if((DisSysConfigInfo.standaloneornet == DISP_NET) && Outage_flag[gun] == 0)   //0=没有复位，条数才可加1
        {
            StoreRecodeCurNum();  //网络时，记录总条数+1;
            AORBRecodeNUM.RecordeNUM[gun] = RECODECONTROL.RecodeCurNum;
        }
        return 1;
    }

    return 0;
}





//=====充电过程中，无电量或者电量异常停止充电=======
uint8_t electricity_err(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    static uint32_t  ChargeEnergybuf[2]= {0},lasttime[2]= {0},elect_err_flag[2]= {0};      //elect_err_flag=无电量标志位
    if(CHARGING != pstChTcb->ucState)
    {
        ChargeEnergybuf[gun] = 0;   //插枪、空闲时  电量buf清零
        elect_err_flag[gun] = 0;  //插枪、空闲时  记录次数清零
        lasttime[gun] = rt_tick_get();
        return 0;
    }

    if((dlt645_info[gun].out_cur > 1.0) || (pstChTcb->stRn8209.usCurrent > 100))  //内外电表大于1A时
    {
        if((rt_tick_get() >= lasttime[gun] ? (rt_tick_get() - lasttime[gun]):((0xFFFFFFFF - lasttime[gun]) + rt_tick_get())) >= (CH_TIME_15_SEC))
        {
            if(pstChTcb->stCh.uiChargeEnergy > ChargeEnergybuf[gun])  //每个枪的电量 > 上一次的电量
            {
                ChargeEnergybuf[gun] = pstChTcb->stCh.uiChargeEnergy;
                lasttime[gun] = rt_tick_get();
                elect_err_flag[gun] = 0;
                return 0;
            }
            else
            {
                lasttime[gun] = rt_tick_get();
                ++elect_err_flag[gun];
            }
        }
    }

    if(elect_err_flag[gun] > 12)    //3分钟电量不动就是异常停止
    {
        elect_err_flag[gun] = 0;
        return 1;
    }
    return 0;
}




/**
 * @brief 6V是否持续低电流
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_low_current(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        return 0;
    }

    if(DisSysConfigInfo.energymeter == 0)
    {
        if(dlt645_info[gun].out_cur >= 1.0)
        {
            pstChTcb->stLowCurr.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }
    else
    {
        if(pstChTcb->stRn8209.usCurrent >= CH_CURR_THRESHOLD) 	     /* 电流没有小于门槛电流 */
        {
            pstChTcb->stLowCurr.ucFlg = CH_DEF_FLG_DIS;
            return 0;
        }
    }

    if(CH_DEF_FLG_DIS == pstChTcb->stLowCurr.ucFlg)
    {
        pstChTcb->stLowCurr.ucFlg   = CH_DEF_FLG_EN;
        pstChTcb->stLowCurr.uiTick  = rt_tick_get();
        return 0;
    }


	if(((rt_tick_get() > pstChTcb->stLowCurr.uiTick) ? ((rt_tick_get() - pstChTcb->stLowCurr.uiTick)) : \
            ((0xFFFFFFFF - pstChTcb->stLowCurr.uiTick) + rt_tick_get())) < (CH_TIME_60_SEC * 30))
    {
        return 0;      /*无电流持续30分*/
    }
	
	
//    if(rt_tick_get() - pstChTcb->stLowCurr.uiTick <= (CH_TIME_60_SEC * 30))  
//    {
//        return 0;
//    }
    return 1;
}



/**
 * @brief 9V是否持续低电流
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */

uint8_t ch_is_low_current_9V(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(CHARGING != pstChTcb->ucState)
    {
        pstChTcb->stLowCurr.uiTick  = rt_tick_get();
        return 0;
    }

    if(DisSysConfigInfo.energymeter == 0)
    {
        if(dlt645_info[gun].out_cur >= 1.0)
        {
            pstChTcb->stLowCurr.ucFlg = CH_DEF_FLG_DIS;
            pstChTcb->stCP9Vstate.ucFlg   = CH_DEF_FLG_DIS;     /* 持续9V置标志0 */
            return 0;
        }
    }
    else
    {
        if(pstChTcb->stRn8209.usCurrent >= CH_CURR_THRESHOLD) 	     /* 电流没有小于门槛电流 */
        {
            pstChTcb->stLowCurr.ucFlg = CH_DEF_FLG_DIS;  //清空
            pstChTcb->stCP9Vstate.ucFlg   = CH_DEF_FLG_DIS;     /* 持续9V置标志0 */
            return 0;
        }
    }

    if(CH_DEF_FLG_DIS == pstChTcb->stLowCurr.ucFlg)
    {
        pstChTcb->stLowCurr.ucFlg   = CH_DEF_FLG_EN;
        pstChTcb->stLowCurr.uiTick  = rt_tick_get();

        return 0;
    }

    if(ch_is_under_cp9v(pstChTcb) == 0 )   //无电流持续2min+9v持续2min
    {
        return 0;
    }

    return 1;
}












/**
 * @brief
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
//uint8_t ch_is_end_condition(CH_TASK_T *pstChTcb)
//{
//    /*    uint32_t uiParam = 0;

//        switch(pstChTcb->stChCtl.ucChMode)
//    	{
//            case 0:
//    			uiParam = 0;
//    	        break;
//    		case 1:
//    			uiParam = pstChTcb->stCh.stChInfo.uiTotalMoney/1000;
//    			break;
//    		case 2:
//    			uiParam = pstChTcb->stCh.stChInfo.uiChTime;
//    			break;
//    		case 3:
//    			uiParam = pstChTcb->stCh.stChInfo.uiChTotalQ;
//    		    break;
//    		default:
//    			break;
//    	}

//        uiParam = uiParam;
//    	if(uiParam >= pstChTcb->stChCtl.uiStopParam)
//    	{
//            return 1;
//    	}
//    */
//    return 0;
//}

extern _m1_card_info m1_card_info;		 //M1卡相关信息

uint8_t money_not_enough(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    //==判断金额大于    单机/单机预约模式下    卡类型是收费卡
    if((AB_GUNcard_info[gun].balance < ((pstChTcb->stCh.uiAllEnergy / 100) + 100)) && (m1_card_info.CardType==C0card || m1_card_info.CardType==C1card) && (DisSysConfigInfo.standaloneornet == DISP_CARD || DisSysConfigInfo.standaloneornet == DISP_CARD_mode))
    {
        return 1;
    }
    return 0;
}



//充电中循环处理是否达到充电条件
//0：自动充满  1:按电量充电  2:按时间充电(单位1分钟)  3:按金额充电(单位0.01元)  4:自动充满
uint8_t ch_is_end_condition(CH_TASK_T *pstChTcb)
{
    uint32_t Ch_condition = 0, ch_systime = 0;
    switch(pstChTcb->stChCtl.ucChMode)
    {
    case 1:
        Ch_condition = pstChTcb->stCh.uiChargeEnergy/1000;  //1：按电量充电
        break;

    case 2:   //2:按时间充电(单位1分钟)
        Ch_condition = pstChTcb->stCh.timestart+(pstChTcb->stChCtl.uiStopParam*60);
        break;

    case 3:   //3：金额充值
        Ch_condition = pstChTcb->stCh.uiAllEnergy/10000;  //3:按金额充电
        break;

    default:
        break;  //其他的方式充电
    }

    ch_systime=time(RT_NULL);  //获取当前的时间
    if((pstChTcb->stChCtl.ucChMode==2)&&(ch_systime >= Ch_condition))  //2按时间来充，当前时间必须 >= 定时的时间停止
    {
        return 1;
    }

    if((pstChTcb->stChCtl.ucChMode==1)&&(Ch_condition >= pstChTcb->stChCtl.uiStopParam)) //1按电量充  当前的电量 >= 要充的电量停止
    {
        return 1;
    }

    if((pstChTcb->stChCtl.ucChMode==3)&&(Ch_condition >= pstChTcb->stChCtl.uiStopParam)) //3按金额充
    {
        return 1;
    }

    return 0;
}

/**
 * @brief 充电是否停止
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_is_stp(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(ch_is_fault(&pstChTcb->stIOFault) && pstChTcb->stIOFault.ucFrist > 0)
    {
        if(pstChTcb->stIOFault.ucFrist == INPUT_YX3_EM)
        {
            return EM_STOP; //急停
        }
        else
        {
            return STOP_OTHEN;
        }
    }


    /* 大致为1s中时间 */
    if(((PRECHARGE == pstChTcb->ucState )  ||  (CHARGING == pstChTcb->ucState ) || (WAIT_CAR_READY == pstChTcb->ucState) )/* 预充 */
            && (ch_is_cp_off(gun,pstChTcb)))
    {
        if(CP_9V == pstChTcb->stGunStat.ucCpStat)                           /* 9v 断开继电器,但不停止充电,见标准 */
        {
            //ch_ctl_disable(); //修改流程
            rt_kprintf("cp==9v,ch disable,cp volt:%d\r\n",pstChTcb->stGunStat.uiCpVolt) ;
        }
        else
        {
            return ((CP_12V == pstChTcb->stGunStat.ucCpStat) ? UNPLUG : CP_LINK_ABN);
        }
    }

    //当前YKC 且为网络状态  判断余额
#if(NET_YX_SELCT == XY_YKC)
    if(DisSysConfigInfo.standaloneornet == DISP_NET)
    {
        if(YKC_Balance_judgment(gun)==1)
        {
            return  E_END_APP_BALANC;
        }
    }
#endif

    if(CH_STOP_CTL == pstChTcb->stChCtl.ucCtl)
    {
        if((_4G_GetStartType(gun) ==  _4G_APP_START )&&(DisSysConfigInfo.standaloneornet == DISP_NET))
        {
            return E_END_BY_APP;   //4G  APP主动停止
        }
        return E_END_BY_CARD;   //刷卡主动停止
    }


    if(electricity_err(gun,pstChTcb) == 1)
    {
        return R_POWER_DOWN;     /* 电表异常-无电能 */
    }





//    if(ch_is_low_current(gun,pstChTcb))  //6V 持续60s*5无电流，自动停止充电
//    {
//        return NO_CURRENT;
//    }


    if(ch_is_low_current_9V(gun,pstChTcb) == 1 )   //无电流持续2min+9v持续2min   无电流9V停止
    {
        return NO_CURRENT_9V;
    }


    if(money_not_enough(gun,pstChTcb))  /*单机/单机预约 收费卡   判断单机余额不足*/
    {
        return E_END_APP_BALANC;
    }
    if(ch_is_end_condition(pstChTcb))
    {
        return END_CONDITION;
    }

    if(ch_is_over_current(gun,pstChTcb))
    {
        return  OVER_CURRENT;   /* 过流 */
    }

    if(ch_is_over_volt(gun,pstChTcb))
    {
        return OVER_VOLT;      /* 过压 */
    }

    if(ch_is_under_volt(gun,pstChTcb))
    {
        return UNDER_VOLT;    /* 欠压 */
    }

    return 0;
}

time_t    tTime = 0;
/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint32_t get_q_money(_GUN_NUM gun,uint32_t uiQ,uint32_t *puiaE,uint32_t *pfwuiaE,uint32_t *pfsuiaE,uint32_t *pfsfwuiaE)
{
    uint32_t  uiSec = 0;
    uint32_t  uiFeeIndex = 0;
    uint32_t  uiFee = 0,uifwFee = 0;
    uint32_t  uiTmp = 0,uifwTmp;
    uint8_t   i = 0,num = 0;
    RATE_T * pstChRate;

    if(gun >= GUN_MAX)
    {
        return 0;
    }
    tTime  = time(RT_NULL);    /* 前面的time(RT_NULL)获取的时间就为当前屏幕上面的时间 */
    tTime += BEIJI_TIMESTAMP_OFFSET;        /*获取当前的时间戳是从1970-1-1  8.0.0开始计算的，所以下面取当天对应的时间段时，要先加8个小时的时间戳 */
    uiSec  = tTime % CH_DAT_SEC;   /*获取当天的从0点的时间戳*/
    uiFeeIndex = uiSec / CH_HALF_HOUR_SEC;  /* 获取当前对应的时间段 */

    if(DisSysConfigInfo.standaloneornet == DISP_NET)   //网络版本
    {
#if(NET_YX_SELCT == XY_YKC)
        pstChRate = &stChRateA;
#else
        pstChRate = &stChRate;
#endif
    }
    else
    {
        pstChRate = &stChRate;
    }


    if(uiFeeIndex < 48)
    {
        switch(pstChRate->ucSegNum[uiFeeIndex])
        {
        case 0:                                               /* 尖 */
            uiFee = pstChRate->Prices[0] + pstChRate->fwPrices[0];
            uifwFee = pstChRate->fwPrices[0];
            stChTcb[gun].stCh.jfpg_kwh[0] += uiQ;
            break;
        case 1:                                               /* 峰 */
            uiFee = pstChRate->Prices[1] + pstChRate->fwPrices[1];
            uifwFee = pstChRate->fwPrices[1];
            stChTcb[gun].stCh.jfpg_kwh[1] += uiQ;
            break;
        case 2:                                               /* 平 */
            uiFee = pstChRate->Prices[2] + pstChRate->fwPrices[2];
            uifwFee = pstChRate->fwPrices[2];
            stChTcb[gun].stCh.jfpg_kwh[2] += uiQ;
            break;
        case 3:                                               /* 谷 */
            uiFee = pstChRate->Prices[3] + pstChRate->fwPrices[3];
            uifwFee = pstChRate->fwPrices[3];
            stChTcb[gun].stCh.jfpg_kwh[3] += uiQ;
            break;
        default:
            rt_kprintf("fee seg fault\r\n");
            return 0;
        }
    }
    else
    {
        rt_kprintf("get fee index fault\r\n");
        return 0;
    }

    uiTmp = uiQ * uiFee;                                 /* 电量3位小数 费率5位费率 */
    uiTmp = uiTmp / 10000;                               /* 总8位小数,费率需要保存4位小数 */

    if((uiTmp % 10000) >= 5000)                           /* 满5进1 */
    {
        uiTmp += 1;
    }


    uifwTmp = uiQ * uifwFee;                                 /* 电量3位小数 费率5位费率 */
    uifwTmp = uifwTmp / 10000;                               /* 总8位小数,费率需要保存4位小数 */

    if((uifwTmp % 10000) >= 5000)                           /* 满5进1 */
    {
        uifwTmp += 1;
    }

    puiaE[pstChRate->ucSegNum[uiFeeIndex]] += uiTmp;
    pfwuiaE[pstChRate->ucSegNum[uiFeeIndex]] += uifwTmp;


    //分时计算
    for(i=0; i<uiFeeIndex; i++)
    {
        if(uiFeeIndex < 47)
        {
            if(pstChRate->ucSegNum[i]!=pstChRate->ucSegNum[i+1])
            {
                num++;
            }
        }
    }

    pfsuiaE[num] += uiTmp;
    pfsfwuiaE[num] += uifwTmp;
    return 1;
}

/**
 * @brief 充电信息更新总电量
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
void ch_info_update(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    uint32_t  uiE     = 0, uiDE = 0;
    uint32_t  uiTime  = 0;

    if(gun >= GUN_MAX)
    {
        return;
    }

    if(DisSysConfigInfo.energymeter == 0)
    {
        uiE  = dlt645_info[gun].cur_hwh * 1000;      /* 小数点三位 */
        uiDE = uiE - pstChTcb->stCh.uiDChQ;
    }
    else
    {
        uiE  = pstChTcb->stRn8209.uiE;
        uiDE = uiE >= pstChTcb->stCh.uiDChQ ? uiE - pstChTcb->stCh.uiDChQ
               : RN8209D_MAX_E - pstChTcb->stCh.uiDChQ + uiE;
    }


    if(uiDE > 500)   //一次性操作了0.5度  说明有问题
    {
        pstChTcb->stCh.uiDChQ   = uiE; 	  //记录上一次读取的电量（外部电表第一次读电量）
        return;
    }

    //如果断电续充时拷贝上次的信息
    if(Outage_flag[gun])  //哪把枪
    {
        memcpy((void *)&pstChTcb->stCh,(void *)&DChargeInfo[gun],sizeof(DChargeInfo[gun]));   //断电接着上一个订单充电，拷贝充电信息
        pstChTcb->stCh.uiChargeEnergy += uiDE; //赋值完成后，再加一下当前的小电量
        Outage_flag[gun] = 0;  //赋值完成后清零一下
    }

    if(0 < uiDE)
    {
        pstChTcb->stCh.uiChargeEnergy += uiDE;

        get_q_money(gun,uiDE,pstChTcb->stCh.uiaChargeEnergy,pstChTcb->stCh.uifwChargeEnergy,pstChTcb->stCh.uifsChargeEnergy,pstChTcb->stCh.uifsfwChargeEnergy);
        pstChTcb->stCh.uiDChQ          = uiE; 	  //记录上一次读取的电量
        pstChTcb->stCh.uiAllEnergy = pstChTcb->stCh.uiaChargeEnergy[0] + pstChTcb->stCh.uiaChargeEnergy[1] \
                                     + pstChTcb->stCh.uiaChargeEnergy[2] + pstChTcb->stCh.uiaChargeEnergy[3];
    }
}

/**
 * @brief 充电完成函数
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
void ch_cplt_func(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    uint8_t ucState    = 0;
    uint8_t ucGunState = 0;

    if(ch_is_fault(&pstChTcb->stIOFault) || ch_is_over_current(gun,pstChTcb) || ch_is_over_volt(gun,pstChTcb) || ch_is_under_volt(gun,pstChTcb) || electricity_err(gun,pstChTcb))
    {
        ucState = CHARGER_FAULT;
    }
    else
    {
        if(ch_is_insert(&pstChTcb->stGunStat))
        {
            ucState = INSERT;
            pstChTcb->ucRunState = PT_STATE_CHARGING_STOP_NOT_GUN;
        }
        else
        {
            ucState = STANDBY;
            pstChTcb->ucRunState = PT_STATE_IDLE;
        }
    }
    ch_jump_new_stat(pstChTcb,ucState);
    memset(&pstChTcb->stChCtl,0,sizeof(CH_CTL_T));
}

/**
 * @brief 充电停止函数
 * @param[in] pstChTcb 指向充电任务的数据
 *            ucReason 停止原因
 * @param[out]
 * @return
 * @note
 */
void ch_stop_func(_GUN_NUM gun,CH_TASK_T *pstChTcb,uint8_t ucReason)
{
    //==小鹤单独=A枪停止时，A枪PWM 100%
    if(show_XY_name == show_XH )
    {
        if(gun==GUN_A)
        {
            cp_pwm_full();
            if(stChTcb[GUN_B].ucState == CHARGING)
            {
                cp2_pwm_ch_puls();
            }
        }

        //==小鹤单独===B枪停止时，A枪PWM 100%
        if(gun==GUN_B)
        {
            cp_pwm9_full();
            if(stChTcb[GUN_A].ucState == CHARGING)
            {
                cp1_pwm_ch_puls();
            }
        }
    }
    else
    {
        if(gun==GUN_A)
        {
            cp_pwm_full();
        }
        else
        {
            cp_pwm9_full();
        }
    }
    //rt_thread_mdelay(CM_TIME_1_SEC);
    ch_ctl_disable(gun);

    if(ch_is_insert(&pstChTcb->stGunStat))
    {
        pstChTcb->ucRunState = PT_STATE_CHARGING_STOP_NOT_GUN;
        pstChTcb->stChCtl.ucCtl = CH_STOP_CTL;
    }
    else
    {
        pstChTcb->ucRunState = PT_STATE_IDLE;
        pstChTcb->stChCtl.ucCtl = CH_STOP_CTL;
    }
    ch_jump_new_stat(pstChTcb, WAIT_STOP);
}

/**
 * @brief 等待车就绪
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_wait_car_ready(_GUN_NUM gun ,CH_TASK_T *pstChTcb)
{
    uint8_t ucReason = 0;

    do
    {
        ucReason = ch_is_stp(gun,pstChTcb);

        /* 充电停止 */
        if(0 < ucReason)
        {
            break;
        }
        /* 一分钟车还未就绪 */
        if(rt_tick_get() - pstChTcb->uiJumpTick > CH_TIME_60_SEC)
        {
            ucReason = READY_TIMEOUT;
            break;
        }

        /* 车就绪 */
        if(CP_6V == pstChTcb->stGunStat.ucCpStat)
        {
            pstChTcb->stGunStat.ucCount_6v = 0;
            ch_ctl_enable(gun);
            ch_jump_new_stat(pstChTcb,PRECHARGE);
        }
        return 0;
    } while(0);

    mq_service_ch_send_dip(CH_TO_DIP_STARTFAIL ,ucReason ,gun,NULL);
    ch_stop_func(gun,pstChTcb,ucReason);

    return ucReason;
}

/**
 * @brief 预充电
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
void ch_precharge(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(gun >= GUN_MAX)
    {
        return;
    }
    pstChTcb->ucRunState = PT_STATE_CHARGING;
    //ch_info_update(gun,pstChTcb);    //测试一下，不刷新

    ch_jump_new_stat(pstChTcb,CHARGING);
    mq_service_ch_send_dip(CH_TO_DIP_STARTSUCCESS ,0 ,gun,NULL);  //启动成功
}

/**
 * @brief 充电
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_charging(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    uint8_t ucReason = 0;
    if(gun >= GUN_MAX)
    {
        return 0;
    }

    //10s后开启pwm波
    static  uint32_t lasttime[2] = {0};
    static  uint8_t timeflag[2] = {0}, flag_status[2] = {1,1}; //赋值时间标志位和执行PWM波标志位
    static  uint32_t  uiE = 0;

    if(DisSysConfigInfo.energymeter == 0)
    {
        if(timeflag[gun]==0)
        {
            lasttime[gun] = rt_tick_get();
            timeflag[gun]=1;
        }

        if(((rt_tick_get() >= lasttime[gun]) ? ((rt_tick_get() - lasttime[gun]) >= CM_TIME_5_SEC) : \
                ((rt_tick_get() + (0xffffffffU - lasttime[gun])) >= CM_TIME_5_SEC)) &&(flag_status[gun] ==1) )
        {
            //只有充电后10s后才会运行
            uiE  = dlt645_info[gun].cur_hwh * 1000;  //小数点3位
            pstChTcb->stCh.uiDChQ  = uiE;   //10s后，读出电表电量后，赋予当前的值，然后下面再进行电量累计
            flag_status[gun] = 0;
        }

        //必须在读出初始值才可以更新信息
        if(flag_status[gun] == 0)
        {
            ch_info_update(gun,pstChTcb);
        }
    }
    else if(DisSysConfigInfo.energymeter == 1)
    {
        ch_info_update(gun,pstChTcb);       //内部电表时，直接更新
    }


    ucReason = ch_is_stp(gun,pstChTcb);

    if((0 < ucReason) && (stChTcb[gun].ucState == CHARGING))
    {
        flag_status[gun] =1;
        timeflag[gun] = 0;

        if(ucReason==EM_STOP)
        {
            ch_stop_func(gun,pstChTcb,ucReason);
            mq_service_ch_send_dip(CH_TO_DIP_STOP ,ucReason,gun,NULL);   //急停时，A和B都会发送停止
            return ucReason;
        }
        else
        {
            ch_stop_func(gun,pstChTcb,ucReason);
            mq_service_ch_send_dip(CH_TO_DIP_STOP ,ucReason,gun,NULL);   //发送停止
            return ucReason;
        }
    }
    return 0;
}

/**
 * @brief 充电等待停止
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_wait_stop(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
//    do
//    {
//        if(CH_IS_POWERDOWN(pstChTcb->stFault.uiFaultFlg))
//        {
//            /* 掉电 */
//            break;
//        }
//
//        if(rt_tick_get() - pstChTcb->uiJumpTick > CH_TIME_30_SEC)
//        {
//            /* 进入停止状态 */
//            break;
//        }
//
//        if(ch_is_low_current(pstChTcb))
//        {
//            /* 确定无电流 */
//            break;
//        }
//
//        return 0;
//
//    } while(0);

    ch_cplt_func(gun,pstChTcb);
    return 1;
}
/**
 * @brief 故障处理
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
uint8_t ch_fault_proc(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    uint8_t ucState = 0;

    do
    {
        if((0 == pstChTcb->stIOFault.uiFaultFlg) && (ch_is_over_current(gun,pstChTcb) == 0) && (ch_is_over_volt(gun,pstChTcb) == 0) && (ch_is_under_volt(gun,pstChTcb) == 0))
        {
            pstChTcb->stIOFault.ucFrist = 0;
            if(ch_is_insert(&pstChTcb->stGunStat))
            {
                ucState = INSERT;
                pstChTcb->ucRunState = PT_STATE_INSERT_GUN_NOCHARGING;
            }
            else
            {
                ucState = STANDBY;
                pstChTcb->ucRunState = PT_STATE_IDLE;
            }
            ucState = STANDBY;
            break;
        }
        return 0;

    } while(0);

    ch_jump_new_stat(pstChTcb,ucState);

    return 1;
}

/**
 * @brief 充电状态机循环处理
 * @param[in] pstChTcb 指向充电任务的数据
 * @param[out]
 * @return
 * @note
 */
void ch_loop_proc(_GUN_NUM gun,CH_TASK_T *pstChTcb)
{
    if(gun >= GUN_MAX)
    {
        return;
    }
    switch(pstChTcb->ucState)
    {
    case STANDBY:
        ch_standy_proc(gun,pstChTcb);
        break;
    case INSERT:
        ch_insert_proc(gun,pstChTcb);
        break;
    case WAIT_CAR_READY:
        ch_wait_car_ready(gun,pstChTcb);
        break;
    case PRECHARGE:
        ch_precharge(gun,pstChTcb);
        break;
    case CHARGING:
        ch_charging(gun,pstChTcb);
        break;
    case WAIT_STOP:
        ch_wait_stop(gun,pstChTcb);  //充电等待停止
        break;
    case CHARGER_FAULT:
        ch_fault_proc(gun,pstChTcb);
        break;
    case POWER_DOWN:
        break;
    default:
        break;
    }
}

