/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "LuaMetalMap.h"

#include "LuaInclude.h"

#include "LuaHandle.h"
#include "Map/MetalMap.h"
#include "Map/ReadMap.h"

/******************************************************************************/
/******************************************************************************/

bool LuaMetalMap::PushReadEntries(lua_State* L)
{
#define REGISTER_LUA_CFUNC(x) \
	lua_pushstring(L, #x);      \
	lua_pushcfunction(L, x);    \
	lua_rawset(L, -3)

	REGISTER_LUA_CFUNC(GetMetalMapSize);
	REGISTER_LUA_CFUNC(GetMetalAmount);
	REGISTER_LUA_CFUNC(GetMetalExtraction);

	return true;
}

bool LuaMetalMap::PushCtrlEntries(lua_State* L)
{
#define REGISTER_LUA_CFUNC(x) \
	lua_pushstring(L, #x);      \
	lua_pushcfunction(L, x);    \
	lua_rawset(L, -3)

	REGISTER_LUA_CFUNC(SetMetalAmount);

	return true;
}

int LuaMetalMap::GetMetalMapSize(lua_State* L)
{
	lua_pushnumber(L, readMap->metalMap->GetSizeX());
	lua_pushnumber(L, readMap->metalMap->GetSizeZ());
	return 2;
}

int LuaMetalMap::GetMetalAmount(lua_State* L)
{
	const int x = luaL_checkint(L, 1);
	const int z = luaL_checkint(L, 2);
	// GetMetalAmount automatically clamps the value
	lua_pushnumber(L, readMap->metalMap->GetMetalAmount(x, z));
	return 1;
}

int LuaMetalMap::SetMetalAmount(lua_State* L)
{
	const int x = luaL_checkint(L, 1);
	const int z = luaL_checkint(L, 2);
	const float m = luaL_checkfloat(L, 3);
	// SetMetalAmount automatically clamps the value
	readMap->metalMap->SetMetalAmount(x, z, m);
	return 0;
}

int LuaMetalMap::GetMetalExtraction(lua_State* L)
{
	const int x = luaL_checkint(L, 1);
	const int z = luaL_checkint(L, 2);
	// GetMetalExtraction automatically clamps the value
	lua_pushnumber(L, readMap->metalMap->GetMetalExtraction(x, z));
	return 1;
}




/******************************************************************************/
/******************************************************************************/
