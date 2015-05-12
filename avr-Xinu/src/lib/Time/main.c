/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#define BAUD 9600L
#define STK500_UBRR (16000000UL)/(BAUD*8L)-1L		// U2X0 set for doubling

#include <avr/io.h>
#include "/opt/local/avr/include/stdio.h"

#include <time.h>
#include <date.h>

double calcSunriseUTC(double JD, double latitude, double longitude);
double calcSunsetUTC(double JD, double latitude, double longitude);
double calcSunset(int, int, int, double, double);
double calcSunrise(int, int, int, double, double);
double calcJD(int year, int month, int day);

typedef int64_t BIGtime_t;


#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>


static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

static int
uart_putchar(char c, FILE *stream)
{
	
	if (c == '\n')
	uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

static void
init_uart()
{
	int ubrr = STK500_UBRR;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	/* set 2X transmit and receive clock */
	UCSR0A = (1<<U2X0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1 stop bit (default) */
	UCSR0C =  (1<<USBS0) | (3<<UCSZ00);
}

void print_tm(const struct tm *tp)
{
	if (tp)
		printf("%04d-%02d-%02d %02d:%02d:%02d yday %03d wday %d isdst %d offset %ld %3s\n",
		   tp->tm_year+1900, tp->tm_mon + 1, tp->tm_mday,
		   tp->tm_hour, tp->tm_min, tp->tm_sec,
		   tp->tm_yday, tp->tm_wday, tp->tm_isdst, tp->tm_gmtoff, tp->tm_zone);
	else
		printf ("0");
}

void print_double(double v, int8_t width, uint8_t prec)
{
	char string[15];
	
	printf("%s\n", dtostrf(v,width,prec,string));
}

int main(void)
{
	int i;
	char str[30];
	struct tm dt;
	
	init_uart();
	stdout = &mystdout;
	
	printf("Hello World\n");
	
	time_t t_unix = -880300800;		/* This should be: 08:00:00 GMT on Sunday, Feb 8 1942 */
	printf ("We use this date: %s\n", ascdate(t_unix, str));
	printf ("This should be: 08:00:00 UTC on Sunday 08 Feb 1942.\n");
	
	/* Test backward from the Epoch */
	for (t_unix=0, i=0; i<10; i++, t_unix -= SECPERDY)	{
		printf ("%s\n", ascdate(t_unix, str));
		printf ("%s\n", ascdate(t_unix-1, str));
	}
	
	/* Test from March 8 2015 4:00:00 UTC forward through the start of DST */
	for (t_unix=1425787200, i=0; i<10; i++, t_unix += 3600)	{
		printf ("%s\n", ascdate(t_unix, str));
		printf ("%s\n", ascdate(t_unix-1, str));
	}
	
	dt.tm_sec = 0;
	dt.tm_min = 0;
	dt.tm_hour = 12;			/* local iff tm_gmtoff set */
	dt.tm_mon = 1;
	dt.tm_mday = 1;
	dt.tm_year = 2015-1900;
	dt.tm_gmtoff = ut2ltim(0);
	dt.tm_isdst = 0;
	dt.tm_zone = "PST";
	t_unix = mktime(&dt);
	print_tm(&dt);
	
	/* you can do weird stuff with the structure members, as long as you do not overflow them,
	 * if you then normalize */
	dt.tm_hour += 24;
	normalizeTm(&dt);
	print_tm(&dt);
	
	/* Conversions can be done at the tm level -- change time zone for instance */
	dt.tm_hour += 3;
	dt.tm_gmtoff += 3*SECPERHR;
	dt.tm_zone = "EST";
	normalizeTm(&dt);
	print_tm(&dt);

	/* Convert EST to UTC and print a timestamp */
	dt.tm_hour += 5;
	dt.tm_gmtoff = 0;
	normalizeTm(&dt);
	printf("%s\n", makeTimestamp(str, &dt));
	
	/* Test scan a timestamp and print it */
	scanTimestamp("Tue Mar 10 00:00:00 UTC 2015", &dt);
	printf("%s\n", makeTimestamp(str, &dt));
	
	/* Test normalization of getUTC and mktime */
	struct tm ds;
	dt.tm_sec = ds.tm_sec = 0;
	dt.tm_min = ds.tm_min = 0;
	dt.tm_hour = ds.tm_hour = 0;
	dt.tm_mon = ds.tm_mon = 0;
	dt.tm_mday = ds.tm_mday = 1;
	dt.tm_year = ds.tm_year = 1970-1900;
	for (i = 0; i < 30000; i++)	{
		dt.tm_hour++; ds.tm_hour++;
		if (getUTC(&ds) != mktime(&dt))	{
			printf("Fails at %d %d\n", dt.tm_hour, ds.tm_hour);
			printf("%s\n", makeTimestamp(str, &dt));
			printf("%s\n", makeTimestamp(str, &ds));
			break;
		}
	}
	printf("Pass at %d %d\n", dt.tm_hour, ds.tm_hour);
	printf("%s\n", makeTimestamp(str, &dt));
	
	
	
	/************************/
	/* Sunrise and Sunset	*/
	/************************/
	
	double latitude = 40.8441;
	double longitude = -124.0640;
	int year = 2015, month = 5, day = 11;
	char scan_str[] = "%3s %3s %d %4d %2d:%02d:%02d %3s";
	time_t seconds;
	struct tm *ptm;
	
	/* enter tm as 0:00:00UTC for the date */
	dt.tm_year= year-1900;
	dt.tm_mon=month-1;  /* Jan = 0, Feb = 1,.. Dec = 11 */
	dt.tm_mday=day;
	dt.tm_hour=0;
	dt.tm_min=0;
	dt.tm_sec=0;
	seconds = mktime(&dt);
	seconds += calcSunrise(day, month, year, latitude, longitude);
	
	ptm = localtime(&seconds);
	sprintf(str, scan_str, day_name[ptm->tm_wday],
				mon_name[ptm->tm_mon], ptm->tm_mday, ptm->tm_year+1900,
				ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ptm->tm_zone);
	printf("Sunrise  %s\n",str);
	
	dt.tm_year= year-1900;
	dt.tm_mon=month-1;  /* Jan = 0, Feb = 1,.. Dec = 11 */
	dt.tm_mday=day;
	dt.tm_hour=0;
	dt.tm_min=0;
	dt.tm_sec=0;
	seconds = mktime(&dt);
	seconds += calcSunset(day, month, year, latitude, longitude);
	
	ptm = localtime(&seconds);
	sprintf(str, scan_str, day_name[ptm->tm_wday],
			mon_name[ptm->tm_mon], ptm->tm_mday, ptm->tm_year+1900,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ptm->tm_zone);
	printf("Sunset   %s\n",str);

	
    return 0;
}
