#include "shaderlib/cshader.h"
class fbquad_vs11_Static_Index
{
public:
	fbquad_vs11_Static_Index( )
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
#define shaderStaticTest_fbquad_vs11 0
class fbquad_vs11_Dynamic_Index
{
public:
	fbquad_vs11_Dynamic_Index()
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
#define shaderDynamicTest_fbquad_vs11 0
