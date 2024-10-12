#pragma once
#include <stdio.h>

extern "C"
{
    
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

}

namespace nile 
{
class SampleLua
{
public:

SampleLua();
~SampleLua();
void run();

private:
// initialize Lua interpreter
lua_State* L = luaL_newstate();

int callFib(lua_State *L, int value);

int luaAdd(lua_State* L, int a, int b);

// Brain-dead comman line interpreter

void runInterpreter(lua_State *L);

// C Functions that lua can call
static int cAdd(lua_State *L);

static int average(lua_State *L);


};
} // namespace nile


