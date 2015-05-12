//
//  sunset.c
//  Demo
//
//  Created by Michael Minor on 3/15/15.
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


/* Copyright (GPL) 2004   Mike Chirico mchirico@comcast.net
 Updated: Sun Nov 28 15:15:05 EST 2004
 
 Program adapted by Mike Chirico mchirico@comcast.net
 
 Reference:
 http://prdownloads.sourceforge.net/souptonuts/working_with_time.tar.gz?download
 http://www.srrb.noaa.gov/highlights/sunrise/sunrise.html
 
 
 Compile:
 
 gcc -o sunrise -Wall -W -O2 -s -pipe -lm sunrise.c
 
 or for debug output
 
 gcc -o sunrise -DDEBUG=1 -Wall -W -O2 -s -pipe -lm sunrise.c
 
 
 
 
 This can also go in a batch job to calculate the next
 20 days as follows:
 
 #!/bin/bash
 lat=39.95
 long=75.15
 for (( i=0; i <= 20; i++))
 do
 ./sunrise    `date -d "+$i day" "+%Y %m %d"` $lat $long
 done
 
 
 
 */

#include <stdio.h>
#include <time.h>
#include <date.h>
#include <math.h>

double calcSunEqOfCenter(double t);


/* Convert degree angle to radians */

double  degToRad(double angleDeg)
{
	return (M_PI * angleDeg / 180.0);
}

double radToDeg(double angleRad)
{
	return (180.0 * angleRad / M_PI);
}

double calcMeanObliquityOfEcliptic(double t)
{
	double seconds = 21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813)));
	double e0 = 23.0 + (26.0 + (seconds/60.0))/60.0;
	
	return e0;              // in degrees
}

double calcGeomMeanLongSun(double t)
{
	double L = 280.46646 + t * (36000.76983 + 0.0003032 * t);
	while( (int) L >  360 )   {
		L -= 360.0;
		
	}
	while (L <  0) {
		L += 360.0;
	}
	return L;              // in degrees
}

double calcObliquityCorrection(double t)
{
	double e0 = calcMeanObliquityOfEcliptic(t);
	
	
	double omega = 125.04 - 1934.136 * t;
	double e = e0 + 0.00256 * cos(degToRad(omega));
	return e;               // in degrees
}

double calcEccentricityEarthOrbit(double t)
{
	double e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
	return e;               // unitless
}

double calcGeomMeanAnomalySun(double t)
{
	double M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
	
	return M;               // in degrees
}

double calcEquationOfTime(double t)
{
	double epsilon = calcObliquityCorrection(t);
	double  l0 = calcGeomMeanLongSun(t);
	double e = calcEccentricityEarthOrbit(t);
	double m = calcGeomMeanAnomalySun(t);
	double y = tan(degToRad(epsilon)/2.0);
	
	y *= y;
	
	double sin2l0 = sin(2.0 * degToRad(l0));
	double sinm   = sin(degToRad(m));
	double cos2l0 = cos(2.0 * degToRad(l0));
	double sin4l0 = sin(4.0 * degToRad(l0));
	double sin2m  = sin(2.0 * degToRad(m));
	double Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0
	- 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;
	
	return radToDeg(Etime)*4.0;	// in minutes of time
}

double calcTimeJulianCent(double jd)
{
	double T = ( jd - 2451545.0)/36525.0;
	
	return T;
}

double calcSunTrueLong(double t)
{
	double l0 = calcGeomMeanLongSun(t);
	double c = calcSunEqOfCenter(t);
	
	double O = l0 + c;
	return O;               // in degrees
}

double calcSunApparentLong(double t)
{
	double o = calcSunTrueLong(t);
	
	double  omega = 125.04 - 1934.136 * t;
	double  lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
	return lambda;          // in degrees
}

double calcSunDeclination(double t)
{
	double e = calcObliquityCorrection(t);
	double lambda = calcSunApparentLong(t);
	
	double sint = sin(degToRad(e)) * sin(degToRad(lambda));
	double theta = radToDeg(asin(sint));
	return theta;           // in degrees
}

double calcHourAngleSunrise(double lat, double solarDec)
{
	double latRad = degToRad(lat);
	double sdRad  = degToRad(solarDec);
	
	double HA = (acos(cos(degToRad(90.833))/(cos(latRad)*cos(sdRad))-tan(latRad) * tan(sdRad)));
	
	return HA;              // in radians
}

double calcHourAngleSunset(double lat, double solarDec)
{
	double latRad = degToRad(lat);
	double sdRad  = degToRad(solarDec);
	
	
	double HA = (acos(cos(degToRad(90.833))/(cos(latRad)*cos(sdRad))-tan(latRad) * tan(sdRad)));
	
	return -HA;              // in radians
}

/**
 * \brief Calculate Julian date for 0:00:00 Universal Coordinated Time.
 * \param int year CE e.g. 2015
 * \param int month (Jan=1, Dec=12)
 * \param int day calendar day of the month
 * \return Julian Date
 */

/*	The Julian Day Number (JDN) is the integer assigned to a whole solar day in the
 *	Julian day count starting from noon Greenwich Mean Time, with Julian day number 0
 *	assigned to the day starting at noon on January 1, 4713 BC, proleptic Julian
 *	calendar (November 24, 4714 BC, in the proleptic Gregorian calendar).
 *	[1] For example, the Julian day number for January 1, 2000, was 2,451,545.
 */

/**	The Julian date (JD) of any instant is the Julian day number for the preceding
 *	noon in Greenwich Mean Time plus the fraction of the day since that instant.
 *	Julian dates are expressed as a Julian day number with a decimal fraction added.
 *	For example, the Julian Date for 00:30:00.0 UTC January 1, 2013, is 2,456,293.520833.
 */
double calcJD(int year, int month, int day)
{
	
	if (month <= 2) {
		year -= 1;
		month += 12;
	}
	int A = floor(year/100);
	int B = 2 - A + floor(A/4);
	
	double JD = floor(365.25*(year + 4716)) + floor(30.6001*(month+1)) + day + B - 1524.5;
	return JD;
}

/* Time (Julian) since January 1, 2000 */
double calcJDFromJulianCent(double t)
{
	double JD = t * 36525.0 + 2451545.0;
	return JD;
}

double calcSunEqOfCenter(double t)
{
	double m = calcGeomMeanAnomalySun(t);
	
	double mrad = degToRad(m);
	double sinm = sin(mrad);
	double sin2m = sin(mrad+mrad);
	double sin3m = sin(mrad+mrad+mrad);
	
	double C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
	return C;		// in degrees
}

/************************************************************************************
 *	Need:	year month date latitude longitude
 *			Latitude and longitude must be in degrees and or fraction of degrees.
 *			Listings of Latitude and Longitude usually give longitude as degrees
 *			West of the prime meridian (-, negative, 0 to -180)
 *			and degrees East of the prime meridian (+, positive, 0 to 180)
 *			We need positive degrees WEST of the prime meridian.
 *			Example: (Just outside Arcata, California, USA) 2015 1 1 40.8441 124.0640
 *				Sunrise: Jan 1 2015  7:40:48 PST
 *				Sunset: Jan 1 2015 16:59:44 PST
 *
 ***********************************************************************************/
/**
 * \brief Calculate Sunrise for 0:00:00 Coordinated Universal Time.
 * \param JD -- Julian Date
 * \param latitude -- in degrees of location
 * \param longitude -- positive degrees WEST of the prime meridian.
 * \return Sunrise in minutes past 0:00:00 UTC
 */

double calcSunriseUTC(double JD, double latitude, double longitude)
{
	double t = calcTimeJulianCent(JD);
	
	// *** First pass to approximate sunrise
	
	double  eqTime = calcEquationOfTime(t);
	double  solarDec = calcSunDeclination(t);
	double  hourAngle = calcHourAngleSunrise(latitude, solarDec);
	double  delta = longitude - radToDeg(hourAngle);
	double  timeDiff = 4 * delta;	// in minutes of time
	double  timeUTC = 720 + timeDiff - eqTime;	// in minutes
	double  newt = calcTimeJulianCent(calcJDFromJulianCent(t) + timeUTC/1440.0);

	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);
	
	hourAngle = calcHourAngleSunrise(latitude, solarDec);
	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes
	
	return timeUTC;
}

double calcSunsetUTC(double JD, double latitude, double longitude)
{
	double t = calcTimeJulianCent(JD);
	
	// *** First pass to approximate sunset
	
	double  eqTime = calcEquationOfTime(t);
	double  solarDec = calcSunDeclination(t);
	double  hourAngle = calcHourAngleSunset(latitude, solarDec);
	double  delta = longitude - radToDeg(hourAngle);
	double  timeDiff = 4 * delta;	// in minutes of time
	double  timeUTC = 720 + timeDiff - eqTime;	// in minutes
	double  newt = calcTimeJulianCent(calcJDFromJulianCent(t) + timeUTC/1440.0);
	
	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);
	
	hourAngle = calcHourAngleSunset(latitude, solarDec);
	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes (720=MIN_PER_DAY/2)
	
	return timeUTC;
}
