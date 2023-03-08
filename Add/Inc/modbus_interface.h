#ifndef MODBUS_INTERFACE_H
#define MODBUS_INTERFACE_H
#include "main.h"
#define bodeRate_veryHigh   115200
#define bodeRate_High       9600
#define bodeRate_Low        4800
/*------------------------------------------*/
#define Catgr_CO    'C'
#define Catgr_CH4   'H'
#define Catgr_WSp   'S'
#define Catgr_WDr   'D'
/*------------------------------------------*/
typedef struct 
{
    u8  CONum[4];
    u8  WindDir[4];
    u8  WindSp[4];
    u8  Temp[4];
    u8  Moi[4];
}UpBufferTemplate;
/*------------------------------------------*/
#define Updatelength    10
/*------------------------------------------*/
extern UpBufferTemplate UpBuffer[Updatelength];
extern int bufferDepth;  
/*------------------------------------------*/
void Read_SB(u8 catgr, int stage);
void Read_Wind(u8 catgr, int stage);
void Read_TemMoi(int stage);
int getSensorData(void);

#endif 
