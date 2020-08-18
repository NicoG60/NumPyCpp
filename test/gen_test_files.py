import numpy as np
from random import random
import os

# Create directory structure if needed
test_dir  = os.path.dirname(os.path.realpath(__file__))
files_dir = os.path.join(test_dir, "files")
types_dir = os.path.join(files_dir, "numpy-types")

os.makedirs(name=types_dir, exist_ok=True)

# Create types simple array
int8  = np.array([-1, 0, 1, 2, 3], dtype=np.int8)
int16 = np.array([-1, 0, 1, 2, 3], dtype=np.int16)
int32 = np.array([-1, 0, 1, 2, 3], dtype=np.int32)
int64 = np.array([-1, 0, 1, 2, 3], dtype=np.int64)

uint8  = np.array([-1, 0, 1, 2, 3], dtype=np.uint8)
uint16 = np.array([-1, 0, 1, 2, 3], dtype=np.uint16)
uint32 = np.array([-1, 0, 1, 2, 3], dtype=np.uint32)
uint64 = np.array([-1, 0, 1, 2, 3], dtype=np.uint64)

float16 = np.array([-1, 0, 1, 2, 3], dtype=np.float16)
float32 = np.array([-1, 0, 1, 2, 3], dtype=np.float32)
float64 = np.array([-1, 0, 1, 2, 3], dtype=np.float64)

boolean = np.array([-1, 0, 1, 2, 3], dtype=np.bool)

# Save them
np.save(os.path.join(types_dir, 'int8.npy'),  int8)
np.save(os.path.join(types_dir, 'int16.npy'), int16)
np.save(os.path.join(types_dir, 'int32.npy'), int32)
np.save(os.path.join(types_dir, 'int64.npy'), int64)

np.save(os.path.join(types_dir, 'uint8.npy'),  uint8)
np.save(os.path.join(types_dir, 'uint16.npy'), uint16)
np.save(os.path.join(types_dir, 'uint32.npy'), uint32)
np.save(os.path.join(types_dir, 'uint64.npy'), uint64)

np.save(os.path.join(types_dir, 'float16.npy'), float16)
np.save(os.path.join(types_dir, 'float32.npy'), float32)
np.save(os.path.join(types_dir, 'float64.npy'), float64)

np.save(os.path.join(types_dir, 'bool.npy'), boolean)

# Save npz
np.savez(
    file=os.path.join(files_dir, 'npz-with-f16.npz'),
    int8    = int8,   
    int16   = int16,  
    int32   = int32, 
    int64   = int64,
    uint8   = uint8,
    uint16  = uint16,
    uint32  = uint32,
    uint64  = uint64,
    float16 = float16,
    float32 = float32,
    float64 = float64,
    bool    = boolean
)

np.savez(
    file=os.path.join(files_dir, 'npz-all-types.npz'),
    int8    = int8,   
    int16   = int16,  
    int32   = int32, 
    int64   = int64,
    uint8   = uint8,
    uint16  = uint16,
    uint32  = uint32,
    uint64  = uint64,
    float32 = float32,
    float64 = float64,
    bool    = boolean
)

# Creates some bigger files
# File will be like weather resource data
# let's say we have 20 years worth of data, 1 data point every 3 hours
# it will generate some random datas, with a constant timestamp step

years = 20
in_hours = years*365*24 # approx, does not vount leap years and stuff
nb_points = int(in_hours / 3)

timestamp = np.zeros(nb_points, np.int64)
wave_h    = np.zeros(nb_points, np.float64)
wave_p    = np.zeros(nb_points, np.float64)
wave_dir  = np.zeros(nb_points, np.float64)
wind_sp   = np.zeros(nb_points, np.float64)
wind_dir  = np.zeros(nb_points, np.float64)

npy = np.zeros(nb_points, np.dtype([
    ('timestamp', np.int64),
    ('wave_h'   , np.float64),
    ('wave_p'   , np.float64),
    ('wave_dir' , np.float64),
    ('wind_sp'  , np.float64),
    ('wind_dir' , np.float64)
]))

for i in range(nb_points):
    timestamp[i] = i * 3 * 3600
    wave_h[i]    = random() * 10
    wave_p[i]    = random() / 2
    wave_dir[i]  = random() * 360
    wind_sp[i]   = random() * 50
    wind_dir[i]  = random() * 360

    npy[i]['timestamp'] = timestamp[i]
    npy[i]['wave_h']    = wave_h[i]
    npy[i]['wave_p']    = wave_p[i]
    npy[i]['wave_dir']  = wave_dir[i]
    npy[i]['wind_sp']   = wind_sp[i]
    npy[i]['wind_dir']  = wind_dir[i]

np.save(os.path.join(files_dir, 'huge.npy'), npy)
np.savez(
    file=os.path.join(files_dir, 'huge.npz'),
    timestamp = timestamp,
    wave_h    = wave_h,
    wave_p    = wave_p,
    wave_dir  = wave_dir,
    wind_sp   = wind_sp,
    wind_dir  = wind_dir
)

