/*******************************************************************************
 *          Copyright (c) 2020-2050,  Co., Ltd.
 *                              All Right Reserved.
 * @file DispStatus.c
 * @note
 * @brief 提供系统状态信息,提供显示屏显示中文使用,此文本格式为ANSI
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
#include <string.h>
#include <stdlib.h>
#include "DispShowStatus.h"
#include "DispKeyFunction.h"
#include "MenuDisp .h"
#include "DwinProtocol.h"
#include "ch_port.h"


typedef struct
{
    _SHOW_NUM ShowNum;   //显示状态
    const char *pStr;    //地址
    uint16_t ShowLen;    //长度
} _SHOW_STATUS;

_SHOW_STATUS ShowStatus[SHOW_MAX] =
{
    {SHOW_XY_HY,			"汇誉"				,4 },
    {SHOW_XY_YKC,			"云快充"			,6 },
    {SHOW_XY_AP,			"安培"				,4 },
    {SHOW_XY_HFQG,		"合肥乾谷"		,8 },
    {SHOW_XY_XH,			"蓝锂"			,4 },
    {SHOW_XY_KL,			"库仑YKC-域"	,10},
    {SHOW_XY_YL3,			"预留3"				,5 },
    {SHOW_XY_YL4,			"预留4"				,5 },
    {SHOW_START_APP,  "APP启动",	7},

    {SHOW_START_CARD,  "刷卡启动",8},

    {SHOW_BILL,  			"已结算",		6},
    {SHOW_NOTBIL,  		"未结算",		6},

    {SHOW_STOP_ERR_NONE,	"正常停止",	8},
    {SHOW_STOP_FAIL,	  	"故障停止",	8},
    {SHOW_STOP_LESS,	  	"余额不足",	8},
    {SHOW_GUNA,  			"A枪",		3},
    {SHOW_GUNB,  		  "B枪",		3},
    {SHOW_GUN_IN,  			"已插枪",		6},
    {SHOW_GUN_NO,  		  "未插枪",		6},
    {SHOW_GUN_NOPAY,  		  "待结算",		6},
};

/**
 * @brief 显示时间
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
//uint8_t Dis_ShowTime(uint16_t add_show ,_BSPRTC_TIME gRTC)
//{
//	char * buf[30];
//	ST_Menu* CurMenu = GetCurMenu();
//	if(CurMenu == NULL)
//	{
//		return FALSE;
//	}
//	memset(buf,0,sizeof(buf));
//
//	snprintf((char *)buf, sizeof(buf), "20%02x-%02x-%02x %02x:%02x:%02x",(gRTC.Year),(gRTC.Month),(gRTC.Day),\
//		(gRTC.Hour),(gRTC.Minute),(gRTC.Second));


//	PrintStr(add_show,(uint8_t *)buf,30);   //显示状态
//	SetVariColor((add_show & 0xFFC0 ) + 0x30,WHITE);
//	return TRUE;
//}

/**
 * @brief 显示状态
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t Dis_ShowStatus(uint16_t add_show,_SHOW_NUM show_num )
{
    uint8_t i = 0;
    ST_Menu* CurMenu = GetCurMenu();
    if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
    {
        return 0;
    }

    for(i = 0; i < SHOW_MAX; i++)
    {
        if(ShowStatus[i].ShowNum == show_num)
        {
            PrintStr(add_show,(uint8_t *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态
            break;
        }
    }
    return 1;
}

/**
 * @brief 拷贝
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t Dis_ShowCopy(uint8_t* pdata,_SHOW_NUM show_num)
{

    uint8_t i = 0;
    ST_Menu* CurMenu = GetCurMenu();
    if((show_num >= SHOW_MAX) || (CurMenu == NULL))
    {
        return 0;
    }

    for(i = 0; i < SHOW_MAX; i++)
    {
        if(ShowStatus[i].ShowNum == show_num)
        {
            memcpy(pdata,(uint8_t *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);
            break;
        }
    }
    return 1;
}




typedef struct
{
    STOP_REASON Stop_reasonnum;  //显示状态
    const char *pStr;    //地址
    uint16_t ShowLen;    //长度
} _SHOW_Reason;

_SHOW_Reason Showstopreason[STOP_MAX] =
{
    {NORMAL,			 "正常停止"	 ,8 },
    {R_POWER_DOWN,	"无电量"	,6 },
    {LEAKAGE_FAULT,	 "漏电故障"	,8},
    {SPD_FAULT,		  "防雷故障"	,8 },
    {OTHER_FAULT,		"其他错误"	,8 },
    {DOOR_OPEN,			"门已打开"	,8},
    {EM_STOP,			"急停按下"		,8 },
    {OVER_VOLT,			"过压"		,4 },
    {OVER_CURRENT,  "过流",	4},
    {READY_TIMEOUT,  "就绪失败",8},
    {CP_LINK_ABN,  		"CP异常",	6},
    {UNPLUG,  		    "非法拔枪",		8},
    {END_CONDITION,	"达到条件",	8},
    {NO_CURRENT,	  "无电流6V",	8},
    {NO_CURRENT_9V,	  "无电流9V",	8},
    {E_END_BY_APP,	 "App结束",	7},
    {E_END_BY_CARD,  	"刷卡结束",		8},
    {UNDER_VOLT,  		 "欠压",		4},
    {STOP_CHARGEFULL,	 "充满",	4},
    {STOP_OTHEN,  	"其他",		4},
    {E_END_APP_BALANC,  "余额不足",8}
};



//=======显示停止全部原因================
uint8_t Dis_Showstop_reason(uint8_t* pdata,STOP_REASON Stop_reasonnum)
{

    uint8_t i = 0;
    ST_Menu* CurMenu = GetCurMenu();
    if((Stop_reasonnum >= STOP_MAX) || (CurMenu == NULL))
    {
        return 0;
    }

    for(i = 0; i < STOP_MAX; i++)
    {
        if(Showstopreason[i].Stop_reasonnum == Stop_reasonnum)
        {
            memcpy(pdata,(uint8_t *)Showstopreason[i].pStr,Showstopreason[i].ShowLen);
            break;
        }
    }
    return 1;
}





