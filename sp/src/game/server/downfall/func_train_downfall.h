#ifndef FUNC_TRAIN_DOWNFALL_H
#define FUNC_TRAIN_DOWNFALL_H

#include "trains.h"

class CFuncTrainDownfall : public CFuncTrackTrain
{
public:
	DECLARE_CLASS(CFuncTrainDownfall, CFuncTrackTrain);
	DECLARE_DATADESC();

	void InputApplyBrakes(inputdata_t& inputdata);
	void InputEnableControls(inputdata_t& inputdata);
	void InputDisableControls(inputdata_t& inputdata);
	void InputSetManualAccelSpeed(inputdata_t& inputdata);
	void InputSetManualDecelSpeed(inputdata_t& inputdata);

	void ApplyBrakes(float flTimeToStop);
	void EnableControls();
	void DisableControls();
	void SetManualAccelSpeed(float flSpeed);
	void SetManualDecelSpeed(float flSpeed);

	virtual void SetSpeed(float flSpeed, bool bAccel = false);

	virtual bool OnControls(CBaseEntity *pControls);
	inline bool CanBeControlled()
	{
		return (m_spawnflags & SF_TRACKTRAIN_NOCONTROL) != 0;
	}
};

#endif