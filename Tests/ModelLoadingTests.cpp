#include "Model.h"
#include "TestHelpers.h"

#include <stdexcept>
#include <string>
#include <vector>

// ModelData only parses files on the CPU, so it can be tested without a GPU.
// The tests run from Project/, so the game's own models are under "Models/".

namespace
{
    const std::string kQuadFile = std::string{ TEST_DATA_DIR } + "quad.obj";
}

TEST( ModelLoading, QuadSharesVerticesBetweenTriangles )
{
    Model::ModelData data;
    data.LoadModel( kQuadFile );

    // 6 corners in the file, but the two triangles share 2 of them
    EXPECT_EQ( data.vertices.size(), 4u );
    EXPECT_EQ( data.indices, ( std::vector<uint32_t>{ 0, 1, 2, 0, 2, 3 } ) );
}

TEST( ModelLoading, FlipsYAxisForVulkan )
{
    Model::ModelData data;
    data.LoadModel( kQuadFile );

    // The file has this corner at y = 2. Vulkan's y axis points down, so the loader negates it
    ExpectVecNear( data.vertices[ 2 ].position, { 1.f, -2.f, 0.f } );
}

TEST( ModelLoading, ReadsVertexAttributes )
{
    Model::ModelData data;
    data.LoadModel( kQuadFile );
    const Model::Vertex& vertex = data.vertices[ 1 ];

    ExpectVecNear( vertex.color, { 0.f, 1.f, 0.f } );
    ExpectVecNear( vertex.normal, { 0.f, 0.f, 1.f } );
    EXPECT_FLOAT_EQ( vertex.uv.x, 1.f );
    EXPECT_FLOAT_EQ( vertex.uv.y, 0.f );
}

TEST( ModelLoading, CollisionTrianglesFollowIndexBuffer )
{
    Model::ModelData data;
    data.LoadModel( kQuadFile );

    ASSERT_EQ( data.triangles.size(), data.indices.size() );
    for ( size_t i = 0; i < data.indices.size(); i++ )
    {
        EXPECT_EQ( data.triangles[ i ], data.vertices[ data.indices[ i ] ].position ) << "at index " << i;
    }
}

TEST( ModelLoading, CubeKeepsCornersWithDifferentNormalsSeparate )
{
    Model::ModelData data;
    data.LoadModel( "Models/Cube.obj" );

    // 8 corners, each shared by 3 faces that need their own normal: 8 * 3 = 24 vertices.
    // 6 faces * 2 triangles * 3 indices = 36 indices.
    EXPECT_EQ( data.vertices.size(), 24u );
    EXPECT_EQ( data.indices.size(), 36u );
}

TEST( ModelLoading, JsonSetsTransformAndLoadsMesh )
{
    Model::ModelData data;
    data.LoadJSON( "Models/Cube.json" );

    ExpectVecNear( data.m_Transform.translation, { 0.f, -10.f, 20.f } );
    ExpectVecNear( data.m_Transform.scale, glm::vec3{ 2.f } );
    EXPECT_EQ( data.vertices.size(), 24u );
}

TEST( ModelLoading, MissingFileThrows )
{
    Model::ModelData data;

    EXPECT_THROW( data.LoadModel( std::string{ TEST_DATA_DIR } + "does_not_exist.obj" ), std::runtime_error );
}
