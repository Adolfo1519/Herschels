/*
 This is an attempt to create a .cpp file that will run the PointTo2 plugin dialog
*/

//#include "Oculars.hpp"
//load in the dialog header and .ui file
#include "PointTo2Dialog.hpp"
#include "ui_pointTo2Dialog.h"
#include "PointTo2.hpp"
#include "StelApp.hpp"
#include "StelGui.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelMainView.hpp"
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

PointTo2Dialog::PointTo2Dialog(PointTo2* pluginPtr,
               QList<Sweep *>* sweeps)
        : StelDialog("PointTo2")
	, plugin(pluginPtr)
	, sweepMapper(Q_NULLPTR)

{
    ui = new Ui_pointTo2DialogForm();
	this->sweeps = sweeps;
    sweepTableModel = new PropertyBasedTableModel_Point(this);
	Sweep* sweepModel = Sweep::sweepModel();
    QList<QObject *> * sweeps_obj = reinterpret_cast<QList<QObject *> *>(sweeps);
    sweepTableModel->init(sweeps_obj,
			    sweepModel,
			    sweepModel->propertyMap());
	
	QRegExp nameExp("^\\S.*");
    validatorName = new QRegExpValidator(nameExp, this);
}

PointTo2Dialog::~PointTo2Dialog()
{
	sweepTableModel->disconnect();
	
	delete ui;
	ui = Q_NULLPTR;
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ********************************************************************* */
void PointTo2Dialog::retranslate()
{
	if (dialog) {
		ui->retranslateUi(dialog);
        //initAboutText();
	}
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Slot Methods
#endif
/* ********************************************************************* */
void PointTo2Dialog::closeWindow()
{
	setVisible(false);
	StelMainView::getInstance().scene()->setActiveWindow(0);
}

//********************************************************************
//Delete the sweep you are highlighting
void PointTo2Dialog::deleteSelectedSweep()
{
	sweepTableModel->removeRows(ui->sweepListView->currentIndex().row(), 1);
	ui->sweepListView->setCurrentIndex(sweepTableModel->index(0, 1));
	plugin->updateLists();
}

//Insert a new sweep
void PointTo2Dialog::insertNewSweep()
{
	sweepTableModel->insertRows(sweepTableModel->rowCount(), 1);
	ui->sweepListView->setCurrentIndex(sweepTableModel->index(sweepTableModel->rowCount() - 1, 1));
}

void PointTo2Dialog::moveUpSelectedSweep()
{
	int index = ui->sweepListView->currentIndex().row();
	if (index > 0)
	{
		sweepTableModel->moveRowUp(index);
		plugin->updateLists();
	}
}

void PointTo2Dialog::moveDownSelectedSweep()
{
	int index = ui->sweepListView->currentIndex().row();
	if (index >= 0 && index < sweepTableModel->rowCount() - 1)
	{
		sweepTableModel->moveRowDown(index);
		plugin->updateLists();
	}
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Protected Methods
#endif
/* ********************************************************************* */
void PointTo2Dialog::createDialogContent()
{
	ui->setupUi(dialog);
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));
	ui->sweepListView->setModel(sweepTableModel);
	
#ifdef Q_OS_WIN
	//Kinetic scrolling for tablet pc and pc
	QList<QWidget *> addscroll;
	addscroll << ui->textBrowser << ui->sweepListView << ui->ccdListView << ui->ocularListView << ui->lensListView;
	installKineticScrolling(addscroll);
#endif
	
	//Now the rest of the actions.
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));

//*************************************************************
//This stuff doesn't seem necessary, but we'll see

/*	connectBoolProperty(ui->checkBoxControlPanel,          "Oculars.flagGuiPanelEnabled");
	connectBoolProperty(ui->checkBoxInitialFOV,            "Oculars.flagInitFOVUsage");
	connectBoolProperty(ui->checkBoxInitialDirection,      "Oculars.flagInitDirectionUsage");
	connectBoolProperty(ui->checkBoxResolutionCriterion,   "Oculars.flagShowResolutionCriterions");
	connectBoolProperty(ui->requireSelectionCheckBox,      "Oculars.flagRequireSelection");
	connectBoolProperty(ui->limitStellarMagnitudeCheckBox, "Oculars.flagLimitMagnitude");
	connectBoolProperty(ui->hideGridsLinesCheckBox,        "Oculars.flagHideGridsLines");
	connectBoolProperty(ui->scaleImageCircleCheckBox,      "Oculars.flagScaleImageCircle");
	connectBoolProperty(ui->semiTransparencyCheckBox,      "Oculars.flagSemiTransparency");
	connectBoolProperty(ui->checkBoxDMSDegrees,            "Oculars.flagDMSDegrees");
	connectBoolProperty(ui->checkBoxTypeOfMount,           "Oculars.flagAutosetMountForCCD");
	connectDoubleProperty(ui->arrowButtonScaleDoubleSpinBox, "Oculars.arrowButtonScale");
*/

//****************************************************************

	// The add & delete buttons
	connect(ui->addSweep,          SIGNAL(clicked()), this, SLOT(insertNewSweep()));
	connect(ui->deleteSweep,       SIGNAL(clicked()), this, SLOT(deleteSelectedSweep()));
	
	// Validators
	ui->sweepName->setValidator(validatorName);
	
    //initAboutText();

	connect(ui->pushButtonMoveSweepUp,      SIGNAL(pressed()), this, SLOT(moveUpSelectedSweep()));
	connect(ui->pushButtonMoveSweepDown,    SIGNAL(pressed()), this, SLOT(moveDownSelectedSweep()));
//    connect(ui->pushButtonMoveSweepUp,      SIGNAL(pressed()),

//Each entry in our menu of sweeps is added through this mapper. The
//mapper will associate each part (like sweepName or sweepStartRA) 
//with a particular line in the .ini file. This is how we can 
//manipulate the parameters of the sweeps. 
	// The sweep mapper
	sweepMapper = new QDataWidgetMapper();
    sweepMapper->setModel(sweepTableModel);
	sweepMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
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

//**************************************************************
/*void PointTo2Dialog::setLabelsDescriptionText(bool state)
{
	if (state)
	{
		// TRANSLATORS: tFOV for binoculars (tFOV = True Field of View)
		ui->labelFOV->setText(q_("tFOV:"));
		// TRANSLATORS: Magnification factor for binoculars
		ui->labelFL->setText(q_("Magnification factor:"));
		ui->labelFS->setText(q_("Diameter:"));
	}
	else
	{
		ui->labelFOV->setText(q_("aFOV:"));
		ui->labelFL->setText(q_("Focal length:"));
		ui->labelFS->setText(q_("Field stop:"));
	}
}
*/ //Don't worry about any of this, I don't think we will use it

//*************************************************************
//Don't worry about the "about" text portion, either. Just leave it as is, or delete it. I don't care. 
/*
void PointTo2Dialog::initAboutText()
{
	// Regexp to replace {text} with an HTML link.
	QRegExp a_rx = QRegExp("[{]([^{]*)[}]");

	//BM: Most of the text for now is the original contents of the About widget.
	QString html = "<html><head><title></title></head><body>";

	html += "<h2>" + q_("Oculars Plug-in") + "</h2><table width=\"90%\">";
	html += "<tr width=\"30%\"><td><strong>" + q_("Version") + ":</strong></td><td>" + OCULARS_PLUGIN_VERSION + "</td></tr>";
	html += "<tr><td><strong>" + q_("License") + ":</strong></td><td>" + OCULARS_PLUGIN_LICENSE + "</td></tr>";
	html += "<tr><td><strong>" + q_("Author") + ":</strong></td><td>Timothy Reaves &lt;treaves@silverfieldstech.com&gt;</td></tr>";
	html += "<tr><td rowspan=5><strong>" + q_("Contributors") + ":</strong></td><td>Bogdan Marinov</td></tr>";
	html += "<tr><td>Pawel Stolowski (" + q_("Barlow lens feature") + ")</td></tr>";
	html += "<tr><td>Alexander Wolf</td></tr>";
	html += "<tr><td>Rumen G. Bogdanovski &lt;rumen@skyarchive.org&gt;</td></tr>";
	html += "<tr><td>Georg Zotti</td></tr>";
	html += "</table>";

	//Overview
	html += "<h3>" + q_("Overview") + "</h3>";

	html += "<p>" + q_("This plugin is intended to simulate what you would see through an eyepiece.  This configuration dialog can be used to add, modify, or delete eyepieces and telescopes, as well as CCD Sensors.  Your first time running the app will populate some samples to get your started.") + "</p>";
	html += "<p>" + q_("You can choose to scale the image you see on the screen.") + " ";
	html +=         q_("This is intended to show you a better comparison of what one eyepiece/telescope combination will be like when compared to another.") + " ";
	html +=         q_("The same eyepiece in two different telescopes of differing focal length will produce two different exit pupils, changing the view somewhat.") + " ";
	html +=         q_("The trade-off of this is that, with the image scaled, a large part of the screen can be wasted.") + " ";
	html +=         q_("Therefore I recommend that you leave it off, unless you feel you have a need of it.") + "</p>";
	html += "<p>" + q_("You can toggle a crosshair in the view.  Ideally, I wanted this to be aligned to North.  I've been unable to do so.  So currently it aligns to the top of the screen.") + "</p>";
	html += "<p>" + QString(q_("You can toggle a Telrad finder; this can only be done when you have not turned on the Ocular view.  This feature draws three concentric circles of 0.5%1, 2.0%1, and 4.0%1, helping you see what you would expect to see with the naked eye through the Telrad (or similar) finder.")).arg(QChar(0x00B0)) + "</p>";
	html += "<p>" + q_("If you find any issues, please let me know.  Enjoy!") + "</p>";
*/

//****************************************************************
//If we can, let's avoid keyboard shortcuts for now as well.

/*
	//Keys
	html += "<h3>" + q_("Hot Keys") + "</h3>";
	html += "<p>" + q_("The plug-in's key bindings can be edited in the Keyboard shortcuts editor (F7).") + "</p>";

	StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
	Q_ASSERT(gui);
	StelActionMgr* actionMgr = StelApp::getInstance().getStelActionManager();
	Q_ASSERT(actionMgr);
	StelAction* actionOcular = actionMgr->findAction("actionShow_Ocular");
	Q_ASSERT(actionOcular);
	StelAction* actionMenu = actionMgr->findAction("actionShow_Ocular_Menu");
	Q_ASSERT(actionMenu);
	QKeySequence ocularShortcut = actionOcular->getShortcut();
	QString ocularString = ocularShortcut.toString(QKeySequence::NativeText);
	ocularString = ocularString.toHtmlEscaped();
	if (ocularString.isEmpty())
		ocularString = q_("[no key defined]");
	QKeySequence menuShortcut = actionMenu->getShortcut();
	QString menuString = menuShortcut.toString(QKeySequence::NativeText);
	menuString = menuString.toHtmlEscaped();
	if (menuString.isEmpty())
		menuString = q_("[no key defined]");

	html += "<ul>";
	html += "<li>";
	html += QString("<strong>%1:</strong> %2").arg(ocularString).arg(q_("Switches on/off the ocular overlay."));
	html += "</li>";
	
	html += "<li>";
	html += QString("<strong>%1:</strong> %2").arg(menuString).arg(q_("Opens the pop-up navigation menu."));
	html += "</li>";

	html += "<li>";
	html += QString("<strong>%1:</strong> %2").arg("Alt+M").arg(q_("Rotate reticle pattern of the eyepiece clockwise."));
	html += "</li>";

	html += "<li>";
	html += QString("<strong>%1:</strong> %2").arg("Shift+Alt+M").arg(q_("Rotate reticle pattern of the eyepiece сounterclockwise."));
	html += "</li>";

	html += "</ul>";

	html += "<h3>" + q_("Links") + "</h3>";
	html += "<p>" + QString(q_("Support is provided via the Github website.  Be sure to put \"%1\" in the subject when posting.")).arg("Oculars plugin") + "</p>";
	html += "<p><ul>";
	// TRANSLATORS: The text between braces is the text of an HTML link.
	html += "<li>" + q_("If you have a question, you can {get an answer here}.").toHtmlEscaped().replace(a_rx, "<a href=\"https://groups.google.com/forum/#!forum/stellarium\">\\1</a>") + "</li>";
	// TRANSLATORS: The text between braces is the text of an HTML link.
	html += "<li>" + q_("Bug reports and feature requests can be made {here}.").toHtmlEscaped().replace(a_rx, "<a href=\"https://github.com/Stellarium/stellarium/issues\">\\1</a>") + "</li>";
	// TRANSLATORS: The text between braces is the text of an HTML link.
	html += "<li>" + q_("If you want to read full information about this plugin and its history, you can {get info here}.").toHtmlEscaped().replace(a_rx, "<a href=\"http://stellarium.sourceforge.net/wiki/index.php/Oculars_plugin\">\\1</a>") + "</li>";
	html += "</ul></p></body></html>";

	QString htmlStyleSheet(gui->getStelStyle().htmlStyleSheet);
	ui->textBrowser->document()->setDefaultStyleSheet(htmlStyleSheet);

	ui->textBrowser->setHtml(html);
*/
}
