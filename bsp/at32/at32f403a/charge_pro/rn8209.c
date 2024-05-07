/*******************************************************************************
 * @file rn8209.c
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description 计量芯片
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include <string.h>
#include "chtask.h"
#include "drv_gpio.h"
#include "rthw.h"


#include "rn8209d.h"

#if RN8209D_USE_SPI_TYPE == RN8209D_HW_SPI
#include <drv_spi.h>
#endif

/* drv_gpio.c PB2 */
/*csh22  PB2 */
#define    RN8209D_RST_PIN    18
/* drv_gpio.c PE8 */
#define    RN8209D_CS_PIN     72

#define    RN8209D_HARD_RST_EN()    rt_pin_write(RN8209D_RST_PIN, PIN_LOW)
#define    RN8209D_HARD_RST_DIS()   rt_pin_write(RN8209D_RST_PIN, PIN_HIGH)

#define    RN8209D_CS_EN()    rt_pin_write(RN8209D_CS_PIN, PIN_LOW)
#define    RN8209D_CS_DIS()   rt_pin_write(RN8209D_CS_PIN, PIN_HIGH)

#define    RN8209D_CURR_OFFSET     0xF56F

#if RN8209D_USE_SPI_TYPE == RN8209D_HW_SPI
struct rt_spi_device *pstRn8209dev;
#else
/* drv_gpio.c PE9 */
#define    RN8209D_SCK_PIN    73
/* drv_gpio.c PE7 */
#define    RN8209D_SDI_PIN    71
/* drv_gpio.c PE10 */
#define    RN8209D_SDO_PIN    74

#define    RN8209D_SCK_HIGH()   rt_pin_write(RN8209D_SCK_PIN, PIN_HIGH)
#define    RN8209D_SCK_LOW()    rt_pin_write(RN8209D_SCK_PIN, PIN_LOW)

#define    RN8209D_SDI_HIGH()   rt_pin_write(RN8209D_SDI_PIN, PIN_HIGH)
#define    RN8209D_SDI_LOW()    rt_pin_write(RN8209D_SDI_PIN, PIN_LOW)

#define    RN8209D_SDO_READ()   rt_pin_read(RN8209D_SDO_PIN)
#endif

/**
 * @brief rn8209 驱动初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_dev_init(void)
{
#if RN8209D_USE_SPI_TYPE == RN8209D_HW_SPI
    struct rt_spi_configuration stSpiCfg = {0};

    pstRn8209dev = (struct rt_spi_device *)rt_device_find("spi10");

    RT_ASSERT(pstRn8209dev != RT_NULL);

    stSpiCfg.data_width = 8;
    stSpiCfg.mode       = RT_SPI_MASTER | RT_SPI_MODE_1 | RT_SPI_MSB | RT_SPI_NO_CS;
    stSpiCfg.max_hz     = 400000U;
    rt_spi_configure(pstRn8209dev, &stSpiCfg);
#else
    rt_pin_mode(RN8209D_SCK_PIN,PIN_MODE_OUTPUT);
    rt_pin_mode(RN8209D_SDI_PIN,PIN_MODE_OUTPUT);
    rt_pin_mode(RN8209D_SDO_PIN,PIN_MODE_INPUT);
#endif

    rt_pin_mode(RN8209D_RST_PIN,PIN_MODE_OUTPUT);
    rt_pin_mode(RN8209D_CS_PIN,PIN_MODE_OUTPUT);
    RN8209D_CS_DIS();

    RN8209D_HARD_RST_DIS();
}

#if RN8209D_USE_SPI_TYPE == RN8209D_SF_SPI
/**
 * @brief rn8209 写一字节数据
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_write_char(uint8_t ucData)
{
    uint8_t ucWriteData = ucData;
    uint8_t i = 0;

    for(i = 0; i < 8; i++)
    {
        if(ucWriteData & 0x80)
        {
            RN8209D_SDI_HIGH();
        }
        else
        {
            RN8209D_SDI_LOW();
        }
        RN8209D_SCK_HIGH();
        rt_hw_us_delay(4);
        ucWriteData <<= 1;
        RN8209D_SCK_LOW();
        rt_hw_us_delay(4);

    }
}
/**
 * @brief rn8209 读一字节数据
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t rn8209d_read_char(void)
{
    uint8_t ucReadData = 0;
    uint8_t i = 0;

    for(i = 0; i < 8; i++)
    {
        RN8209D_SCK_HIGH();
        rt_hw_us_delay(4);
        ucReadData <<= 1;
        RN8209D_SCK_LOW();
        rt_hw_us_delay(2);
        if(RN8209D_SDO_READ())
        {
            ucReadData |= 0x01;
        }
        rt_hw_us_delay(2);
    }

    return 	ucReadData;
}
/**
 * @brief rn8209 发送和接收
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_send_then_recv(uint8_t *send_buf,rt_size_t  send_length,void *recv_buf,rt_size_t recv_length)
{
    rt_size_t     size = 0;
    uint8_t      *pRxBuf = RT_NULL;

    if(send_buf != RT_NULL && send_length > 0)
    {
        for(size = 0; size < send_length; size++)
        {
            rn8209d_write_char(send_buf[size]);
        }
    }

    if(recv_buf != RT_NULL && recv_length > 0)
    {
        pRxBuf = (uint8_t      *)recv_buf;
        for(size = 0; size < recv_length; size++)
        {
            pRxBuf[size] = rn8209d_read_char();
        }
    }

}
#endif

/**
 * @brief rn8209 读寄存器数据
 * @param[in] ucRegAddr: 寄存器地址
 *            pucBuf: 接收数据缓冲区
 *            ucLen: 数据长度
 * @param[out]
 * @return
 * @note
 */
void rn8209d_read_reg_nocheck(uint8_t ucRegAddr,uint8_t *pucBuf,uint8_t ucLen)
{
//    struct rt_spi_message stMsg1 = {0};
//	struct rt_spi_message stMsg2 = {0};

//    stMsg1.send_buf   = &ucRegAddr;
//    stMsg1.recv_buf   = RT_NULL;
//    stMsg1.length     = 1;
//    stMsg1.cs_take    = 1;
//    stMsg1.cs_release = 0;
//    stMsg1.next       = &stMsg2;

//    stMsg2.send_buf   = RT_NULL;
//    stMsg2.recv_buf   = pucBuf;
//    stMsg2.length     = ucLen;
//    stMsg2.cs_take    = 0;
//    stMsg2.cs_release = 1;
//    stMsg2.next       = RT_NULL;

//    rt_spi_transfer_message(pstRn8209dev, &stMsg1);
    RN8209D_CS_EN();
    //rt_spi_transfer_message(pstRn8209dev, &stMsg1);
#if RN8209D_USE_SPI_TYPE == RN8209D_HW_SPI
    rt_spi_send_then_recv(pstRn8209dev,&ucRegAddr,1,pucBuf,ucLen);
#else
    rn8209d_send_then_recv(&ucRegAddr,1,pucBuf,ucLen);
#endif
    RN8209D_CS_DIS();
}
/**
 * @brief rn8209 读寄存器数据
 * @param[in] ucRegAddr: 寄存器地址
 *            pucBuf: 接收数据缓冲区
 *            ucLen: 数据长度
 * @param[out]
 * @return
 * @note
 */
uint8_t rn8209d_read_reg(uint8_t ucRegAddr,uint8_t *pucBuf,uint8_t ucLen)
{
    uint8_t  ucRxBuf[4] = {0};
    uint8_t  *pstr = (uint8_t *)0;

    if(ucLen == 0 || ucLen > 4)
    {
        return 1;
    }

    rn8209d_read_reg_nocheck(ucRegAddr,pucBuf,ucLen);
    memset(ucRxBuf,1,4);
    rn8209d_read_reg_nocheck(RN8209D_RDATA_REG,ucRxBuf,4);

    pstr = ucRxBuf + (4 - ucLen);

    if(memcmp(pstr,pucBuf,ucLen) == 0)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief rn8209 写寄存器
 * @param[in] ucRegAddr: 寄存器地址
 *         	  pucBuf: 接收数据缓冲区
 *            ucLen: 数据长度
 * @param[out]
 * @return
 * @note
 */
uint8_t rn8209d_write_reg_nocheck(uint8_t ucRegAddr,uint8_t *pucBuf,uint8_t ucLen)
{
    uint8_t ucTxBuf[5] = {0};

    if(ucLen == 0 || ucLen > 4)
    {
        return 1;
    }

    /* 发送写数据命令,寄存器地址(bit.7=1) */
    ucTxBuf[0] = ucRegAddr | 0x80;
    memcpy(&ucTxBuf[1],pucBuf,ucLen);

//    stMsg1.send_buf   = ucTxBuf;
//    stMsg1.recv_buf   = RT_NULL;
//    stMsg1.length     = ucLen + 1;
//    stMsg1.cs_take    = 1;
//    stMsg1.cs_release = 1;
//    stMsg1.next       = RT_NULL;

//    rt_spi_transfer_message(pstRn8209dev, &stMsg1);
    RN8209D_CS_EN();
#if RN8209D_USE_SPI_TYPE == RN8209D_HW_SPI
    rt_spi_send(pstRn8209dev,ucTxBuf,ucLen + 1);
#else
    rn8209d_send_then_recv(ucTxBuf,ucLen + 1,RT_NULL,0);
#endif
    RN8209D_CS_DIS();
    return 0;
}

/**
 * @brief rn8209 写寄存器数据
 * @param[in] ucRegAddr: 寄存器地址
 *            pucBuf: 接收数据缓冲区
 *            ucLen: 数据长度
 * @param[out]
 * @return
 * @note
 */
uint8_t rn8209d_write_reg(uint8_t ucRegAddr,uint8_t *pucBuf,uint8_t ucLen)
{
    uint8_t ucRxBuf[4] = {0};

    if(ucLen == 0 || ucLen > 2)
    {
        return 1;
    }

    rn8209d_write_reg_nocheck(ucRegAddr,pucBuf,ucLen);

    rn8209d_read_reg_nocheck(RN8209D_WDATA_REG,ucRxBuf,2);

    if(memcmp(ucRxBuf,pucBuf,ucLen) == 0)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief rn8209 获取电压(电压, kb值还未加)
 * @param[in]
 * @param[out]
 * @return  返回的电压值 0.1V  KB 值还没有做
 * @note
 */

uint32_t rn8209d_get_volt(_GUN_NUM gun)
{
    uint8_t  ucBuf[3]    = {0};
    uint32_t uiRetValue  = 0;
    uint32_t uiRetVolt   = 0;
    uint32_t ulTmp       = 0;

	#warning "YXY"
		rn8209d_read_reg(RN8209D_URMS_REG,ucBuf,3);
		
//		/*csh  分别读出AB枪的电压值*/
//		if(gun==GUN_A) 
//		{
//		 rn8209d_read_reg(RN8209D_URMS_REG,ucBuf,3); 
//		}else
//		{
//		 rn8209d_read_reg(RN8209D_URMS_REG,ucBuf,3);
//		}
    

    uiRetValue = ((uint32_t)ucBuf[0] << 16) + ((uint32_t)ucBuf[1] << 8) + ((uint32_t)ucBuf[2]);

    /* 有效值最高位为0 值为0 */
    if(uiRetValue & RN8209D_RMS_MASK)
    {
        return 0;
    }

    ulTmp = uiRetValue * RN8209D_REF_VOLT / RN8209D_ADC_VALUE;
    /* 计算出的电压单位为mv */
    uiRetVolt  = ulTmp * RN8209D_VOLT_CURR_LIM_RES / (RN8209D_VOLT_RES * RN8209D_VOLT_VGA);
    /* 单位为0.1V */
    uiRetVolt /= 100;

    uiRetVolt = uiRetVolt * 97 / 100;
    return uiRetVolt;
}

/**
 * @brief rn8209 获取电流
 * @param[in]
 * @param[out]
 * @return 返回的电流值 0.01A  KB 值还没有做
 * @note
 */
uint32_t rn8209d_get_current(_GUN_NUM gun)
{
    uint8_t  ucBuf[3]    = {0};
    uint32_t uiRetValue  = 0;
    uint32_t uiRetCurr   = 0;
    uint64_t ullTmp      = 0;

	#warning "YXY"
    //rn8209d_read_reg(RN8209D_IBRMS_REG,ucBuf,3);
				
		/*csh  分别读出AB枪的电流值*/
		if(gun==GUN_A) 
		{
			rn8209d_read_reg(RN8209D_IARMS_REG,ucBuf,3);
		}else
		{
		  rn8209d_read_reg(RN8209D_IBRMS_REG,ucBuf,3);
		}
		

    uiRetValue = ((uint32_t)ucBuf[0] << 16) + ((uint32_t)ucBuf[1] << 8) + ((uint32_t)ucBuf[2]);
    /* 有效值最高位为0 值为0 */
    if(uiRetValue & RN8209D_RMS_MASK)
    {
        return 0;
    }

    /* uV 单位 */
    ullTmp = (uint64_t)uiRetValue * RN8209D_REF_VOLT * 1000 / RN8209D_ADC_VALUE;
    /* 单位 0.1ma */
    uiRetCurr  = ullTmp *  RN8209D_CURR_SENSOR_RATIO / (RN8209D_CURR_RES * RN8209D_CURR_VGA);
    /* 0.01A */
    uiRetCurr /= 100;
    uiRetCurr = uiRetCurr * 94 / 100;
    return uiRetCurr;
}

/**
 * @brief rn8209 获取总电量
 * @param[in]
 * @param[out]
 * @return 返回的总电量单位0.001kWh
 * @note
 */
uint32_t rn8209d_get_TotalE(_GUN_NUM gun,uint32_t *puiTotalE)
{
    uint8_t  ucBuf[3] = {0};
    uint32_t uiRetValue = 0;
    uint64_t ullE       = 0;

		// RN8209D_ENERGYP_REG   A枪电量的值
		// RN8209D_ENERGYD_REG  B枪电量的值
		
    /* 读电量寄存器成功 */
    if(((gun == GUN_B)&&(rn8209d_read_reg(RN8209D_ENERGYD_REG,ucBuf,3) == 0))||((gun == GUN_A)&&(rn8209d_read_reg(RN8209D_ENERGYP_REG,ucBuf,3) == 0)))
    {
        uiRetValue = ((uint32_t)ucBuf[0] << 16) + ((uint32_t)ucBuf[1] << 8) + ((uint32_t)ucBuf[2]);
        ullE       = (uint64_t)uiRetValue * 1000;
        /* 3位小数 */
        uiRetValue = ullE / RN8209D_PULSE_CONSTANT;
        /* 读寄存器成功更新 */
        *puiTotalE = uiRetValue;
        return 0;
    }
		
    /* 读失败不更新 */
    return 1;
}

/**
 * @brief rn8209 电流偏置寄存器校验
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_curr_offst_calibration(void)
{
    uint32_t    uiValue = 0;
    uint32_t    i = 0;
    uint32_t    uiSum = 0;
    uint8_t ucaBuf[4] = {0};

    rt_thread_mdelay(350);

    for(i = 0; i < 12; i++)
    {
        rn8209d_read_reg(RN8209D_IBRMS_REG,ucaBuf,3);
        uiValue = ((uint32_t)ucaBuf[0] << 16) + ((uint32_t)ucaBuf[1] << 8) + ((uint32_t)ucaBuf[2]);
        memset(ucaBuf,0,sizeof(ucaBuf));

        if(i > 0)
        {
            uiSum += uiValue;
        }

        rt_thread_mdelay(350);
    }

    uiSum /= 11;
    uiSum *= uiSum;
    uiSum  = ~uiSum;

    ucaBuf[0] = (uint8_t)(uiSum >> 16);
    ucaBuf[1] = (uint8_t)(uiSum >> 8);

    /* 写使能 */
    ucaBuf[2] = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,&ucaBuf[2],1);

    rn8209d_write_reg(RN8209D_IBRMSOS_REG,ucaBuf,2);
}
/**
 * @brief 写电流偏置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_write_curr_offset(void)
{
    uint8_t ucaBuf[2] = {0};
    uint8_t ucTmp = 0;

    ucaBuf[0] = (uint8_t)(RN8209D_CURR_OFFSET >> 16);
    ucaBuf[1] = (uint8_t)(RN8209D_CURR_OFFSET >> 8);

    /* 写使能 */
    ucTmp = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, &ucTmp, 1);

    rn8209d_write_reg(RN8209D_IBRMSOS_REG,ucaBuf,2);
}
/**
 * @brief 写有功功率校正寄存器，目前是人工算的只针对这个测试主板
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rn8209d_write_gpqb(void)
{
    uint8_t ucaBuf[2] = {0};
    uint8_t ucTmp = 0;

    ucaBuf[0] = 0xF7;
    ucaBuf[1] = 0xA0;

    ucTmp = RN8209D_WRITE_EN;
    /* 写使能 */
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,&ucTmp,1);
    rn8209d_write_reg(RN8209D_GPQB_REG,ucaBuf,2);
}
/**
 * @brief rn8209d 初始化
 * @param[in]
 * @param[out]
 * @return 0 : fail 1: success
 * @note
 */
uint8_t rn8209d_init(void)
{
    uint8_t ucBuf[4] = {0};
    uint8_t ucRBuf[4] = {0};
    uint8_t ucCnt = 0;
    uint8_t ucRet = 0;

    rt_thread_mdelay(100);

    rn8209d_dev_init();

    RN8209D_HARD_RST_EN();

    rt_thread_mdelay(100);

    RN8209D_HARD_RST_DIS();

    rt_thread_mdelay(100);

    /* 读8209ID */
    rn8209d_read_reg_nocheck(RN8209D_DEVICEID,ucRBuf,3);

    if((ucRBuf[0] != 0x82) || (ucRBuf[1] != 0x09) || (ucRBuf[2] != 0x00))
    {
        /* id 错误 */
        rt_kprintf("get rn8209d id failed \n");
        return 1;
    }

    ucBuf[0] = RN8209D_CMD_RESET;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    rt_thread_mdelay(100);

    /* 写使能 */
    ucBuf[0] = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    /* 指令复位 */
    ucBuf[0] = RN8209D_CMD_RESET;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    rt_thread_mdelay(10);

    /* 写使能 */
    ucBuf[0] = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    /* 系统控制寄存器 */
    /* 开启通道B, A 通道增益2, B 通道增益2 电压增益2 */
    ucBuf[0] = 0x00;
    ucBuf[1] = 0x55;
    rn8209d_write_reg(RN8209D_SYSCON_REG,ucBuf,2);

    memset(ucRBuf,0,4);
    rn8209d_read_reg(RN8209D_SYSCON_REG,ucRBuf,2);
    if(memcmp(ucRBuf,ucBuf,2) != 0)
    {
        ucRet = 1;
        goto _exit_rn8209_init;
    }

    /* 写使能 */
    ucBuf[0] = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    /* 写HFCONST,脉冲常数3200,220V,5A,电压引脚0.22V,电流0.0125V PGA都为2 */
    /* osci＝3.579545MHz时,HFConst的计算公式如下：
     * HFConst＝INT[16.1079*0.22*2*0.0125*2*10^11/(3200*220*5)]
     */
    ucBuf[0] = 0x13;
    ucBuf[1] = 0xA9;
    rn8209d_write_reg(RN8209D_HFCONST_REG,ucBuf,2);

    memset(ucRBuf,0,4);
    rn8209d_read_reg(RN8209D_HFCONST_REG,ucRBuf,2);
    if(memcmp(ucRBuf,ucBuf,2) != 0)
    {
        ucRet = 1;
        goto _exit_rn8209_init;
    }

    ucBuf[0] = RN8209D_WRITE_EN;
    /* 写使能 */
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG, ucBuf, 1);

    /* 启动功率 */
    ucBuf[0] = 0x01;
    ucBuf[1] = 0x2D;
    if(rn8209d_write_reg(RN8209D_PSTART_REG,ucBuf,2))
    {
        ucRet = 1;
        goto _exit_rn8209_init;
    }

    ucBuf[0] = 0x01;
    ucBuf[1] = 0x2D;
    rn8209d_write_reg(RN8209D_DSTART_REG,ucBuf,2);

    ucBuf[0] = RN8209D_WRITE_EN;
    /* 写使能 */
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,ucBuf,1);

    /* 计量控制 */
    /* 电能读后不清零 */
    ucBuf[0] = 0x00;
    ucBuf[1] = 0x03;
    if(rn8209d_write_reg(RN8209D_EMUCON_REG,ucBuf,2))
    {
        ucRet = 1;
        goto _exit_rn8209_init;
    }

    /* 写使能 */
    ucBuf[0] = RN8209D_WRITE_EN;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,ucBuf,1);

    /* 计量控制2 */
    ucBuf[0] = 0x00;
    ucBuf[1] = 0xB0;
    if(rn8209d_write_reg(RN8209D_EMUCON2_REG,ucBuf,2))
    {
        ucRet = 1;
        goto _exit_rn8209_init;
    }

    do
    {
        /* 指定计量的通道为B */
        ucBuf[0] = RN8209D_B_CH_EN;
			
        rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,ucBuf,1);
        rt_thread_mdelay(5);
			
			   /* 指定计量的通道为A */
        ucBuf[0] = RN8209D_A_CH_EN;
        rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,ucBuf,1);
			

        memset(ucBuf,0,4);
        rn8209d_read_reg(RN8209D_EMUSTATUS_REG,ucBuf,3);
        if((ucBuf[0] & 0x20))
        {
//            rn8209d_curr_offst_calibration();
//            rn8209d_write_curr_offset();
            rn8209d_write_gpqb();
            break;
        }
    } while(++ucCnt <= 5);

_exit_rn8209_init:
    /* 写禁止 */
    ucBuf[0] = RN8209D_WRITE_DIS;
    rn8209d_write_reg_nocheck(RN8209D_SPC_CMD_REG,ucBuf,1);
    ucRet = ucCnt > 5 ? 1 : 0;
    return ucRet;
}

