#pragma once
// Include engine headers before this one, so GLM sees the engine's GLM_FORCE_* settings first
#include <gtest/gtest.h>
#include <glm/glm.hpp>

// Compares every component of two matrices, and reports which one is off
template <glm::length_t C, glm::length_t R>
void ExpectMatNear( const glm::mat<C, R, float>& actual, const glm::mat<C, R, float>& expected, float tolerance = 1e-5f )
{
    for ( glm::length_t column = 0; column < C; column++ )
    {
        for ( glm::length_t row = 0; row < R; row++ )
        {
            EXPECT_NEAR( actual[ column ][ row ], expected[ column ][ row ], tolerance )
                << "at column " << column << ", row " << row;
        }
    }
}

inline void ExpectVecNear( const glm::vec3& actual, const glm::vec3& expected, float tolerance = 1e-5f )
{
    EXPECT_NEAR( actual.x, expected.x, tolerance ) << "x";
    EXPECT_NEAR( actual.y, expected.y, tolerance ) << "y";
    EXPECT_NEAR( actual.z, expected.z, tolerance ) << "z";
}
