#include "cbase.h"
#include "func_train_downfall.h"

extern void FixupAngles(QAngle &v);

LINK_ENTITY_TO_CLASS(func_train_downfall, CFuncTrainDownfall);

BEGIN_DATADESC(CFuncTrainDownfall)
	DEFINE_INPUTFUNC( FIELD_FLOAT, "ApplyBrakes", InputApplyBrakes),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableControls", InputEnableControls),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableControls", InputDisableControls),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetManualAccelSpeed", InputSetManualAccelSpeed),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetManualDecelSpeed", InputSetManualDecelSpeed),
END_DATADESC()

void CFuncTrainDownfall::InputApplyBrakes(inputdata_t& inputdata)
{
	ApplyBrakes(inputdata.value.Float());
}

void CFuncTrainDownfall::InputEnableControls(inputdata_t& inputdata)
{
	EnableControls();
}

void CFuncTrainDownfall::InputDisableControls(inputdata_t& inputdata)
{
	DisableControls();
}

void CFuncTrainDownfall::InputSetManualAccelSpeed(inputdata_t& inputdata)
{
	SetManualAccelSpeed(inputdata.value.Float());
}

void CFuncTrainDownfall::InputSetManualDecelSpeed(inputdata_t& inputdata)
{
	SetManualDecelSpeed(inputdata.value.Float());
}

void CFuncTrainDownfall::ApplyBrakes(float flTimeToStop)
{
	m_flDesiredSpeed = 0;
	m_flSpeedChangeTime = flTimeToStop;
}

void CFuncTrainDownfall::EnableControls(void)
{
	m_spawnflags &= ~SF_TRACKTRAIN_NOCONTROL;
}

void CFuncTrainDownfall::DisableControls(void)
{
	m_spawnflags |= SF_TRACKTRAIN_NOCONTROL;
}

void CFuncTrainDownfall::SetManualAccelSpeed(float flSpeed)
{
	m_flAccelSpeed = flSpeed;
}

void CFuncTrainDownfall::SetManualDecelSpeed(float flSpeed)
{
	m_flDecelSpeed = flSpeed;
}

void CFuncTrainDownfall::Stop(void)
{
	SoundStop();
	BaseClass::Stop();
}

void CFuncTrainDownfall::SetSpeed(float flSpeed, bool bAccel)
{
	BaseClass::SetSpeed(flSpeed, true); // Always accelerate, never snap to a speed!
}

void CFuncTrainDownfall::UpdateTrainOrientation(CPathTrack *pNext, CPathTrack *pNextNext, const Vector &nextPos, float flInterval)
{
	if (!m_ppath)
		return;

	CPathTrack *pNextNode = NULL;
	CPathTrack *pPrevNode = NULL;

	Vector nextFront = GetLocalOrigin();
	Vector prevFront = GetLocalOrigin();

	nextFront.z -= m_height;
	prevFront.z -= m_height;
	if (m_length > 0)
	{
		m_ppath->LookAhead(nextFront, IsDirForward() ? m_length : -m_length, 0, &pNextNode);
		m_ppath->LookAhead(prevFront, IsDirForward() ? -m_length : m_length, 0, &pPrevNode);
	}
	else
	{
		m_ppath->LookAhead(nextFront, IsDirForward() ? 100 : -100, 0, &pNextNode);
		m_ppath->LookAhead(prevFront, IsDirForward() ? -100 : 100, 0, &pPrevNode);
	}
	nextFront.z += m_height;
	prevFront.z += m_height;

	Vector vecFaceDir;

	if (IsDirForward())
		vecFaceDir = nextFront - GetLocalOrigin();
	else
		vecFaceDir = prevFront - GetLocalOrigin();

	QAngle angles;
	VectorAngles(vecFaceDir, angles);
	FixupAngles(angles);

	QAngle curAngles = GetLocalAngles();
	FixupAngles(curAngles);

	DoUpdateOrientation(curAngles, angles, flInterval);
}

bool CFuncTrainDownfall::OnControls(CBaseEntity* pControls)
{
	if (!CanBeControlled())
		return false;

	return BaseClass::OnControls(pControls);
}
