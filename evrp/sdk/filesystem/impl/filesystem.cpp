#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/filesystem/impl/posixfilesystem.h"

IFileSystem *createFileSystem() {
  return new PosixFileSystem();
}
