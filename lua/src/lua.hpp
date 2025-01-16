// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#ifndef LUA_ESP32_HPP
#define LUA_ESP32_HPP

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#endif