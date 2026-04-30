#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/filesystem/posixfilesystem.h"

std::unique_ptr<IFileSystem> createFileSystem() {
  return std::make_unique<PosixFileSystem>();
}
