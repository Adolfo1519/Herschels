/*
 This is the header for the SweepTools namespace (function package)
 */

#ifndef SWEEPTOOLS_HPP_
#define SWEEPTOOLS_HPP_

#include <cmath>
#include "VecMath.hpp"

#include <QVariantMap>
#include <QDateTime>
#include <QString>

// astronomical unit (km)
#define AU 149597870.691
#define AU_KM (1.0/149597870.691)
// Parsec (km)
#define PARSEC 30.857e12
// speed of light (km/sec)
#define SPEED_OF_LIGHT 299792.458

#define stelpow10f(x) std::exp((x) * 2.3025850930f)

#include <QDebug>
#include <QObject>
#include <QSettings>

//class Telescope;
//class Lens;

//! define namespace SweepTools
namespace SweepTools 
{
	
	/*Qstring findCenter(const Qstring startRA, 
			   const Qstring endRA, 
			   const Qstring startDec, 
			   const Qstring endDec) const*/
	QString findMidDec(const QString startRA,
                   	const QString endRA) const

	QString findMidDec(const QString startDec,
                   	const QString endDec) const

	QDateTime calcTime(const QString startRA, 
			   const QDateTime& dateTime) const

	
};


#endif /* SWEEPTOOLS_HPP_ */
