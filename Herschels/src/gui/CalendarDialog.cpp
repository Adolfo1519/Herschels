//load in the dialog header and .ui file
#include "CalendarDialog.hpp"

#include "ui_calendarDialog.h"

#include "Herschels.hpp"
#include "Oculars.hpp"

#include "StelApp.hpp"
#include "StelGui.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelMainView.hpp"
#include "StelMainScriptAPI.hpp"
#include "StelMovementMgr.hpp"
#include "StelTranslator.hpp"
#include "StelActionMgr.hpp"
#include "StelUtils.hpp"

#include <QAbstractItemModel>
#include <QDataWidgetMapper>
#include <QDebug>
#include <QFrame>
#include <QModelIndex>
#include <QSettings>
#include <QStandardItemModel>
#include <limits>
#include <QRegExpValidator>
#include <QTextCharFormat>

CalendarDialog::CalendarDialog(Herschels* pluginPtr,
                                       HerschelsDialog* herschelsDialogPtr,
                                       QList<Sweep *> sweeps)
        : StelDialog("Calendar")
        , plugin(pluginPtr)
        , herschelsDialog(herschelsDialogPtr)

{
    ui = new Ui_calendarDialogForm();

    this->sweeps = sweeps;



}

CalendarDialog::~CalendarDialog()
{

	delete ui;
    ui = Q_NULLPTR;
}

QDate CalendarDialog::outputDateSelected()
{
    return ui->calendarWidget->selectedDate();
}

void CalendarDialog::findSweeps()
{
    ui->sweepsFoundView->clear();
    sweepsFound.clear();
    for (int i=0; i<sweeps.count(); i++)
    {
        Sweep* sweep = plugin->sweeps[i];
        QString dateStr = sweep->date();
        QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
        QDate date = dateTime.date();
        if (date == ui->calendarWidget->selectedDate())
        {
            sweepsFound.append(sweep->name());
            ui->sweepsFoundView->addItem(sweep->name());
        }
    }
}

void CalendarDialog::syncDates()
{
//    QDate newdate = herschelsDialog->outputDateSelected();
    ui->calendarWidget->setSelectedDate(herschelsDialog->outputDateSelected());
}



/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ********************************************************************* */
void CalendarDialog::retranslate()
{
	if (dialog) {
		ui->retranslateUi(dialog);
        initAboutText();
    }
}

void CalendarDialog::selectSweep(QModelIndex ind)
{
    int sweepInd = ind.row();
    QString sweepName = sweepsFound.value(sweepInd);
    for (int i=0; i<sweeps.count(); i++)
    {
        Sweep* sweep = plugin->sweeps[i];
        QString nameStr = sweep->name();

        if (nameStr == sweepName)
        {
            plugin->selectSweepAtIndex(i);
            return;
        }
    }

}

void CalendarDialog::paintSweepDates()
{
    QMap<QDate, QTextCharFormat> dateToText = ui->calendarWidget->dateTextFormat();
    for (int i=0; i<sweeps.count(); i++)
    {
        Sweep* sweep = plugin->sweeps[i];
        QString dateStr = sweep->date();
        QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
        QDate date = dateTime.date();
        QTextCharFormat dateFormat = dateToText.value(date);
        dateFormat.setForeground(QBrush(Qt::red, Qt::SolidPattern));
        ui->calendarWidget->setDateTextFormat(date, dateFormat);
        ui->calendarWidget->headerTextFormat();
       // ui->calendarWidget->set
        //ui->calendarWidget->setDateEditEnabled(true);
    }

}

void CalendarDialog::connectChangedDate()
{
    herschelsDialog->reflectSelectedDate(ui->calendarWidget->selectedDate());
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Slot Methods
#endif
/* ********************************************************************* */
void CalendarDialog::closeWindow()
{
	setVisible(false);
    StelMainView::getInstance().scene()->setActiveWindow(0);
    emit dialogClosed(1);
}

//********************************************************************

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Protected Methods
#endif
/* ********************************************************************* */
void CalendarDialog::createDialogContent()
{
    qWarning() << "creating the Calendar Dialog Content";
	ui->setupUi(dialog);
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));

#ifdef Q_OS_WIN
	//Kinetic scrolling for tablet pc and pc
	QList<QWidget *> addscroll;
    addscroll << ui->textBrowser << ui->sweepListView ;
	installKineticScrolling(addscroll);
#endif

	//Now the rest of the actions.
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->collapseDialog,   SIGNAL(pressed()), this, SLOT(closeWindow()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));

//*************************************************************

    int index = plugin->getSelectedSweepIndex();
    sweep = sweeps[index];
    ui->calendarWidget->setSelectedDate(herschelsDialog->outputDateSelected());
    findSweeps();
    connect(ui->calendarWidget,   SIGNAL(selectionChanged()),   this,   SLOT(findSweeps()));
//****************************************************************

    //qWarning() << "Set Up Buttons";
    initAboutText();
    connect(plugin,     SIGNAL(selectedSweepChanged(int)),    this, SLOT(showDate(int)));
    connect(ui->sweepsFoundView,   SIGNAL(clicked(QModelIndex)),  this, SLOT(selectSweep(QModelIndex)));

    //change weekend colors
    QTextCharFormat formatSaturday = ui->calendarWidget->weekdayTextFormat(Qt::Saturday);
    QTextCharFormat formatSunday = ui->calendarWidget->weekdayTextFormat(Qt::Sunday);
    formatSaturday.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    formatSunday.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    ui->calendarWidget->setWeekdayTextFormat(Qt::Saturday, formatSaturday);
    ui->calendarWidget->setWeekdayTextFormat(Qt::Sunday, formatSunday);
    //ui->calendarWidget->setDateEditEnabled(false);
    //color dates with sweeps
    paintSweepDates();
    connect(ui->calendarWidget,    SIGNAL(selectionChanged()),    this,    SLOT(connectChangedDate()));



}
//**************************************************************

//Don't worry about the "about" text portion, either. Just leave it as is, or delete it. I don't care.

void CalendarDialog::initAboutText()
{

}


void CalendarDialog::showDate(int index)
{
    Sweep* sweep = plugin->sweeps[index];
    QString dateStr = sweep->date();
    QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
    QDate date = dateTime.date();
    ui->calendarWidget->setSelectedDate(date);
}
