#ifndef _SWEEPEFFECTSDIALOG_HPP_
#define _SWEEPEFFECTSDIALOG_HPP_

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

class Ui_sweepEffectsDialogForm;

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
class SweepEffectsDialog : public StelDialog//, virtual public QMainWindow
{
	Q_OBJECT
   // QGraphicsScene *m_scene;
public:
        SweepEffectsDialog(Herschels* plugin, HerschelsDialog *herschelsDialogPtr, QList<Sweep *> sweeps);
        virtual ~SweepEffectsDialog();


public slots:
	void closeWindow();

	void retranslate();
//Just a dumb empty function so I can use "Main Script API"
    void updateCircles();


protected:
	//! Initialize the dialog widgets and connect the signals/slots
	virtual void createDialogContent();
    Ui_sweepEffectsDialogForm* ui;

private slots:
    void initAboutText();
    //void setLabelsDescriptionText(bool state);

private:
    void setUpScene(Sweep *sweepEx);
    Herschels* plugin;
    HerschelsDialog* herschelsDialog;

    QList<Sweep *>			sweeps;
    Sweep *	 sweep;
    //Sweep* sweep;

    StelMainScriptAPI * stelMainScript;
    class OcularsGuiPanel * guiPanel;

    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;

    qreal newX = 0;
    qreal newY = 0;
    qreal radiusX = 0;
    qreal radiusY = 0;

    double startRARads, endRARads, deltaRARads;
    double startDecRads, endDecRads, deltaDecRads;

};


#endif // _SWEEPEFFECTSDIALOG_HPP_
