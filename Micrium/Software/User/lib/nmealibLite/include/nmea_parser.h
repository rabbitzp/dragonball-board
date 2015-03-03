/*
*********************************************************************************************************
*
*                                      GPS(nmea) info parser
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : nmea_parser.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*--------------macros declare here-----------------------------*/
/* * Distance units */
#define NMEA_TUD_YARDS      (1.0936)        /**< Yeards, meter * NMEA_TUD_YARDS = yard */
#define NMEA_TUD_KNOTS      (1.852)         /**< Knots, kilometer / NMEA_TUD_KNOTS = knot */
#define NMEA_TUD_MILES      (1.609)         /**< Miles, kilometer / NMEA_TUD_MILES = mile */

/* * Speed units */
#define NMEA_TUS_MS         (3.6)           /**< Meters per seconds, (k/h) / NMEA_TUS_MS= (m/s) */


/*--------------enums declare here------------------------------*/

/*--------------struct declare here-----------------------------*/
/**
* Date and time data
* @see nmea_time_now
*/
typedef struct _nmeaTIME
{
	u16     year;       /**< Years since 1900 */
	u16     mon;        /**< Months since January - [0,11] */
	u16     day;        /**< Day of the month - [1,31] */
	u16     hour;       /**< Hours since midnight - [0,23] */
	u16     min;        /**< Minutes after the hour - [0,59] */
	u16     sec;        /**< Seconds after the minute - [0,59] */
	u16     hsec;       /**< Hundredth part of second - [0,99] */
} nmeaTIME;

/**
* RMC packet information structure (Recommended Minimum sentence C)
*/
typedef struct _nmeaGPRMC
{
	nmeaTIME	utc;			/**< UTC of position */
	u8			status;			/**< Status (A = active or V = void) */
	u8			ns;				/**< [N]orth or [S]outh */
	u8			ew;				/**< [E]ast or [W]est */
	u32			direction;		/**< Track angle in degrees True */
	u32			declination;	/**< Magnetic variation degrees (Easterly var. subtracts from true course) */
	u8			declin_ew;		/**< [E]ast or [W]est */
	u8			mode;			/**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */
	u8			lat[16];	    /**< Latitude in NDEG - [degree][min].[sec/60] */
	u8			lon[16];	    /**< Longitude in NDEG - [degree][min].[sec/60] */
	u8			speed[8];	    /**< Speed over the ground in knots */    
    u8          course[8];      /**< direction, 000.0~359.9, base N */    
} nmeaGPRMC;

/**
* RMC packet information structure (Recommended Minimum sentence C)
*/
typedef struct _nmeaINFO
{
	u8			id;		        /**< device id */    
	u8			status;			/**< Status (A = active or V = void) */
	u8			ns;				/**< [N]orth or [S]outh */
	u8			ew;				/**< [E]ast or [W]est */
	nmeaTIME	utc;			/**< UTC of position */    
	double  	lat;	        /**< Latitude in NDEG - [degree][min].[sec/60] */
	double		lon;	        /**< Longitude in NDEG - [degree][min].[sec/60] */
	double		speed;	        /**< Speed over the ground in knots */    
    double      course;         /**< direction, 000.0~359.9, base N */    
} nmeaINFO;


/*--------------functions declare here-----------------------------*/
u32     nmea_parse(const char *buff, u32 buff_sz, nmeaGPRMC *info);

float   nmea_caldistance(float lat1, float lng1, float lat2, float lng2);

/*---------------extern vars & functinos here-----------------------*/

#if defined(__cplusplus)
}
#endif

#endif
