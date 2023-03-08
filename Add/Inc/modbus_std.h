#ifndef MODBUS_STD_H
#define MODBUS_STD_H
#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------*/
#define TemMoi_Adr  0x01 
#define CH4_Adr     0x02
#define CO_Adr      0x03
#define windSp_Adr  0x04
#define windDr_Adr  0x05


//recvice buffer length
#define RecvSB_length1    9
#define RecvSB_length2    7
#define RecvWind_length   9
#define RecvTemMoi_length 9

const u16 u16CrcTalbeAbs[] =
{
0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 
0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 
0x5000, 0x9C01, 0x8801, 0x4400, };

u8 readCO_1[] =
{
CO_Adr, 0x03, 0x60, 0x01,
0x00, 0X02, 0x8A, 0x29,
};

u8 readCO_2[] = 
{
CO_Adr, 0x03, 0x20, 0x31, 
0x00, 0x01, 0xDF, 0xE7
};

u8 readCH4_1[] =
{
CH4_Adr, 0x03, 0x60, 0x01,
0x00, 0x02, 0x8B, 0xF8,
};

u8 readCH4_2[] = 
{
CH4_Adr, 0x03, 0x20, 0x31, 
0x00, 0x01, 0xDE, 0x36
};

u8 read_windSp[] =
{
windSp_Adr, 0x03, 0x00, 0x00,
0x00, 0x02, 0xC4, 0x5E
};

u8 read_windDr[] = 
{
windDr_Adr, 0x03, 0x00, 0x00,
0x00, 0x02, 0xC5, 0x8F
};

u8 read_TemMoi[] =
{
TemMoi_Adr, 0x03, 0x00, 0x00, 
0x00, 0x02, 0xC4, 0x0B    
};
u8 write_TMAdr[] =
{
0xFF, 0x06, 0x00, 0x64,
0x00, 0x05, 0x5C, 0x0A
};
struct WindLevelTable
{
    u16 windlevel;
    float windSpLow;
    float windSpHigh;
}WindSpTable[13] = 
{
    {0x00, 0.0,   0.2},
    {0x01, 0.3,   1.5},
    {0x02, 1.6,   3.3},
    {0x03, 3.4,   5.4},
    {0x04, 5.5,   7.9},
    {0x05, 8.0,  10.7},
    {0x06, 10.8, 13.8},
    {0x07, 13.9, 17.1},
    {0x08, 17.2, 20.7},
    {0x09, 20.8, 24.4},
    {0x10, 24.5, 28.4},
    {0x11, 28.5, 32.6},
    {0x12, 32.7, 36.9},        
};

char WindDrTable[][4] = 
{
    {'N'},
    {"NNE"},
    {"NE"},
    {"E"},
    {"ESE"},
    {"SE"},
    {"SSE"},
    {"S"},
    {"SSW"},
    {"SW"},
    {"WSW"},
    {"W"},
    {"WNW"},
    {"NW"},
    {"NNW"}
};
          
#ifdef __cplusplus
}
#endif

#endif 

