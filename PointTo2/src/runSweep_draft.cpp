void PointTo::runSweep(const StelCore *core)
{
	const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
	StelPainter painter(prj);
	StelProjector::StelProjectorParams params = core->getCurrentStelProjectorParams();

	double inner = 0.5 * params.viewportFovDiameter * params.devicePixelsPerPixel;
	
	Sweep *sweep = sweeps[selectedSweepIndex];
	QString name = sweep->name
	QString startRA = sweep->startRA;
	QString endRA = sweep->endRA;
	QString startDec = sweep->startDec;
	QString endDec = sweep->endDec;
	QString date = sweep->date;

	QString midRA = SweepTools::findMidRA(startRA,endRA);
	QString midDec = SweepTools::findMidDec(startDec,endDec);
	
	QDateTime time = SweepTools::calcTime(startRA, date);
	QString times = time.toString("yyyy'-'MM'-'dd'T'hh:mm:ss.z");

	core.setDate(times, "utc");
	
	StelMovementMgr.zoomTo(0.5, 2);
	core.moveToRaDec(midRA,midDec);

 
}


