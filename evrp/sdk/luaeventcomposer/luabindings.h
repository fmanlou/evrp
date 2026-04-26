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

int runScriptWithWriter(const char* path, InputEventWriter* writer);

int executeChunk(InputEventWriter* writer, const char* chunk);

int playbackLuaFileIntoCollector(const char* path,
                                 PlaybackEventCollector* collector);

int playbackLuaChunkIntoCollector(const char* chunk,
                                  PlaybackEventCollector* collector);

}
}
