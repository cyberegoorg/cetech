#pragma once

#include "math_types.h"
#include "math.h"

namespace cetech1 {
    namespace vector2 {
        static const Vector2 ZERO = {0.0f, 0.0f};
        static const Vector2 UNIT = {1.0f, 1.0f};
        
        static const Vector2 UP = {0.0f, 1.0f};
        static const Vector2 DOWN = {0.0f, -1.0f};
        static const Vector2 RIGHT = {1.0f, 0.0f};
        static const Vector2 LEFT = {-1.0f, 0.0f};
    }
    
    namespace vector2 {
        inline float len(const Vector2& v1);
        inline float len_sq(const Vector2& v1);
        inline float len_inv(const Vector2& v1);

        inline Vector2 normalized(const Vector2& v1);
        
        inline float dot(const Vector2& v1, const Vector2& v2);
        inline float cross(const Vector2& v1, const Vector2& v2);
    }

    inline Vector2 operator+ (const Vector2& v1, const Vector2& v2);
    inline Vector2 operator- (const Vector2& v1, const Vector2& v2);
    inline Vector2 operator* (const Vector2& v1, const Vector2& v2);
    inline Vector2 operator/ (const Vector2& v1, const Vector2& v2);

    inline Vector2 operator* (const Vector2& v1, const float s);
    inline Vector2 operator/ (const Vector2& v1, const float s);


    namespace vector2 {
        inline float dot(const Vector2& v1, const Vector2& v2) {
            return (v1.x * v2.x) + (v1.y * v2.y);
        }
        
        inline float cross(const Vector2& v1, const Vector2& v2){
            return (v1.x * v2.y) - (v1.y * v2.x);
        }

        inline float len(const Vector2& v1) {
            return math::fast_sqrt(len_sq(v1));
        }
        
        inline float len_sq(const Vector2& v1) {
            return dot(v1, v1);
        }

        inline float len_inv(const Vector2& v1){
            return math::inv_sqrt(len_sq(v1));
        }

        inline Vector2 normalized(const Vector2& v1) {
            return v1 * len_inv(v1);
        }
    }

    inline Vector2 operator+ (const Vector2& v1, const Vector2& v2) {
        Vector2 res;

        res.x = v1.x + v2.x;
        res.y = v1.y + v2.y;

        return res;
    }

    inline Vector2 operator- (const Vector2& v1, const Vector2& v2) {
        Vector2 res;

        res.x = v1.x - v2.x;
        res.y = v1.y - v2.y;

        return res;
    }

    inline Vector2 operator*(const Vector2& v1, const Vector2& v2) {
        Vector2 res;

        res.x = v1.x * v2.x;
        res.y = v1.y * v2.y;

        return res;
    }

    inline Vector2 operator/(const Vector2& v1, const Vector2& v2) {
        Vector2 res;

        res.x = v1.x / v2.x;
        res.y = v1.y / v2.y;

        return res;
    }

    inline Vector2 operator* (const Vector2& v1, const float s) {
        Vector2 res;

        res.x = v1.x * s;
        res.y = v1.y * s;

        return res;
    }

    inline Vector2 operator/ (const Vector2& v1, const float s) {
        Vector2 res;
        const float inv_s = 1 / s;

        res.x = v1.x * inv_s;
        res.y = v1.y * inv_s;

        return res;
    }
}
