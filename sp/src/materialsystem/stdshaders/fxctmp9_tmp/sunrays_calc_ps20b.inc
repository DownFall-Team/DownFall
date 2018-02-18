#include "shaderlib/cshader.h"
class sunrays_calc_ps20b_Static_Index
{
public:
	sunrays_calc_ps20b_Static_Index( )
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
#define shaderStaticTest_sunrays_calc_ps20b 0
class sunrays_calc_ps20b_Dynamic_Index
{
private:
	int m_nINTENSITY;
#ifdef _DEBUG
	bool m_bINTENSITY;
#endif
public:
	void SetINTENSITY( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nINTENSITY = i;
#ifdef _DEBUG
		m_bINTENSITY = true;
#endif
	}
	void SetINTENSITY( bool i )
	{
		m_nINTENSITY = i ? 1 : 0;
#ifdef _DEBUG
		m_bINTENSITY = true;
#endif
	}
public:
	sunrays_calc_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bINTENSITY = false;
#endif // _DEBUG
		m_nINTENSITY = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bINTENSITY;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nINTENSITY ) + 0;
	}
};
#define shaderDynamicTest_sunrays_calc_ps20b psh_forgot_to_set_dynamic_INTENSITY + 0
