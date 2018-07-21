
//load in the dialog header and .ui file
#include "SweepEffectsDialog.hpp"
#include "ui_sweepEffectsDialog.h"
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

SweepEffectsDialog::SweepEffectsDialog(Herschels* pluginPtr,
                                       HerschelsDialog* herschelsDialogPtr,
                                       QList<Sweep *> sweeps)
        : StelDialog("SweepEffects")
        , plugin(pluginPtr)
        , herschelsDialog(herschelsDialogPtr)

{    
    ui = new Ui_sweepEffectsDialogForm();

    //I changed the default height. it was 661, now it is 249

    this->sweeps = sweeps;



}

SweepEffectsDialog::~SweepEffectsDialog()
{

	delete ui;
    ui = Q_NULLPTR;
}

void SweepEffectsDialog::updateCircles()
{
    StelCore *core = StelApp::getInstance().getCore();

    QBrush nightBrush(Qt::darkBlue);
    QPen outlinePen(Qt::white);
    outlinePen.setWidth(2);

    const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
    QString currentlyAt;
    Vec2i centerScreen(prj->getViewportPosX() + prj->getViewportWidth() / 2,
               prj->getViewportPosY() + prj->getViewportHeight() / 2);
    Vec3d centerPosition;
    prj->unProject(centerScreen[0], centerScreen[1], centerPosition);
    double cx, cy;
    StelUtils::rectToSphe(&cx,&cy,core->j2000ToEquinoxEqu(core->altAzToJ2000(centerPosition,StelCore::RefractionOff),StelCore::RefractionOff));//centerPosition);

    //core->equinoxEquToJ2000(centerPosition, StelCore::RefractionOff)); // Calculate RA/DE (J2000.0) and show it...
    double currentX = cx;//plugin->cx;
    double currentY = cy;//plugin->cy;
    if (startRARads > 3.14159265) {startRARads-=2*3.14159265;}
    newX = (scene->width())*(currentX - startRARads)/deltaRARads;
    newY = ((scene->height())*(currentY - startDecRads)/deltaDecRads)-(radiusY/2.0)-(ui->refreshSweep->height()/2.0);

    //qWarning() << "current location is " << currentX << ", " << currentY << ": " << currentX*180/3.14159 << ", " << currentY*180/3.14159;
    //qWarning() << "starting positions were " << startRARads << ", " << startDecRads;
    //qWarning() << "window parameters are " << deltaRARads << ", " << deltaDecRads;
    //qWarning() << "NewX and NewY are " << newX << ", " << newY;

    ellipse->setPos(newX, newY);

}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ********************************************************************* */
void SweepEffectsDialog::retranslate()
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
void SweepEffectsDialog::closeWindow()
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
void SweepEffectsDialog::createDialogContent()
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
    scene = new QGraphicsScene(dialog);
    ui->sweepView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->sweepView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->sweepView->setScene(scene);
    qWarning() << "Setting up the Sweep Effects Dialog";
    QBrush blackBrush(Qt::black);

    QBrush nightBrush(Qt::darkBlue);
    QPen outlinePen(Qt::black);
    QPen ocularPen(Qt::white);
    outlinePen.setWidth(2);

    scene->setBackgroundBrush(blackBrush);

    dialog->setFixedHeight(249+ui->refreshSweep->height());
    dialog->setFixedWidth(650);

    //get box sweep parameters from Herschels
    int index = plugin->getSelectedSweepIndex();
    sweep = sweeps[index];

    int screenHeight = 249; //(dialog->height()/2) - (ui->refreshSweep->height())/2.0
    int screenWidth = 650; //dialog->width()

    scene->setSceneRect(0, 0, screenWidth, screenHeight);//-ui->refreshSweep->height(), screenWidth, screenHeight);
    qWarning() << "The scene size is " << scene->width() << ", " << scene->height();

    setUpScene(sweep);

    newX = -radiusX/2.0;//-(dialogWidth);
    newY = 0;//radiusY/2.0-ui->refreshSweep->height();//(-dialog->height()/2.0) + radiusY/2.0-ui->refreshSweep->height();//dialogHeight;

    ellipse = scene->addEllipse(newX, newY, radiusX, radiusY, ocularPen, nightBrush);

//****************************************************************

    initAboutText();
    connect(ui->refreshSweep,   SIGNAL(pressed()),  herschelsDialog, SLOT(resetSweep()));

}
//**************************************************************

//Don't worry about the "about" text portion, either. Just leave it as is, or delete it. I don't care.

void SweepEffectsDialog::initAboutText()
{

}

void SweepEffectsDialog::setUpScene(Sweep*  sweepEx)
{
    StelCore *core = StelApp::getInstance().getCore();
    StelMovementMgr * stelMovementMgr=core->getMovementMgr();

    startRARads = StelUtils::getDecAngle(sweepEx->startRA());
    endRARads = StelUtils::getDecAngle(sweepEx->endRA());
    deltaRARads = endRARads - startRARads;
    if (deltaRARads < -3.14159)
    {
        startRARads -= 2.0*3.14159265;
        deltaRARads = endRARads - startRARads;

    }
    startDecRads = StelUtils::getDecAngle(sweepEx->startDec());
    endDecRads = StelUtils::getDecAngle(sweepEx->endDec());
    deltaDecRads = endDecRads-startDecRads;

    double fovRads = (stelMovementMgr->getAimFov())*3.14159265/180.0;
    radiusX = (scene->width())*fovRads/(std::abs(deltaRARads));
    radiusY = (scene->height())*fovRads/(std::abs(deltaDecRads));
    qWarning() << "ellipse parameters are " << radiusX << ", " << radiusY << ": fov is " << fovRads;

}

