#ifndef _CALENDARDIALOG_HPP_
#define _CALENDARDIALOG_HPP_

#include <QObject>
//#include "CCD.hpp"
#include "Oculars.hpp"
#include "OcularsGuiPanel.hpp"
#include "HerschelsDialog.hpp"

#include "PropertyBasedTableModel_Point.hpp"
#include "StelDialog.hpp"
#include "StelStyle.hpp"
#include "Sweep.hpp"
#include "StelMainScriptAPI.hpp"

#include <QMainWindow>
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QTimer>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

/*#include "Telescope.hpp"
#include "Lens.hpp"
*/
class HerschelsDialog;
class Ui_calendarDialogForm;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QDoubleValidator;
class QIntValidator;
class QRegExpValidator;
class QModelIndex;
class QStandardItemModel;
QT_END_NAMESPACE

class Herschels;

//! @ingroup pointto2
class CalendarDialog : public StelDialog//, virtual public QMainWindow
{
	Q_OBJECT
   // QGraphicsScene *m_scene;
public:
        CalendarDialog(Herschels* plugin, HerschelsDialog* herschelsDialogPtr, QList<Sweep *> sweeps);
        virtual ~CalendarDialog();
        QDate outputDateSelected();
public slots:
	void closeWindow();
    void findSweeps();
    void syncDates();
	void retranslate();
    void selectSweep(QModelIndex ind);
    void paintSweepDates();
    void connectChangedDate();
//Just an empty function so I can use "Main Script API"

signals:
    void dialogClosed(int);

protected:
	//! Initialize the dialog widgets and connect the signals/slots
	virtual void createDialogContent();
    Ui_calendarDialogForm* ui;

private slots:
    void initAboutText();
    //void setLabelsDescriptionText(bool state);
    void showDate(int index);

private:
    Herschels* plugin;
    HerschelsDialog* herschelsDialog;

    QList<Sweep *>			sweeps;
    Sweep *	 sweep;
    //Sweep* sweep;
    QList<QString> sweepsFound;

    StelMainScriptAPI * stelMainScript;

};


#endif // _CALENDARDIALOG_HPP_
