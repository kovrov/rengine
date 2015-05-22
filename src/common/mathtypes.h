#pragma once

#include <math.h>

namespace rengine {

struct vec2 {

    vec2(float x, float y) : x(x), y(y) { }
    vec2(float v = 0.0f) : x(v), y(v) { }

    vec2 operator+(const vec2 &o) const {
        vec2 r = o;
        r.x += x;
        r.y += y;
        return r;
    }
    vec2 &operator+=(const vec2 &o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    vec2 &operator=(float v) {
        x = v;
        y = v;
        return *this;
    }
    bool operator==(const vec2 &o) {
        return o.x == x && o.y == y;
    }

    float x;
    float y;
};

struct vec3 {

    vec3(float x, float y, float z) : x(x), y(y), z(z) { }
    vec3(float v = 0.0f) : x(v), y(v), z(v) { }

    vec3 operator+(const vec3 &o) const {
        vec3 r = o;
        r.x += x;
        r.y += y;
        r.z += z;
        return r;
    }
    vec3 &operator+=(const vec3 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    vec3 &operator=(float v) {
        x = v;
        y = v;
        z = v;
        return *this;
    }
    bool operator==(const vec3 &o) {
        return o.x == x && o.y == y && o.z == z;
    }

    float x;
    float y;
    float z;
};

struct vec4 {

    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    vec4(float v = 0.0f) : x(v), y(v), z(v), w(v) { }

    vec4 operator+(const vec4 &o) const {
        vec4 r = o;
        r.x += x;
        r.y += y;
        r.z += z;
        r.w += w;
        return r;
    }
    vec4 &operator+=(const vec4 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }
    vec4 &operator=(float v) {
        x = v;
        y = v;
        z = v;
        w = v;
        return *this;
    }
    bool operator==(const vec4 &o) {
        return o.x == x && o.y == y && o.z == z && o.w == w;
    }

    float x;
    float y;
    float z;
    float w;
};

struct mat4 {
    enum Type {
        Identity         = 0x00,
        Translation      = 0x01,
        Complex          = 0xff,
    };

    mat4()
        : m{ 1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 }
        , type(Identity)
    {
    }

    mat4(float m11, float m12, float m13, float m14,
         float m21, float m22, float m23, float m24,
         float m31, float m32, float m33, float m34,
         float m41, float m42, float m43, float m44,
         Type type = Complex)
        : m{ m11, m12, m13, m14,
             m21, m22, m23, m24,
             m31, m32, m33, m34,
             m41, m42, m43, m44 }
        , type(type)
    {
    }

    mat4 operator*(const mat4 &o) {
        return mat4(m[ 0] * o.m[0] + m[ 1] * o.m[4] + m[ 2] * o.m[ 8] + m[ 3] * o.m[12],
                    m[ 0] * o.m[1] + m[ 1] * o.m[5] + m[ 2] * o.m[ 9] + m[ 3] * o.m[13],
                    m[ 0] * o.m[2] + m[ 1] * o.m[6] + m[ 2] * o.m[10] + m[ 3] * o.m[14],
                    m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 2] * o.m[11] + m[ 3] * o.m[15],

                    m[ 4] * o.m[0] + m[ 5] * o.m[4] + m[ 6] * o.m[ 8] + m[ 7] * o.m[12],
                    m[ 4] * o.m[1] + m[ 5] * o.m[5] + m[ 6] * o.m[ 9] + m[ 7] * o.m[13],
                    m[ 4] * o.m[2] + m[ 5] * o.m[6] + m[ 6] * o.m[10] + m[ 7] * o.m[14],
                    m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 6] * o.m[11] + m[ 7] * o.m[15],

                    m[ 8] * o.m[0] + m[ 9] * o.m[4] + m[10] * o.m[ 8] + m[11] * o.m[12],
                    m[ 8] * o.m[1] + m[ 9] * o.m[5] + m[10] * o.m[ 9] + m[11] * o.m[13],
                    m[ 8] * o.m[2] + m[ 9] * o.m[6] + m[10] * o.m[10] + m[11] * o.m[14],
                    m[ 8] * o.m[3] + m[ 9] * o.m[7] + m[10] * o.m[11] + m[11] * o.m[15],

                    m[12] * o.m[0] + m[13] * o.m[4] + m[14] * o.m[ 8] + m[15] * o.m[12],
                    m[12] * o.m[1] + m[13] * o.m[5] + m[14] * o.m[ 9] + m[15] * o.m[13],
                    m[12] * o.m[2] + m[13] * o.m[6] + m[14] * o.m[10] + m[15] * o.m[14],
                    m[12] * o.m[3] + m[13] * o.m[7] + m[14] * o.m[11] + m[15] * o.m[15],
                    Type(type | o.type)) ;
    }

    static mat4 fromTranslate(float dx, float dy, float dz = 0) {
        return mat4(1, 0, 0, dx,
                    0, 1, 0, dy,
                    0, 0, 1, dz,
                    0, 0, 0, 1, Translation);
    }

    static mat4 fromScale(float sx, float sy, float sz = 1) {
        return mat4(sx,  0,  0, 0,
                     0, sy,  0, 0,
                     0,  0, sz, 0,
                     0,  0,  0, 1, Complex);
    }

    static mat4 fromRotate2D(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4(c, -s, 0, 0,
                    s,  c, 0, 0,
                    0,  0, 1, 0,
                    0,  0, 0, 1, Complex);
    }

    bool isIdentity() const { return type == Identity; }

    float operator()(int c, int r) {
        return m[c*4 + r];
    }

    float m[16];
    Type type;

};

}
