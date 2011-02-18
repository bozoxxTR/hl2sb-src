//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "iclassmap.h"
#include "utldict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class classentry_t
{
public:
	classentry_t()
	{
		mapname[ 0 ] = 0;
		factory = 0;
		size = -1;
	}

	char const *GetMapName() const
	{
		return mapname;
	}

	void SetMapName( char const *newname )
	{
		Q_strncpy( mapname, newname, sizeof( mapname ) );
	}

#if defined ( LUA_SDK )
	char const *GetClassName() const
	{
		return classname;
	}

	void SetClassName( char const *newname )
	{
		Q_strncpy( classname, newname, sizeof( classname ) );
	}
#endif

	DISPATCHFUNCTION	factory;
	int					size;
private:
	char				mapname[ 40 ];
#if defined ( LUA_SDK )
	char				classname[ 40 ];
#endif
};

class CClassMap : public IClassMap
{
public:
	virtual void			Add( const char *mapname, const char *classname, int size, DISPATCHFUNCTION factory /*= 0*/ );
	virtual const char		*Lookup( const char *classname );
	virtual C_BaseEntity	*CreateEntity( const char *mapname );
	virtual int				GetClassSize( const char *classname );

private:
	CUtlDict< classentry_t, unsigned short > m_ClassDict;
};

IClassMap& GetClassMap( void )
{
	static CClassMap g_Classmap;
	return g_Classmap;
}

void CClassMap::Add( const char *mapname, const char *classname, int size, DISPATCHFUNCTION factory = 0 )
{
#if defined ( LUA_SDK )
	int c = m_ClassDict.Count();
	int i;

	for ( i = 0; i < c; i++ )
	{
		classentry_t *lookup = &m_ClassDict[ i ];
		if ( !lookup )
			continue;

		if ( !Q_stricmp( lookup->GetMapName(), mapname ) )
		{
			m_ClassDict.RemoveAt( i );
		}
	}
#else
	const char *map = Lookup( classname );
	if ( map && !Q_strcasecmp( mapname, map ) )
		return;

	if ( map )
	{
		int index = m_ClassDict.Find( classname );
		Assert( index != m_ClassDict.InvalidIndex() );
		m_ClassDict.RemoveAt( index );
	}
#endif

	classentry_t element;
	element.SetMapName( mapname );
	element.factory = factory;
	element.size = size;
#if defined ( LUA_SDK )
	element.SetClassName( classname );
	m_ClassDict.Insert( mapname, element );
#else
	m_ClassDict.Insert( classname, element );
#endif
}

#if defined ( LUA_SDK )
const char *CClassMap::Lookup( const char *classname )
{
	int c = m_ClassDict.Count();
	int i;

	for ( i = 0; i < c; i++ )
	{
		classentry_t *lookup = &m_ClassDict[ i ];
		if ( !lookup )
			continue;

		if ( Q_stricmp( lookup->GetClassName(), classname ) )
			continue;

		return lookup->GetMapName();
	}

	return NULL;
}
#else
const char *CClassMap::Lookup( const char *classname )
{
	unsigned short index;
	static classentry_t lookup; 

	index = m_ClassDict.Find( classname );
	if ( index == m_ClassDict.InvalidIndex() )
		return NULL;

	lookup = m_ClassDict.Element( index );
	return lookup.GetMapName();
}
#endif

C_BaseEntity *CClassMap::CreateEntity( const char *mapname )
{
	int c = m_ClassDict.Count();
	int i;

	for ( i = 0; i < c; i++ )
	{
		classentry_t *lookup = &m_ClassDict[ i ];
		if ( !lookup )
			continue;

		if ( Q_stricmp( lookup->GetMapName(), mapname ) )
			continue;

		if ( !lookup->factory )
		{
#if defined( _DEBUG )
			Msg( "No factory for %s/%s\n", lookup->GetMapName(), m_ClassDict.GetElementName( i ) );
#endif
			continue;
		}

		return ( *lookup->factory )();
	}

	return NULL;
}

int CClassMap::GetClassSize( const char *classname )
{
	int c = m_ClassDict.Count();
	int i;

	for ( i = 0; i < c; i++ )
	{
		classentry_t *lookup = &m_ClassDict[ i ];
		if ( !lookup )
			continue;

		if ( Q_strcmp( lookup->GetMapName(), classname ) )
			continue;

		return lookup->size;
	}

	return -1;
}
