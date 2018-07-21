/*
 The actual SweepTools file that defines the namespace
 */

#include "Sweep.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"
#include "StelApp.hpp"
#include <QString>
#include "StelModule.hpp"
#include "StelProjector.hpp"
#include "StelObjectType.hpp"
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
    QString midpointRA = StelUtils::radToHmsStr(midpointRAd, true);

    return midpointRA;
}

QString findMidDec(const QString startDec,
                   const QString endDec)
{
    double startDecd = StelUtils::getDecAngle(startDec);
    double endDecd = StelUtils::getDecAngle(endDec);

    double midpointDecd = startDecd + ((endDecd - startDecd)/2.0);

    QString midpointDec = StelUtils::radToDmsStr(midpointDecd, true, true);

    return midpointDec;
}

//*****************************************************************

//have to make sure all the times during calculations are in jds
//the locSidTime is in rads
QDateTime calcTime(const QString startRA, const QDateTime& dateTime, const double locSidTime)
{
    double shift = StelUtils::getDecAngle(startRA)-locSidTime;
    double goalTime = StelUtils::qDateTimeToJd(dateTime) + (shift/(2*3.14159265));
    return StelUtils::jdToQDateTime(goalTime);
    /*
    double jd = StelUtils::qDateTimeToJd(dateTime);
    qWarning() << "after conversion, the julian date corresponds to " << StelUtils::jdToQDateTime(jd);
    double tu = jd - 2451545.0;
    double ERA = 2.0*3.14159*(0.7790572732640 + 1.00273781191135448*tu); //This is in radians
    //double ERAtm = ERA - floor(ERA);
    double startRAd = StelUtils::getDecAngle(startRA); //radians
    double ha = ERA - startRAd;
    double sidtime = ERA - ha;
    double tgoal = ((sidtime/(2.0*3.14159)) - 0.7790572732640)/1.00273781191135448;
    qWarning() << "goal time is " << StelUtils::jdFractionToQTime(-tgoal);
    qWarning() << "The day is " << floor(jd);
    if (tgoal < 0.0) {tgoal *= -1.0;}
    double tdGoal = floor(jd) + tgoal-0.5;
    qWarning() << StelUtils::jdToQDateTime(tdGoal);
    qWarning() << StelUtils::jdToQDateTime(tdGoal-0.5);
    //QTime time = jdFractionToQTime(tgoal);
    QDateTime goalTime = StelUtils::jdToQDateTime(tdGoal);
    return goalTime;
	//The format for input is "yyyy'-'MM'-'dd'T'hh:mm:ss.z"
  */
}


//*************************************************************************

double timeDifSecs(const QString startRA, const QString endRA)
{
    double startRAd = StelUtils::getDecAngle(startRA);
    double endRAd = StelUtils::getDecAngle(endRA);

    double deltaRads = endRAd-startRAd;
    if (deltaRads < -3.14159)
    {
        startRAd -= 2.0*3.14159265;
        deltaRads = endRAd - startRAd;

    }
    double deltaTSecs = deltaRads/((2.0*3.14159265)/(24.0*3600.0));
    //QString deltaT = StelUtils::radToHmsStr(deltaRads);
    //double StelUtils::

    return deltaTSecs;
}

//******************************************************************************

double sweepRate(const QString startDec,
                 const QString endDec,
                 const double deltaTRA,
                 const double currentFov,
                 const double moveRate,
                 const double deltaTime)
{
    QSettings * conf = StelApp::getInstance().getSettings();

    float keyMoveSpeed = conf->value("navigation/move_speed").toFloat();
    qWarning() << "current keyMoveSpeed is "<< keyMoveSpeed;
    double currentRate = moveRate;//keyMoveSpeed*deltaTime*1000*currentFov;//30.0;
    qWarning() << "current rate is " << currentRate;
    double startDecRads = StelUtils::getDecAngle(startDec);
    double endDecRads = StelUtils::getDecAngle(endDec);
    double deltaDecRads = startDecRads - endDecRads;
    qWarning() << "deltaDecRads is" << deltaDecRads;
    //QString deltaDec = StelUtils::radToDecDegStr(deltaDecRads,4, false, false);
    //qWarning() << "deltaDec is " + deltaDec;
    double deltaDecDb = deltaDecRads*180./3.4159265; //deltaDec.toDouble();
    qWarning() << deltaDecDb;
    double desiredRate = ((deltaDecDb)/deltaTRA)*currentFov*keyMoveSpeed*1000;
    qWarning() << "desired rate is "<< desiredRate;
    double movementFactor = desiredRate/currentRate;
    return movementFactor;

}

//************************************************************************
//fov is in rads, time is in secs

double timePerSweep(double startRARads,
                    double endRARads,
                    double timeDif,
                    double fov)
{
    double overlap = fov/2.0;
    qWarning() << "The overlap, half the fov in radians, is " << overlap;
    double deltaRA = endRARads - startRARads;
    if (deltaRA < -3.14159)
    {
        startRARads -= 2.0*3.14159265;
        deltaRA = endRARads - startRARads;

    }
    qWarning() << deltaRA;
    if (deltaRA < 0) {deltaRA = -deltaRA;}

    double sweepsRequired = deltaRA/overlap;
    if (overlap > deltaRA)
    {
        qWarning() << "fov is bigger than distance between endpoints, defaulting to 10 sweeps";
        sweepsRequired = 10.0;
    }
    qWarning() << "The number of necessary sweeps (deltaRA/overlap) is " << sweepsRequired;
    double secsPerSweep = timeDif/sweepsRequired;

    return secsPerSweep;

}

}
