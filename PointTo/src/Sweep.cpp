/*
 The actual Sweep function that will take in data and point to somewhere
 */

#include "Sweep.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"
/*
#include "Telescope.hpp"
#include "Lens.hpp"
*/
//These objects are those defined as Q_Object in the header
Sweep::Sweep()
	/*: m_binoculars(false),
	  m_permanetCrosshair(false),
	  m_appearentFOV(0.0),
	  m_effectiveFocalLength(0.0),
	  m_fieldStop(0.0)*/
{
}
//Constructor
Sweep::Sweep(const QObject& other)
	: m_startRA(other.property("startRA").toString()),
	  m_endRA(other.property("endRA").toString()),
	  m_startDec(other.property("startDec").toString()),
	  m_endDec(other.property("endDec").toString()),
	  m_date(other.property("date").toString()),
      m_name(other.property("name").toString())
	  //m_reticlePath(other.property("reticlePath").toString())
{
}
//Destructor
Sweep::~Sweep()
{
}

static QMap<int, QString> mapping;
QMap<int, QString> Sweep::propertyMap(void)
{
	if(mapping.isEmpty()) {
		mapping = QMap<int, QString>();
		mapping[0] = "name";
		mapping[1] = "startRA";
		mapping[2] = "endRA";
		mapping[3] = "startDec";
		mapping[4] = "endDec";
		mapping[5] = "date";
		//mapping[6] = "reticlePath";
	}
	return mapping;
}


/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Instance Methods
#endif
/* ********************************************************************* */
/*
const double lens_multipler = (lens != Q_NULLPTR ? lens->getMultipler() : 1.0f);
	double actualFOV = 0.0;
	if (m_binoculars) {
		actualFOV = appearentFOV();
	} else if (fieldStop() > 0.0) {
		actualFOV =  fieldStop() / (telescope->focalLength() * lens_multipler) * 57.3;
	} else {
		//actualFOV = apparent / mag
		actualFOV = appearentFOV() / (telescope->focalLength() * lens_multipler / effectiveFocalLength());
	}
	return actualFOV;
}
*/
//have to make sure all the times during calculations are in jds
/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Accessors & Mutators
#endif
/* ********************************************************************* */
QString Sweep::sweepName(void) const
{
	return m_name;
}

void Sweep::setSweepName(const QString & aName)
{
	m_name = aName;
}

QString Sweep::startRA(void) const
{
	return m_startRA;
}

void Sweep::setStartRA(const QString & sRA)
{
	m_startRA = sRA;
}

QString Sweep::endRA(void) const
{
	return m_endRA;
}

void Sweep::setEndRA(const QString & eRA)
{
	m_endRA = eRA;
}

QString Sweep::startDec(void) const
{
	return m_startDec;
}

void Sweep::setStartDec(const QString & sDec)
{
	m_startDec = sDec;
}

QString Sweep::endDec(void) const
{
	return m_endDec;
}

void Sweep::setEndDec(const QString & eDec)
{
	m_endDec = eDec;
}

QString Sweep::date(void) const
{
	return m_date;
}

void Sweep::setDate(const QString & dt)
{
	m_date = dt;
}
/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Static Methods
#endif
/* ********************************************************************* */

Sweep * Sweep::sweepFromSettings(const QSettings *theSettings, const int sweepIndex)
{
	Sweep* sweep = new Sweep();
	QString prefix = "sweep/" + QVariant(sweepIndex).toString() + "/";

	sweep->setSweepName(theSettings->value(prefix + "name", "").toString());
	
	sweep->setStartRA(theSettings->value(prefix + "startRA", "").toString());
	sweep->setEndRA(theSettings->value(prefix + "endRA", "").toString());

	sweep->setStartDec(theSettings->value(prefix + "startDec", "").toString());
	sweep->setEndDec(theSettings->value(prefix + "endDec", "").toString());
	
	sweep->setDate(theSettings->value(prefix + "date", "").toString());
		
	
	return sweep;
}

void Sweep::writeToSettings(QSettings * settings, const int index)
{
	QString prefix = "sweep/" + QVariant(index).toString() + "/";
	settings->setValue(prefix + "name", this->sweepName());
	settings->setValue(prefix + "startRA", this->startRA());
	settings->setValue(prefix + "endRA", this->endRA());
	settings->setValue(prefix + "startDec", this->startDec());
	settings->setValue(prefix + "endDec", this->endDec());
	settings->setValue(prefix + "date", this->date());
	//settings->setValue(prefix + "reticlePath", this->reticlePath());
}

/*
QString startRA = "07h24m0.00s";
    QString endRA = "08h10m0.00s";
    QString startDec = "24d12m00.0s";
    QString endDec = "21d52m00.0s";
   
QString date = "1787-01-24T22:00:30.0";
"yyyy'-'MM'-'dd'T'hh:mm:ss.z"
  */  

Sweep * Sweep::sweepModel(void)
{
	Sweep* model = new Sweep();
	model->setSweepName("My Sweep");
	model->setStartRA("07h24m0.00s");
	model->setEndRA("08h10m0.00s");
	model->setStartDec("24d12m00.0s");
	model->setEndDec("21d52m00.0s");
	model->setDate("1787-01-24T22:00:30.0");
	//model->setReticlePath("");
	return model;
}
