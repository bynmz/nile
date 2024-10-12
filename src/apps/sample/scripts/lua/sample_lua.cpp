#include "sample_lua.hpp"

namespace nile 
{
SampleLua::SampleLua() {
// load Lua base libraries (print / math / etc)
luaL_openlibs(L);

lua_register(L, "cAdd", SampleLua::cAdd);
lua_register(L, "average", SampleLua::average);
luaL_dofile(L, "../src/apps/sample/scripts/lua/myFile.lua");
}

SampleLua::~SampleLua() {}

void SampleLua::run()
{

int fibResult = callFib(L, 10);
printf("Result from calling fib(10) from C: %d \n", fibResult);

int luaAddResult = luaAdd(L, 4, 6);
printf("Result from calling add(4,6) from C: %d \n", luaAddResult);

runInterpreter(L);


// Cleanup: Deallocate all space associated with the lua state */
lua_close(L);

// Hack to prevent program from ending immediately
printf( "Press enter to exit..." );
getchar();
}

int SampleLua::callFib(lua_State *L, int value)
{
    // Push the fib function on the top of the lua stack
    lua_getglobal(L, "fib");

    // Push the argument (the number 13) on the stack
    lua_pushnumber(L, 13);

    // call the function with 1 argument, returning a single result. Note that the function actually
    // returns 2 results -- we just want one of them. The second result will *not* be pushed on the
    // lua stack, so we don't need to clean up after it
    lua_call(L, 1, 1);

    // Get the result from the lua stack
    int result = (int)lua_tointeger(L, -1);

    // Clean up. If we don't do this last step, we'll leak stack memory.
    lua_pop(L, 1);

    return result;
}

int SampleLua::luaAdd(lua_State* L, int a, int b)
{
    // Push the add function on the top of the lua stack
    lua_getglobal(L, "add");

    // Push the first argument on the top of the lua stack
    lua_pushnumber(L, a);

    // Push the second argument on the top of the lua stack
    lua_pushnumber(L, b);

    // Call the function with 2 arguments, returning 1 result
    lua_call(L, 2, 1);

    // Get the result
    int sum = (int)lua_tointeger(L, -1);

    // The one result that was returned needs to be popped off. If the 3rd
    // parameter to lua_call was larger than 1, we would need to pop off more
    // elements from the lua stack.
    lua_pop(L, 1);

    return sum;
}

// Brain-dead comman line interpreter

void SampleLua::runInterpreter(lua_State *L)
{
    char inputBuffer[500];
    while (true)
    {
        printf(">");
        fgets(inputBuffer, sizeof inputBuffer, stdin);
        luaL_dostring(L, inputBuffer);
    }
}

// C Functions that lua can call
int SampleLua::cAdd(lua_State *L)
{
    // Step 1: extract the parameters from the lua stack:
    double n1 = lua_tonumber(L,1);
    double n2 = lua_tonumber(L,2);

    // Step 2: Do the actual calculation. Normally, this will be more interesting than a single sum!
    double sum = n1 + n2;

    // Step 3: Push the result on the lua stack.
    lua_pushnumber(L,sum);

    // Return the number of arguments we pushed onto the stack (that is, the number of return values this
    // function has
    return 1;
}

int SampleLua::average(lua_State *L)
{
    // Get the number of parameters
    int n = lua_gettop(L);

    double sum = 0;
    int i;

    // loop through each argument, adding them up
    for (i = 1; i <= n; i++)
    {
        sum += lua_tonumber(L, i);
    }

    // push the average on the lua stack
    lua_pushnumber(L, sum / n);

    // push the sum on the lua stack
    lua_pushnumber(L, sum);

    // return the number of results we pushed on the stack
    return 2;
}

} // namespace nile



