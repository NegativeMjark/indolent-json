import ctypes

lib = ctypes.cdll.LoadLibrary("./libijson.so");

def compact(value):
    out = ctypes.create_string_buffer(len(value))
    out_len = lib.ijson_compact(value, len(value), out)
    return out.raw[:out_len]
