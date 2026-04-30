#include "evrp/sdk/luaeventcomposer/luabindings.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "evrp/sdk/cursor/cursorpos.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/inputeventwriter.h"
#include "evrp/sdk/keyboard/keyboardeventwriter.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/mouse/mouseeventwriter.h"
#include "evrp/sdk/playbackeventcollector.h"

namespace evrp {
namespace lua {

namespace {

KeyboardEventWriter* gKeyboard = nullptr;
MouseEventWriter* gMouse = nullptr;

static bool getDryRun(lua_State* L) {
  lua_getglobal(L, "evrp");
  lua_getfield(L, -1, "dry_run");
  bool dryRun = lua_toboolean(L, -1);
  lua_pop(L, 2);
  return dryRun;
}

int luaPress(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gKeyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  bool ok = gKeyboard->press(static_cast<unsigned short>(code));
  lua_pushboolean(L, ok);
  return 1;
}

int luaRelease(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gKeyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  bool ok = gKeyboard->release(static_cast<unsigned short>(code));
  lua_pushboolean(L, ok);
  return 1;
}

int luaClick(lua_State* L) {
  lua_Integer code = luaL_checkinteger(L, 1);
  if (code < 0 || code > 0xFFFF) {
    return luaL_error(L, "invalid key code: %lld", (long long)code);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gKeyboard) {
    return luaL_error(L, "evrp keyboard not initialized");
  }
  unsigned short c = static_cast<unsigned short>(code);
  bool ok = gKeyboard->press(c) && gKeyboard->release(c);
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseMove(lua_State* L) {
  lua_Integer dx = luaL_checkinteger(L, 1);
  lua_Integer dy = luaL_checkinteger(L, 2);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->move(static_cast<int>(dx), static_cast<int>(dy));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseMoveToScreen(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->moveToScreen(static_cast<int>(x), static_cast<int>(y));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseGetPosition(lua_State* L) {
  int x = 0, y = 0;
  bool ok = gCursor && gCursor->getPosition(&x, &y);
  if (!ok) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  return 2;
}

int luaMouseIsCursorAvailable(lua_State* L) {
  lua_pushboolean(L, gCursor && gCursor->isAvailable());
  return 1;
}

int luaMouseMoveTo(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->moveTo(static_cast<int>(x), static_cast<int>(y));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseMoveToScaled(lua_State* L) {
  lua_Integer x = luaL_checkinteger(L, 1);
  lua_Integer y = luaL_checkinteger(L, 2);
  lua_Integer width = luaL_checkinteger(L, 3);
  lua_Integer height = luaL_checkinteger(L, 4);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->moveToScaled(static_cast<int>(x), static_cast<int>(y),
                                    static_cast<int>(width),
                                    static_cast<int>(height));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseScrollV(lua_State* L) {
  lua_Integer value = luaL_checkinteger(L, 1);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->scrollV(static_cast<int>(value));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseScrollH(lua_State* L) {
  lua_Integer value = luaL_checkinteger(L, 1);
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->scrollH(static_cast<int>(value));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseButtonDown(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->buttonDown(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseButtonUp(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->buttonUp(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

int luaMouseButtonClick(lua_State* L) {
  lua_Integer btn = luaL_checkinteger(L, 1);
  if (btn < 0 || btn > 0xFFFF) {
    return luaL_error(L, "invalid button code: %lld", (long long)btn);
  }
  if (getDryRun(L)) {
    lua_pushboolean(L, true);
    return 1;
  }
  if (!gMouse) {
    return luaL_error(L, "evrp mouse not initialized");
  }
  bool ok = gMouse->buttonClick(static_cast<unsigned short>(btn));
  lua_pushboolean(L, ok);
  return 1;
}

void registerEvrpTable(lua_State* L) {
  lua_newtable(L);

  lua_pushboolean(L, false);
  lua_setfield(L, -2, "dry_run");

  lua_newtable(L);
  lua_pushcfunction(L, luaPress);
  lua_setfield(L, -2, "press");
  lua_pushcfunction(L, luaRelease);
  lua_setfield(L, -2, "release");
  lua_pushcfunction(L, luaClick);
  lua_setfield(L, -2, "click");

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

  lua_newtable(L);
  lua_pushcfunction(L, luaMouseMove);
  lua_setfield(L, -2, "move");
  lua_pushcfunction(L, luaMouseMoveToScreen);
  lua_setfield(L, -2, "move_to_screen");
  lua_pushcfunction(L, luaMouseMoveTo);
  lua_setfield(L, -2, "move_to");
  lua_pushcfunction(L, luaMouseMoveToScaled);
  lua_setfield(L, -2, "move_to_scaled");
  lua_pushcfunction(L, luaMouseGetPosition);
  lua_setfield(L, -2, "get_position");
  lua_pushcfunction(L, luaMouseIsCursorAvailable);
  lua_setfield(L, -2, "is_cursor_available");
  lua_pushcfunction(L, luaMouseScrollV);
  lua_setfield(L, -2, "scroll_v");
  lua_pushcfunction(L, luaMouseScrollH);
  lua_setfield(L, -2, "scroll_h");
  lua_pushcfunction(L, luaMouseButtonDown);
  lua_setfield(L, -2, "button_down");
  lua_pushcfunction(L, luaMouseButtonUp);
  lua_setfield(L, -2, "button_up");
  lua_pushcfunction(L, luaMouseButtonClick);
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

}

namespace {

int runLuaWithBindings(KeyboardEventWriter* keyboard, MouseEventWriter* mouse,
                       bool fromFile, const char* path, const char* chunk) {
  if (!keyboard || !mouse) {
    return LUA_ERRRUN;
  }
  if (fromFile && !path) {
    return LUA_ERRRUN;
  }
  if (!fromFile && (!chunk || !chunk[0])) {
    return LUA_ERRRUN;
  }
  lua_State* L = luaL_newstate();
  if (!L) {
    return LUA_ERRMEM;
  }
  luaL_openlibs(L);

  gKeyboard = keyboard;
  gMouse = mouse;
  registerEvrpTable(L);

  int err;
  if (fromFile) {
    err = luaL_dofile(L, path);
  } else {
    err = luaL_loadbuffer(L, chunk, std::strlen(chunk), "=(playback)");
    if (err == LUA_OK) {
      err = lua_pcall(L, 0, 0, 0);
    }
  }
  gKeyboard = nullptr;
  gMouse = nullptr;

  if (err != LUA_OK) {
    const char* msg = lua_tostring(L, -1);
    logError("Lua error: {}", msg ? msg : "(null)");
    lua_pop(L, 1);
  }
  lua_close(L);
  return err;
}

}

int runScriptWithWriter(const char* path, InputEventWriter* writer) {
  if (!path || !writer) {
    return LUA_ERRRUN;
  }
  return runLuaWithBindings(writer->keyboardWriter(), writer->mouseWriter(),
                            true, path, nullptr);
}

int runScript(const char* path) {
  auto fs = createEnhancedFileSystem();
  InputEventWriter writer(fs.get());
  return runScriptWithWriter(path, &writer);
}

int executeChunk(InputEventWriter* writer, const char* chunk) {
  if (!writer || !chunk) {
    return LUA_ERRRUN;
  }
  return runLuaWithBindings(writer->keyboardWriter(), writer->mouseWriter(),
                            false, nullptr, chunk);
}

int playbackLuaFileIntoCollector(const char* path,
                                 PlaybackEventCollector* collector) {
  if (!path || !collector) {
    return LUA_ERRRUN;
  }
  collector->clear();
  KeyboardEventWriter keyboard(collector);
  MouseEventWriter mouse(collector, gCursor);
  return runLuaWithBindings(&keyboard, &mouse, true, path, nullptr);
}

int playbackLuaChunkIntoCollector(const char* chunk,
                                  PlaybackEventCollector* collector) {
  if (!chunk || !collector) {
    return LUA_ERRRUN;
  }
  collector->clear();
  KeyboardEventWriter keyboard(collector);
  MouseEventWriter mouse(collector, gCursor);
  return runLuaWithBindings(&keyboard, &mouse, false, nullptr, chunk);
}

}
}
