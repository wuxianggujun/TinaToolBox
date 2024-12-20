#include "CalamineWrapper.hpp"
#include "tinacalamine/src/lib.rs.h"

namespace ffi {
    rust::Box<FileHandle> openFile(rust::Str filePath) {
        return rust_open_file(filePath);
    }
}