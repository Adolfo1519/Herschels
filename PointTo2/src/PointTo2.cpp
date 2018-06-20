/*
This is the PointTo2 plugin main class. It can just take in essentially the corners of a box in
the sky and point to the center of that box.
*/
#include "PointTo2.hpp"
#include "SweepTools.hpp"
#include "Sweep.hpp"
#include "PointTo2Dialog.hpp"

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
#include "StelMainScriptAPI.hpp"
#include "StelMainView.hpp"
#include "StelModuleMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelPainter.hpp"
#include "StelProjector.hpp"
#include "StelTextureMgr.hpp"
#include "StelTranslator.hpp"
#include "SolarSystem.hpp"
#include "StelUtils.hpp"
#include "StelPropertyMgr.hpp"
#include "LandscapeMgr.hpp"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QGraphicsWidget>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QSignalMapper>

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
StelModule* PointTo2StelPluginInterface::getStelModule() const
{
    return new PointTo2();
}

//Q_INIT_RESOURCE(PointTo2);
//Sweep *sweep = new Sweep;
//QString midRA = sweep->endRA();
StelPluginInfo PointTo2StelPluginInterface::getPluginInfo() const
{
    // Allow to load the resources when used as a static plugin
    Q_INIT_RESOURCE(PointTo2);

    StelPluginInfo info;
    info.id = "PointTo2";
    info.displayedName = N_("PointTo2");
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
//This step initializes all of the members of PointTo2 with a particular value as part of
//defining the constructor
PointTo2::PointTo2():
    selectedSweepIndex(-1),
    arrowButtonScale(1.5),
    usageMessageLabelID(-1),
    flagCardinalPointsMain(false),
    flagAdaptationMain(false),
    flagSemiTransparency(false),
    flagDMSDegrees(false),
    flagShowSweep(false),
    sweepsSignalMapper(Q_NULLPTR),
    pointto2Dialog(Q_NULLPTR),
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

    sweepsSignalMapper = new QSignalMapper(this);

    setObjectName("PointTo2");
    conf = StelApp::getInstance().getSettings();

#ifdef Q_OS_MAC
    qt_set_sequence_auto_mnemonic(true);
#endif
}

PointTo2::~PointTo2()
{
    delete pointto2Dialog;
    pointto2Dialog = Q_NULLPTR;

    qDeleteAll(sweeps);
    sweeps.clear();
}

QSettings* PointTo2::appSettings()
{
    return settings;
}


/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark StelModule Methods
#endif
/* ****************************************************************************************************************** */
bool PointTo2::configureGui(bool show)
{
    if (show)
    {
        pointto2Dialog->setVisible(true);
    }

    return ready;
}

void PointTo2::deinit()
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
void PointTo2::point(StelCore* core)
{
    if (selectedSweepIndex > sweeps.count())
    {
        qWarning() << "PointTo2: the selected sweep index of "
               << selectedSweepIndex << " is greater than the sensor count of "
               << sweeps.count() << ". Module disabled!";
        ready = false;
    }

    if (ready)
    {
        if (selectedSweepIndex > -1)
        {
            runSweep(core);
        }
    }
}
//****************************************************************
//! Determine which "layer" the plugin's drawing will happen on.
double PointTo2::getCallOrder(StelModuleActionName actionName) const
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

void PointTo2::handleMouseClicks(class QMouseEvent* event)
{
    StelCore *core = StelApp::getInstance().getCore();
    const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000, StelCore::RefractionAuto);
    StelProjector::StelProjectorParams params = core->getCurrentStelProjectorParams();
    float ppx = params.devicePixelsPerPixel;


    StelMovementMgr *movementManager = core->getMovementMgr();

    }

void PointTo2::handleKeys(QKeyEvent* event)
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
            case Qt::Key_M:
                double multiplier = 1.0;
                if (event->modifiers().testFlag(Qt::ControlModifier))
                {
                    multiplier = 0.1;
                }
                if (event->modifiers().testFlag(Qt::AltModifier))
                {
                    multiplier = 5.0;
                }
                //qDebug() << reticleRotation;
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

void PointTo2::init()
{
    qDebug() << "PointTo2 plugin - Press ALT-s for configuration.";
    //conf->setValue("AngleMeasure/enable_at_startup", true);

    //enableSweep(conf->value("PointTo2/enable_at_startup", true).toBool());
    // Load settings from sweeps.ini
    try {
        validateAndLoadIniFile();
        qWarning() << "ini File Loaded";
        // assume all is well
        ready = true;

        int sweepCount = settings->value("sweep_count", 0).toInt();
        int actualSweepCount = sweepCount;
        for (int index = 0; index < sweepCount; index++)
        {
            Sweep *newSweep = Sweep::sweepFromSettings(settings, index);
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

        pointto2Dialog = new PointTo2Dialog(this, &sweeps);
        initializeActivationActions();

        // For historical reasons, name of .ini entry and description of checkbox (and therefore flag name) are reversed.
        setFlagDMSDegrees( ! settings->value("use_decimal_degrees", false).toBool());
        setFlagUseSemiTransparency(settings->value("use_semi_transparency", false).toBool());
        setArrowButtonScale(settings->value("arrow_scale", 1.5).toDouble());

        StelPropertyMgr* propMgr=StelApp::getInstance().getStelPropertyManager();

    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: unable to locate sweep.ini file or create a default one for Sweeps plugin: " << e.what();
        ready = false;
    }

    //connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslateGui()));
    connect(this, SIGNAL(selectedSweepChanged(int)), this, SLOT(updateSweep()));
    StelApp& app = StelApp::getInstance();



}

/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Private slots Methods
#endif
/* ******************************************************************************************************************
*/
//Have to figure out exactly what this one is doing, because it might be necessary for aborting sweeps mid-sweep
void PointTo2::sweepChanged()
{
    // We only zoom if in Sweep mode.
    if (flagShowSweep)
    {
        // If we are already in Sweep mode, we must reset scalings because run() also resets.
        StelSkyDrawer *skyDrawer = StelApp::getInstance().getCore()->getSkyDrawer();
        run(true);
    }
}

void PointTo2::run(bool wantRun)
{
    if (flagShowSweep && selectedSweepIndex == -1)
    {
        flagShowSweep = false;
    }

    if (flagShowSweep)
    {
        StelCore *core = StelApp::getInstance().getCore();

        if (!wantRun)
        {
            StelSkyDrawer *skyDrawer = core ->getSkyDrawer();
            StelMovementMgr *movementManager = core->getMovementMgr();
        }
        runSweep(core);
    }
    else
    {
        restore();
    }
}

void PointTo2::restore()
{
    StelCore *core = StelApp::getInstance().getCore();

    StelMainScriptAPI *mainScriptAPI;
    mainScriptAPI->goHome();
}


//No idea what this function is doing
void PointTo2::updateSweep(void)
{
    StelTextureMgr& manager = StelApp::getInstance().getTextureManager();
    //Load OpenGL textures
    StelTexture::StelTextureParams params;
    params.generateMipmaps = true;
}

/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Slots Methods
#endif
/* ****************************************************************************************************************** */
void PointTo2::updateLists()
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

    }
}

void PointTo2::openSettings(bool toggle)
{
    if (toggle)
    {
        qWarning() << "Will try to open dialog";
        pointto2Dialog->setVisible(true);
        qWarning() << "Dialog opened";
    }

}

void PointTo2::enableSweep(bool enableSweepMode)
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

void PointTo2::decrementSweepIndex()
{
    selectedSweepIndex--;
    if (selectedSweepIndex == -1)
    {
        selectedSweepIndex = sweeps.count() - 1;
    }
    emit(selectedSweepChanged(selectedSweepIndex));
}

void PointTo2::displayPopupMenu()
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
                    label = QString("&%1: %2").arg(availableSweepCount).arg(sweeps[index]->sweepName());
                }
                else
                {
                    label = sweeps[index]->sweepName();
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
        action->setChecked(pointto2Dialog->visible());
        connect(action, SIGNAL(triggered(bool)), pointto2Dialog, SLOT(setVisible(bool)));
        popup->addAction(action);
        popup->addSeparator();

    }

    popup->exec(QCursor::pos());
    delete popup;
}

void PointTo2::retranslateGui()
{

}


void PointTo2::incrementSweepIndex()
{
    selectedSweepIndex++;
    if (selectedSweepIndex == sweeps.count())
    {
        selectedSweepIndex = 0;
    }
    emit(selectedSweepChanged(selectedSweepIndex));
}

void PointTo2::selectSweepAtIndex(int index)
{
    if (index > -2 && index < sweeps.count())
    {
        selectedSweepIndex = index;
        emit(selectedSweepChanged(index));
    }
}
/* ****************************************************************************************************************** */
#if 0
#pragma mark -
#pragma mark Private Methods
#endif
/* ****************************************************************************************************************** */
void PointTo2::initializeActivationActions()
{
    StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    Q_ASSERT(gui);

    QString sweepsGroup = N_("Sweeps");
    actionShowSweep = addAction("actionShow_PointTo2", sweepsGroup, N_("Sweep view"), "openSettings", "Ctrl+Shift+H");

    // Add a toolbar button
    try
    {

            // Create action for enable/disable & hook up signals
            qWarning() << "Setting up ToolBar Button";


           // addAction("actionShow_Point_To2", N_("PointTo2"), N_("Point to2"), pointto2Dialog, "visible");
            //addAction()

            //connect(this, SIGNAL(toggled(bool)),
              //      pointto2Dialog, SLOT(setVisible(bool)));

            qWarning() << "action Added";

            toolbarButton = new StelButton(Q_NULLPTR,
                               QPixmap(":/sweep/bt_telrad_on.png"),
                               QPixmap(":/sweep/bt_telrad_off.png"),
                               QPixmap(":/sweep/bt_ocular_off.png"),
                               "actionShow_PointTo2");

            gui->getButtonBar()->addButton(toolbarButton, "065-pluginsGroup");


    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: unable create toolbar button for PointTo2 plugin: " << e.what();
    }



    actionMenu           = addAction("actionShow_Sweep_Menu",           sweepsGroup, N_("PointTo2 popup menu"), "displayPopupMenu()", "Alt+s");
    actionConfiguration  = addAction("actionOpen_PointTo2_Configuration", sweepsGroup, N_("PointTo2 plugin configuration"), pointto2Dialog, "visible");
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

/*void PointTo2::paintSweepBounds()
{
    int fontSize = StelApp::getInstance().getBaseFontSize();
    StelCore *core = StelApp::getInstance().getCore();
    StelProjector::StelProjectorParams params = core->getCurrentStelProjectorParams();
    Lens *lens = selectedLensIndex >=0  ? lenses[selectedLensIndex] : Q_NULLPTR;

    const StelProjectorP projector = core->getProjection(StelCore::FrameEquinoxEqu);
    double screenFOV = params.fov;

    Vec2i centerScreen(projector->getViewportPosX() + projector->getViewportWidth() / 2,
               projector->getViewportPosY() + projector->getViewportHeight() / 2);

    // draw sensor rectangle
    if(selectedCCDIndex != -1)
    {
        CCD *ccd = ccds[selectedCCDIndex];
        if (ccd)
        {
            StelPainter painter(projector);
            painter.setColor(0.77f, 0.14f, 0.16f, 1.0f);
            painter.setFont(font);
            Telescope *telescope = telescopes[selectedTelescopeIndex];

            const double ccdXRatio = ccd->getActualFOVx(telescope, lens) / screenFOV;
            const double ccdYRatio = ccd->getActualFOVy(telescope, lens) / screenFOV;

            const double fovX = ccd->getActualFOVx(telescope, lens);
            const double fovY = ccd->getActualFOVy(telescope, lens);

            // As the FOV is based on the narrow aspect of the screen, we need to calculate
            // height & width based soley off of that dimension.
            int aspectIndex = 2;
            if (params.viewportXywh[2] > params.viewportXywh[3])
            {
                aspectIndex = 3;
            }
            float width = params.viewportXywh[aspectIndex] * ccdXRatio * params.devicePixelsPerPixel;
            float height = params.viewportXywh[aspectIndex] * ccdYRatio * params.devicePixelsPerPixel;

            double polarAngle = 0;
            // if the telescope is Equatorial derotate the field
            if (telescope->isEquatorial())
            {
                Vec3d CPos;
                Vec2f cpos = projector->getViewportCenter();
                projector->unProject(cpos[0], cpos[1], CPos);
                Vec3d CPrel(CPos);
                CPrel[2]*=0.2;
                Vec3d crel;
                projector->project(CPrel, crel);
                polarAngle = atan2(cpos[1] - crel[1], cpos[0] - crel[0]) * (-180.0)/M_PI; // convert to degrees
                if (CPos[2] > 0) polarAngle += 90.0;
                else polarAngle -= 90.0;
            }

            if (getFlagAutosetMountForCCD())
            {
                StelPropertyMgr* propMgr=StelApp::getInstance().getStelPropertyManager();
                propMgr->setStelPropertyValue("actionSwitch_Equatorial_Mount", telescope->isEquatorial());
                polarAngle = 0;
            }

            if (width > 0.0 && height > 0.0)
            {
                QPoint a, b;
                QTransform transform = QTransform().translate(centerScreen[0], centerScreen[1]).rotate(-(ccd->chipRotAngle() + polarAngle));
                // bottom line
                a = transform.map(QPoint(-width/2.0, -height/2.0));
                b = transform.map(QPoint(width/2.0, -height/2.0));
                painter.drawLine2d(a.x(), a.y(), b.x(), b.y());
                // top line
                a = transform.map(QPoint(-width/2.0, height/2.0));
                b = transform.map(QPoint(width/2.0, height/2.0));
                painter.drawLine2d(a.x(), a.y(), b.x(), b.y());
                // left line
                a = transform.map(QPoint(-width/2.0, -height/2.0));
                b = transform.map(QPoint(-width/2.0, height/2.0));
                painter.drawLine2d(a.x(), a.y(), b.x(), b.y());
                // right line
                a = transform.map(QPoint(width/2.0, height/2.0));
                b = transform.map(QPoint(width/2.0, -height/2.0));
                painter.drawLine2d(a.x(), a.y(), b.x(), b.y());

                if(ccd->hasOAG())
                {
                    const double InnerOAGRatio = ccd->getInnerOAGRadius(telescope, lens) / screenFOV;
                    const double OuterOAGRatio = ccd->getOuterOAGRadius(telescope, lens) / screenFOV;
                    const double prismXRatio = ccd->getOAGActualFOVx(telescope, lens) / screenFOV;
                    float in_oag_r = params.viewportXywh[aspectIndex] * InnerOAGRatio * params.devicePixelsPerPixel;
                    float out_oag_r = params.viewportXywh[aspectIndex] * OuterOAGRatio * params.devicePixelsPerPixel;
                    float h_width = params.viewportXywh[aspectIndex] * prismXRatio * params.devicePixelsPerPixel / 2.0;

                    //painter.setColor(0.60f, 0.20f, 0.20f, .5f);
                    painter.drawCircle(centerScreen[0], centerScreen[1], in_oag_r);
                    painter.drawCircle(centerScreen[0], centerScreen[1], out_oag_r);

                    QTransform oag_transform = QTransform().translate(centerScreen[0], centerScreen[1]).rotate(-(ccd->chipRotAngle() + polarAngle + ccd->prismPosAngle()));

                    // bottom line
                    a = oag_transform.map(QPoint(-h_width, in_oag_r));
                    b = oag_transform.map(QPoint(h_width, in_oag_r));
                    painter.drawLine2d(a.x(),a.y(), b.x(), b.y());
                    // top line
                    a = oag_transform.map(QPoint(-h_width, out_oag_r));
                    b = oag_transform.map(QPoint(h_width, out_oag_r));
                    painter.drawLine2d(a.x(),a.y(), b.x(), b.y());
                    // left line
                    a = oag_transform.map(QPoint(-h_width, out_oag_r));
                    b = oag_transform.map(QPoint(-h_width, in_oag_r));
                    painter.drawLine2d(a.x(),a.y(), b.x(), b.y());
                    // right line
                    a = oag_transform.map(QPoint(h_width, out_oag_r));
                    b = oag_transform.map(QPoint(h_width, in_oag_r));
                    painter.drawLine2d(a.x(),a.y(), b.x(), b.y());
                }

                // Tool for planning a mosaic astrophotography: shows a small cross at center of CCD's
                // frame and equatorial coordinates for epoch J2000.0 of that center.
                // Details: https://bugs.launchpad.net/stellarium/+bug/1404695

                float ratioLimit = 0.25f;
                if (ccdXRatio>=ratioLimit || ccdYRatio>=ratioLimit)
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
                    painter.drawText(a.x(), a.y(), coords, -(ccd->chipRotAngle() + polarAngle));
                    // Dimensions of visible field of view for CCD (red rectangle)
                    a = transform.map(QPoint(-width/2.0, -height/2.0 - fontSize*1.2f));
                    painter.drawText(a.x(), a.y(), getDimensionsString(fovX, fovY), -(ccd->chipRotAngle() + polarAngle));
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
                }
            }
        }
    }

}
*/

//*****************************************************************


//This is the actual function I want running the "sweep". For now all it does is point,
// but with time I hope to make it capable of more things
void PointTo2::runSweep(const StelCore *core)
{
    const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
    StelPainter painter(prj);
    StelProjector::StelProjectorParams params = core->getCurrentStelProjectorParams();

    double inner = 0.5 * params.viewportFovDiameter * params.devicePixelsPerPixel;

    Sweep *sweep = sweeps[selectedSweepIndex];
    QString name = sweep->sweepName();
    QString startRA = sweep->startRA();
    QString endRA = sweep->endRA();
    QString startDec = sweep->startDec();
    QString endDec = sweep->endDec();
    QString date = sweep->date();

    QDateTime dateTime = QDateTime::fromString(date,"yyyy'-'MM'-'dd'T'hh:mm:ss.z");

    QString midRA = SweepTools::findMidRA(startRA,endRA);
    QString midDec = SweepTools::findMidDec(startDec,endDec);

    QDateTime time = SweepTools::calcTime(startRA, dateTime);
    QString times = time.toString("yyyy'-'MM'-'dd'T'hh:mm:ss.z");

    StelMainScriptAPI *mainScriptAPI ;
    mainScriptAPI->setDate(times, "utc");

    StelMovementMgr *stelMovementMgr;
    stelMovementMgr->zoomTo(0.5, 2);

    mainScriptAPI->moveToRaDec(midRA,midDec);


}


//*****************************************************************
void PointTo2::paintText(const StelCore* core)
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
    painter.setColor(0.8f, 0.48f, 0.f, 1.f);
    painter.setBlending(true);

    // Get the X & Y positions, and the line height
    painter.setFont(font);
    QString widthString = "MMMMMMMMMMMMMMMMMMM";
    float insetFromRHS = painter.getFontMetrics().width(widthString);
    StelProjector::StelProjectorParams projectorParams = core->getCurrentStelProjectorParams();
    int xPosition = projectorParams.viewportXywh[2] - projectorParams.viewportCenterOffset[0];
    xPosition -= insetFromRHS;
    int yPosition = projectorParams.viewportXywh[3] - projectorParams.viewportCenterOffset[1];
    yPosition -= 40;
    const int lineHeight = painter.getFontMetrics().height();


    // The Sweep
    if (flagShowSweep && sweep!=Q_NULLPTR)
    {
        QString sweepNumberLabel;
        QString name = sweep->sweepName();
        QString sweepI18n = q_("Sweep");
        if (name.isEmpty())
        {
            sweepNumberLabel = QString("%1 #%2").arg(sweepI18n).arg(selectedSweepIndex);
        }
        else
        {
            sweepNumberLabel = QString("%1 #%2: %3").arg(sweepI18n).arg(selectedSweepIndex).arg(name);
        }
        // The name of the sweep could be really long.
        if (name.length() > widthString.length())
        {
            xPosition -= (insetFromRHS / 2.0);
        }
        painter.drawText(xPosition, yPosition, sweepNumberLabel);
        yPosition-=lineHeight;

    }

}

void PointTo2::validateAndLoadIniFile()
{
    // Insure the module directory exists
    StelFileMgr::makeSureDirExistsAndIsWritable(StelFileMgr::getUserDir()+"/modules/PointTo2");
    StelFileMgr::Flags flags = (StelFileMgr::Flags)(StelFileMgr::Directory|StelFileMgr::Writable);
    QString sweepIniPath = StelFileMgr::findFile("modules/PointTo2/", flags) + "sweep.ini";
    if (sweepIniPath.isEmpty())
        return;

    // If the ini file does not already exist, create it from the resource in the QT resource
    if(!QFileInfo(sweepIniPath).exists())
    {
        QFile src(":/sweep/default_sweep.ini");
        if (!src.copy(sweepIniPath))
        {
            qWarning() << "PointTo2::validateIniFile cannot copy default_sweep.ini resource to [non-existing] "
                      + sweepIniPath;
        }
        else
        {
            qDebug() << "PointTo2::validateIniFile copied default_sweep.ini to " << QDir::toNativeSeparators(sweepIniPath);
            // The resource is read only, and the new file inherits this, so set write-able.
            QFile dest(sweepIniPath);
            dest.setPermissions(dest.permissions() | QFile::WriteOwner);
        }
    }
    else
    {
        qDebug() << "PointTo2::validateIniFile sweep.ini exists at: " << QDir::toNativeSeparators(sweepIniPath) << ". Checking version...";
        QSettings settings(sweepIniPath, QSettings::IniFormat);
        float sweepsVersion = settings.value("sweeps_version", 0.0).toFloat();
        qWarning() << "PointTo2::validateIniFile found existing ini file version " << sweepsVersion;

        if (sweepsVersion < MIN_POINTTO2_INI_VERSION)
        {
            qWarning() << "PointTo2::validateIniFile existing ini file version " << sweepsVersion
                   << " too old to use; required version is " << MIN_POINTTO2_INI_VERSION << ". Copying over new one.";
            // delete last "old" file, if it exists
            QFile deleteFile(sweepIniPath + ".old");
            deleteFile.remove();

            // Rename the old one, and copy over a new one
            QFile oldFile(sweepIniPath);
            if (!oldFile.rename(sweepIniPath + ".old"))
            {
                qWarning() << "PointTo2::validateIniFile cannot move sweep.ini resource to sweep.ini.old at path  " + QDir::toNativeSeparators(sweepIniPath);
            }
            else
            {
                qWarning() << "PointTo2::validateIniFile sweep.ini resource renamed to sweep.ini.old at path  " + QDir::toNativeSeparators(sweepIniPath);
                QFile src(":/sweep/default_sweep.ini");
                if (!src.copy(sweepIniPath))
                {
                    qWarning() << "PointTo2::validateIniFile cannot copy default_sweep.ini resource to [non-existing] " + QDir::toNativeSeparators(sweepIniPath);
                }
                else
                {
                    qDebug() << "PointTo2::validateIniFile copied default_sweep.ini to " << QDir::toNativeSeparators(sweepIniPath);
                    // The resource is read only, and the new file inherits this...  make sure the new file
                    // is writable by the Stellarium process so that updates can be done.
                    QFile dest(sweepIniPath);
                    dest.setPermissions(dest.permissions() | QFile::WriteOwner);
                }
            }
        }
    }
    settings = new QSettings(sweepIniPath, QSettings::IniFormat, this);
}

//*****************************************************************
void PointTo2::hideUsageMessageIfDisplayed()
{
    if (usageMessageLabelID > -1)
    {
        LabelMgr *labelManager = GETSTELMODULE(LabelMgr);
        labelManager->setLabelShow(usageMessageLabelID, false);
        labelManager->deleteLabel(usageMessageLabelID);
        usageMessageLabelID = -1;
    }
}

Sweep* PointTo2::selectedSweep()
{
    if (selectedSweepIndex >= 0 && selectedSweepIndex < sweeps.count())
    {
        return sweeps[selectedSweepIndex];
    }
    return Q_NULLPTR;
}

QMenu* PointTo2::addSweepSubmenu(QMenu* parent)
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
            label = QString("&%1: %2").arg(index).arg(sweeps[index]->sweepName());
        }
        else
        {
            label = sweeps[index]->sweepName();
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
void PointTo2::setFlagDMSDegrees(const bool b)
{
    flagDMSDegrees = b;
    settings->setValue("use_decimal_degrees", !b);
    settings->sync();
    emit flagDMSDegreesChanged(b);
}

bool PointTo2::getFlagDMSDegrees() const
{
    return flagDMSDegrees;
}


void PointTo2::setFlagUseSemiTransparency(const bool b)
{
    flagSemiTransparency = b;
    settings->setValue("use_semi_transparency", b);
    settings->sync();
    emit flagUseSemiTransparencyChanged(b);
}

bool PointTo2::getFlagUseSemiTransparency() const
{
    return flagSemiTransparency;
}

void PointTo2::setArrowButtonScale(const double val)
{
    arrowButtonScale = val;
    settings->setValue("arrow_scale", val);
    settings->sync();
    emit arrowButtonScaleChanged(val);
}

double PointTo2::getArrowButtonScale() const
{
    return arrowButtonScale;
}

QString PointTo2::getDimensionsString(double fovX, double fovY) const
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

