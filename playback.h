#pragma once

#include <string>

// Playback events from file into the input subsystem via real device files.
// When quiet is false, also print each event line to stdout.
// Returns 0 on success, 1 on error.
int playback_file_to_uinput(const std::string& path, bool quiet = false);
