#include <lua.h>
#include <lapi.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>

extern void printk(char* s);
extern void printki(long n);
extern void printkid(long n);


void lua_stuff()
{
	printk("lua_stuff() starting\n");

	lua_State *l;
	l = luaL_newstate();

	luaL_openlibs(l);

	int lsv = luaL_loadstring(l, "print(\"Hello from 64-bit Lua!\")");
	if (lsv != LUA_OK) {
		if (lsv == LUA_ERRSYNTAX) {
			printk("LUA_ERRSYNTAX\n");
		} else if (lsv == LUA_ERRMEM) {
			printk("LUA_ERRMEM\n");
		} else if (lsv == LUA_ERRGCMM) {
			printk("LUA_ERRGCMM\n");
		} else {
			printki(lsv); printk("\n");
		}
	}

	int pcv = lua_pcall(l, 0, LUA_MULTRET, 0);
	if (pcv != LUA_OK) {
		if (pcv == LUA_ERRRUN) {
			printk("LUA_ERRRUN\n");
		} else if (pcv == LUA_ERRMEM) {
			printk("LUA_ERRMEM\n");
		} else if (pcv == LUA_ERRGCMM) {
			printk("LUA_ERRGCMM\n");
		} else {
			printki(pcv); printk("\n");
		}
	}

}
