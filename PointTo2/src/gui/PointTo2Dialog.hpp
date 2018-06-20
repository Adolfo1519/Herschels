/*
 This is the header file for an attempt to write an interactive PointTo2 plugin
 */

#ifndef _POINTTO2DIALOG_HPP_
#define _POINTTO2DIALOG_HPP_

#include <QObject>
/*#include "CCD.hpp"
#include "Ocular.hpp"
*/
#include "PropertyBasedTableModel_Point.hpp"
#include "StelDialog.hpp"
#include "StelStyle.hpp"
#include "Sweep.hpp"
/*#include "Telescope.hpp"
#include "Lens.hpp"
*/

class Ui_pointTo2DialogForm;		//hopefully once Kouxudong creates the xml file, I can fill this part in

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QDoubleValidator;
class QIntValidator;
class QRegExpValidator;
class QModelIndex;
class QStandardItemModel;
QT_END_NAMESPACE

class PointTo2;

//! @ingroup oculars
class PointTo2Dialog : public StelDialog
{
	Q_OBJECT

public:
        PointTo2Dialog(PointTo2* plugin, QList<Sweep *>* sweeps);
        virtual ~PointTo2Dialog();

public slots:
	void closeWindow();
	void deleteSelectedSweep();

	void insertNewSweep();

	void moveUpSelectedSweep();
	void moveDownSelectedSweep();

	void retranslate();

protected:
	//! Initialize the dialog widgets and connect the signals/slots
	virtual void createDialogContent();
    Ui_pointTo2DialogForm* ui;

private slots:
    //void initAboutText();
    //void setLabelsDescriptionText(bool state);

private:
        PointTo2* plugin;

	QDataWidgetMapper*		sweepMapper;
	QList<Sweep *>*			sweeps;
    PropertyBasedTableModel_Point*	sweepTableModel;
    QRegExpValidator*		validatorName;
};

#endif // _POINTTO2DIALOG_HPP_
