/*
 The actual SweepTools file that defines the namespace
 */

#include "Ocular.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"
#include <QString>

namespace SweepTools
{
//I'm not clever enough to make one function with two outputs
/*Qstring findCenter(const QString startRA, 
			  const QString endRA, 
			  const QString startDec, 
			  const QString endDec) const
{

	double startRAd = StelUtils::getDecAngle(startRA);	
	double endRAd = StelUtils::getDecAngle(endRA);	
	double startDecd = StelUtils::getDecAngle(startDec);	
	double endDecd = StelUtils::getDecAngle(endDec);	

	double midpointRAd = endRAd - startRAd;
	double midpointDecd = endDecd - startDecd;

	QString midpointRA = StelUtils::radToHmsStr(midpointRAd, true);
	QString midpointDec = StelUtils::radToHmsStr(midpointDecd, true);

	return midpointRA;
}
*/

//******************************************************************

//These two functions take in the starting RA and Dec and will spit out the center coordinates of our box
QString findMidRA(const QString startRA,
              const QString endRA)
{

    double startRAd = StelUtils::getDecAngle(startRA);
    double endRAd = StelUtils::getDecAngle(endRA);

    double midpointRAd = startRAd + ((endRAd - startRAd)/2.0);
    cout << "endRAd = " << endRAd << endl;
    cout << "startRAd = " << startRAd << endl;
    cout << "midpointRAd = " << midpointRAd << endl;
    QString midpointRA = StelUtils::radToHmsStr(midpointRAd, true);

    return midpointRA;
}

QString findMidDec(const QString startDec,
                   const QString endDec)
{
    double startDecd = StelUtils::getDecAngle(startDec);
    double endDecd = StelUtils::getDecAngle(endDec);

    double midpointDecd = startDecd + ((endDecd - startDecd)/2.0);
    cout << "endDecd = " << endDecd << endl;
    cout << "startDecd = " << startDecd << endl;
    cout << "midpointDecd = " << midpointDecd << endl;

    QString midpointDec = StelUtils::radToDmsStr(midpointDecd, true, true);

    return midpointDec;
}

//*****************************************************************

//have to make sure all the times during calculations are in jds
QDateTime calcTime(const QString startRA, const QDateTime& dateTime)
{
    double jd = StelUtils::qDateTimeToJd(dateTime);
    double tu = jd - 2451545.0;
    double ERA = 2.0*3.14159*(0.7790572732640 + 1.00273781191135448*tu); //This is in radians
    double ERAtm = ERA - floor(ERA);
    double startRAd = StelUtils::getDecAngle(startRA); //radians
    double ha = ERA - startRAd;
    double sidtime = ERA - ha;
    double tgoal = ((sidtime/(2.0*3.14159)) - 0.7790572732640)/1.00273781191135448;
    double tdGoal = jd + tgoal+0.5;
    //QTime time = jdFractionToQTime(tgoal);
    QDateTime goalTime = StelUtils::jdToQDateTime(tdGoal);
    return goalTime;
	//The format for input is "yyyy'-'MM'-'dd'T'hh:mm:ss.z"
  
}

}
