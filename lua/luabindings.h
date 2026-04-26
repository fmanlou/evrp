#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

class InputEventWriter;
class PlaybackEventCollector;

namespace evrp {
namespace lua {

int runScript(const char* path);

/// Runs a .lua file using the given writer (local evdev).
int runScriptWithWriter(const char* path, InputEventWriter* writer);

int executeChunk(InputEventWriter* writer, const char* chunk);

/// Runs Lua from file; on LUA_OK, \a collector holds input events (not sent yet).
int playbackLuaFileIntoCollector(const char* path,
                                 PlaybackEventCollector* collector);

/// Runs Lua from a string chunk; on LUA_OK, \a collector holds input events.
int playbackLuaChunkIntoCollector(const char* chunk,
                                  PlaybackEventCollector* collector);

}
}
