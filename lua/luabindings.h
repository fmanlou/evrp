#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

class InputEventWriter;

namespace evrp {
namespace lua {

int runScript(const char* path);

/// Runs a .lua file using the given writer (local evdev or remote via
/// InputEventWriter::setRemotePlayback).
int runScriptWithWriter(const char* path, InputEventWriter* writer);

int executeChunk(InputEventWriter* writer, const char* chunk);

}
}
