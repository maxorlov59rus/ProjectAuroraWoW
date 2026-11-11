/**
 * Aurora core PCH (Precompiled Header)
 *
 * Includes common Aurora headers so that the custom layer compiles fast and
 * consistently across the project. Adjust include paths if you change the
 * folder layout.
 */
#pragma once

// Core C++ standard headers
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <functional>

// AzerothCore headers are not included here on purpose: the Aurora layer
// should depend on AzerothCore only through its own modules or via explicit
// includes in the places where the API is used. This keeps the Aurora layer
// buildable both as a standalone test tool and as a linked library.
