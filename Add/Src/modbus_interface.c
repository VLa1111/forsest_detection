#include "modbus_interface.h"
#include "modbus_std.h"
#include "usart.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
UpBufferTemplate UpBuffer[Updatelength];
int bufferDepth = 2;
u8 RecvSB_1[RecvSB_length1];
u8 RecvSB_2[RecvSB_length2];
u8 RecvWind[RecvWind_length];
u8 RecvTemMoi[RecvTemMoi_length];
u16 CRCRes,CRCSTD;
u16 Crc16(u8 *pchMsg, u8 wDataLen)
{
    u16 wCRC = 0xFFFF;
    u8 chChar;
    while (wDataLen--)
    {
        chChar = *pchMsg++;
        wCRC =   [(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
        wCRC = u16CrcTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
    }
    return wCRC;
}

// 波特率 : 9600
// 结果 : (float)NDNum
void Read_SB(u8 catgr, int stage)
{
    int RecvFlag = 0;
    u8 *tempCode1;
    u8 *tempCode2;
    int templength1 = 0;
    int templength2 = 0;
    if (catgr == Catgr_CO)
    {
        tempCode1 = readCO_1;
        tempCode2 = readCO_2;
        templength1 = sizeof(readCO_1);
        templength2 = sizeof(readCO_2);
    }
    else if (catgr == Catgr_CH4)
    {
        tempCode1 = readCH4_1;
        tempCode2 = readCH4_2;
        templength1 = sizeof(readCH4_1);
        templength2 = sizeof(readCH4_2);
    }
    while (!RecvFlag)
    {
        UART2Send(tempCode1, templength1);
        HAL_Delay(100);
        UART2Recv(RecvSB_1, RecvSB_length1);
        if (USART2_RX_LEN > 0)
        {
            u16 CRCRes = Crc16(RecvSB_1, RecvSB_length1 - 2);
            u16 CRCSTD = (RecvSB_1[RecvSB_length1 - 1] << 8) + RecvSB_1[RecvSB_length1 - 2];
            if (CRCRes == CRCSTD)
            {
                RecvFlag = 1;
            }
        }
    }
    u16 CO_ND = (RecvSB_1[3] << 8) + RecvSB_1[4];
    USART2_RX_LEN = 0;
    RecvFlag = 0;
    while (!RecvFlag)
    {
        UART2Send(tempCode2, templength2);
        HAL_Delay(100);
        UART2Recv(RecvSB_2, RecvSB_length2);
        if (USART2_RX_LEN > 0)
        {
            u16 CRCRes = Crc16(RecvSB_2, RecvSB_length2 - 2);
            u16 CRCSTD = (RecvSB_2[RecvSB_length2 - 1] << 8) + RecvSB_2[RecvSB_length2 - 2];
            if (CRCRes == CRCSTD)
            {
                RecvFlag = 1;
            }
        }
    }
    u16 pointPos = ((RecvSB_2[3] << 8) + RecvSB_2[4]);
    char tempNum = 0;
    char temPoint = 0;
    sprintf(&tempNum, "%d", CO_ND);
    sprintf(&temPoint, "%d", pointPos);
    int CONum = atoi(&tempNum);
    int Point = atoi(&temPoint);
    for (int i = 0; i < Point; i++)
    {
        CONum = CONum * 1.0 / 10;
    }
    memcpy(UpBuffer[stage].CONum, &CONum, 4);
}

// 波特率:4800
// 结果 : (char*)ResDir, (char*)ResAg, (float)NumSp
void Read_Wind(u8 catgr, int stage)
{
    u8 *tempCode;
    int RecvFlag = 0;
    int templength = 0;
    u16 windSp = 0;
    u16 windDr = 0;
    u16 windAg = 0;
    char *ResDir;
    int NumSp;
    if (catgr == Catgr_WSp)
    {
        tempCode = read_windSp;
        templength = sizeof(read_windSp);
    }
    else if (catgr == Catgr_WDr)
    {
        tempCode = read_windDr;
        templength = sizeof(read_windDr);
    }
    while (!RecvFlag)
    {
        UART2Send(tempCode, templength);
        //HAL_Delay(100);
			UART2Recv(RecvWind, RecvWind_length);
			HAL_Delay(1000);
        if (USART2_RX_LEN > 0)
        {
            u16 CRCRes = Crc16(RecvWind, RecvWind_length - 2);
            u16 CRCSTD = (RecvWind[RecvWind_length - 1] << 8) + RecvWind[RecvWind_length - 2];
            if (CRCRes == CRCSTD)
            {
                RecvFlag = 1;
            }
        }
    }
    if (catgr == Catgr_WDr)
    {
        windAg = (RecvWind[3] << 8) + RecvWind[4];
        windDr = (RecvWind[5] << 8) + RecvWind[6];
        for (int i = 0; i < 0x0F; i++)
        {
            if (windDr == i)
            {
                ResDir = WindDrTable[i];
            }
        }
        char ResAg;
        sprintf(&ResAg, "%d", windAg);
    }
    else if (catgr == Catgr_WSp)
    {
        windSp = (RecvWind[3] << 8) + RecvWind[4];
        char tempSp;
        sprintf(&tempSp, "%d", windSp);
        NumSp = atoi(&tempSp);

        for (int i = 0; i < 13; i++)
        {
            if (windSp == WindSpTable[i].windlevel)
            {
                while (!(WindSpTable[i].windSpLow <= NumSp && WindSpTable[i].windSpHigh >= NumSp))
                {
                    NumSp = NumSp * 1.0 / 10;
                }
            }
        }
    }
    memcpy(UpBuffer[stage].WindDir, ResDir, 4);
    memcpy(UpBuffer[stage].WindSp, &NumSp, 4);
}

// 波特率:9600
// 结果:(float)ResTem, (float)ResMoi
void Read_TemMoi(int stage)
{
    int RecvFlag = 0;
    float per = 0.1;
    char tempTem, tempMoi;
    while (!RecvFlag)
    {
			  UART2Send(read_TemMoi, sizeof(read_TemMoi));
				//HAL_Delay(1000);
        UART2Recv(RecvTemMoi, RecvTemMoi_length);
				HAL_Delay(1000);
        if (USART2_RX_LEN > 0)
        {
            CRCRes = Crc16(RecvTemMoi, RecvTemMoi_length - 2);
            CRCSTD = (RecvTemMoi[RecvTemMoi_length - 1] << 8) + RecvTemMoi[RecvTemMoi_length - 2];
            if (CRCRes == CRCSTD)
            {
                RecvFlag = 1;
            }
        }
    }
    u16 TemMoi_tem = (RecvTemMoi[3] << 8) + RecvTemMoi[4];
    u16 TemMoi_Moi = (RecvTemMoi[5] << 8) + RecvTemMoi[6];
    sprintf(&tempTem, "%d", TemMoi_tem);
    sprintf(&tempMoi, "%d", TemMoi_Moi);
    float ResTem = atoi(&tempTem) * per;
    float ResMoi = atoi(&tempMoi) * per;
    memcpy(UpBuffer[stage].Temp, &ResTem, 4);
    memcpy(UpBuffer[stage].Moi, &ResMoi, 4);
}

int getSensorData(void)
{
    int cnt = 0;
    bufferDepth = 3;
    int selectTunnel = CurrentModeIsDevice;
    if (Uart2Tunnel(bodeRate_High,selectTunnel))
    {
        HAL_Delay(100);
        for (int i = 0; i < bufferDepth; i++)
        {
            Read_SB(Catgr_CO,i);
            HAL_Delay(100);
            Read_SB(Catgr_CH4,i);
            // HAL_Delay(100);
//            Read_TemMoi(i);
//            HAL_Delay(100);
//            Uart2Tunnel(bodeRate_Low, selectTunnel);
//            HAL_Delay(100);
//            Read_Wind(Catgr_WDr,i);
//            HAL_Delay(100);
//            Read_Wind(Catgr_WSp,i);
            if (sizeof(UpBuffer[i]) == 20)
            {
                cnt++;
            }
        }
        if (cnt == bufferDepth)
        {
            return 1;
        }
    }
    return 0;
}
