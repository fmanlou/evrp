#pragma once

#include <memory>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

class InputEventWriter;

namespace evrp::device::api {
class IPlayback;
}

namespace evrp {
namespace lua {

int runScript(const char* path);

/// Runs a .lua file using the given writer (local evdev or remote via
/// InputEventWriter::setRemotePlayback).
int runScriptWithWriter(const char* path, InputEventWriter* writer);

int executeChunk(InputEventWriter* writer, const char* chunk);

/// Runs a .lua file sending input through the device playback service.
int runScriptWithPlayback(const char* path, device::api::IPlayback* playback);

/// Reuses one remote-backed writer for many inline Lua chunks (playback files).
class RemoteLuaChunkRunner {
 public:
  explicit RemoteLuaChunkRunner(device::api::IPlayback* playback);
  ~RemoteLuaChunkRunner();

  RemoteLuaChunkRunner(const RemoteLuaChunkRunner&) = delete;
  RemoteLuaChunkRunner& operator=(const RemoteLuaChunkRunner&) = delete;
  RemoteLuaChunkRunner(RemoteLuaChunkRunner&&) = default;
  RemoteLuaChunkRunner& operator=(RemoteLuaChunkRunner&&) = default;

  int executeChunk(const char* chunk);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
}
