/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
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
#ifndef	__4G_MAIN_H_
#define	__4G_MAIN_H_
#include "stdint.h"
#include "common.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
#define  IO_4G_ONOFF    79
#define  IO_4G_RES  	78
#define  IO_4G_PWR  	77



#define NET_SENDRECV_PRINTF		0   //1需要打印4G发送接收数据


#define		BSP_4G_SENDNET1		30	//主动读取数据
#define		BSP_4G_SENDNET2		31	//主动读取数据


#define  	APP_START_CHARGE	3	//APP开始充电
#define  	APP_STOP_CHARGE		4	//APP停止充电
#define  	APP_START_VIN		5	//VIN鉴权成功


#define GPRS_IP2   	"114.55.186.206"
#define GPRS_PORT2   9800

//#define GPRS_IP1     "47.96.77.209"    //合肥乾古
//#define GPRS_PORT1   5001



//#define GPRS_IP1     "121.199.192.223"    //云快充
//#define GPRS_PORT1   8767

//#define GPRS_IP1     "101.37.179.163"    //云快充
//#define GPRS_PORT1   8767

//#define GPRS_IP1     "120.55.83.107"   //汇誉
//#define GPRS_PORT1   44444

//安培快充测试
//#define GPRS_IP1     "120.55.183.164"
//#define GPRS_PORT1   5738
//安培正式
#define GPRS_IP1     "114.55.186.206"
#define GPRS_PORT1   5738

//#define GPRS_IP1     "120.76.100.197"
//#define GPRS_PORT1   10002


//#define GPRS_IP1     "122.114.122.174"
//#define GPRS_PORT1   35403


#define LINK_NUM     0 //哪一路网络
#define LINK_NET_NUM 2 // 最多连接2个


#define	XY_HY  0
#define	XY_YKC 1
#define	XY_AP  2
#define	XY_YL1 3
#define	XY_YL2 4
#define	XY_YL3 5
#define	XY_YL4 6
#define	XY_YL5 7
#define	XY_MAX 8
#define NET_YX_SELCT XY_YKC

//======上面选择YKC时  选择显示YKC/小鹤/塑云（二维码、IP端口、协议显示）
#define show_YKC   0    //显示YKC
#define show_XH    1    //显示小鹤   ！！注意小鹤要求是双枪7KW，PWM有变化
#define show_KL    2    //KL
#define show_XY_name  show_KL


#define doguser    1   //0不用看门狗  1使用看门狗
#define FIRMWARE_VERSION	"240417"     /*******双枪软件版本号*********/


typedef struct
{
    uint8_t XYSelece;	//协议选择
    uint8_t IP[4];
    uint16_t port;			//端口
    char  pIp[16];				//IP
    uint8_t NetNum;			//网络个数
} _NET_CONFIG_INFO;			//网络配置信息

extern _NET_CONFIG_INFO  NetConfigInfo[XY_MAX];

#define GPRS_UART 	   		USART1

typedef enum
{
    _4G_APP_START = 0,		//app启动
    _4G_APP_CARD,			//卡启动
    _4G_APP_VIN,				//VIN码启动
    _4G_APP_MAX,
} _4G_START_TYPE;


//启动的时候网络是在线还是离线
typedef enum
{
    NET_STATE_ONLINE = 0,
    NET_STATE_OFFLINE,
    NET_STATE_MAX,
} _START_NET_STATE;

typedef enum {
    RECV_NOT_DATA = 0,		//没有接收到数据
    RECV_FOUND_DATA			//有数据
} _RECV_DATA_STATUS;

typedef struct
{
    uint32_t AllLen;         //总长度为HTTP专门使用
    uint32_t DownPackLen;		//下载当前包的长度

    uint8_t DataBuf[1500];	//接收数据缓存
    uint16_t len;			//接收数据长度
    _RECV_DATA_STATUS RecvStatus;	//接收状态
} _RECV_DATA_CONTROL;

typedef enum
{
    STATE_ERR = 0,
    STATE_OK,
} _4G_STATE;

typedef enum
{
    MODE_DATA = 0,			//数据模式
    MODE_HTTP,				//http模式
} _4G_MODE;

typedef enum
{
    _4G_RESPOND_ERROR = 0,                    //0:4G无回复或回复异常
    _4G_RESPOND_OK,                           //1:4G回复正常
    _4G_RESPOND_AGAIN,						//再次发送
} _4G_RESPOND_TYPE;

typedef struct
{
    char* ATCode;                              //AT指令
    uint8_t (*Fun)(uint8_t *pdata, uint16_t len);    //对应的处理函数
} _4G_AT_FRAME;

typedef struct {
    uint16_t cmd;
    uint8_t  (*recvfunction)(uint8_t *,uint16_t);
} _4G_RECV_TABLE;

typedef struct
{
    uint32_t 		curtime;
    uint32_t		lasttime;
    uint32_t    	cycletime;           // 超时时间      0表示不需要周期性发送
    uint8_t 		(*Sendfunc)(void);  //发送函数
} _4G_SEND_TABLE;					//周期性发送

typedef struct
{
    //在线交易记录
    uint8_t ResendBillState;  //0表示不需要重发   1表示需要重发
    uint32_t CurTime;			//当前发送时间发送时间
    uint32_t LastTime;		//上一次发送时间
    //离线交易记录
    //是否发送取决于交易记录个数
    uint8_t OffLineNum;			//离线交易记录个数 0
    uint32_t OFFLineCurTime;			//当前发送时间发送时间
    uint32_t OFFLineLastTime;		//上一次发送时间
    uint8_t SendCount;			//发送计数，最多发送三次
} _RESEND_BILL_CONTROL;

extern _RESEND_BILL_CONTROL ResendBillControl[GUN_MAX];


typedef struct
{
    char ServerAdd[128];
    char ServerPassword[50];
    uint8_t crc;
} _HTTP_INFO;
extern _HTTP_INFO HttpInfo;





/*****************************************************************************
* Function     : 4G_RecvFrameDispose
* Description  :4G接收
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t _4G_RecvFrameDispose(uint8_t * pdata,uint16_t len);

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   Pre4GBill(_GUN_NUM gun,uint8_t *pdata);

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendDevState(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : YKC_SendBalanceAck
* Description  : 发送更新余额应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   YKC_SendBalanceAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : _4G_SendStOPtAck
* Description  : 停止应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _4G_SendStopAck(_GUN_NUM gun);


/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t    _4G_SendQueryRate(void);


/*****************************************************************************
* Function     : _4G_SendRateMode
* Description  : 发送计费模型
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t    _4G_SendRateMode(void);

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  :4G发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t  _4G_SendFrameDispose();


/*****************************************************************************
* Function     : HFQG_SendStartAck
* Description  : 合肥乾古开始充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   _4G_SendStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     :Module_SIM7600Test
* Description  :模块是否存在
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_SIM7600Test(void);

/*****************************************************************************
* Function     :ModuleSIM7600_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_ConnectServer(uint8_t num,uint8_t* pIP,uint16_t port);

/*****************************************************************************
* Function     :SIM7600_RecvDesposeCmd
* Description  :命令模式下模块接收处理
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600_RecvDesposeCmd(uint8_t *pdata,uint32_t len);

/*****************************************************************************
* Function     : SIM7600Reset
* Description  : 模块复位
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600Reset();

/*****************************************************************************
* Function     : SIM7600CloseNet
* Description  : 关闭网络
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600CloseNet(uint8_t num);

/*****************************************************************************
* Function     : APP_GetSIM7600Status
* Description  : 获取模块是否存在
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetSIM7600Status();

/*****************************************************************************
* Function     : APP_GetModuleConnectState
* Description  : 连接服务器状态
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetModuleConnectState(uint8_t num);

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetAppRegisterState(uint8_t num);

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t  APP_SetAppRegisterState(uint8_t num,_4G_STATE state);

/*****************************************************************************
* Function     : APP_RecvDataControl
* Description  :
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日
*****************************************************************************/
_RECV_DATA_CONTROL	* APP_RecvDataControl(uint8_t num);

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 读取数据请求
* Input        :num  哪个socket
				len   数据长度
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CIPRXGET(uint8_t num);

/*****************************************************************************
* Function     : UART_4GWrite
* Description  :串口写入，因多个任务用到了串口写入，因此需要加互斥锁
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2020-11-26     叶喜雨
*****************************************************************************/
uint8_t UART_4GWrite(uint8_t* const FrameBuf, const uint16_t FrameLen);


/*****************************************************************************
* Function     :ModuleSIM7600_SendData
* Description  :发送数据
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_SendData(uint8_t num,uint8_t* pdata,uint16_t len);

/*****************************************************************************
* Function     :APP_SetNetNotConect
* Description  :设置网络未连接
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetNetNotConect(uint8_t num);


/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取网络状态
* Input        : 那一路
* Output       : TRUE:表示有网络	FALSE:表示无网络
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
uint8_t  APP_GetNetState(uint8_t num);

/*****************************************************************************
* Function     : Send_AT_CSQ
* Description  : 读取信号强度
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CSQ(void);

/*****************************************************************************
* Function     : APP_GetCSQNum
* Description  : 获取型号强度值
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_GetCSQNum(void);

/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint8_t *  APP_GetBatchNum(uint8_t gun);

/*****************************************************************************
* Function     : APP_GetNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint32_t APP_GetNetMoney(uint8_t gun);

/*****************************************************************************
* Function     : APP_GetResendBillState
* Description  : 获取是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t APP_GetResendBillState(uint8_t gun);

/*****************************************************************************
* Function     : APP_SetResendBillState
* Description  : 设置是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
void APP_SetResendBillState(uint8_t gun,uint8_t state);

/*****************************************************************************
* Function     : ReSendBill
* Description  : 重发订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t  ReSendBill(_GUN_NUM gun,uint8_t* pdata,uint8_t ifquery);

/*****************************************************************************
* Function     : _4G_SendRateAck
* Description  : 费率应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendRateAck(uint8_t cmd);


/*****************************************************************************
* Function     : _4G_SendSetTimeAck
* Description  : 校时应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendSetTimeAck(void);


/*****************************************************************************
* Function     : ZF_SendFrameDispose
* Description  : 周期性发送政府平台数据
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   ZF_SendFrameDispose(void);

/*****************************************************************************
* Function     : ZF_SendStartCharge
* Description  : 启动帧
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   ZF_SendStartCharge(void);


/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t ZF_SendBill(void);

/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   HY_SendQueryRateAck(void);


/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
uint8_t   YKC_SendSJDataGunACmd13(void);

/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
uint8_t   YKC_SendSJDataGunBCmd13(void);

/*****************************************************************************
* Function     : _4G_SendCardInfo
* Description  : 发送卡鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendCardInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : _4G_SendVinInfo
* Description  : 发送Vin鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendVinInfo(_GUN_NUM gun);
/*****************************************************************************
* Function     : _4G_SendCardVinCharge
* Description  : 发送卡充电
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendCardVinCharge(_GUN_NUM gun);


/*****************************************************************************
* Function     : AP_SendCardWLAck
* Description  : 发送卡白名单应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  AP_SendCardWLAck(void);

/*****************************************************************************
* Function     : AP_SendVinWLAck
* Description  : 发送VIN白名单应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  AP_SendVinWLAck(void);

/*****************************************************************************
* Function     : AP_SendVinCardResAck
* Description  : 清空白名单应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  AP_SendVinCardResAck(void);

/*****************************************************************************
* Function     : APP_GetStartNetState
* Description  : 获取启动方式网络状态
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_GetStartNetState(uint8_t gun);

/*****************************************************************************
* Function     : _4G_SetStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _4G_SetStartType(uint8_t gun ,_4G_START_TYPE  type);

/*****************************************************************************
* Function     : APP_SetStartNetState
* Description  : 设置启动方式网络状态
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_SetStartNetState(uint8_t gun ,_START_NET_STATE  type);

/*****************************************************************************
* Function     : PreAPOffLineBill
* Description  : 保存离线交易记录
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  PreAPOffLineBill(_GUN_NUM gun,uint8_t *pdata);

/*****************************************************************************
* Function     : PreAPFSOffLineBill
* Description  : 上传分时交易记录   充电完成后，在B12发送完成后上报。
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  PreAPFSOffLineBill(_GUN_NUM gun,uint8_t *pdata);

/*****************************************************************************
* Function     : ReSendOffLineBill
* Description  :
* Input        : 发送离线交易记录订单
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t  ReSendOffLineBill(void);

/*****************************************************************************
* Function     : _4G_GetStartType
* Description  : 获取快充启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
_4G_START_TYPE   _4G_GetStartType(uint8_t gun);
/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_MODE APP_GetSIM7600Mode(void);

/*****************************************************************************
* Function     :Module_HTTPDownload
* Description  :HTTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_HTTPDownload(_HTTP_INFO *info);

/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetSIM7600Mode(_4G_MODE mode);

uint8_t Send_AT_CIPMODE(void);
uint8_t  APP_YKCSENDFEILV(uint32_t timeout);
void  JumpToProgramCode(void);

/*****************************************************************************
* Function     : _HY_RestUpdataData
* Description  : 复位更新数据
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
#if NET_YX_SELCT == XY_HY
uint8_t   _HY_RestUpdataData(_GUN_NUM gun);
#endif

#endif
/************************(C)COPYRIGHT 2021 汇誉科技*****END OF FILE****************************/

