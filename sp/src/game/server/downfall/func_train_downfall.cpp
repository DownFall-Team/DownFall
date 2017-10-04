#include "cbase.h"
#include "func_train_downfall.h"

#include "tier0/memdbgon.h"

extern void FixupAngles(QAngle &v);

LINK_ENTITY_TO_CLASS(func_train_downfall, CFuncTrainDownfall);

BEGIN_DATADESC(CFuncTrainDownfall)
	DEFINE_INPUTFUNC( FIELD_FLOAT, "ApplyBrakes", InputApplyBrakes),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableControls", InputEnableControls),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableControls", InputDisableControls),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetManualAccelSpeed", InputSetManualAccelSpeed),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetManualDecelSpeed", InputSetManualDecelSpeed),

	DEFINE_THINKFUNC( VisualizeThink )

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

void CFuncTrainDownfall::EnableControls()
{
	m_spawnflags &= ~SF_TRACKTRAIN_NOCONTROL;
}

void CFuncTrainDownfall::DisableControls()
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

void CFuncTrainDownfall::Stop()
{
	SoundStop();
	BaseClass::Stop();
}

void CFuncTrainDownfall::SetSpeed(float flSpeed, bool bAccel)
{
	BaseClass::SetSpeed( flSpeed, true ); // Always accelerate, never snap to a speed!
}

float LerpDegrees( float start, float end, float amount )
{
	if ( fabsf( end - start ) > 180 )
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
	const float value = start + ( end - start ) * amount;

	if (value >= 0 && value <= 360)
		return value;

	return fmodf(value, 360);
}

ConVar sv_downfall_train_smoothing_mode("sv_downfall_train_smoothing_mode", "2", FCVAR_CHEAT, "Options are: 0.valve, 1.approach, 2.quaternion_slerp or 3.spline_quaternion_slerp");
ConVar sv_downfall_train_smoothness("sv_downfall_train_smoothness", "0.29", FCVAR_CHEAT);
ConVar sv_show_train_path( "sv_show_train_path", "0", FCVAR_CHEAT );

void CFuncTrainDownfall::UpdateTrainOrientation(CPathTrack *pNext, CPathTrack *pNextNext, const Vector &nextPos, float flInterval)
{
	if ( !sv_downfall_train_smoothing_mode.GetBool() )
	{
		return BaseClass::UpdateTrainOrientation(pNext, pNextNext, nextPos, flInterval);
	}

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

	switch ( sv_downfall_train_smoothing_mode.GetInt() )
	{
	case 1:
	{
		QAngle vecAngVel;
		vecAngVel.x = UTIL_AngleDistance( UTIL_ApproachAngle( wantedAngles.x, curAngles.x, m_flSpeed * sv_downfall_train_smoothness.GetFloat() ), curAngles.x ) * sv_downfall_train_smoothness.GetFloat();
		vecAngVel.y = UTIL_AngleDistance( UTIL_ApproachAngle( wantedAngles.y, curAngles.y, m_flSpeed * sv_downfall_train_smoothness.GetFloat() ), curAngles.y ) * sv_downfall_train_smoothness.GetFloat();
		vecAngVel.z = UTIL_AngleDistance( UTIL_ApproachAngle( wantedAngles.z, curAngles.z, m_flSpeed * sv_downfall_train_smoothness.GetFloat() ), curAngles.z ) * sv_downfall_train_smoothness.GetFloat();

		SetLocalAngularVelocity( vecAngVel );
		return;
	}

	case 2:
	{
		Quaternion smoothedQuaternion;
		QuaternionSlerp( RadianEuler( curAngles ), RadianEuler( wantedAngles ), gpGlobals->frametime * ( 1 / sv_downfall_train_smoothness.GetFloat() ), smoothedQuaternion );
		QAngle smoothedAngles;
		QuaternionAngles( smoothedQuaternion, smoothedAngles );

		const float vx = UTIL_AngleDistance( smoothedAngles.x, curAngles.x );
		const float vy = UTIL_AngleDistance( smoothedAngles.y, curAngles.y );
		const float vz = UTIL_AngleDistance( smoothedAngles.z, curAngles.z );

		/*if (fabsf(vx) < 0.1f)
		vx = 0;

		if (fabsf(vy) < 0.1f)
		vy = 0;

		if (fabsf(vz) < 0.1f)
		vz = 0;*/

		if ( flInterval != 0 )
		SetLocalAngularVelocity( QAngle( vx / flInterval, vy / flInterval, vz / flInterval ) );
		return;
	}

	case 3:
	{
		float p = 0;
		CPathTrack *pForwardSplineNode = IsDirForward() ? pNextNode : pPrevNode;
		CPathTrack *pBackwardSplineNode = IsDirForward() ? pPrevNode : pNextNode;

		if ( pForwardSplineNode && pBackwardSplineNode )
		{
			Vector vecSegment = pForwardSplineNode->GetLocalOrigin() - pBackwardSplineNode->GetLocalOrigin();
			const float flSegmentLen = vecSegment.Length();
			if ( flSegmentLen )
			{
				Vector vecCurOffset = GetLocalOrigin() - pBackwardSplineNode->GetLocalOrigin();
				p = vecCurOffset.Length() / flSegmentLen;
			}
		}

		p = SimpleSplineRemapVal( p, 0.0f, 1.0f, 0.0f, 1.0f );

		Quaternion smoothedQuaternion;
		QuaternionSlerp( RadianEuler( curAngles ), RadianEuler( wantedAngles ), gpGlobals->frametime * p * sv_downfall_train_smoothness.GetFloat(), smoothedQuaternion );
		QAngle smoothedAngles;
		QuaternionAngles( smoothedQuaternion, smoothedAngles );

		float vx = UTIL_AngleDistance( smoothedAngles.x, curAngles.x );
		float vy = UTIL_AngleDistance( smoothedAngles.y, curAngles.y );
		float vz = UTIL_AngleDistance( smoothedAngles.z, curAngles.z );

		if ( fabsf( vx ) < 0.1f )
			vx = 0;

		if ( fabsf( vy ) < 0.1f )
			vy = 0;

		if ( fabsf( vz ) < 0.1f )
			vz = 0;

		if ( flInterval == 0 )
			flInterval = 0.1f;

		SetLocalAngularVelocity( QAngle( vx / flInterval, vy / flInterval, vz / flInterval ) );
		return;
	}

	NO_DEFAULT
	}

	return BaseClass::UpdateTrainOrientation( pNext, pNextNext, nextPos, flInterval );
}

bool CFuncTrainDownfall::OnControls(CBaseEntity* pControls)
{
	return CanBeControlled() && BaseClass::OnControls(pControls);
}

void CFuncTrainDownfall::Spawn()
{
	BaseClass::Spawn();

	SetContextThink( &CFuncTrainDownfall::VisualizeThink, gpGlobals->curtime + 0.1f, "VisualizeThink" );
}

void VisitPath( CPathTrack* current, const Vector* pos, const Color* clr )
{
	if ( current->HasBeenVisited() )
		return;

	if ( pos )
	{
		current->Visit();
		NDebugOverlay::Line( *pos, current->GetAbsOrigin(), clr->r(), clr->g(), clr->b(), true, NDEBUG_PERSIST_TILL_NEXT_SERVER );
	}

	if ( current->m_pnext )
	{
		Color next( 0, 128, 255 );
		VisitPath( current->m_pnext, &current->GetAbsOrigin(), clr ? clr : &next );
	}

	if ( current->m_paltpath )
	{
		Color alt( 0, 255, 128 );
		VisitPath( current->m_paltpath, &current->GetAbsOrigin(), &alt );
	}

	if ( current->m_pprevious && !current->m_pprevious->HasBeenVisited() )
	{
		Color prev( 255, 128, 0 );
		VisitPath( current->m_pprevious, &current->GetAbsOrigin(), clr ? clr : &prev );
	}
}

void CFuncTrainDownfall::VisualizeThink()
{
	if ( sv_show_train_path.GetBool() )
	{
		CPathTrack::BeginIteration();
		VisitPath( m_ppath, NULL, NULL );
		CPathTrack::EndIteration();

		// show segment of path train is actually on
		if ( m_ppath && m_ppath->GetNext() )
		{
			NDebugOverlay::HorzArrow( m_ppath->GetAbsOrigin(), m_ppath->GetNext()->GetAbsOrigin(), 5.0f, 255, 0, 0, 255, false, NDEBUG_PERSIST_TILL_NEXT_SERVER );
		}
		SetContextThink( &CFuncTrainDownfall::VisualizeThink, gpGlobals->curtime, "VisualizeThink" );
		return;
	}
	SetContextThink( &CFuncTrainDownfall::VisualizeThink, gpGlobals->curtime + 1.f, "VisualizeThink" );
}
