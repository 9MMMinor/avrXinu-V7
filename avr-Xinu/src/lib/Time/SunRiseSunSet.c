/**
 *  \file SunRiseSunSet.c
 *  \project SunsetCalculation
 */

//
//  Created by Michael Minor on 1/1/15.
//  Copyright (c) 2015.
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//

#include <stdio.h>
#include "time.h"
#include "date.h"
#include <math.h>

#define TRUE 1
#define FALSE 0


//Source:
//	Almanac for Computers, 1990
//	published by Nautical Almanac Office
//	United States Naval Observatory
//	Washington, DC 20392

//Inputs:
//	day, month, year:      date of sunrise/sunset
//	latitude, longitude:   location for sunrise/sunset
//	zenith:                Sun's zenith for sunrise/sunset
//	  offical      = 90 degrees 50'
//	  civil        = 96 degrees
//	  nautical     = 102 degrees
//	  astronomical = 108 degrees
//
//	NOTE: longitude is positive for East and negative for West
//        NOTE: the algorithm assumes the use of a calculator with the
//        trig functions in "degree" (rather than "radian") mode. Most
//       programming languages assume radian arguments, requiring back
//        and forth convertions. The factor is 180/pi. So, for instance,
//        the equation RA = atan(0.91764 * tan(L)) would be coded as RA
//        = (180/pi)*atan(0.91764 * tan((pi/180)*L)) to give a degree
//        answer with a degree input for L.
//
//
//1. first calculate the day of the year
//
//
//	N1 = floor(275 * month / 9)
//	N2 = floor((month + 9) / 12)
//	N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3))
//	N = N1 - (N2 * N3) + day - 30
//
//2. convert the longitude to hour value and calculate an approximate time
//
//	lngHour = longitude / 15
//
//	if rising time is desired:
//	  t = N + ((6 - lngHour) / 24)
//	if setting time is desired:
//	  t = N + ((18 - lngHour) / 24)
//
//3. calculate the Sun's mean anomaly
//
//	M = (0.9856 * t) - 3.289
//
//4. calculate the Sun's true longitude
//
//	L = M + (1.916 * sin(M)) + (0.020 * sin(2 * M)) + 282.634
//	NOTE: L potentially needs to be adjusted into the range [0,360) by adding/subtracting 360
//
//5a. calculate the Sun's right ascension
//
//	RA = atan(0.91764 * tan(L))
//	NOTE: RA potentially needs to be adjusted into the range [0,360) by adding/subtracting 360
//
//5b. right ascension value needs to be in the same quadrant as L
//
//	Lquadrant  = (floor( L/90)) * 90
//	RAquadrant = (floor(RA/90)) * 90
//	RA = RA + (Lquadrant - RAquadrant)
//
//5c. right ascension value needs to be converted into hours
//
//	RA = RA / 15
//
//6. calculate the Sun's declination
//
//	sinDec = 0.39782 * sin(L)
//	cosDec = cos(asin(sinDec))
//
//7a. calculate the Sun's local hour angle
//
//	cosH = (cos(zenith) - (sinDec * sin(latitude))) / (cosDec * cos(latitude))
//
//	if (cosH >  1)
//	  the sun never rises on this location (on the specified date)
//	if (cosH < -1)
//	  the sun never sets on this location (on the specified date)
//
//7b. finish calculating H and convert into hours
//
//	if rising time is desired:
//	  H = 360 - acos(cosH)
//	if setting time is desired:
//	  H = acos(cosH)
//
//	H = H / 15
//
//8. calculate local mean time of rising/setting
//
//	T = H + RA - (0.06571 * t) - 6.622
//
//9. adjust back to UTC
//
//	UT = T - lngHour
//	NOTE: UT potentially needs to be adjusted into the range [0,24) by adding/subtracting 24
//
//10. convert UT value to local time zone of latitude/longitude
//
//	localT = UT + localOffset
//

double
calcSunriseSunset(int year, int month, int day, double latitude, double longitude, int sunrise)
{
		//longitude  =  13.41;		berlin
		//latitude   =  52.52;
		//longitude  =  12.30;		rome
		//latitude   =  41.54;
		//longitude  = -74.01;		new york
		//latitude   =  40.71;
		//longitude  = -124.0640;	arcata, CA
		//latitude   =  40.8441;

	double zenith = 90.83333333333333;
	double D2R = 3.14159 / 180;
	double R2D = 180 / 3.14159;
	//1. first calculate the day of the year
	int dy = day_of_year(year, month-1, day);

	//2. convert the longitude to hour value and calculate an approximate time
	double lnHour = longitude / 15.0;
	double tt;
	if (sunrise)	{
		tt = dy + ((6.0 - lnHour) / 24.0);
	}
	else	{
		tt = dy + ((18.0 - lnHour) / 24.0);
	}

	//3. calculate the Sun's mean anomaly
	//	M = (0.9856 * t) - 3.289
	double M = (0.9856 * tt) - 3.289;

	//4. calculate the Sun's true longitude
	//	L = M + (1.916 * sin(M)) + (0.020 * sin(2 * M)) + 282.634
	//	NOTE: L potentially needs to be adjusted into the range [0,360) by adding/subtracting 360
	double L = M + (1.916 * sin(M * D2R)) + (0.020 * sin(2 * M * D2R)) + 282.634;
	if (L >= 360.0)	{
		L -= 360.0;
	}
	else if (L < 0.0)	{
		L += 360.0;
	}

	//5a. calculate the Sun's right ascension
	//	RA = atan(0.91764 * tan(L))
	//	NOTE: RA potentially needs to be adjusted into the range [0,360) by adding/subtracting 360

	double RA = R2D * atan(0.91764 * tan(L * D2R));
	if (RA >= 360.0)	{
		RA -= 360.0;
	}
	else if (RA < 0.0)	{
		RA += 360.0;
	}

	//5b. right ascension value needs to be in the same quadrant
	//	Lquadrant  = (floor( L/90)) * 90
	//	RAquadrant = (floor(RA/90)) * 90
	//	RA = RA + (Lquadrant - RAquadrant)
	int32_t Lquadrant = floor(L / (90.0)) * 90.0;
	int32_t RAquadrant = floor(RA / 90.0) * 90.0;
	RA += (Lquadrant - RAquadrant);

	//5c. right ascension value needs to be converted into hours
	//	RA = RA / 15
	RA /= 15.0;

	//6. calculate the Sun's declination
	//	sinDec = 0.39782 * sin(L)
	//	cosDec = cos(asin(sinDec))
	double sinDec = 0.39782 * sin(L * D2R);
	double cosDec = cos(asin(sinDec));

	//7. calculate the Sun's local hour angle
	//	cosH = (cos(zenith) - (sinDec * sin(latitude))) / (cosDec * cos(latitude))
	//
	//	if (cosH >  1)
	//	  the sun never rises on this location (on the specified date)
	//	if (cosH < -1)
	//	  the sun never sets on this location (on the specified date)
	double cosH = (cos(zenith * D2R) - (sinDec * sin(latitude * D2R))) / (cosDec * cos(latitude * D2R));
	//	if rising time is desired:
	//	  H = 360 - acos(cosH)
	//	if setting time is desired:
	//	  H = acos(cosH)
	//
	//	H = H / 15
	double H;
	if (sunrise)	{
		H = 360.0 - (R2D * acos(cosH));
	}
	else	{
		H = R2D * acos(cosH);
	}
	H /= 15.0;

	//8. calculate local mean time of rising/setting
	//	T = H + RA - (0.06571 * t) - 6.622
	//	NOTE: local mean time, T, must be adjusted into the range [0,24)!!!!
	double T = H + RA - (0.06571 * tt) - 6.622;
	if (T >= 24.0)
		T -= 24.0;
	else if (T < 0.0)
		T += 24.0;

	//9. adjust back to UTC
	//	UT = T - lngHour
	// Can be less than 0 (the day before) or greater than 24 (the next day)
	double UT = T - lnHour;

	//10.
	//convert to seconds

	return (UT*3600.0);
}

/**
 * \brief Calculate Sunrise
 *
 *	Calculate seconds past 0:00:00 UTC of the sunrise at the given
 *	location on the given calendar day.
 *
 * \param day -- day of the month (1-31).
 * \param month -- month of the year (1-12).
 * \param year -- calendar year (e.g. 2015).
 * \param latitude -- in degrees of location.
 * \param longitude -- positive for East and negative for West of the prime meridian.
 * \return Sunrise in seconds past 0:00:00 UTC.
 */
double calcSunrise(int day, int month, int year, double latitude, double longitude)
{
	
	return ( calcSunriseSunset(year, month, day, latitude, longitude, TRUE) );
}

/**
 * \brief Calculate Sunset
 *
 *	Calculate seconds past 0:00:00 UTC of the sunset at the given
 *	location on the given calendar day.
 *
 * \param day -- day of the month (1-31).
 * \param month -- month of the year (1-12).
 * \param year -- calendar year (e.g. 2015).
 * \param latitude -- in degrees of location.
 * \param longitude -- positive for East and negative for West of the prime meridian.
 * \return Sunset in seconds past 0:00:00 UTC.
 */
double calcSunset(int day, int month, int year, double latitude, double longitude)
{
	
	return ( calcSunriseSunset(year, month, day, latitude, longitude, FALSE) );
}
