/*
 This is an attempt to create a .cpp file that will run the Herschels plugin dialog
*/

//load in the dialog header and .ui file
#include "HerschelsDialog.hpp"
#include "ui_herschelsDialog.h"
#include "Herschels.hpp"
#include "Oculars.hpp"
#include "CalendarDialog.hpp"

#include "StelApp.hpp"
#include "StelGui.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelMainView.hpp"
#include "StelMainScriptAPI.hpp"
#include "StelTranslator.hpp"
#include "StelActionMgr.hpp"

#include <QAbstractItemModel>
#include <QDataWidgetMapper>
#include <QDebug>
#include <QFrame>
#include <QModelIndex>
#include <QSettings>
#include <QStandardItemModel>
#include <limits>
#include <QRegExpValidator>

HerschelsDialog::HerschelsDialog(Herschels* pluginPtr,
                               QList<Sweep *>* sweeps)
        : StelDialog("Herschels")
        , plugin(pluginPtr)
        , sweepMapper(Q_NULLPTR)

{
    ui = new Ui_herschelsDialogForm();
    calendarDialog= new CalendarDialog(pluginPtr, this, *sweeps);

    //Establish a connection between our sweep data entry and the table model
    this->sweeps = sweeps;
    sweepTableModel = new PropertyBasedTableModel_Point(this);
	Sweep* sweepModel = Sweep::sweepModel();
    sweepTableModel->init(reinterpret_cast<QList<QObject *> *>(sweeps),
                          sweepModel,
                          sweepModel->propertyMap());


    QRegExp nameExp("^\\S.*");
    validatorName = new QRegExpValidator(nameExp, this);
}

HerschelsDialog::~HerschelsDialog()
{
	sweepTableModel->disconnect();

	delete ui;
    ui = Q_NULLPTR;
}

QDate HerschelsDialog::outputDateSelected()
{
    return ui->calendarWidget->selectedDate();
}

void HerschelsDialog::reflectSelectedDate(QDate inputDate)
{
    ui->calendarWidget->setSelectedDate(inputDate);
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ********************************************************************* */
void HerschelsDialog::retranslate()
{
	if (dialog) {
		ui->retranslateUi(dialog);
        initAboutText();
	}
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Slot Methods
#endif
/* ********************************************************************* */
void HerschelsDialog::closeWindow()
{
	setVisible(false);
	StelMainView::getInstance().scene()->setActiveWindow(0);
}

//********************************************************************
//Delete the sweep you are highlighting
void HerschelsDialog::deleteSelectedSweep()
{
	sweepTableModel->removeRows(ui->sweepListView->currentIndex().row(), 1);
	ui->sweepListView->setCurrentIndex(sweepTableModel->index(0, 1));
	plugin->updateLists();
}

void HerschelsDialog::runSelectedSweep()
{
    qWarning() << "running sweep";
    int index = ui->sweepListView->currentIndex().row();
    plugin->selectSweepAtIndex(index);
    plugin->runSweep();

}

void HerschelsDialog::resetSweep()
{
    if (plugin->flagShowSweep)
    {
        plugin->flagPlayMovie = false;
        plugin->restoreDefaults();
        ui->timeRate->setValue(1.00);
        qWarning() << "stopSweep";
    }
}

void HerschelsDialog::submitChanges()
{
    sweepMapper->submit();
    qWarning() << "print name to test submission";
    qWarning() << ui->sweepName->text();
    plugin->updateLists();

}

void HerschelsDialog::enableOcularsPlugin(bool flag)
{
//    plugin->enableOcularsPlugin(flag);
}

void HerschelsDialog::projectSelectedSweep(int checkState)
{
    bool b = checkState != Qt::Unchecked;
    plugin->setFlagProjectSweeps(b);
}

//Insert a new sweep
void HerschelsDialog::insertNewSweep()
{
	sweepTableModel->insertRows(sweepTableModel->rowCount(), 1);
	ui->sweepListView->setCurrentIndex(sweepTableModel->index(sweepTableModel->rowCount() - 1, 1));
    qWarning() << "adding new Sweep";
    qWarning() << ui->sweepName->text();
    qWarning() << sweepTableModel->data(ui->sweepListView->currentIndex()).toString();

/*    QList<Sweep*> sweeps;
    sweeps = QList<Sweep *>();
    qWarning() << "Sweep List generated";
    int selectedSweepIndex = ui->sweepListView->currentIndex().row();
    Sweep *sweep = sweeps[selectedSweepIndex];
    qWarning() << sweep->date();
*/
}

void HerschelsDialog::enableMovieMode()
{
    plugin->movieMode();
}

void HerschelsDialog::moveUpSelectedSweep()
{
    int index = ui->sweepListView->currentIndex().row();
	if (index > 0)
	{
        ui->sweepListView->setCurrentIndex(sweepTableModel->index(index-1, 1));
        plugin->decrementSweepIndex();
        plugin->updateLists();
        qWarning() << "change sweep up";
    //	sweepTableModel->moveRowUp(index);
    //	plugin->updateLists();
    }
}

void HerschelsDialog::moveDownSelectedSweep()
{
	int index = ui->sweepListView->currentIndex().row();
	if (index >= 0 && index < sweepTableModel->rowCount() - 1)
	{
        ui->sweepListView->setCurrentIndex(sweepTableModel->index(index+1, 1));
        plugin->incrementSweepIndex();
        plugin->updateLists();
        qWarning() << "change sweep down";
//		sweepTableModel->moveRowDown(index);
    //	plugin->updateLists();
    }
}

void HerschelsDialog::setSweepIndex(int row)
{
    ui->sweepListView->setCurrentIndex(sweepTableModel->index(row, 1));
}

void HerschelsDialog::pointToSelectedSweep()
{
    int index = ui->sweepListView->currentIndex().row();
    plugin->selectSweepAtIndex(index);
    plugin->pointToSweep();
}

void HerschelsDialog::indexToIntToShowDate(QModelIndex index)
{
    qWarning() << "Show date of selected sweep from ListView";
    showDate(index.row());
}

void HerschelsDialog::convertAndChoose(QModelIndex index)
{
    plugin->selectSweepAtIndex(index.row());
}

void HerschelsDialog::showDate(int index)
{
    Sweep* sweep = plugin->sweeps[index];
    QString dateStr = sweep->date();
    QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
    QDate date = dateTime.date();
    ui->calendarWidget->setSelectedDate(date);
}

void HerschelsDialog::submitDate()
{
    plugin->currentDateOnCalendar = ui->calendarWidget->selectedDate();
}



/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Protected Methods
#endif
/* ********************************************************************* */
void HerschelsDialog::createDialogContent()
{
	ui->setupUi(dialog);
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));

    //establish the Sweep model we've set up and connect it to the List View
    ui->sweepListView->setModel(sweepTableModel);

#ifdef Q_OS_WIN
	//Kinetic scrolling for tablet pc and pc
	QList<QWidget *> addscroll;
    addscroll << ui->textBrowser << ui->sweepListView ;
	installKineticScrolling(addscroll);
#endif

	//Now the rest of the actions.
    ui->Tabs->setCurrentIndex(0);
    this->showDate(plugin->selectedSweepIndex);
    //closing the dialog and making it moveable
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));
    //connecting the various events when you select sweeps
    connect(plugin,     SIGNAL(selectedSweepChanged(int)),    this, SLOT(setSweepIndex(int)));
    connect(plugin,     SIGNAL(selectedSweepChanged(int)),    this, SLOT(showDate(int)));
    connect(ui->calendarWidget,   SIGNAL(selectionChanged()),   calendarDialog,   SLOT(syncDates()));
    connect(ui->calendarWidget,    SIGNAL(selectionChanged()),   this,  SLOT(submitDate()));
    connect(ui->sweepListView,   SIGNAL(clicked(QModelIndex)),   this,   SLOT(indexToIntToShowDate(QModelIndex)));

    //make it look like you're throwing the calendar back to the main dialog
    connect(calendarDialog,   SIGNAL(dialogClosed(int)),   ui->Tabs,    SLOT(setCurrentIndex(int)));


//*************************************************************************************************************
//connect the check boxes to flags in the plugin (they check for true and uncheck for false

    connectBoolProperty(ui->enableOcularView,          "Herschels.flagUseOculars");
    connectBoolProperty(ui->sweepEffect,                "Herschels.flagShowSweepEffects");
    connectBoolProperty(ui->originalImageCheck,         "Herschels.flagShowOriginalImage");
    connectBoolProperty(ui->filterSweeps,                "Herschels.flagFilterSweeps");
    connectBoolProperty(ui->projectSweep,                "Herschels.flagProjectSweeps");

   // connectBoolProperty(ui->projectSweep,                   "Herschels.flagProjectSweeps");

//****************************************************************

    qWarning() << "Set Up Buttons";

    // The add & delete buttons (uncomment these lines if you add them back
    //connect(ui->addSweep,          SIGNAL(clicked()), this, SLOT(insertNewSweep()));
    //connect(ui->deleteSweep,       SIGNAL(clicked()), this, SLOT(deleteSelectedSweep()));

    // The play button
    connect(ui->playSweep,          SIGNAL(clicked()), this, SLOT(runSelectedSweep()));

    //The submit button
    connect(ui->submitSweep,        SIGNAL(clicked()), this, SLOT(submitChanges()));

    //The stop button
    connect(ui->stopSweep,          SIGNAL(clicked()), this, SLOT(resetSweep()));

    //The point to button
    connect(ui->pointToSweep,    SIGNAL(clicked()),    this,    SLOT(pointToSelectedSweep()));

    //The movie mode button
    connect(ui->movieMode,      SIGNAL(clicked()),      this,    SLOT(enableMovieMode()));


    //The time rate spinner
    stelMainScript = new StelMainScriptAPI(this);
    connect(ui->timeRate,            SIGNAL(valueChanged(double)),  plugin,  SLOT(setRequestedTimeRate(double)));
    //connect()
    //enable project sweep checkbox
    //connect(ui->projectSweep,       SIGNAL(stateChanged(int)),  this, SLOT(projectSelectedSweep(int)));

    //new calendar dialog button
    connect(ui->openNewCalendar,    SIGNAL(clicked()),     this,      SLOT(openNewDialog()));

    // Validator
	ui->sweepName->setValidator(validatorName);

    initAboutText();

    //The up and down buttons
    connect(ui->pushButtonMoveSweepDown,      SIGNAL(pressed()), this, SLOT(moveDownSelectedSweep()));
    connect(ui->pushButtonMoveSweepUp,    SIGNAL(pressed()), this, SLOT(moveUpSelectedSweep()));

    //enable better connection between selecting sweeps and pointing to them
    connect(ui->sweepListView,   SIGNAL(clicked(QModelIndex)), this, SLOT(convertAndChoose(QModelIndex)));


//Each entry in our menu of sweeps is added through this mapper. The
//mapper will associate each part (like sweepName or sweepStartRA)
//with a particular line in the .ini file. This is how we can
//manipulate the parameters of the sweeps.
	// The sweep mapper
	sweepMapper = new QDataWidgetMapper();
    sweepMapper->setModel(sweepTableModel);
    sweepMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
	sweepMapper->addMapping(ui->sweepName,                  0);
	sweepMapper->addMapping(ui->sweepStartRA,               1);
	sweepMapper->addMapping(ui->sweepEndRA,                 2);
	sweepMapper->addMapping(ui->sweepStartDec,              3);
	sweepMapper->addMapping(ui->sweepEndDec,         	4);
	sweepMapper->addMapping(ui->sweepDate,			5);
	sweepMapper->toFirst();
	connect(ui->sweepListView->selectionModel() , SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
		sweepMapper, SLOT(setCurrentModelIndex(QModelIndex)));
    ui->sweepListView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->sweepListView->setCurrentIndex(sweepTableModel->index(0, 1));

    qWarning() << "ui all mapped to model, buttons all connected";

    //This flashes the calendar dialog in order to quickly set it up and sync everything
    //It is not the best way to do it but I couldn't think of anything else
    calendarDialog->setVisible(true);
    calendarDialog->setVisible(false);

    //change weekend colors
    QTextCharFormat formatSaturday = ui->calendarWidget->weekdayTextFormat(Qt::Saturday);
    QTextCharFormat formatSunday = ui->calendarWidget->weekdayTextFormat(Qt::Sunday);
    formatSaturday.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    formatSunday.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    ui->calendarWidget->setWeekdayTextFormat(Qt::Saturday, formatSaturday);
    ui->calendarWidget->setWeekdayTextFormat(Qt::Sunday, formatSunday);

    onlyPaintSweepDates();
}
//**************************************************************
//Don't worry about the "about" text portion, either. Just leave it as is, or delete it. I don't care.

void HerschelsDialog::initAboutText()
{

}

void HerschelsDialog::stopScript()
{

}

void HerschelsDialog::openNewDialog()
{
    calendarDialog->setVisible(true);
    ui->Tabs->setCurrentIndex(0);
}

void HerschelsDialog::onlyPaintSweepDates()
{
    QMap<QDate, QTextCharFormat> dateToText = ui->calendarWidget->dateTextFormat();
    for (int i=0; i<sweeps->count(); i++)
    {
        Sweep* sweep = plugin->sweeps[i];
        QString dateStr = sweep->date();
        QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
        QDate date = dateTime.date();
        QTextCharFormat dateFormat = dateToText.value(date);
        dateFormat.setForeground(QBrush(Qt::red, Qt::SolidPattern));
        ui->calendarWidget->setDateTextFormat(date, dateFormat);
    }

}
