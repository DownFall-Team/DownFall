#include "shaderlib/cshader.h"
class skymask_ps20b_Static_Index
{
private:
	int m_nDRAWWHITE;
#ifdef _DEBUG
	bool m_bDRAWWHITE;
#endif
public:
	void SetDRAWWHITE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWWHITE = i;
#ifdef _DEBUG
		m_bDRAWWHITE = true;
#endif
	}
	void SetDRAWWHITE( bool i )
	{
		m_nDRAWWHITE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDRAWWHITE = true;
#endif
	}
private:
	int m_nCOMBINE;
#ifdef _DEBUG
	bool m_bCOMBINE;
#endif
public:
	void SetCOMBINE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nCOMBINE = i;
#ifdef _DEBUG
		m_bCOMBINE = true;
#endif
	}
	void SetCOMBINE( bool i )
	{
		m_nCOMBINE = i ? 1 : 0;
#ifdef _DEBUG
		m_bCOMBINE = true;
#endif
	}
private:
	int m_nVIEWMODEL;
#ifdef _DEBUG
	bool m_bVIEWMODEL;
#endif
public:
	void SetVIEWMODEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVIEWMODEL = i;
#ifdef _DEBUG
		m_bVIEWMODEL = true;
#endif
	}
	void SetVIEWMODEL( bool i )
	{
		m_nVIEWMODEL = i ? 1 : 0;
#ifdef _DEBUG
		m_bVIEWMODEL = true;
#endif
	}
public:
	skymask_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bDRAWWHITE = false;
#endif // _DEBUG
		m_nDRAWWHITE = 0;
#ifdef _DEBUG
		m_bCOMBINE = false;
#endif // _DEBUG
		m_nCOMBINE = 0;
#ifdef _DEBUG
		m_bVIEWMODEL = false;
#endif // _DEBUG
		m_nVIEWMODEL = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bDRAWWHITE && m_bCOMBINE && m_bVIEWMODEL;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nDRAWWHITE ) + ( 2 * m_nCOMBINE ) + ( 6 * m_nVIEWMODEL ) + 0;
	}
};
#define shaderStaticTest_skymask_ps20b psh_forgot_to_set_static_DRAWWHITE + psh_forgot_to_set_static_COMBINE + psh_forgot_to_set_static_VIEWMODEL + 0
class skymask_ps20b_Dynamic_Index
{
public:
	skymask_ps20b_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_skymask_ps20b 0
