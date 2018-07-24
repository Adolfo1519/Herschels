#ifndef _STOPDIALOG_HPP_
#define _STOPDIALOG_HPP_

#include <QObject>
//#include "CCD.hpp"
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

class Ui_stopDialogForm;

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
class StopDialog : public StelDialog//, virtual public QMainWindow
{
	Q_OBJECT
   // QGraphicsScene *m_scene;
public:
        StopDialog(Herschels* plugin, HerschelsDialog *herschelsDialogPtr);
        virtual ~StopDialog();


public slots:
	void closeWindow();

	void retranslate();


protected:
	//! Initialize the dialog widgets and connect the signals/slots
	virtual void createDialogContent();
    Ui_stopDialogForm* ui;

private slots:
    void initAboutText();
    //void setLabelsDescriptionText(bool state);

private:
    Herschels* plugin;
    HerschelsDialog* herschelsDialog;


    StelMainScriptAPI * stelMainScript;

};


#endif // _STOPDIALOG_HPP_
