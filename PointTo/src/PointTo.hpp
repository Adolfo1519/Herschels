/*
	the header file for the PointTo function, which will slew the telescope to the midpoint of a particular obsevation area on a date and time 
*/

#ifndef _POINTTO_HPP_
#define _POINTTO_HPP_

/*#include "CCD.hpp"
#include "Lens.hpp"
#include "Ocular.hpp"
#include "OcularDialog.hpp"
*/
#include "StelModule.hpp"
#include "StelTexture.hpp"
#include "Telescope.hpp"
#include "VecMath.hpp"
#include "StelUtils.hpp"
#include "SweepTools.hpp"
#include "Sweep.hpp"
#include <QFont>
#include <QSettings>

#define MIN_POINTTO_INI_VERSION 1.0

QT_BEGIN_NAMESPACE
class QKeyEvent;
class QMenu;
class QMouseEvent;
class QPixmap;
class QSettings;
class QSignalMapper;
QT_END_NAMESPACE

class StelButton;
class StelAction;

/*! @defgroup pointto PointTo Plug-in
@{
@}
*/

//! Main class of the PointTo plug-in.
//! @class PointTo
//! @ingroup pointto
class PointTo : public StelModule
{
	Q_OBJECT

	Q_PROPERTY(bool enableOcular     READ getEnableOcular     WRITE enableOcular     NOTIFY enableOcularChanged)
	
	Q_PROPERTY(int selectedSweepIndex       READ getSelectedSweepIndex       WRITE selectSweepAtIndex       NOTIFY selectedSweepChanged)
	
	Q_PROPERTY(bool flagInitFOVUsage             READ getFlagInitFovUsage             WRITE setFlagInitFovUsage NOTIFY flagInitFOVUsageChanged) 
	Q_PROPERTY(bool flagInitDirectionUsage       READ getFlagInitDirectionUsage       WRITE setFlagInitDirectionUsage NOTIFY flagInitDirectionUsageChanged) 
	Q_PROPERTY(bool flagHideGridsLines     READ getFlagHideGridsLines      WRITE setFlagHideGridsLines      NOTIFY flagHideGridsLinesChanged)
	Q_PROPERTY(bool flagSemiTransparency   READ getFlagUseSemiTransparency WRITE setFlagUseSemiTransparency NOTIFY flagUseSemiTransparencyChanged) 
	Q_PROPERTY(bool flagDMSDegrees         READ getFlagDMSDegrees          WRITE setFlagDMSDegrees          NOTIFY flagDMSDegreesChanged)
	
	Q_PROPERTY(double arrowButtonScale     READ getArrowButtonScale        WRITE setArrowButtonScale        NOTIFY arrowButtonScaleChanged)

	//BM: Temporary, until the GUI is finalized and some other method of getting
	//info from the main class is implemented.
	friend class PointToGuiPanel;

public:
	PointTo();
	virtual ~PointTo();
	static QSettings* appSettings();

	virtual void point(StelCore* core);
		
	///////////////////////////////////////////////////////////////////////////
	// Methods defined in the StelModule class
	virtual void init();
	virtual void deinit();
	virtual bool configureGui(bool show=true);
	virtual double getCallOrder(StelModuleActionName actionName) const;
	//! Returns the module-specific style sheet.
	//! This method is needed because the MovementMgr classes handleKeys() method consumes the event.
	//! Because we want the ocular view to track, we must intercept & process ourselves.  Only called
	//! while flagShowOculars or flagShowCCD == true.
	virtual void handleKeys(class QKeyEvent* event);
	virtual void handleMouseClicks(class QMouseEvent* event);
	virtual void update(double) {;}

	QString getDimensionsString(double fovX, double fovY) const;
	QString getFOVString(double fov) const;

public slots:
	//! Update the ocular, telescope and sensor lists after the removal of a member.
	//! Necessary because of the way model/view management in the OcularDialog
	//! is implemented.
	void updateLists();
	void SweepReset();
	void decrementSweepIndex();
	void displayPopupMenu();
	//! This method is called with we detect that our hot key is pressed.  It handles
	//! determining if we should do anything - based on a selected object.
	void incrementSweepIndex();
	
	void selectSweepAtIndex(int index);           //!< index in the range of -1:ccds.count(), else call is ignored
	int getSelectedSweepIndex() const {return selectedSweepIndex; }
	void enableSweep(bool enableSweepMode)
	bool getEnableSweep() const { return flagShowSweep; }
	
	void setFlagDMSDegrees(const bool b);
	bool getFlagDMSDegrees(void) const;

	
	void setArrowButtonScale(const double val);
	double getArrowButtonScale() const;

signals:
	void enableSweepChanged(bool value);
	void selectedSweepChanged(int value);
	
	void arrowButtonScaleChanged(double value);
	void flagDMSDegreesChanged(bool value);
	
private slots:
	//! Signifies a change in ocular or telescope.  Sets new zoom level.
	//I'm sure this part is important but I don't know why

	void sweepChanged();
	//void updateSweep(void);

private:
	//! Set up the Qt actions needed to activate the plugin.
	void initializeActivationActions();


//**************************************************************
	//! Renders the Sweep bounding box on-screen.  A telescope must be selected, or this call does nothing.
	//void paintSweepBounds();
//*****************************************************************
	void runSweep(const StelCore * core);	

	//! Paints the text about the current object selections to the upper right hand of the screen.
	//! Should only be called from a 'ready' state; currently from the draw() method.
	void paintText(const StelCore * core);

	
	//! This method is responsible for ensuring a valid ini file for the plugin exists.  It first checks to see
	//! if one exists in the expected location.  If it does not, a default one is copied into place, and the process
	//! ends.  However, if one does exist, it opens it, and looks for the oculars_version key.  The value (or even
	//! presence) is used to determine if the ini file is usable.  If not, it is renamed, and a new one copied over.
	//! It does not try to copy values over.
	//! Once there is a valid ini file, it is loaded into the settings attribute.
	void validateAndLoadIniFile();

	//! toggles the actual ocular view.
	//! Record the state of the GridLinesMgr and other settings beforehand, so that they can be reset afterwards.
	//! @param zoomedIn if true, this zoom operation is starting from an already zoomed state.
	//!		False for the original state.
	void run(bool zoomedIn);

	//! This method is called by the zoom() method, when this plugin is toggled on; it resets the zoomed view.
//	void zoomOcular();

	void hideUsageMessageIfDisplayed();

	//! A list of all the oculars defined in the ini file.  Must have at least one, or module will not run.
	QList<Sweep *> sweeps;
	
	int selectedSweepIndex;           //!< index of the current Sweep, in the range of -1:sweeps.count().  -1 means no Sweep is selected.
	double arrowButtonScale;        //!< allows scaling of the GUI "previous/next" Ocular/CCD/Telescope etc. buttons

	QFont font;			//!< The font used for drawing labels.
	//bool flagShowSweep;		//!< flag used to track if we are in CCD mode.
	int usageMessageLabelID;	//!< the id of the label showing the usage message. -1 means it's not displayed.

	bool flagCardinalPointsMain;	//!< Flag to track if CardinalPoints was displayed at activation.
	bool flagAdaptationMain;	//!< Flag to track if adaptationCheckbox was enabled at activation.

	
	bool flagDMSDegrees;             //!< Use decimal degrees in CCD frame display
	
	PointToDialog *pointtoDialog;
	bool ready; //!< A flag that determines that this module is usable.  If false, we won't open.

	StelAction * actionShowSweep;
	StelAction * actionConfiguration;
	StelAction * actionMenu;
	StelAction * actionSweepIncrement;
	StelAction * actionSweepDecrement;

};



#include <QObject>
#include "StelPluginInterface.hpp"

//! This class is used by Qt to manage a plug-in interface
class PointToStelPluginInterface : public QObject, public StelPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID StelPluginInterface_iid)
	Q_INTERFACES(StelPluginInterface)
public:
	virtual StelModule* getStelModule() const;
	virtual StelPluginInfo getPluginInfo() const;
	virtual QObjectList getExtensionList() const { return QObjectList(); }
};

#endif /*_POINTTO_HPP_*/
