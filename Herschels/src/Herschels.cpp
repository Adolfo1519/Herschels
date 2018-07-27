/*
This is the Herschels plugin main class. It can just take in essentially the corners of a box in
the sky and point to the center of that box.
*/
#include "Herschels.hpp"
#include "SweepTools.hpp"
#include "Sweep.hpp"
#include "HerschelsDialog.hpp"
#include "CalendarDialog.hpp"
#include "SweepEffectsDialog.hpp"
#include "StopDialog.hpp"

#include "Ocular.hpp"
#include "Oculars.hpp"

#include "GridLinesMgr.hpp"
#include "LabelMgr.hpp"
#include "ConstellationMgr.hpp"
#include "AsterismMgr.hpp"
#include "SkyGui.hpp"
#include "StelActionMgr.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelLocaleMgr.hpp"
#include "StelLocation.hpp"
#include "StelMainScriptAPI.hpp"
#include "StelMainView.hpp"
#include "StelModuleMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelObserver.hpp"
#include "StelPainter.hpp"
#include "StelProjector.hpp"
#include "StelSkyDrawer.hpp"
#include "StelSphereGeometry.hpp"
#include "StelTextureMgr.hpp"
#include "StelTranslator.hpp"
#include "SolarSystem.hpp"
#include "StelUtils.hpp"
#include "StelPropertyMgr.hpp"
#include "LandscapeMgr.hpp"
#include "LabelMgr.hpp"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QGraphicsWidget>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QSignalMapper>
#include <QTextStream>


#include <cmath>

extern void qt_set_sequence_auto_mnemonic(bool b);

static QSettings *settings; //!< The settings as read in from the ini file.

/* ****************************************************************************************************************** */
/*#if 0
#pragma mark -
#pragma mark StelModuleMgr Methods
#endif
*//* ****************************************************************************************************************** */
//! This method is the one called automatically by the StelModuleMgr just
//! after loading the dynamic library
StelModule* HerschelsStelPluginInterface::getStelModule() const
{
    return new Herschels();
}

StelPluginInfo HerschelsStelPluginInterface::getPluginInfo() const
{
    // Allow to load the resources when used as a static plugin
    Q_INIT_RESOURCE(Herschels);

    StelPluginInfo info;
    info.id = "Herschels";
    info.displayedName = N_("Herschels");
    info.authors = "Adolfo Carvalho";
    info.contact = "asc5@rice.edu";
    info.description = N_("pans to a particular place in the sky");
    return info;
}


/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Instance Methods
#endif
/* ****************************************************************************************************************** */
//This step initializes all of the members of Herschels with a particular value as part of
//defining the constructor
Herschels::Herschels():
    selectedSweepIndex(-1),
    arrowButtonScale(1.5),
    usageMessageLabelID(-1),
    flagCardinalPointsMain(false),
    flagAdaptationMain(false),
    flagSemiTransparency(false),
    flagDMSDegrees(false),
    flagFilterSweeps(false),
    flagShowSweep(false),
    flagUseOculars(true),
    flagProjectSweeps(false),
    flagShowSweepEffects(false),
    flagPlayMovie(false),
    flagShowOriginalImage(true),
    sweepsSignalMapper(Q_NULLPTR),
    herschelsDialog(Q_NULLPTR),
    sweepEffectsDialog(Q_NULLPTR),
    ready(false),
    actionShowSweep(Q_NULLPTR),
    actionConfiguration(Q_NULLPTR),
    actionMenu(Q_NULLPTR),
    actionSweepIncrement(Q_NULLPTR),
    actionSweepDecrement(Q_NULLPTR)
{
    // Font size is 14
    font.setPixelSize(StelApp::getInstance().getBaseFontSize()+1);

    sweeps = QList<Sweep *>();
    regions = QList<SphericalConvexPolygon>();
    sweepsSignalMapper = new QSignalMapper(this);

    setObjectName("Herschels");
    conf = StelApp::getInstance().getSettings();

#ifdef Q_OS_MAC
    qt_set_sequence_auto_mnemonic(true);
#endif
}

Herschels::~Herschels()
{
    delete herschelsDialog;
    herschelsDialog = Q_NULLPTR;

    delete sweepEffectsDialog;
    sweepEffectsDialog = Q_NULLPTR;

    delete stopDialog;
    stopDialog = Q_NULLPTR;

    qDeleteAll(sweeps);
    sweeps.clear();

    //qDeleteAll(regions);
    //regions.clear();
}

QSettings* Herschels::appSettings()
{
    return settings;
}


/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ****************************************************************************************************************** */
bool Herschels::configureGui(bool show)
{
    if (show)
    {
        herschelsDialog->setVisible(true);
    }

    return ready;
}

void Herschels::deinit()
{
    // update the ini file.
    settings->remove("sweep");
    int index = 0;
    foreach(Sweep* sweep, sweeps)
    {
        sweep->writeToSettings(settings, index);
        index++;
    }

    settings->setValue("sweep_count", sweeps.count());

  //  StelCore *core = StelApp::getInstance().getCore();
 //   StelSkyDrawer *skyDrawer = core->getSkyDrawer();

    disconnect(this, SIGNAL(selectedSweepChanged(int)), this, SLOT(updateSweep()));
    //disconnect(&StelApp::getInstance(), SIGNAL(colorSchemeChanged(const QString&)), this, SLOT(setStelStyle(const QString&)));
    disconnect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslateGui()));
}

//***************************************************************
//Use this as the function that will point where we want
//! Draw any parts on the screen which are for our module
/*void Herschels::point()
{
    if (selectedSweepIndex > sweeps.count())
    {
        qWarning() << "Herschels: the selected sweep index of "
               << selectedSweepIndex << " is greater than the sensor count of "
               << sweeps.count() << ". Module disabled!";
        ready = false;
    }

    if (ready)
    {
        if (selectedSweepIndex > -1)
        {
            runSweep();
        }
    }
}*/
//****************************************************************
//! Determine which "layer" the plugin's drawing will happen on.
double Herschels::getCallOrder(StelModuleActionName actionName) const
{
    double order = 1000.0; // Very low priority, unless we interact.

    if (actionName==StelModule::ActionHandleKeys ||
        actionName==StelModule::ActionHandleMouseMoves ||
        actionName==StelModule::ActionHandleMouseClicks)
    {
        // Make sure we are called before MovementMgr (we need to even call it once!)
        order = StelApp::getInstance().getModuleMgr().getModule("StelMovementMgr")->getCallOrder(actionName) - 1.0;
    }
    else if (actionName==StelModule::ActionDraw)
    {
        order = GETSTELMODULE(LabelMgr)->getCallOrder(actionName) + 100.0;
    }

    return order;
}

void Herschels::handleMouseClicks(class QMouseEvent* event)
{
    //if (!flagProjectSweeps) {movementManager->handleMouseClicks(event);}
    if (flagProjectSweeps)
    {
        if (flagUseOculars && flagShowSweep)
        {
            return;
        }
        //foreach (const SphericalConvexPolygon poly, regions) {qWarning() << regions(i);}
        //qWarning() << regions(0);
        for (int sweepInd = 0; sweepInd < sweeps.count(); sweepInd++)
        {
            SphericalConvexPolygon currentRegion;
            currentRegion = regions.at(sweepInd);
            Vec3d v;
            const StelProjectorP prj = StelApp::getInstance().getCore()->getProjection(StelCore::FrameJ2000);
            int x, y; x = event->x(); y = event->y();
            if (prj->unProject(x,y,v))
            {
                Vec3d win;
                prj->project(v,win);

                float dx = x - win.v[0];
                float dy = y - win.v[1];
                prj->unProject(x+dx, y+dy, v);

            }

            if (currentRegion.contains(v))
            {

                if (regionsVisible.value(sweepInd).checkValid())
                {
                    StelApp::getInstance().getStelObjectMgr().unSelect();
                    event->setAccepted(true);
                    selectSweepAtIndex(sweepInd);
                }
                //qWarning() << selectedSweep()->name();
                //qWarning() << (regionsVisible.value(sweepInd).checkValid());//(currentRegion)
                return;
            }
        }
    }

}

void Herschels::handleKeys(QKeyEvent* event)
{
    // We only care about the arrow keys.  This flag tracks that.
    bool consumeEvent = false;

    StelCore *core = StelApp::getInstance().getCore();
    StelMovementMgr *movementManager = core->getMovementMgr();
    if (event->type() == QEvent::KeyPress)
    {
        // Direction and zoom replacements
        switch (event->key())
        {
            case Qt::Key_Left:
                movementManager->turnLeft(true);
                consumeEvent = true;
                break;
            case Qt::Key_Right:
                movementManager->turnRight(true);
                consumeEvent = true;
                break;
            case Qt::Key_Up:
                if (!event->modifiers().testFlag(Qt::ControlModifier))
                {
                    movementManager->turnUp(true);
                }
                consumeEvent = true;
                break;
            case Qt::Key_Down:
                if (!event->modifiers().testFlag(Qt::ControlModifier))
                {
                    movementManager->turnDown(true);
                }
                consumeEvent = true;
                break;
            case Qt::Key_PageUp:
                movementManager->zoomIn(true);
                consumeEvent = true;
                break;
            case Qt::Key_PageDown:
                movementManager->zoomOut(true);
                consumeEvent = true;
                break;
            case Qt::Key_Shift:
                movementManager->moveSlow(true);
                consumeEvent = true;
                break;

        }
    }
    else
    {
        // When a deplacement key is released stop moving
        switch (event->key())
        {
            case Qt::Key_Left:
                movementManager->turnLeft(false);
                consumeEvent = true;
                break;
            case Qt::Key_Right:
                movementManager->turnRight(false);
                consumeEvent = true;
                break;
            case Qt::Key_Up:
                movementManager->turnUp(false);
                consumeEvent = true;
                break;
            case Qt::Key_Down:
                movementManager->turnDown(false);
                consumeEvent = true;
                break;
            case Qt::Key_PageUp:
                movementManager->zoomIn(false);
                consumeEvent = true;
                break;
            case Qt::Key_PageDown:
                movementManager->zoomOut(false);
                consumeEvent = true;
                break;
            case Qt::Key_Shift:
                movementManager->moveSlow(false);
                consumeEvent = true;
                break;
        }
        if (consumeEvent)
        {
            // We don't want to re-center the object; just hold the current position.
            movementManager->setFlagLockEquPos(true);
        }
    }
    if (consumeEvent)
    {
        event->accept();
    }
    else
    {
        event->setAccepted(false);
    }
}

void Herschels::init()
{
    qDebug() << "Herschels plugin - Press ALT-s for configuration.";

    // Load settings from sweeps.ini
    try {
        validateAndLoadIniFile();
        qWarning() << "ini File Loaded";
        // assume all is well
        ready = true;

        //find all of the sweeps we have on file
        int sweepCount = settings->value("sweep_count", 0).toInt();
        int actualSweepCount = sweepCount;
        for (int index = 0; index < sweepCount; index++)
        {
            Sweep *newSweep = Sweep::sweepFromSettings(settings, index);
            QVariant sweepstartRA = settings->value("sweep/"+QString::number(index)+"/startRA");
            //qWarning() << sweepstartRA;
            if (newSweep != Q_NULLPTR)
            {
                sweeps.append(newSweep);
            }
            else
            {
                actualSweepCount--;
            }
        }
        if (actualSweepCount < 1)
        {
            if (actualSweepCount < sweepCount)
            {
                qWarning() << "The Sweeps ini file appears to be corrupt; delete it.";
            }
            else
            {
                qWarning() << "There are no sweeps defined for the Sweeps plugin; plugin will be disabled.";
            }
            ready = false;
        }
        else
        {
            selectedSweepIndex = 0;
        }
        //initialize our dialogs
        qWarning() << "call to Dialogs";
        herschelsDialog = new HerschelsDialog(this, &sweeps);
        stopDialog = new StopDialog(this, herschelsDialog);
        calendarDialog = new CalendarDialog(this, herschelsDialog, sweeps);

        setStopDialogPos();

        qWarning() << "Attmpt to set up actions";
        initializeActivationActions();
        qWarning() << "actions set up";

        // Add a toolbar button
        try
        {
            StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
            Q_ASSERT(gui);

                // Create action for enable/disable & hook up signals
                //Sweep *sweep = sweeps[selectedSweepIndex];

                toolbarButton = new StelButton(gui->getButtonBar(),
                                   QPixmap(":/sweep/RAS_button_on.png"),
                                   QPixmap(":/sweep/RAS_button_off.png"),
                                   QPixmap(":/sweep/bt_ocular_off.png"),
                                   this->actionConfiguration, true);

                gui->getButtonBar()->addButton(toolbarButton, "065-pluginsGroup");


        }
        catch (std::runtime_error& e)
        {
            qWarning() << "WARNING: unable create toolbar button for Herschels plugin: " << e.what();
        }
        // For historical reasons, name of .ini entry and description of checkbox (and therefore flag name) are reversed.
        setFlagDMSDegrees( ! settings->value("use_decimal_degrees", false).toBool());
        setFlagUseSemiTransparency(settings->value("use_semi_transparency", false).toBool());
        setArrowButtonScale(settings->value("arrow_scale", 1.5).toDouble());


    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: unable to locate sweep.ini file or create a default one for Sweeps plugin: " << e.what();
        ready = false;
    }

    connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslateGui()));
    connect(this, SIGNAL(selectedSweepChanged(int)), this, SLOT(updateSweep()));
    //StelApp& app = StelApp::getInstance();



}

/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Private slots Methods
#endif
/* ******************************************************************************************************************
*/
//Have to figure out exactly what this one is doing, because it might be necessary for aborting sweeps mid-sweep
void Herschels::sweepChanged()
{
    // We only zoom if in Sweep mode.
    if (flagShowSweep)
    {
        // If we are already in Sweep mode, we must reset scalings because run() also resets.
        flagStopSweep = true;
        restoreDefaults();
        run(true);
    }
    else
    {
        pointToSweep();
    }
}

void Herschels::run(bool wantRun)
{
    if (flagShowSweep && selectedSweepIndex == -1)
    {
        flagShowSweep = false;
    }

    if (flagShowSweep && wantRun)
    {

        runSweep();
    }
    else
    {
        restoreDefaults();
    }
}

void Herschels::restore()
{

    StelMainScriptAPI *mainScriptAPI= new StelMainScriptAPI(this);
    mainScriptAPI->goHome();
}


//No idea what this function is doing
void Herschels::updateSweep(void)
{
    //Load OpenGL textures
    StelTexture::StelTextureParams params;
    params.generateMipmaps = true;
}

void Herschels::stopScript(void)
{

}

void Herschels::setStopDialogPos()
{
    StelCore *core = StelApp::getInstance().getCore();
    // Get the X & Y positions
    StelProjector::StelProjectorParams projectorParams = core->getCurrentStelProjectorParams();
    int xPosition = projectorParams.viewportXywh[2] - projectorParams.viewportCenterOffset[0];
    xPosition -= 0;//insetFromRHS;
    int yPosition = projectorParams.viewportXywh[3] - projectorParams.viewportCenterOffset[1];
    yPosition -= 0;//projectorParams.viewportXywh[3]/(4.0);//projectorParams.viewportCenterOffset[1]/2.0; //250;
    QPoint wantedPos = QPoint(xPosition, yPosition);
    stopDialog->handleMovedTo(wantedPos);
}

/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Slots Methods
#endif
/* ****************************************************************************************************************** */
void Herschels::updateLists()
{
    if (sweeps.isEmpty())
    {
        selectedSweepIndex = -1;
        enableSweep(false);
    }
    else
    {
        if (selectedSweepIndex >= sweeps.count())
            selectedSweepIndex = sweeps.count() - 1;

        if (flagShowSweep)
                emit selectedSweepChanged(selectedSweepIndex);

    }
    settings->sync();

}

void Herschels::openSettings(bool toggle)
{
    if (toggle)
    {
        qWarning() << "Will try to open dialog";
        herschelsDialog->setVisible(true);
        qWarning() << "Dialog opened";
    }

}

void Herschels::enableSweep(bool enableSweepMode)
{
    if (enableSweepMode)
    {
        qWarning() << "Sweep Enabled";

        // Check to ensure that we have enough sweeps, as they may have been edited in the config dialog
        if (sweeps.count() == 0)
        {
            selectedSweepIndex = -1;
            qWarning() << "No sweeps found";
            qWarning() << sweeps;
            QList<Sweep* > sweeps2;
            qWarning() << sweeps2;
        }
        else if (sweeps.count() > 0 && selectedSweepIndex == -1)
        {
            selectedSweepIndex = 0;
        }
    }
    if (!ready  || selectedSweepIndex == -1)
    {
        qWarning() << "The Sweeps module has been disabled.";
        return;
    }

    StelCore *core = StelApp::getInstance().getCore();
    LabelMgr* labelManager = GETSTELMODULE(LabelMgr);

    // Toggle the Sweep view on & off. To toggle on, we want to ensure there is a selected object.
    if (!flagShowSweep)
    {
        if (usageMessageLabelID == -1)
        {
            QFontMetrics metrics(font);
            QString labelText = q_("");
            StelProjector::StelProjectorParams projectorParams = core->getCurrentStelProjectorParams();
            int xPosition = projectorParams.viewportCenter[0] + projectorParams.viewportCenterOffset[0];
            xPosition = xPosition - 0.5 * (metrics.width(labelText));
            int yPosition = projectorParams.viewportCenter[1] + projectorParams.viewportCenterOffset[1];
            yPosition = yPosition - 0.5 * (metrics.height());
            const char *tcolor = "#99FF99";
            usageMessageLabelID = labelManager->labelScreen(labelText, xPosition, yPosition,
                                    true, font.pixelSize(), tcolor);
        }
    }
    else
    {
        if (selectedSweepIndex != -1)
        {
            // remove the usage label if it is being displayed.
            hideUsageMessageIfDisplayed();
            flagShowSweep = enableSweepMode;

        }
    }

    emit enableSweepChanged(flagShowSweep);
}

void Herschels::decrementSweepIndex()
{
    selectedSweepIndex--;
    if (selectedSweepIndex == -1)
    {
        selectedSweepIndex = sweeps.count() - 1;
    }
    emit(selectedSweepChanged(selectedSweepIndex));
    settings->sync();

}
//This does not really do anything right now
void Herschels::displayPopupMenu()
{
    QMenu * popup = new QMenu(&StelMainView::getInstance());

    if (flagShowSweep)
    {
        // We are watching a Sweep
        // we want to show all the other sweeps to choose from now
        if (!sweeps.isEmpty())
        {
            popup->addAction(q_("&Previous sweep"), this, SLOT(decrementSweepIndex()));
            popup->addAction(q_("&Next sweep"), this, SLOT(incrementSweepIndex()));
            QMenu* submenu = new QMenu(q_("Select &sweep"), popup);
            int availableSweepCount = 0;
            for (int index = 0; index < sweeps.count(); ++index)
            {
                QString label;
                if (availableSweepCount < 10)
                {
                    label = QString("&%1: %2").arg(availableSweepCount).arg(sweeps[index]->name());
                }
                else
                {
                    label = sweeps[index]->name();
                }
                //BM: Does this happen at all any more?
                QAction* action = Q_NULLPTR;

                if (index == selectedSweepIndex)
                {
                    action->setCheckable(true);
                    action->setChecked(true);
                }
            }
            popup->addMenu(submenu);
            popup->addSeparator();
        }


    }
    else
    {
        // We are not currently in a sweep
        //we want to show the sweeps
        QAction* action = new QAction(q_("Configure &Sweeps"), popup);
        action->setCheckable(true);
        action->setChecked(herschelsDialog->visible());
        connect(action, SIGNAL(triggered(bool)), herschelsDialog, SLOT(setVisible(bool)));
        popup->addAction(action);
        popup->addSeparator();

    }

    popup->exec(QCursor::pos());
    delete popup;
}

void Herschels::retranslateGui()
{

}


void Herschels::incrementSweepIndex()
{
    selectedSweepIndex++;
    if (selectedSweepIndex == sweeps.count())
    {
        selectedSweepIndex = 0;
    }
    emit(selectedSweepChanged(selectedSweepIndex));
    settings->sync();
}

void Herschels::selectSweepAtIndex(int index)
{
    if (index > -2 && index < sweeps.count())
    {
        selectedSweepIndex = index;
        emit(selectedSweepChanged(selectedSweepIndex));
    }
    settings->sync();
}
/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Private Methods
#endif
/* ****************************************************************************************************************** */
void Herschels::initializeActivationActions()
{
    StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    Q_ASSERT(gui);

    QString sweepsGroup = N_("Sweeps");

    actionMenu           = addAction("actionShow_Sweep_Menu",           sweepsGroup, N_("Herschels popup menu"), "displayPopupMenu()", "Alt+s");
    actionConfiguration  = addAction("actionOpen_Herschels_Configuration", sweepsGroup, N_("Herschels plugin configuration"), herschelsDialog, "visible");
    // Select next sweep via keyboard
    addAction("actionShow_Sweep_Increment", sweepsGroup, N_("Select next sweep"), "incrementSweepIndex()", "");
    // Select previous sweep via keyboard
    addAction("actionShow_Sweep_Decrement", sweepsGroup, N_("Select previous sweep"), "decrementSweepIndex()", "");
    connect(this, SIGNAL(selectedSweepChanged(int)),       this, SLOT(sweepChanged()));
    // these connections are now made in the Dialog setup, and they connect to properties!
    connect(sweepsSignalMapper,        SIGNAL(mapped(int)), this, SLOT(selectSweepAtIndex(int)));

}

//******************************************************************
//Use this function to paint the boundaries of the box of our observations

void Herschels::draw(StelCore *core)
{
    StelMovementMgr * stelMovementMgr=core->getMovementMgr();

    if (flagProjectSweeps)
    {
        //regions = QList<SphericalConvexPolygon>();
        int initialIndex = selectedSweepIndex;
        regionsVisible.clear();
        for (int ind = 0; ind<sweeps.count(); ind++)
        {
            selectedSweepIndex = ind;
            paintSweepBounds();
        }
        selectedSweepIndex = initialIndex;
    }
    if (flagUseOculars && flagShowSweep)
    {
        paintText(core);
        showSweepImage();
        //paintSweepImage(core);
        //sweepEffectsDialog->setVisible(true);//setVisible(true);
        //sweepEffectsDialog->updateCircles();
    }
    if (flagShowSweep)
    {
        paintText(core);
    }
    if (flagShowSweepEffects && flagShowSweep)
    {
        sweepEffectsDialog->updateCircles();
        paintText(core);
    }

    if (flagStopSweep) {i = 5000; stelMovementMgr->smoothPan(0.0, 0.0, 0.0, false);}
    else
    {
        hideSweepImage();
    }
}

void Herschels::paintSweepBounds()
{
    StelCore *core = StelApp::getInstance().getCore();
    StelProjector::StelProjectorParams params = core->getCurrentStelProjectorParams();


    //Lens *lens = selectedLensIndex >=0  ? lenses[selectedLensIndex] : Q_NULLPTR;

    const StelProjectorP projector = core->getProjection(StelCore::FrameEquinoxEqu);
    double screenFOV = params.fov;

    Vec2i centerScreen(projector->getViewportPosX() + projector->getViewportWidth() / 2,
               projector->getViewportPosY() + projector->getViewportHeight() / 2);

    // draw sensor rectangle
    if(selectedSweepIndex != -1)
    {
        Sweep *sweep = sweeps[selectedSweepIndex];
        if (sweep)
        {
            //qWarning() << "paint box";

            StelPainter painter(projector);
            painter.setColor(0.77f, 0.14f, 0.16f, 1.0f);
            painter.setFont(font);
            Vec3d pos, screenPos;

            const QString startRA = sweep->startRA();
            const QString endRA = sweep->endRA();
            const QString startDec = sweep->startDec();
            const QString endDec = sweep->endDec();
            //qWarning() << startRA << ", " << startDec;
            double startDecRads = StelUtils::getDecAngle(startDec);
            double endDecRads = StelUtils::getDecAngle(endDec);
            double startRARads = StelUtils::getDecAngle(startRA);
            double endRARads = StelUtils::getDecAngle(endRA);

            double deltaRARads = endRARads - startRARads;
            double deltaDecRads = endDecRads - startDecRads;
            if (deltaRARads < -3.14159)
            {
                startRARads -= 2.0*3.14159265;
                deltaRARads = endRARads - startRARads;
            }
            const double deltaRA = (180.0/3.14159265) * deltaRARads;
            const double deltaDec = (180.0/3.14159265) * deltaDecRads;

            double sweepXRatio = deltaRA/screenFOV;
            double sweepYRatio = deltaDec/screenFOV;

            if (sweepXRatio < 0.0)
            {
                sweepXRatio = -sweepXRatio;
            }
            if (sweepYRatio < 0.0)
            {

                sweepYRatio = -sweepYRatio;
            }


            // As the FOV is based on the narrow aspect of the screen, we need to calculate
            // height & width based soley off of that dimension.
            int aspectIndex = 2;
            if (params.viewportXywh[2] > params.viewportXywh[3])
            {
                aspectIndex = 3;
            }
            float width = params.viewportXywh[aspectIndex] * sweepXRatio * params.devicePixelsPerPixel;
            float height = params.viewportXywh[aspectIndex] * sweepYRatio * params.devicePixelsPerPixel;

            double polarAngle = 0;

            if (width > 0.0 && height > 0.0)
            {
                QPoint a, b;
                QTransform transform = QTransform().translate(centerScreen[0], centerScreen[1]).rotate(-(polarAngle));


                Vec3d startPosTop;
                StelUtils::spheToRect(startRARads,
                                      startDecRads,
                                      startPosTop);

                Vec3d endPosTop;
                StelUtils::spheToRect(endRARads,
                                      startDecRads,
                                      endPosTop);

                Vec3d startPosBot;
                StelUtils::spheToRect(startRARads,
                                      endDecRads,
                                      startPosBot);

                Vec3d endPosBot;
                StelUtils::spheToRect(endRARads,
                                      endDecRads,
                                      endPosBot);

                //qWarning() << startPosTop;
                //qWarning() << pos.set(StelUtils::getDecAngle(endRA))
                //qWarning() << startPosBot;
                //qWarning() << endPosTop;
                //qWarning() << endPosBot;
                painter.setBlending(true);
                painter.setLineSmooth(true);
                StelUtils::spheToRect((endRARads + startRARads)/2.0,
                                      (endDecRads + startDecRads)/2.0,
                                      pos);
                //QMultiMap<QString, SphericalConvexPolygon*> regions;
                SphericalConvexPolygon region = SphericalConvexPolygon(startPosTop, endPosTop, endPosBot, startPosBot);
                //regions.insert(selectedSweepIndex, &region);
                regions.append(region);
                bool flagShouldInclude;
                if (flagFilterSweeps)
                {
                    QString dateStr = sweep->date();
                    QDateTime dateTime = QDateTime::fromString(dateStr,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
                    QDate date = dateTime.date();
                    if ((date.month() == currentDateOnCalendar.month()) && (date.year() == currentDateOnCalendar.year()))
                    {
                        flagShouldInclude = true;
                    }
                    else
                    {
                        flagShouldInclude = false;
                    }
                }
                if (!flagFilterSweeps) {flagShouldInclude = true;}
                if (projector->getBoundingCap().intersects(region) && flagShouldInclude &&//projector->project(pos, screenPos) &&
                     screenPos[0]>projector->getViewportPosX() &&
                        screenPos[0] < projector->getViewportPosX() + projector->getViewportWidth() &&
                            screenPos[1]>projector->getViewportPosY() &&
                                screenPos[1] < projector->getViewportPosY() + projector->getViewportHeight())
                {
                   // qWarning() << "Is the region in the viewport> " << projector->getBoundingCap().intersects(region);
                                  //region.getIntersection(projector->getBoundingCap());

                    painter.drawGreatCircleArc(startPosTop, endPosTop, Q_NULLPTR);
                    //qWarning() << "paint the whole box";
                    painter.drawGreatCircleArc(endPosTop, endPosBot, Q_NULLPTR);
                    painter.drawGreatCircleArc(startPosBot, endPosBot, Q_NULLPTR);
                    painter.drawGreatCircleArc(startPosTop, startPosBot, Q_NULLPTR);
                    painter.drawText(startPosTop, sweep->name());

                    //This is how StelSkyPolygons generates the polygons (vertices is a vector of RA,Dec coords in Vec3d)
                    //skyConvexPolygons.append(SphericalConvexPolygon(vertices[0], vertices[1], vertices[2], vertices[3]));
                    //255, 152, 121
                    float r,g,b, trans;
                    r = 255.f/255.f; g = 152.f/255.f; b = 121.f/255.f; trans = 1.0f;//0.25f;

                    painter.setColor(0.77f, 0.14f, 0.16f, 0.25f);

                    painter.drawSphericalRegion(&region);
                    regionsVisible.insert(selectedSweepIndex, region);
                    //SphericalRegion* region =  new Spher
                    //painter.drawSphericalRegion();
                    //float a = i*M_PI/180;
                    //pos.set(sin(a),cos(a), 0.f);
                    //painter.drawGreatCircleArc(pos, Vec3d(pos[0], pos[1], h), Q_NULLPTR);

                }

                painter.setBlending(false);
                painter.setLineSmooth(false);

                /*float ratioLimit = 0.25f;
                if (sweepXRatio>=ratioLimit || sweepYRatio>=ratioLimit)
                {
                    // draw cross at center
                    float cross = width>height ? height/50.f : width/50.f;
                    a = transform.map(QPoint(0.f, -cross));
                    b = transform.map(QPoint(0.f, cross));
                    painter.drawLine2d(a.x(), a.y(), b.x(), b.y());
                    a = transform.map(QPoint(-cross, 0.f));
                    b = transform.map(QPoint(cross, 0.f));
                    painter.drawLine2d(a.x(), a.y(), b.x(), b.y());
                    // calculate coordinates of the center and show it
                    Vec3d centerPosition;
                    projector->unProject(centerScreen[0], centerScreen[1], centerPosition);
                    double cx, cy;
                    QString cxt, cyt;
                    StelUtils::rectToSphe(&cx,&cy,core->equinoxEquToJ2000(centerPosition, StelCore::RefractionOff)); // Calculate RA/DE (J2000.0) and show it...
                    bool withDecimalDegree = StelApp::getInstance().getFlagShowDecimalDegrees();
                    if (withDecimalDegree)
                    {
                        cxt = StelUtils::radToDecDegStr(cx, 5, false, true);
                        cyt = StelUtils::radToDecDegStr(cy);
                    }
                    else
                    {
                        cxt = StelUtils::radToHmsStr(cx, true);
                        cyt = StelUtils::radToDmsStr(cy, true);
                    }
                    // Coordinates of center of visible field of view for CCD (red rectangle)
                    QString coords = QString("%1: %2/%3").arg(qc_("RA/Dec (J2000.0) of cross", "abbreviated in the plugin")).arg(cxt).arg(cyt);
                    a = transform.map(QPoint(-width/2.0, height/2.0 + 5.f));
                    painter.drawText(a.x(), a.y(), coords, -(polarAngle));
                    // Dimensions of visible field of view for CCD (red rectangle)
                    a = transform.map(QPoint(-width/2.0, -height/2.0 - fontSize*1.2f));
                    //painter.drawText(a.x(), a.y(), getDimensionsString(fovX, fovY), -(ccd->chipRotAngle() + polarAngle));

                    // Horizontal and vertical scales of visible field of view for CCD (red rectangle)
                    //TRANSLATORS: Unit of measure for scale - arcseconds per pixel
                    QString unit = q_("\"/px");
                    QString scales = QString("%1%3 %4 %2%3")
                            .arg(QString::number(fovX*3600*ccd->binningX()/ccd->resolutionX(), 'f', 4))
                            .arg(QString::number(fovY*3600*ccd->binningY()/ccd->resolutionY(), 'f', 4))
                            .arg(unit)
                            .arg(QChar(0x00D7));
                    a = transform.map(QPoint(width/2.0 - painter.getFontMetrics().width(scales), -height/2.0 - fontSize*1.2f));
                    painter.drawText(a.x(), a.y(), scales, -(ccd->chipRotAngle() + polarAngle));
                    // Rotation angle of visible field of view for CCD (red rectangle)
                    QString angle = QString("%1%2").arg(QString::number(ccd->chipRotAngle(), 'f', 1)).arg(QChar(0x00B0));
                    a = transform.map(QPoint(width/2.0 - painter.getFontMetrics().width(angle), height/2.0 + 5.f));
                    painter.drawText(a.x(), a.y(), angle, -(ccd->chipRotAngle() + polarAngle));
                }*/
            }
        }
    }

}

void Herschels::paintSweepImage(StelCore *core)
{

    if (flagShowOriginalImage && flagShowSweep)
    {
        const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
        StelPainter painter(prj);

        // Get the X & Y positions
        StelProjector::StelProjectorParams projectorParams = core->getCurrentStelProjectorParams();
        int xPosition = 20;//projectorParams.viewportXywh[2] - projectorParams.viewportCenterOffset[0];
        xPosition -= 0;//insetFromRHS;
        int yPosition = projectorParams.viewportXywh[3] - projectorParams.viewportCenterOffset[1];
        yPosition -= projectorParams.viewportXywh[3]/(4.0);//projectorParams.viewportCenterOffset[1]/2.0; //250;

        QString filename = ":/sweep/"+sweeps[selectedSweepIndex]->name();
        filename.append(".png");
        filename.replace(" ", "");
        //Change this to change to scale of the image that appears
        float scale = 1.0;
        QString sweepImagePath = StelFileMgr::findFile(filename);
        qWarning() << sweepImagePath;
        if (sweepImagePath.isEmpty())
        {
            qWarning() << "image " << filename << " not found";
            flagFileNotFound = true;
        }
        if (!(sweepImagePath.isEmpty())){flagFileNotFound=false;}
        if (!flagFileNotFound)
        {
            QPixmap pixmap(filename);
            if (pixmap.isNull())
            {
                qWarning() << "image " << filename << " not found";
                flagFileNotFound = true;
            }
            //qWarning() << "painting image at " << xPosition << ", " << yPosition;
            tex = StelMainView::getInstance().scene()->addPixmap(pixmap.scaled(pixmap.size()*scale));
            tex->setPos(xPosition, yPosition);
        }

    }
    //else {tex->hide();}
}

void Herschels::showSweepImage()
{
    if (flagShowOriginalImage)
    {
        tex->show();
    }
}

void Herschels::hideSweepImage()
{
    if (!flagShowOriginalImage)
    {
        tex->hide();
        //qWarning() << "concealing image";
    }
}


//*****************************************************************
void Herschels::establishDefaults()
{
    qWarning() << "establishing defaults";
    StelCore *core = StelApp::getInstance().getCore();
    StelMainScriptAPI *mainScriptAPI = new StelMainScriptAPI(this);
    StelMovementMgr * stelMovementMgr=core->getMovementMgr();

    defaultDate = mainScriptAPI->getDate("utc");
    defaultMovementFactor = stelMovementMgr->getMovementSpeedFactor();
    initialFOV = stelMovementMgr->getInitFov();



}

void Herschels::restoreDefaults()
{
    flagStopSweep = true;
    flagShowSweep = false;
    setFlagShowOriginalImage(false);
    hideSweepImage();
    StelCore *core = StelApp::getInstance().getCore();
    StelMainScriptAPI *mainScriptAPI = new StelMainScriptAPI(this);
    StelMovementMgr * stelMovementMgr=core->getMovementMgr();
    stelMovementMgr->turnDown(false);
    stelMovementMgr->turnUp(false);
    stelMovementMgr->smoothPan(10.0, 0.0, 10.0, false);
    StelProperty* turnOnOcular = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.enableOcular");
    turnOnOcular->setValue(false);
    StelProperty* turnOffSelReq = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.flagRequireSelection");
    turnOffSelReq->setValue(true);
    mainScriptAPI->setDate(defaultDate);
    stelMovementMgr->setMovementSpeedFactor(defaultMovementFactor);
    stelMovementMgr->setInitFov(initialFOV);
    stelMovementMgr->setEquatorialMount(false);
    mainScriptAPI->setTimeRate(1.0);
    LabelMgr * labelMgr = GETSTELMODULE(LabelMgr);
    labelMgr->setLabelShow(label, false);
    if (flagUseOculars) {flagProjectSweeps = true;}
    sweepEffectsDialog->setVisible(false);
    stopDialog->setVisible(false);
    mainScriptAPI->goHome();
}

//***********************************************************************
/* Note: this function works based on time entirely. It is set up so that
 * stellarium points to the correct location according to where the coordinates
 * (in Caroline's records) would have been on the date the sweep was done. Then,
 * the sweep runs based on how long it would take to paint the box in the sky
 * at a constant rate as the Earth turns.
 *
 * Any changes to status, such as movement with arrow keys and changes to
 * the ocular or field of view are not accounted for in the course of the
 * sweep. The sweep's runtime is calculated before-hand and will not change
 * to make up for changes along the way.
 *
 * Also, the "Stop Sweep" button does not quite stop it instantly. It will
 * give the user control to select another sweep, but the motion will not
 * fully stop until the view is done panning to the top or bottom of the
 * sweep. When it reverses direction, it assesses whether or not there is
 * a stop command.
 *
 */
void Herschels::runSweep()
{
    StelApp::getInstance().getStelObjectMgr().unSelect();
    //save the settings before running the sweep
    establishDefaults();
    flagStopSweep=false;
    setStopDialogPos();

    stopDialog->setVisible(true);
    //initialize core and sweep, and extract elements of the sweep
    StelCore *core = StelApp::getInstance().getCore();
    Sweep *sweep = sweeps[selectedSweepIndex];
    QString name = sweep->name();
    QString startRA = sweep->startRA();
    QString endRA = sweep->endRA();
    QString startDec = sweep->startDec();
    QString endDec = sweep->endDec();
    QString date = sweep->date();
    //convert date from QString to QDateTime in the format Stellarium prefers
    QDateTime dateTime = QDateTime::fromString(date,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");
    qWarning() << dateTime;

    //initialize the location and position
    StelLocation location = core->getCurrentLocation();
    StelObserver * position = new StelObserver(location);

    //qWarning() << "initial trans matrix is " << (core->matVsop87ToJ2000.transpose()*position->getRotEquatorialToVsop87()).toString();

    //set up label and initialize MainScriptAPI and Movement Manager
    LabelMgr * labelMgr = GETSTELMODULE(LabelMgr);
    labelMgr->deleteLabel(label);
    StelMainScriptAPI *mainScriptAPI = new StelMainScriptAPI(this);
    StelMovementMgr * stelMovementMgr=core->getMovementMgr();

    //Set date and location
    herschelsDialog->setVisible(false);
    mainScriptAPI->setObserverLocation("Bath, Britain (UK)", 0.1);
    mainScriptAPI->wait(1.0);

    location = core->getCurrentLocation();
    position = new StelObserver(location);
    double jdSecond = core->computeDeltaT(StelUtils::qDateTimeToJd(dateTime));
    double locSidTime = (position->getHomePlanet()->getSiderealTime(StelUtils::qDateTimeToJd(dateTime), StelUtils::qDateTimeToJd(dateTime) + (jdSecond/86400.0))+position->getCurrentLocation().longitude)*(3.14159265/180.0);
    QDateTime time = SweepTools::calcTime(startRA, dateTime, locSidTime);
    QString times = time.toString("yyyy'-'MM'-'dd'T'hh:mm:ss.z");
    core->setJD(StelUtils::qDateTimeToJd(time));
    //Display label
    QString color = "#FFFFFF";

    QString lbl = name+" " + time.date().toString("yyyy'-'MM'-'dd");//times.remove('T20:40:00.0');
    label = labelMgr->labelScreen(lbl, 20, 20, false, 20, color);
    labelMgr->setLabelShow(label, true);
    mainScriptAPI->wait(2);

    //set our location
    double dec_past, ra_past;
    Vec3d startingPos;
    StelUtils::spheToRect(StelUtils::getDecAngle(startRA),
                          StelUtils::getDecAngle(startDec),
                          startingPos);
    //Vec3d convertedPos = core->j2000ToEquinoxEqu(startingPos,StelCore::RefractionOff);
    qWarning() << "default date" << defaultDate;
    qWarning() << "current date is " << mainScriptAPI->getDate();

    Vec3d convertedPos = core->equinoxEquToJ2000(startingPos,StelCore::RefractionOn);

    StelUtils::rectToSphe(&ra_past, &dec_past, convertedPos);

    //qWarning() << "suggested aimUp is "<< stelMovementMgr->mountFrameToJ2000(Vec3d(0., 0., 1.));
//    qWarning() << "initial coords are " << startingPos << " vs " << convertedPos;
 //   qWarning() << "converted coords are" << StelUtils::radToHmsStr(ra_past) << ", " << StelUtils::radToDmsStr(dec_past);
 //   qWarning() << core->j2000ToEquinoxEqu(startingPos, StelCore::RefractionOn);

    stelMovementMgr->zoomTo(1.0,2);
    Vec3d aimUp = stelMovementMgr->mountFrameToJ2000(Vec3d(0.,0.,1.0));
    stelMovementMgr->moveToJ2000(convertedPos, aimUp);

    qWarning() << startRA << ", " << startDec;

    //set up oculars if we choose to use them
    //The Herschel telescope parameters are hard coded. If you change their location
    //in the Ocular plugin you have to change it in this conditional as well
    if (flagUseOculars)
    {
        flagProjectSweeps = false;
        //flagShowSweepEffects = true;
        StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.flagGuiPanelEnabled")->setValue(false);
        StelProperty* turnOffSelReq = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.flagRequireSelection");
        turnOffSelReq->setValue(false);

        StelProperty* chooseTel = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.selectedTelescopeIndex");
        StelProperty* chooseOc = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.selectedOcularIndex");
        chooseTel->setValue(2);
        chooseOc->setValue(7);
        StelProperty* turnOnOcular = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.enableOcular");
        turnOnOcular->setValue(true);
    }
    else if (!flagUseOculars)
    {
        StelProperty* turnOnOcular = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.enableOcular");
        turnOnOcular->setValue(false);

        StelProperty* turnOffSelReq = StelApp::getInstance().getStelPropertyManager()->getProperty("Oculars.flagRequireSelection");
        turnOffSelReq->setValue(true);
    }
    qWarning() << "call to Sweep Effects Dialog";

    sweepEffectsDialog = new SweepEffectsDialog(this, herschelsDialog, sweeps);
    if (flagShowSweepEffects)
    {
        sweepEffectsDialog->setVisible(true);
        stopDialog->setVisible(false);
    }

    flagShowSweep = true;
    paintSweepImage(core);

    double fieldOfView = stelMovementMgr->getAimFov();
    mainScriptAPI->wait(1.0);
    double fovRad = fieldOfView*3.14159265/180.0;

    //establish the geometry and time of the sweep
    double timeDif = SweepTools::timeDifSecs(startRA,endRA);
    double rate = mainScriptAPI->getTimeRate();
    qWarning() << startDec;
    qWarning()<< endDec;

    double startDecRads = StelUtils::getDecAngle(startDec);
    double endDecRads = StelUtils::getDecAngle(endDec);
    double startRARads = StelUtils::getDecAngle(startRA);
    double endRARads = StelUtils::getDecAngle(endRA);

    double deltaRARads = endRARads - startRARads;
    if (deltaRARads < -3.14159)
    {
        startRARads -= 2.0*3.14159265;
        deltaRARads = endRARads - startRARads;

    }
    const double deltaDecRads = endDecRads - startDecRads;
    const double deltaRADegs = deltaRARads*(180.0/3.1459265);
    const double deltaDecDegs = deltaDecRads*(180.0/3.14159265);

    Vec3d initial, final, initialAltAz, finalAltAz;
    StelUtils::spheToRect(startRARads, startDecRads, initial);
    StelUtils::spheToRect(endRARads, endDecRads, final);
    initialAltAz = core->equinoxEquToAltAz(initial, StelCore::RefractionOn);
    finalAltAz = core->equinoxEquToAltAz(final, StelCore::RefractionOn);
    double startAzRads, startAltRads, endAzRads, endAltRads;
    StelUtils::rectToSphe(&startAzRads, &startAltRads, initialAltAz);
    StelUtils::rectToSphe(&endAzRads, &endAltRads, finalAltAz);

 //   qWarning() << "the altitude ranges from " << startAltRads*180/3.14159 << "to " << endAltRads*180/3.14159;


//    qWarning() << "delta RA Degs = " << deltaRADegs;
//    qWarning() << "delta Dec Degs = " << deltaDecDegs;

    double secsPerSweep = SweepTools::timePerSweep(startRARads,
                                                   endRARads,
                                                   timeDif,
                                                   fovRad);
    double numSweeps = timeDif/secsPerSweep;
   // long double sweepsUp = std::floor(numSweeps/2.0);
   // long double sweepsDown = std::floor((numSweeps+1.0)/2.0);
    qWarning() << "total time is " << timeDif;
    qWarning() << "time per sweeps is " << secsPerSweep;

    //run the actual sweep
    for (i=0; i<(numSweeps/2.0); i++)
    {
        if (!flagStopSweep)
        {
           // stelMovementMgr->setEquatorialMount(true);
            double raMotion = 0;
            if (startAltRads > (85.0*3.14159265/180))
            {
                raMotion = -deltaRADegs/numSweeps;
                qWarning() << "deltaRA is " << deltaRADegs;
                stelMovementMgr->setEquatorialMount(true);
            }
            float step = secsPerSweep/(100.0*rate);
            qWarning() << "step set at " << step;
            for (float tim = 0; tim < (secsPerSweep/rate);)
            {
                stelMovementMgr->smoothPan(raMotion, deltaDecDegs, (secsPerSweep/rate), true);
                qWarning() << "moving up";
                mainScriptAPI->wait(step);
                stelMovementMgr->smoothPan(raMotion, deltaDecDegs, (secsPerSweep/rate), false);
                tim += step;
                qWarning() << "in loop, step is " << step;
                qWarning() << "time so far is " << tim;
                if (flagStopSweep) {tim = (secsPerSweep/rate)+100.0;}
            }
            if (flagStopSweep)
            {
                i = numSweeps;
            }
            for (float tim = 0; tim < (secsPerSweep/rate);)
            {
                stelMovementMgr->smoothPan(raMotion, -deltaDecDegs, (secsPerSweep/rate), true);
                qWarning() << "moving Down";
                mainScriptAPI->wait(step);
                stelMovementMgr->smoothPan(raMotion, -deltaDecDegs, (secsPerSweep/rate), false);
                tim += step;
                qWarning() << "in loop, step is " << step;
                qWarning() << "time so far is " << tim;
                if (flagStopSweep) {tim = (secsPerSweep/rate)+100.0;}
            }
//            stelMovementMgr->smoothPan(raMotion, -deltaDecDegs, secsPerSweep/rate, true);
//            //qWarning() << "move up";
//            //qWarning() << "motion time = " << secsPerSweep;
//            mainScriptAPI->wait(secsPerSweep/rate);
//            stelMovementMgr->smoothPan(raMotion, -deltaDecDegs, secsPerSweep/rate, false);

        }
        if (flagStopSweep)
        {
            i = numSweeps;
        }
    }




   /* Vec3d  targetVector;
    StelUtils::spheToRect(endRARads, endDecRads, targetVector);
    //StelUtils::strToVec3f();
    //Vec3d  aimUp;
    StelUtils::spheToRect(0.0, 3.14159265/2.0, aimUp);
    //mainScriptAPI->setMountMode("azimuthal");
    mainScriptAPI->wait(0.5);
    //stelMovementMgr->moveToAltAzi(targetVector, aimUp, 2.0);
    //mainScriptAPI->setMountMode("equatorial");
*/
    //reestablish the defaults
    mainScriptAPI->wait(0.5);
    labelMgr->setLabelShow(label, false);
    //mainScriptAPI->setGuiVisible(true);
    stopDialog->setVisible(false);
    herschelsDialog->setVisible(true);

    restoreDefaults();
    mainScriptAPI->goHome();

}

void Herschels::movieMode()
{
    flagPlayMovie = true;
    for (int ind = 0; ind < sweeps.count(); ind++)
    {
        if (flagPlayMovie)
        {
            pointToSweep();
            runSweep();
            if (!flagPlayMovie) {ind = 100000;}
            if (selectedSweepIndex == sweeps.count()-1)
            {
                restoreDefaults();
            }
            else
            {
                incrementSweepIndex();
            }

        }

    }
}

void Herschels::pointToSweep()
{
    Sweep *sweep = sweeps[selectedSweepIndex];
    QString startRA = sweep->startRA();
    QString endRA = sweep->endRA();
    QString startDec = sweep->startDec();
    QString endDec = sweep->endDec();
    QString date = sweep->date();

    QString midRA = SweepTools::findMidRA(startRA,endRA);
    QString midDec = SweepTools::findMidDec(startDec,endDec);

    StelMainScriptAPI *mainScriptAPI = new StelMainScriptAPI(this);

    mainScriptAPI->moveToRaDec(midRA,midDec);


}


//*****************************************************************

void Herschels::enableOcularsPlugin(const bool flag)
{
    //ocularsPlugin = new  ;
    qWarning() << "attempting to turn on Oculars, toggled = " << flag;
    //ocularsPlugin->actionShowOcular;
    ocularsPlugin->setFlagRequireSelection(false);
    qWarning() << "turn off selection requirement";

    ocularsPlugin->enableOcular(flag);

}

//************************************************************************
void Herschels::paintText(const StelCore* core)
{
    const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
    StelPainter painter(prj);

    // Get the current information
    Sweep *sweep = Q_NULLPTR;
    if(selectedSweepIndex != -1)
    {
        sweep = sweeps[selectedSweepIndex];
    }

    // set up the color and the GL state
    //painter.setColor(0.8f, 0.48f, 0.f, 1.f);
    painter.setColor(1.f, 1.f, 1.f, 1.f);
    painter.setBlending(true);

    // Get the X & Y positions, and the line height
    painter.setFont(font);
    QString widthString = "MMMMMMMMMMMMMMMMMMM";
    float insetFromRHS = painter.getFontMetrics().width(widthString);
    StelProjector::StelProjectorParams projectorParams = core->getCurrentStelProjectorParams();
    int xPosition = 20;//projectorParams.viewportXywh[2] - projectorParams.viewportCenterOffset[0];
    xPosition -= 0;//insetFromRHS;
    int yPosition = projectorParams.viewportXywh[3] - projectorParams.viewportCenterOffset[1];
    yPosition -= projectorParams.viewportXywh[3]/(4.0);//projectorParams.viewportCenterOffset[1]/2.0; //250;
    const int lineHeight = painter.getFontMetrics().height();


    // The Sweep
    if (flagShowSweep && sweep!=Q_NULLPTR)
    {
        QString sweepNumberLabel;
        QString name = sweep->name();
        QString sweepI18n = q_("Sweep");
        if (name.isEmpty())
        {
            sweepNumberLabel = QString("%1 #%2").arg(sweepI18n).arg(selectedSweepIndex);
        }
        else
        {
            sweepNumberLabel = name; //QString("%1 #%2: %3").arg(sweepI18n).arg(selectedSweepIndex).arg(name);
        }
        // The name of the sweep could be really long.
        if (name.length() > widthString.length())
        {
            xPosition -= (insetFromRHS / 2.0);
        }
        QString date = sweep->date();
        date.remove(QRegExp("T20:40:00.0"));
        //date.
        painter.drawText(xPosition, yPosition, sweepNumberLabel);
        yPosition-=lineHeight;
        painter.drawText(xPosition, yPosition, "Date: " + date);
        yPosition-=lineHeight;
        painter.drawText(xPosition, yPosition, "Start RA: " + sweep->startRA());
        yPosition-=lineHeight;
        painter.drawText(xPosition, yPosition, "End RA: " + sweep->endRA());
        yPosition-=lineHeight;
        painter.drawText(xPosition, yPosition, "Top Dec: " + sweep->startDec());
        yPosition-=lineHeight;
        painter.drawText(xPosition, yPosition, "Bottom Dec: " + sweep->endDec());
        yPosition-=lineHeight;

        QString currentlyAt;
        Vec2i centerScreen(prj->getViewportPosX() + prj->getViewportWidth() / 2,
                   prj->getViewportPosY() + prj->getViewportHeight() / 2);
        Vec3d centerPosition;
        prj->unProject(centerScreen[0], centerScreen[1], centerPosition);
        //double cx, cy;
        QString cxt, cyt;
        StelUtils::rectToSphe(&cx,&cy,core->j2000ToEquinoxEqu(core->altAzToJ2000(centerPosition,StelCore::RefractionOff),StelCore::RefractionOff));//centerPosition);
        //core->equinoxEquToJ2000(centerPosition, StelCore::RefractionOff)); // Calculate RA/DE (J2000.0) and show it...
        cxt = StelUtils::radToHmsStr(cx, true);
        cyt = StelUtils::radToDmsStr(cy, true);

        currentlyAt = cxt + ", " + cyt;
        painter.drawText(xPosition, yPosition, "Currently At: " + currentlyAt);
        yPosition-=lineHeight;

//        QString str;
//        QTextStream oss(&str);
//        QString infoString;

//        infoString += sweepNumberLabel + "<br>";
//        infoString += "Date: " + date + "<br>";
//        infoString += "Start RA: " + sweep->startRA() + "<br>";
//        infoString += "End RA: " + sweep->endRA() + "<br>";
//        infoString += "Top Dec: " + sweep->startDec() + "<br>";
//        infoString += "Bottom Dec: " + sweep->endDec() + "<br>";
//        infoString += "currently At: " + currentlyAt + "<br>";
//        oss << infoString;



    }

}

void Herschels::validateAndLoadIniFile()
{
    // Insure the module directory exists
    StelFileMgr::makeSureDirExistsAndIsWritable(StelFileMgr::getUserDir()+"/modules/Herschels");
    StelFileMgr::Flags flags = (StelFileMgr::Flags)(StelFileMgr::Directory|StelFileMgr::Writable);
    QString sweepIniPath = StelFileMgr::findFile("modules/Herschels/", flags) + "sweep.ini";
    qWarning() << sweepIniPath;
    if (sweepIniPath.isEmpty())
        return;

    // If the ini file does not already exist, create it from the resource in the QT resource
    if(!QFileInfo(sweepIniPath).exists())
    {
        QFile src(":/sweep/default_sweep.ini");
        if (!src.copy(sweepIniPath))
        {
            qWarning() << "Herschels::validateIniFile cannot copy default_sweep.ini resource to [non-existing] "
                      + sweepIniPath;
        }
        else
        {
            qDebug() << "Herschels::validateIniFile copied default_sweep.ini to " << QDir::toNativeSeparators(sweepIniPath);
            // The resource is read only, and the new file inherits this, so set write-able.
            QFile dest(sweepIniPath);
            dest.setPermissions(dest.permissions() | QFile::WriteOwner);
        }
    }
    else
    {
        qDebug() << "Herschels::validateIniFile sweep.ini exists at: " << QDir::toNativeSeparators(sweepIniPath) << ". Checking version...";
        QSettings settings(sweepIniPath, QSettings::IniFormat);
        float sweepsVersion = settings.value("sweeps_version", 0.0).toFloat();
        qWarning() << "Herschels::validateIniFile found existing ini file version " << sweepsVersion;

        if (sweepsVersion < MIN_HERSCHELS_INI_VERSION)
        {
            qWarning() << "Herschels::validateIniFile existing ini file version " << sweepsVersion
                   << " too old to use; required version is " << MIN_HERSCHELS_INI_VERSION << ". Copying over new one.";
            // delete last "old" file, if it exists
            QFile deleteFile(sweepIniPath + ".old");
            deleteFile.remove();

            // Rename the old one, and copy over a new one
            QFile oldFile(sweepIniPath);
            if (!oldFile.rename(sweepIniPath + ".old"))
            {
                qWarning() << "Herschels::validateIniFile cannot move sweep.ini resource to sweep.ini.old at path  " + QDir::toNativeSeparators(sweepIniPath);
            }
            else
            {
                qWarning() << "Herschels::validateIniFile sweep.ini resource renamed to sweep.ini.old at path  " + QDir::toNativeSeparators(sweepIniPath);
                QFile src(":/sweep/default_sweep.ini");
                if (!src.copy(sweepIniPath))
                {
                    qWarning() << "Herschels::validateIniFile cannot copy default_sweep.ini resource to [non-existing] " + QDir::toNativeSeparators(sweepIniPath);
                }
                else
                {
                    qDebug() << "Herschels::validateIniFile copied default_sweep.ini to " << QDir::toNativeSeparators(sweepIniPath);
                    // The resource is read only, and the new file inherits this...  make sure the new file
                    // is writable by the Stellarium process so that updates can be done.
                    QFile dest(sweepIniPath);
                    dest.setPermissions(dest.permissions() | QFile::WriteOwner);
                }
            }
        }
    }
    qWarning() << "use ini file to setup Settings";
    settings = new QSettings(sweepIniPath, QSettings::IniFormat, this);
    qWarning() << "ini ready to go";
    //Sweep *newSweep = Sweep::sweepFromSettings(settings, index);
    QVariant sweepstartRA = settings->value("sweep/"+QString::number(3)+"/startRA");
    qWarning() << sweepstartRA;


}

//*****************************************************************
void Herschels::hideUsageMessageIfDisplayed()
{
    if (usageMessageLabelID > -1)
    {
        LabelMgr *labelManager = GETSTELMODULE(LabelMgr);
        labelManager->setLabelShow(usageMessageLabelID, false);
        labelManager->deleteLabel(usageMessageLabelID);
        usageMessageLabelID = -1;
    }
}

Sweep* Herschels::selectedSweep()
{
    if (selectedSweepIndex >= 0 && selectedSweepIndex < sweeps.count())
    {
        return sweeps[selectedSweepIndex];
    }
    return Q_NULLPTR;
}

QMenu* Herschels::addSweepSubmenu(QMenu* parent)
{
    Q_ASSERT(parent);

    QMenu *submenu = new QMenu(q_("&Sweep"), parent);
    submenu->addAction(q_("&Previous sweep"), this, SLOT(decrementSweepIndex()));
    submenu->addAction(q_("&Next sweep"), this, SLOT(incrementSweepIndex()));
    submenu->addSeparator();
    submenu->addAction(q_("None"), this, SLOT(disableSweep()));

    for (int index = 0; index < sweeps.count(); ++index)
    {
        QString label;
        if (index < 10)
        {
            label = QString("&%1: %2").arg(index).arg(sweeps[index]->name());
        }
        else
        {
            label = sweeps[index]->name();
        }
        QAction* action = submenu->addAction(label, sweepsSignalMapper, SLOT(map()));
        if (index == selectedSweepIndex)
        {
            action->setCheckable(true);
            action->setChecked(true);
        }
        sweepsSignalMapper->setMapping(action, index);
    }
    return submenu;
}
void Herschels::setFlagDMSDegrees(const bool b)
{
    flagDMSDegrees = b;
    settings->setValue("use_decimal_degrees", !b);
    settings->sync();
    emit flagDMSDegreesChanged(b);
}

bool Herschels::getFlagDMSDegrees() const
{
    return flagDMSDegrees;
}


void Herschels::setFlagUseSemiTransparency(const bool b)
{
    flagSemiTransparency = b;
    settings->setValue("use_semi_transparency", b);
    settings->sync();
    emit flagUseSemiTransparencyChanged(b);
}

bool Herschels::getFlagUseSemiTransparency() const
{
    return flagSemiTransparency;
}

void Herschels::setArrowButtonScale(const double val)
{
    arrowButtonScale = val;
    settings->setValue("arrow_scale", val);
    settings->sync();
    emit arrowButtonScaleChanged(val);
}

double Herschels::getArrowButtonScale() const
{
    return arrowButtonScale;
}

void Herschels::setFlagUseOculars(const bool flag)
{
    flagUseOculars = flag;
    emit flagUseOcularsChanged(flag);
}

bool Herschels::getFlagUseOculars() const
{
    return flagUseOculars;
}

void Herschels::setFlagProjectSweeps(const bool value)
{
    flagProjectSweeps = value;
    emit flagProjectSweepsChanged(value);
}

bool Herschels::getFlagProjectSweeps() const
{
    return flagProjectSweeps;
}

void Herschels::setFlagShowSweepEffects(const bool value)
{
    flagShowSweepEffects = value;
    emit flagShowSweepEffectsChanged(value);
}

bool Herschels::getFlagShowSweepEffects() const
{
    return flagShowSweepEffects;
}

void Herschels::setFlagShowOriginalImage(const bool value)
{
    flagShowOriginalImage = value;
    emit flagShowOriginalImageChanged(value);
}

bool Herschels::getFlagShowOriginalImage() const
{
    return flagShowOriginalImage;
}

void Herschels::setFlagFilterSweeps(const bool value)
{
    flagFilterSweeps = value;
    emit flagFilterSweepsChanged(value);
}

bool Herschels::getFlagFilterSweeps() const
{
    return flagFilterSweeps;
}

QString Herschels::getDimensionsString(double fovX, double fovY) const
{
    QString stringFovX, stringFovY;
    if (getFlagDMSDegrees())
    {
        if (fovX >= 1.0)
        {
            int degrees = (int)fovX;
            float minutes = (int)((fovX - degrees) * 60);
            stringFovX = QString::number(degrees) + QChar(0x00B0) + QString::number(minutes, 'f', 2) + QChar(0x2032);
        }
        else
        {
            float minutes = (fovX * 60);
            stringFovX = QString::number(minutes, 'f', 2) + QChar(0x2032);
        }

        if (fovY >= 1.0)
        {
            int degrees = (int)fovY;
            float minutes = ((fovY - degrees) * 60);
            stringFovY = QString::number(degrees) + QChar(0x00B0) + QString::number(minutes, 'f', 2) + QChar(0x2032);
        }
        else
        {
            float minutes = (fovY * 60);
            stringFovY = QString::number(minutes, 'f', 2) + QChar(0x2032);
        }
    }
    else
    {
        stringFovX = QString::number(fovX, 'f', 5) + QChar(0x00B0);
        stringFovY = QString::number(fovY, 'f', 5) + QChar(0x00B0);
    }

    return stringFovX + QChar(0x00D7) + stringFovY;
}
