/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.h
* Author			: 
* Date First Issued	:
* Version			: 
* Description		:                    
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#ifndef	__HYFRAME_H_
#define	__HYFRAME_H_
#include "common.h"
#include "4GMain.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   HY_RecvFrameDispose(uint8_t * pdata,uint16_t len);

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   HY_SendFrameDispose(void);


/*****************************************************************************
* Function     : HY_SendStartAck
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : PreHYBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   PreHYBill(_GUN_NUM gun,uint8_t *pdata);

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t HY_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendStopAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_GetHYStartType(uint8_t gun);

/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_SetHYStartType(uint8_t gun ,_4G_START_TYPE  type);

/*****************************************************************************
* Function     : HY_SendBillData
* Description  : 发送订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t HY_SendBillData(uint8_t * pdata,uint8_t len,uint8_t ifquery);

/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint8_t *  APP_GetHYBatchNum(uint8_t gun);

/*****************************************************************************
* Function     : HY_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendRateAck(uint8_t cmd);

/*****************************************************************************
* Function     : APP_GetHYNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint32_t APP_GetHYNetMoney(uint8_t gun);


/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩A状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendDevStateA(void);

/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩B状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendDevStateB(void);

/*****************************************************************************
* Function     : HY_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  HY_SendCardInfo(_GUN_NUM gun);
uint8_t  HY_SendUpdataAck(uint8_t num);
#endif
/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

