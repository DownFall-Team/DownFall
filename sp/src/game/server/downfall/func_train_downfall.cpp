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

float LerpDegrees(float start, float end, float amount)
{
	float difference = fabsf(end - start);
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (end > start)
		{
			// We'll add it on to start...
			start += 360;
		}
		else
		{
			// Add it on to end.
			end += 360;
		}
	}

	// Interpolate it.
	float value = (start + ((end - start) * amount));

	// Wrap it..
	float rangeZero = 360;

	if (value >= 0 && value <= 360)
		return value;

	return fmodf(value, rangeZero);
}

ConVar sv_downfall_train_smoothing_mode("sv_downfall_train_smoothing_mode", "approach", FCVAR_CHEAT, "Options are: approach, quaternion_slerp or spline_quaternion_slerp");
ConVar sv_downfall_train_smoothness("sv_downfall_train_smoothness", "3", FCVAR_CHEAT);

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
		vecFaceDir = pNextNode->GetAbsOrigin() - GetAbsOrigin();
	else
		vecFaceDir = pPrevNode->GetAbsOrigin() - GetAbsOrigin();

	QAngle wantedAngles;
	VectorAngles(vecFaceDir, wantedAngles);
	FixupAngles(wantedAngles);

	QAngle curAngles = GetLocalAngles();
	FixupAngles(curAngles);

	if (!Q_strcmp(sv_downfall_train_smoothing_mode.GetString(), "approach"))
	{
		QAngle vecAngVel;
		vecAngVel.x = UTIL_AngleDistance(UTIL_ApproachAngle(wantedAngles.x, curAngles.x, m_flSpeed * sv_downfall_train_smoothness.GetFloat()), curAngles.x) * sv_downfall_train_smoothness.GetFloat();
		vecAngVel.y = UTIL_AngleDistance(UTIL_ApproachAngle(wantedAngles.y, curAngles.y, m_flSpeed * sv_downfall_train_smoothness.GetFloat()), curAngles.y) * sv_downfall_train_smoothness.GetFloat();
		vecAngVel.z = UTIL_AngleDistance(UTIL_ApproachAngle(wantedAngles.z, curAngles.z, m_flSpeed * sv_downfall_train_smoothness.GetFloat()), curAngles.z) * sv_downfall_train_smoothness.GetFloat();

		SetLocalAngularVelocity(vecAngVel);
		return;
	}

	if (!Q_strcmp(sv_downfall_train_smoothing_mode.GetString(), "quaternion_slerp"))
	{
		Quaternion wantedQuaternion = wantedAngles;
		Quaternion curQuaternion = curAngles;

		Quaternion smoothedQuaternion;
		QuaternionSlerp(curQuaternion, wantedQuaternion, gpGlobals->frametime * (1 / sv_downfall_train_smoothness.GetFloat()), smoothedQuaternion);
		QAngle smoothedAngles;
		QuaternionAngles(smoothedQuaternion, smoothedAngles);

		float vx = UTIL_AngleDistance(smoothedAngles.x, curAngles.x);
		float vy = UTIL_AngleDistance(smoothedAngles.y, curAngles.y);
		float vz = UTIL_AngleDistance(smoothedAngles.z, curAngles.z);

		if (fabsf(vx) < 0.1f)
			vx = 0;

		if (fabsf(vy) < 0.1f)
			vy = 0;

		if (fabsf(vz) < 0.1f)
			vz = 0;

		if (flInterval == 0)
			flInterval = 0.1f;

		QAngle vecAngVel(vx / flInterval, vy / flInterval, vz / flInterval);
		SetLocalAngularVelocity(vecAngVel);
		return;
	}

	if (!Q_strcmp(sv_downfall_train_smoothing_mode.GetString(), "spline_quaternion_slerp"))
	{
		float p = 0;
		CPathTrack *pForwardSplineNode = IsDirForward() ? pNextNode : pPrevNode;
		CPathTrack *pBackwardSplineNode = IsDirForward() ? pPrevNode : pNextNode;

		if (pForwardSplineNode && pBackwardSplineNode)
		{
			Vector vecSegment = pForwardSplineNode->GetLocalOrigin() - pBackwardSplineNode->GetLocalOrigin();
			float flSegmentLen = vecSegment.Length();
			if (flSegmentLen)
			{
				Vector vecCurOffset = GetLocalOrigin() - pBackwardSplineNode->GetLocalOrigin();
				p = vecCurOffset.Length() / flSegmentLen;
			}
		}

		p = SimpleSplineRemapVal(p, 0.0f, 1.0f, 0.0f, 1.0f);

		Quaternion wantedQuaternion = wantedAngles;
		Quaternion curQuaternion = curAngles;

		Quaternion smoothedQuaternion;
		QuaternionSlerp(curQuaternion, wantedQuaternion, gpGlobals->frametime * p * sv_downfall_train_smoothness.GetFloat(), smoothedQuaternion);
		QAngle smoothedAngles;
		QuaternionAngles(smoothedQuaternion, smoothedAngles);

		float vx = UTIL_AngleDistance(smoothedAngles.x, curAngles.x);
		float vy = UTIL_AngleDistance(smoothedAngles.y, curAngles.y);
		float vz = UTIL_AngleDistance(smoothedAngles.z, curAngles.z);

		if (fabsf(vx) < 0.1f)
			vx = 0;

		if (fabsf(vy) < 0.1f)
			vy = 0;

		if (fabsf(vz) < 0.1f)
			vz = 0;

		if (flInterval == 0)
			flInterval = 0.1f;

		QAngle vecAngVel(vx / flInterval, vy / flInterval, vz / flInterval);
		SetLocalAngularVelocity(vecAngVel);
		return;
	}
}

bool CFuncTrainDownfall::OnControls(CBaseEntity* pControls)
{
	if (!CanBeControlled())
		return false;

	return BaseClass::OnControls(pControls);
}
