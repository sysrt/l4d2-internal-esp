#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include "vector.h"

inline HMODULE client_dll, engine_dll, server_dll;