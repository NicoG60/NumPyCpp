#ifndef GLOBAL_H
#define GLOBAL_H

#include <array>
#include <filesystem>
namespace fs = std::filesystem;

extern const fs::path FILES_DIR;

extern const fs::path TYPE_DIR;

extern const fs::path NPY_B;

extern const fs::path NPY_I8;
extern const fs::path NPY_I16;
extern const fs::path NPY_I32;
extern const fs::path NPY_I64;

extern const fs::path NPY_U8;
extern const fs::path NPY_U16;
extern const fs::path NPY_U32;
extern const fs::path NPY_U64;

extern const fs::path NPY_F16;
extern const fs::path NPY_F32;
extern const fs::path NPY_F64;

extern const fs::path NPZ;
extern const fs::path NPZ_GOOD;
extern const fs::path NPZ_HUGE;
extern const fs::path NPY_HUGE;

extern const std::array<fs::path, 11> NPY_TYPE_FILES;

#endif // GLOBAL_H
