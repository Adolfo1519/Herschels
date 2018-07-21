/*
 This is the header file for an attempt to write an interactive Herschels plugin
 */

#ifndef _HERSCHELSDIALOG_HPP_
#define _HERSCHELSDIALOG_HPP_

#include <QObject>
//#include "CCD.hpp"
#include "Oculars.hpp"
#include "OcularsGuiPanel.hpp"

#include "CalendarDialog.hpp"
#include "PropertyBasedTableModel_Point.hpp"
#include "StelDialog.hpp"
#include "StelStyle.hpp"
#include "Sweep.hpp"
#include "StelMainScriptAPI.hpp"
/*#include "Telescope.hpp"
#include "Lens.hpp"
*/

class Ui_herschelsDialogForm;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QDoubleValidator;
class QIntValidator;
class QRegExpValidator;
class QModelIndex;
class QStandardItemModel;
QT_END_NAMESPACE

class Herschels;
class CalendarDialog;
//! @ingroup pointto2
class HerschelsDialog : public StelDialog
{
	Q_OBJECT

public:
        HerschelsDialog(Herschels* plugin, QList<Sweep *>* sweeps);
        virtual ~HerschelsDialog();
        QDate outputDateSelected();
        void reflectSelectedDate(QDate inputDate);

public slots:
	void closeWindow();
	void deleteSelectedSweep();

    void runSelectedSweep();
    void submitChanges();
    void resetSweep();
	void insertNewSweep();

    void enableMovieMode();

    void enableOcularsPlugin(bool flag);

    void projectSelectedSweep(int flag);

	void moveUpSelectedSweep();
	void moveDownSelectedSweep();

    void setSweepIndex(int row);
    void pointToSelectedSweep();
    void indexToIntToShowDate(QModelIndex index);
    void convertAndChoose(QModelIndex index);
    void showDate(int index);
    void submitDate();
    void retranslate();

//Just an empty function so I can use "Main Script API"
    void stopScript();

    //opens the calendar dialog
    void openNewDialog();
    void onlyPaintSweepDates();


protected:
	//! Initialize the dialog widgets and connect the signals/slots
	virtual void createDialogContent();
    Ui_herschelsDialogForm* ui;

private slots:
    void initAboutText();
    //void setLabelsDescriptionText(bool state);

private:
        Herschels* plugin;
    CalendarDialog* calendarDialog;
	QDataWidgetMapper*		sweepMapper;
	QList<Sweep *>*			sweeps;
    PropertyBasedTableModel_Point*	sweepTableModel;
    QRegExpValidator*		validatorName;
    StelMainScriptAPI * stelMainScript;
    class OcularsGuiPanel * guiPanel;

};


#endif // _HERSCHELSDIALOG_HPP_
