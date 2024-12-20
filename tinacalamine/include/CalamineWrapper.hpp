#pragma once

#include "rust/cxx.h"

#include <string>

namespace ffi {
	struct FileHandle;
	rust::Box<FileHandle> openFile(rust::Str filePath);
}