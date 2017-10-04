#ifndef FUNC_TRAIN_DOWNFALL_H
#define FUNC_TRAIN_DOWNFALL_H

#include "trains.h"

class CFuncTrainDownfall : public CFuncTrackTrain
{
public:
	DECLARE_CLASS(CFuncTrainDownfall, CFuncTrackTrain);
	DECLARE_DATADESC();

	virtual void Stop(void);
	virtual void SetSpeed(float flSpeed, bool bAccel = false);
	virtual void UpdateTrainOrientation(CPathTrack *pNext, CPathTrack *pNextNext, const Vector &nextPos, float flInterval);
	virtual bool OnControls(CBaseEntity *pControls);

	virtual void Spawn();

	void VisualizeThink();

	void InputApplyBrakes(inputdata_t& inputdata);
	void InputEnableControls(inputdata_t& inputdata);
	void InputDisableControls(inputdata_t& inputdata);
	void InputSetManualAccelSpeed(inputdata_t& inputdata);
	void InputSetManualDecelSpeed(inputdata_t& inputdata);

private:
	bool CanBeControlled() const
	{
		return (m_spawnflags & SF_TRACKTRAIN_NOCONTROL) == 0;
	}
	void ApplyBrakes(float flTimeToStop);
	void EnableControls();
	void DisableControls();
	void SetManualAccelSpeed(float flSpeed);
	void SetManualDecelSpeed(float flSpeed);
};

#endif