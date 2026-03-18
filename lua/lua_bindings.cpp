#include "lua/lua_bindings.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "cursor/cursorpos.h"
#include "filesystem.h"
#include "inputeventwriter.h"
#include "keyboard/keyboardeventwriter.h"
#include "logger.h"
#include "mouse/mouseeventwriter.h"

namespace evrp {
namespace lua {

namespace {

KeyboardEventWriter* g_keyboard = nullptr;
MouseEventWriter* g_mouse = nullptr;

static bool get_dry_run(lua_State* L) {
  lua_getglobal(L, "evrp");
  lua_getfield(L, -1, "dry_run");
  bool dry_run = lua_toboolean(L, -1);
  lua_pop(L, 2);
  return dry_run;
}

int lua_press(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_keyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  bool ok = g_keyboard->press(static_cast<unsigned short>(code));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_release(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_keyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  bool ok = g_keyboard->release(static_cast<unsigned short>(code));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_click(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_keyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  unsigned short c = static_cast<unsigned short>(code);
  bool ok = g_keyboard->press(c) && g_keyboard->release(c);
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_move(lua_State* L) {
  lua_Integer dx = luaL_checkinteger(L, 1);
  lua_Integer dy = luaL_checkinteger(L, 2);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->move(static_cast<int>(dx), static_cast<int>(dy));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_move_to_screen(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->move_to_screen(static_cast<int>(x), static_cast<int>(y));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_get_position(lua_State* L) {
  int x = 0, y = 0;
  bool ok = g_cursor && g_cursor->get_position(&x, &y);
  if (!ok) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  return 2;
}

int lua_mouse_is_cursor_available(lua_State* L) {
  lua_pushboolean(L, g_cursor && g_cursor->is_available());
  return 1;
}

int lua_mouse_move_to(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->move_to(static_cast<int>(x), static_cast<int>(y));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_move_to_scaled(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  lua_Integer width = luaL_checkinteger(L, 3);
  lua_Integer height = luaL_checkinteger(L, 4);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->move_to_scaled(static_cast<int>(x), static_cast<int>(y),
                                    static_cast<int>(width),
                                    static_cast<int>(height));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_scroll_v(lua_State* L) {
  lua_Integer value = luaL_checkinteger(L, 1);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->scroll_v(static_cast<int>(value));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_scroll_h(lua_State* L) {
  lua_Integer value = luaL_checkinteger(L, 1);
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->scroll_h(static_cast<int>(value));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_button_down(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->button_down(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_button_up(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->button_up(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

int lua_mouse_button_click(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (get_dry_run(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!g_mouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = g_mouse->button_click(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

void register_evrp_table(lua_State* L) {
  lua_newtable(L);  // evrp

  lua_pushboolean(L, false);
  lua_setfield(L, -2, "dry_run");

  // evrp.keyboard
  lua_newtable(L);
  lua_pushcfunction(L, lua_press);
  lua_setfield(L, -2, "press");
  lua_pushcfunction(L, lua_release);
  lua_setfield(L, -2, "release");
  lua_pushcfunction(L, lua_click);
  lua_setfield(L, -2, "click");

  // Key code constants (from linux/input-event-codes.h)
  lua_pushinteger(L, KEY_A);
  lua_setfield(L, -2, "KEY_A");
  lua_pushinteger(L, KEY_B);
  lua_setfield(L, -2, "KEY_B");
  lua_pushinteger(L, KEY_C);
  lua_setfield(L, -2, "KEY_C");
  lua_pushinteger(L, KEY_D);
  lua_setfield(L, -2, "KEY_D");
  lua_pushinteger(L, KEY_E);
  lua_setfield(L, -2, "KEY_E");
  lua_pushinteger(L, KEY_F);
  lua_setfield(L, -2, "KEY_F");
  lua_pushinteger(L, KEY_G);
  lua_setfield(L, -2, "KEY_G");
  lua_pushinteger(L, KEY_H);
  lua_setfield(L, -2, "KEY_H");
  lua_pushinteger(L, KEY_I);
  lua_setfield(L, -2, "KEY_I");
  lua_pushinteger(L, KEY_J);
  lua_setfield(L, -2, "KEY_J");
  lua_pushinteger(L, KEY_K);
  lua_setfield(L, -2, "KEY_K");
  lua_pushinteger(L, KEY_L);
  lua_setfield(L, -2, "KEY_L");
  lua_pushinteger(L, KEY_M);
  lua_setfield(L, -2, "KEY_M");
  lua_pushinteger(L, KEY_N);
  lua_setfield(L, -2, "KEY_N");
  lua_pushinteger(L, KEY_O);
  lua_setfield(L, -2, "KEY_O");
  lua_pushinteger(L, KEY_P);
  lua_setfield(L, -2, "KEY_P");
  lua_pushinteger(L, KEY_Q);
  lua_setfield(L, -2, "KEY_Q");
  lua_pushinteger(L, KEY_R);
  lua_setfield(L, -2, "KEY_R");
  lua_pushinteger(L, KEY_S);
  lua_setfield(L, -2, "KEY_S");
  lua_pushinteger(L, KEY_T);
  lua_setfield(L, -2, "KEY_T");
  lua_pushinteger(L, KEY_U);
  lua_setfield(L, -2, "KEY_U");
  lua_pushinteger(L, KEY_V);
  lua_setfield(L, -2, "KEY_V");
  lua_pushinteger(L, KEY_W);
  lua_setfield(L, -2, "KEY_W");
  lua_pushinteger(L, KEY_X);
  lua_setfield(L, -2, "KEY_X");
  lua_pushinteger(L, KEY_Y);
  lua_setfield(L, -2, "KEY_Y");
  lua_pushinteger(L, KEY_Z);
  lua_setfield(L, -2, "KEY_Z");
  lua_pushinteger(L, KEY_ENTER);
  lua_setfield(L, -2, "KEY_ENTER");
  lua_pushinteger(L, KEY_SPACE);
  lua_setfield(L, -2, "KEY_SPACE");
  lua_pushinteger(L, KEY_ESC);
  lua_setfield(L, -2, "KEY_ESC");
  lua_pushinteger(L, KEY_LEFTCTRL);
  lua_setfield(L, -2, "KEY_LEFTCTRL");
  lua_pushinteger(L, KEY_RIGHTCTRL);
  lua_setfield(L, -2, "KEY_RIGHTCTRL");
  lua_pushinteger(L, KEY_LEFTSHIFT);
  lua_setfield(L, -2, "KEY_LEFTSHIFT");
  lua_pushinteger(L, KEY_RIGHTSHIFT);
  lua_setfield(L, -2, "KEY_RIGHTSHIFT");

  lua_setglobal(L, "keyboard");

  // mouse table
  lua_newtable(L);
  lua_pushcfunction(L, lua_mouse_move);
  lua_setfield(L, -2, "move");
  lua_pushcfunction(L, lua_mouse_move_to_screen);
  lua_setfield(L, -2, "move_to_screen");
  lua_pushcfunction(L, lua_mouse_move_to);
  lua_setfield(L, -2, "move_to");
  lua_pushcfunction(L, lua_mouse_move_to_scaled);
  lua_setfield(L, -2, "move_to_scaled");
  lua_pushcfunction(L, lua_mouse_get_position);
  lua_setfield(L, -2, "get_position");
  lua_pushcfunction(L, lua_mouse_is_cursor_available);
  lua_setfield(L, -2, "is_cursor_available");
  lua_pushcfunction(L, lua_mouse_scroll_v);
  lua_setfield(L, -2, "scroll_v");
  lua_pushcfunction(L, lua_mouse_scroll_h);
  lua_setfield(L, -2, "scroll_h");
  lua_pushcfunction(L, lua_mouse_button_down);
  lua_setfield(L, -2, "button_down");
  lua_pushcfunction(L, lua_mouse_button_up);
  lua_setfield(L, -2, "button_up");
  lua_pushcfunction(L, lua_mouse_button_click);
  lua_setfield(L, -2, "button_click");
  lua_pushinteger(L, BTN_LEFT);
  lua_setfield(L, -2, "BTN_LEFT");
  lua_pushinteger(L, BTN_RIGHT);
  lua_setfield(L, -2, "BTN_RIGHT");
  lua_pushinteger(L, BTN_MIDDLE);
  lua_setfield(L, -2, "BTN_MIDDLE");
  lua_setglobal(L, "mouse");

  lua_setglobal(L, "evrp");
}

}  // namespace

int run_script(const char* path) {
  lua_State* L = luaL_newstate();
  if (!L) return -1;
  luaL_openlibs(L);

  FileSystem fs;
  InputEventWriter writer(&fs);
  g_keyboard = writer.keyboard_writer();
  g_mouse = writer.mouse_writer();
  register_evrp_table(L);

  int err = luaL_dofile(L, path);
  g_keyboard = nullptr;
  g_mouse = nullptr;

  if (err != LUA_OK) {
    log_error(std::string("Lua error: ") + lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_close(L);
  return err;
}

int execute_chunk(InputEventWriter* writer, const char* chunk) {
  if (!writer || !chunk) return LUA_ERRRUN;
  lua_State* L = luaL_newstate();
  if (!L) return LUA_ERRMEM;
  luaL_openlibs(L);

  g_keyboard = writer->keyboard_writer();
  g_mouse = writer->mouse_writer();
  register_evrp_table(L);

  int err = luaL_loadbuffer(L, chunk, std::strlen(chunk), "=(playback)");
  if (err == LUA_OK) {
    err = lua_pcall(L, 0, 0, 0);
  }
  g_keyboard = nullptr;
  g_mouse = nullptr;

  if (err != LUA_OK) {
    log_error(std::string("Lua error: ") + lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_close(L);
  return err;
}

}  // namespace lua
}  // namespace evrp
