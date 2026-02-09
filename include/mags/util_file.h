#ifndef MAGS_REWRITE_UTIL_FILE_H
#define MAGS_REWRITE_UTIL_FILE_H
#include "types.h"

#include <string>

namespace mags::io {

Graph read_from_file(const std::string &path);

}

#endif // MAGS_REWRITE_UTIL_FILE_H