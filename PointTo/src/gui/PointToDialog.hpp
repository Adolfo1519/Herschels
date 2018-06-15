/*
 This is the header file for an attempt to write an interactive PointTo plugin
 */

#ifndef _POINTTODIALOG_HPP_
#define _POINTTODIALOG_HPP_

#include <QObject>
/*#include "CCD.hpp"
#include "Ocular.hpp"
*/
#include "PropertyBasedTableModel.hpp"
#include "StelDialog.hpp"
#include "StelStyle.hpp"
#include "Sweep.hpp"
/*#include "Telescope.hpp"
#include "Lens.hpp"
*/

class Ui_pointToDialogForm;		//hopefully once Kouxudong creates the xml file, I can fill this part in

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QDoubleValidator;
class QIntValidator;
class QRegExpValidator;
class QModelIndex;
class QStandardItemModel;
QT_END_NAMESPACE

class PointTo;

//! @ingroup oculars
class PointToDialog : public StelDialog
{
	Q_OBJECT

public:
	PointToDialog(PointTo* plugin, QList<Sweep *>* sweeps);
	virtual ~PointToDialog();

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
    Ui_pointToDialogForm* ui;

private slots:
	void initAboutText();
	void setLabelsDescriptionText(bool state);

private:
	PointTo* plugin;

	QDataWidgetMapper*		sweepMapper;
	QList<Sweep *>*			sweeps;
	PropertyBasedTableModel*	sweepTableModel;
    QRegExpValidator*		validatorName;
};

#endif // _POINTTODIALOG_HPP_
