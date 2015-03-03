/*
*********************************************************************************************************
*
*                                      GPS(nmea) info parser
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : nmea_parser.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <eem_defs.h>
#include "nmea_parser.h"

const float PI              = 3.1415926536;
const u32   EARTH_RADIUS    = 6371004;//地球半径


static u32 nmea_parseGPRMC(const char *buff, u32 buff_sz, nmeaGPRMC *info);

u32 nmea_parse(const char *buff, u32 buff_sz, nmeaGPRMC *info)
{
	/* check para */
	if ((NULL == buff) || (NULL == info) || (0 == buff_sz))
	{
		return UCORE_ERR_PAPA_ERROR;
	}

	/* check header and tail */
	if ('$' != buff[0])
	{
		return UCORE_ERR_PAPA_ERROR;
	}

	if (('\r' != buff[buff_sz - 2]) || ('\n' != buff[buff_sz - 1]))
	{
		return UCORE_ERR_PAPA_ERROR;
	}

	/* ok, start parse, check gps type first */
	if (0 == strncmp(buff, "$GPRMC", 6))
	{
		return nmea_parseGPRMC(buff + 7, buff_sz - 7, info);
	}

	return UCORE_ERR_COMMON_FAILED;
}

u32 nmea_parseGPRMC(const char *buff, u32 buff_sz, nmeaGPRMC *info)
{
	u16 i				= 0;
	u8	ucBlock			= 0;
	u8	ucBlockIndex	= 0;
	u8	ucTempA			= 0;

	/* ok, start parse */
	while (('*' != buff[i]) && (i < buff_sz))
	{
		switch(buff[i])
		{
		case '*':
			break; //finish parse
		case ',':
			ucBlock++;
			ucBlockIndex=0;
			break;
		default:
			switch(ucBlock)
			{
			case 0:         //<1> UTC时间 hhmmss.mmm
				switch(ucBlockIndex)
				{
				case 0:
				case 2:
				case 4:
					ucTempA = buff[i]-'0';
					break;
				case 1:
					info->utc.hour = ucTempA*10 + buff[i]-'0';
					break;
				case 3:
					info->utc.min = ucTempA*10 + buff[i]-'0';
					break;
				case 5:
					info->utc.sec = ucTempA*10 + buff[i]-'0';
					break;
				}
				break;
			case 1:         //<2> 定位状态 A=有效定位, V=无效定位
				info->status = buff[i];
				break;
			case 2:         //<3> 纬度 ddmm.mmmm
				info->lat[ucBlockIndex] = buff[i];
				break;
			case 3:         //<4> 纬度半球 N/S
				info->ns = buff[i];
				break;
			case 4:         //<5> 经度 dddmm.mmmm
				info->lon[ucBlockIndex] = buff[i];
				break;
			case 5:         //<6> 经度半球 E/W
				info->ew = buff[i];
				break;
			case 6:         //<7> 地面速率 000.0~999.9 节
				info->speed[ucBlockIndex] = buff[i];
				break;
			case 7:         //<8> 地面航向 000.0~359.9 度, 以真北为参考基准
				info->course[ucBlockIndex] = buff[i];
				break;
			case 8:         //<9> UTC日期 ddmmyy
				switch(ucBlockIndex)
				{
				case 0:
				case 2:
				case 4:
					ucTempA = buff[i]-'0';
					break;
				case 1:
					info->utc.year = ucTempA*10 + buff[i]-'0';
					break;
				case 3:
					info->utc.mon = ucTempA*10 + buff[i]-'0';
					break;
				case 5:
					info->utc.day = ucTempA*10 + buff[i]-'0';
					break;
				}
				break;
			}
			ucBlockIndex++;
		}
		i++;
	}

	return UCORE_ERR_SUCCESS;
}

float nmea_caldistance(float lat1, float lng1, float lat2, float lng2)
{
	float s = EARTH_RADIUS * acos(1 - (pow((sin((90-lat1)*PI/180) * cos(lng1*PI/180) - sin((90-lat2)*PI/180) * cos(lng2*PI/180)), 2) + 
                                       pow((sin((90-lat1)*PI/180) * sin(lng1*PI/180) - sin((90-lat2)*PI/180) * sin(lng2*PI/180)), 2) + 
                                       pow((cos((90-lat1)*PI/180) - cos((90-lat2)*PI/180)), 2)) / 2);
	return s;
}


#if defined(__cplusplus)
}
#endif
