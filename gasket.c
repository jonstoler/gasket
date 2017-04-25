#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <X11/Xlib.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

static Display *d;
static Window root;
static lua_State *L;
static int width;
static int height;
static char running;
static cairo_t *cr;

int draw(){
	lua_getglobal(L, "draw");
	if(!lua_isnoneornil(L, -1)){
		if(lua_pcall(L, 0, 0, 0)){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(1);
		}
	}
	lua_pop(L, -1);
}

int update(){
	lua_getglobal(L, "update");
	if(!lua_isnoneornil(L, -1)){
		if(lua_pcall(L, 0, 0, 0)){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(1);
		}
	}
	lua_pop(L, -1);
}

int draw_rect(lua_State *L);
int draw_rgb(lua_State *L);

int main() {
	Display *d = XOpenDisplay(NULL);

	int s = DefaultScreen(d);
	root = RootWindow(d, s);

	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, draw_rect);
	lua_setglobal(L, "rect");

	lua_pushcfunction(L, draw_rgb);
	lua_setglobal(L, "rgb");

	if(luaL_loadfile(L, "main.lua")){
		fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
		exit(1);
	}
	if(lua_pcall(L, 0, 0, 0)){
		fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
		exit(1);
	}

	lua_getglobal(L, "width");
	lua_getglobal(L, "height");

	width = (int)luaL_optnumber(L, 1, 0);
	height = (int)luaL_optnumber(L, 2, 0);
	lua_pop(L, 2);

	lua_getglobal(L, "every");
	float every = (float)luaL_optnumber(L, 1, 0);
	lua_pop(L, 1);

	cairo_surface_t *surf = cairo_xlib_surface_create(d, root, DefaultVisual(d, s), width, height);
	cr = cairo_create(surf);

	XSelectInput(d, root, ExposureMask);
	XEvent ev;

	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	
	draw();
	running = 1;
	struct timespec ts = {every, 0};
	while(running){
		XNextEvent(d, &ev);
		if(ev.type == Expose){
			update();
			draw();
		}

		// nanosleep(&ts, NULL);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surf);
	XCloseDisplay(d);

	lua_close(L);

	return 0;
}

int draw_rgb(lua_State *L){
	lua_settop(L, 3);

	float r = (float)luaL_checknumber(L, 1);
	float g = (float)luaL_checknumber(L, 2);
	float b = (float)luaL_checknumber(L, 3);

	cairo_set_source_rgb(cr, r, g, b);
}

int draw_rect(lua_State *L){
	lua_settop(L, 4);

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int w = (int)luaL_checknumber(L, 3);
	int h = (int)luaL_checknumber(L, 4);

	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}
