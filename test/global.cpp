#include "global.h"

const fs::path FILES_DIR  = fs::current_path()/"files";

const fs::path TYPE_DIR = FILES_DIR/"numpy-types";

const fs::path NPY_B = TYPE_DIR/"bool.npy";

const fs::path NPY_I8  = TYPE_DIR/"int8.npy";
const fs::path NPY_I16 = TYPE_DIR/"int16.npy";
const fs::path NPY_I32 = TYPE_DIR/"int32.npy";
const fs::path NPY_I64 = TYPE_DIR/"int64.npy";

const fs::path NPY_U8  = TYPE_DIR/"uint8.npy";
const fs::path NPY_U16 = TYPE_DIR/"uint16.npy";
const fs::path NPY_U32 = TYPE_DIR/"uint32.npy";
const fs::path NPY_U64 = TYPE_DIR/"uint64.npy";

const fs::path NPY_F16 = TYPE_DIR/"float16.npy";
const fs::path NPY_F32 = TYPE_DIR/"float32.npy";
const fs::path NPY_F64 = TYPE_DIR/"float64.npy";

const fs::path NPZ      = FILES_DIR/"numpy-types.npz";
const fs::path NPZ_GOOD = FILES_DIR/"numpy-good.npz";
const fs::path NPZ_HUGE = FILES_DIR/"huge.npz";
const fs::path NPY_HUGE = FILES_DIR/"huge.npy";

const std::array<fs::path, 11> NPY_TYPE_FILES = {
    NPY_B,
    NPY_I8, NPY_I16, NPY_I32, NPY_I64,
    NPY_U8, NPY_U16, NPY_U32, NPY_U64,
                     NPY_F32, NPY_F64
};