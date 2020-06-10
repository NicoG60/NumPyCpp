#ifndef GLOBAL_H
#define GLOBAL_H

#include <numpycpp.h>

#define FOLDER "./files/numpy-types"

#define NPY_B	FOLDER"/bool.npy"
#define NPY_I8	FOLDER"/int8.npy"
#define NPY_I16 FOLDER"/int16.npy"
#define NPY_I32 FOLDER"/int32.npy"
#define NPY_I64 FOLDER"/int64.npy"
#define NPY_U8	FOLDER"/uint8.npy"
#define NPY_U16 FOLDER"/uint16.npy"
#define NPY_U32 FOLDER"/uint32.npy"
#define NPY_U64 FOLDER"/uint64.npy"
#define NPY_F16 FOLDER"/float16.npy"
#define NPY_F32 FOLDER"/float32.npy"
#define NPY_F64 FOLDER"/float64.npy"

#define NPY_SIZE 11

#define NPZ "./files/numpy-types.npz"
#define NPZ_GOOD "./files/numpy-good.npz"
#define NPZ_HUGE "./files/huge.npz"
#define NPY_HUGE "./files/huge.npy"

static const char* files[] = {NPY_B,
                                NPY_I8, NPY_I16, NPY_I32, NPY_I64,
                                NPY_U8, NPY_U16, NPY_U32, NPY_U64,
                                                NPY_F32, NPY_F64};

#endif // GLOBAL_H
