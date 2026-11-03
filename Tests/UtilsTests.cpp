#include "Utils.h"

#include <gtest/gtest.h>
#include <string>

// Model.cpp uses hashCombine to find duplicate vertices while loading a mesh

namespace
{
    template <typename... Values>
    std::size_t Hash( const Values&... values )
    {
        std::size_t seed = 0;
        hashCombine( seed, values... );
        return seed;
    }
}

TEST( HashCombine, SameValuesGiveSameHash )
{
    EXPECT_EQ( Hash( 1, 2.5f, std::string{ "cube" } ), Hash( 1, 2.5f, std::string{ "cube" } ) );
}

TEST( HashCombine, DifferentValuesGiveDifferentHashes )
{
    EXPECT_NE( Hash( 1.f, 2.f, 3.f ), Hash( 1.f, 2.f, 4.f ) );
}

TEST( HashCombine, OrderOfValuesMatters )
{
    // Otherwise a vertex at (1, 2) would collide with one at (2, 1)
    EXPECT_NE( Hash( 1.f, 2.f ), Hash( 2.f, 1.f ) );
}
