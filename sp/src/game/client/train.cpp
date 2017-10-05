//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Train.cpp
//
// implementation of CHudAmmo class
//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CHudTrain: public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudTrain, vgui::Panel );
public:
	CHudTrain( const char *pElementName );
	void Init( void );
	void VidInit( void );
	bool ShouldDraw( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void MsgFunc_Train(bf_read &msg);

private:
	int m_iPos;
	CHudTexture* m_pArSpeedIcons[5];

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "40", "proportional_float" );
	CPanelAnimationVar( int, size, "size", "32" );
};

//
//-----------------------------------------------------
//

DECLARE_HUDELEMENT( CHudTrain );
DECLARE_HUD_MESSAGE( CHudTrain, Train )

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTrain::CHudTrain( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudTrain" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	
	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scheme - 
//-----------------------------------------------------------------------------
void CHudTrain::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTrain::Init(void)
{
	HOOK_HUD_MESSAGE( CHudTrain, Train );

	m_iPos = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTrain::VidInit(void)
{
	m_pArSpeedIcons[0] = gHUD.GetIcon( "train0" );
	m_pArSpeedIcons[1] = gHUD.GetIcon( "train1" );
	m_pArSpeedIcons[2] = gHUD.GetIcon( "train2" );
	m_pArSpeedIcons[3] = gHUD.GetIcon( "train3" );
	m_pArSpeedIcons[4] = gHUD.GetIcon( "train4" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudTrain::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && m_iPos );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTrain::Paint()
{
	if ( m_iPos > 0 && m_iPos < 6 )
		m_pArSpeedIcons[m_iPos - 1]->DrawSelf( icon_xpos, icon_ypos, size, size, gHUD.m_clrYellowish );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTrain::MsgFunc_Train( bf_read &msg )
{
	// update Train data
	m_iPos = msg.ReadByte();
	//static const Color k( 0, 128, 255, 255 );
	//ConColorMsg( k, "Train: %d\n", m_iPos );
}
