#pragma once

#include "pch.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace Buffers
{
    typedef struct WorldViewProjection {
        Matrix world;
        Matrix view;
        Matrix projection;

        char padding[256 - (int)sizeof(Matrix) * 3];
    };

    typedef struct Light {
        Vector4 light;

        char padding[256 - (int)sizeof(Vector4) * 1];
    };

    typedef struct Material {
        Vector4 lightColor;
        Vector4 Ka;
        Vector4 Kd;
        Vector4 Ks;
        Vector4 shininess;

        char padding[256 - (int)sizeof(Vector4) * 5];
    };

    // Asserts to check alignment
    static_assert(sizeof(WorldViewProjection) % 256 == 0, "Struct size must equal 256 or a multiple of this.");
    static_assert(sizeof(Light) % 256 == 0, "Struct size must equal 256 or a multiple of this.");
    static_assert(sizeof(Material) % 256 == 0, "Struct size must equal 256 or a multiple of this.");
}