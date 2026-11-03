#include "FrameInfo.h"
#include "TestHelpers.h"

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    TransformComponent MakeTransform()
    {
        TransformComponent transform{};
        transform.translation = { 1.f, -2.f, 3.f };
        transform.scale = { 2.f, 0.5f, 3.f };
        transform.rotation = { 0.3f, -1.1f, 0.7f };
        return transform;
    }
}

TEST( TransformComponent, DefaultIsIdentity )
{
    TransformComponent transform{};

    ExpectMatNear( transform.mat4(), glm::mat4{ 1.f } );
}

TEST( TransformComponent, MatchesTranslateRotateScaleInYXZOrder )
{
    TransformComponent transform = MakeTransform();

    // mat4() hand-expands this product: scale first, then rotate around Z, X, Y, then translate
    const glm::mat4 identity{ 1.f };
    const glm::mat4 expected =
        glm::translate( identity, transform.translation ) *
        glm::rotate( identity, transform.rotation.y, { 0.f, 1.f, 0.f } ) *
        glm::rotate( identity, transform.rotation.x, { 1.f, 0.f, 0.f } ) *
        glm::rotate( identity, transform.rotation.z, { 0.f, 0.f, 1.f } ) *
        glm::scale( identity, transform.scale );

    ExpectMatNear( transform.mat4(), expected );
}

TEST( TransformComponent, NormalMatrixIsInverseTransposeOfModelMatrix )
{
    TransformComponent transform = MakeTransform();

    const glm::mat3 expected = glm::transpose( glm::inverse( glm::mat3{ transform.mat4() } ) );

    ExpectMatNear( transform.normalMatrix(), expected );
}

TEST( TransformComponent, NormalsStayPerpendicularUnderNonUniformScale )
{
    TransformComponent transform{};
    transform.scale = { 4.f, 1.f, 1.f };
    transform.rotation = { 0.2f, 0.5f, -0.4f };

    // A 45 degree slope: the tangent runs along the surface, the normal points away from it
    const glm::vec3 tangent = glm::normalize( glm::vec3{ 1.f, 1.f, 0.f } );
    const glm::vec3 normal = glm::normalize( glm::vec3{ -1.f, 1.f, 0.f } );

    const glm::mat3 model{ transform.mat4() };
    const glm::vec3 transformedTangent = model * tangent;

    EXPECT_NEAR( glm::dot( transformedTangent, transform.normalMatrix() * normal ), 0.f, 1e-5f );

    // Transforming the normal with the model matrix instead would tilt it, which breaks the lighting
    EXPECT_GT( glm::abs( glm::dot( transformedTangent, glm::normalize( model * normal ) ) ), 0.1f );
}
