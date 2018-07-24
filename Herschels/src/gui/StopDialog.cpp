
//load in the dialog header and .ui file
#include "StopDialog.hpp"
#include "ui_stopDialog.h"
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

StopDialog::StopDialog(Herschels* pluginPtr, HerschelsDialog* herschelsDialogPtr)
        : StelDialog("Stop")
        , plugin(pluginPtr)
        , herschelsDialog(herschelsDialogPtr)

{    
    ui = new Ui_stopDialogForm();

    //I changed the default height. it was 661, now it is 249




}

StopDialog::~StopDialog()
{

	delete ui;
    ui = Q_NULLPTR;
}


/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ********************************************************************* */
void StopDialog::retranslate()
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
void StopDialog::closeWindow()
{
	setVisible(false);
	StelMainView::getInstance().scene()->setActiveWindow(0);
}

//********************************************************************

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Protected Methods
#endif
/* ********************************************************************* */
void StopDialog::createDialogContent()
{
	ui->setupUi(dialog);
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));
    //Uncomment this line if you want to hide the title bar of the dialog
    // ui->TitleBar->hide();
#ifdef Q_OS_WIN
	//Kinetic scrolling for tablet pc and pc
	QList<QWidget *> addscroll;
    addscroll << ui->textBrowser << ui->sweepListView ;
	installKineticScrolling(addscroll);
#endif

	//Now the rest of the actions.
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));

//*************************************************************

    dialog->setFixedHeight(110);
    dialog->setFixedWidth(110);
    connect(ui->pushButton,  SIGNAL(clicked(bool)),    herschelsDialog,   SLOT(resetSweep()));

//****************************************************************

    initAboutText();

}
//**************************************************************

//Don't worry about the "about" text portion, either. Just leave it as is, or delete it. I don't care.

void StopDialog::initAboutText()
{

}

