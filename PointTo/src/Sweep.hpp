/*
 This is the header for the Sweep function
 */

#ifndef SWEEP_HPP_
#define SWEEP_HPP_

#include <QDebug>
#include <QObject>
#include <QString>
#include <QSettings>

//class Telescope;
//class Lens;

//! @ingroup sweeps
class Sweep : public QObject
{
	Q_OBJECT

//This is an object with a variety of properties that will most likely
//not matter to what we are doing. I need to figure out what to
//remove and what to keep.
	
	Q_PROPERTY(QString sweepName READ SweepName WRITE setSweepName)
	Q_PROPERTY(QString startRA READ StartRA WRITE setStartRA)
	Q_PROPERTY(QString endRA READ EndRA WRITE setEndRA)
	Q_PROPERTY(QString startDec READ StartDec WRITE setStartDec)
	Q_PROPERTY(QString endDec READ EndDec WRITE setEndDec)
	Q_PROPERTY(QString date READ Date WRITE setDate)
	//Q_PROPERTY(QString reticlePath READ reticlePath WRITE setReticlePath)
public:
	Sweep();
	Q_INVOKABLE Sweep(const QObject& other);
	virtual ~Sweep();
	static Sweep * sweepFromSettings(const QSettings * theSettings, const int sweepIndex);
	void writeToSettings(QSettings * settings, const int index);
	static Sweep * sweepModel(void);

	QString sweepName(void) const;
	void setSweepName(const QString & SweepName);
	QString startRA(void) const;
	void setStartRA(const QString & startRA);
	QString endRA(void) const;
	void setEndRA(const QString & endRA);
	QString startDec(void) const;
	void setStartDec(const QString & startDec);
	QString endDec(void) const;
	void setEndDec(const QString & endDec);
	QString date(void) const;
	void setDate(const QString & dateStr);

/*
	double appearentFOV(void) const;
	void setAppearentFOV(const double fov);
	double effectiveFocalLength(void) const;
	void setEffectiveFocalLength(const double fl);
	double fieldStop(void) const;
	void setFieldStop(const double fs);
	QString name(void) const;
	void setName(const QString aName);
	QString reticlePath(void) const;
	void setReticlePath(const QString path);

	double actualFOV(const Telescope * telescope, const Lens *lens) const;
	double magnification(const Telescope * telescope, const Lens *lens) const;
*/
	QMap<int, QString> propertyMap(void);

private:
	QString m_startRA;
	QString m_endRA;
	QString m_startDec;
	QString m_endDec;
	QString m_date;
	QString m_name;
//	QString m_reticlePath;	

};


#endif /* SWEEP_HPP_ */
