#include "Camera.h"
#include "FrameInfo.h"
#include "TestHelpers.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ostream>
#include <string>

namespace
{
    const float kFov = glm::radians( 45.f );
    const float kAspect = 16.f / 9.f;
    const float kNear = 0.1f;
    const float kFar = 100.f;

    // Applies the matrix and then the perspective divide, like the GPU does after the vertex shader
    glm::vec3 Project( const glm::mat4& matrix, const glm::vec3& point )
    {
        const glm::vec4 clip = matrix * glm::vec4( point, 1.f );
        return glm::vec3( clip ) / clip.w;
    }

    glm::vec3 TransformPoint( const glm::mat4& matrix, const glm::vec3& point )
    {
        return glm::vec3( matrix * glm::vec4( point, 1.f ) );
    }

    Camera MakePerspectiveCamera()
    {
        Camera camera{};
        camera.SetPerspectiveProjection( kFov, kAspect, kNear, kFar );
        return camera;
    }
}

// Perspective projection

TEST( CameraPerspective, MatchesGlmLeftHandedZeroToOne )
{
    // Vulkan uses a 0..1 depth range, and the engine looks down +Z (left-handed)
    const Camera camera = MakePerspectiveCamera();

    ExpectMatNear( camera.GetProjectionMatrix(), glm::perspectiveLH_ZO( kFov, kAspect, kNear, kFar ) );
}

TEST( CameraPerspective, NearAndFarPlanesMapToDepthZeroAndOne )
{
    const Camera camera = MakePerspectiveCamera();

    EXPECT_NEAR( Project( camera.GetProjectionMatrix(), { 0.f, 0.f, kNear } ).z, 0.f, 1e-5f );
    EXPECT_NEAR( Project( camera.GetProjectionMatrix(), { 0.f, 0.f, kFar } ).z, 1.f, 1e-5f );
}

TEST( CameraPerspective, EdgesOfFieldOfViewMapToEdgesOfScreen )
{
    const Camera camera = MakePerspectiveCamera();
    const float distance = 10.f;
    const float halfHeight = glm::tan( kFov / 2.f ) * distance;
    const float halfWidth = halfHeight * kAspect;

    EXPECT_NEAR( Project( camera.GetProjectionMatrix(), { 0.f, halfHeight, distance } ).y, 1.f, 1e-5f );
    EXPECT_NEAR( Project( camera.GetProjectionMatrix(), { halfWidth, 0.f, distance } ).x, 1.f, 1e-5f );
}

// Orthographic projection

TEST( CameraOrthographic, MapsBoxToClipVolume )
{
    const float left = -4.f, right = 4.f, bottom = -3.f, top = 3.f, nearPlane = 1.f, farPlane = 11.f;
    Camera camera{};
    camera.SetOrthographicProjection( left, right, bottom, top, nearPlane, farPlane );
    const glm::mat4& projection = camera.GetProjectionMatrix();

    // In Vulkan, y = -1 is the top of the screen
    ExpectVecNear( Project( projection, { left, top, nearPlane } ), { -1.f, -1.f, 0.f } );
    ExpectVecNear( Project( projection, { right, bottom, farPlane } ), { 1.f, 1.f, 1.f } );
    ExpectVecNear( Project( projection, { 0.f, 0.f, 6.f } ), { 0.f, 0.f, 0.5f } );
}

// View matrix

TEST( CameraView, CameraPositionMapsToOrigin )
{
    const glm::vec3 position{ 1.f, 2.f, 3.f };
    Camera camera{};
    camera.SetViewTarget( position, { 4.f, -2.f, 8.f } );

    ExpectVecNear( TransformPoint( camera.GetViewMatrix(), position ), glm::vec3{ 0.f } );
}

TEST( CameraView, TargetIsStraightAhead )
{
    const glm::vec3 position{ 1.f, 2.f, 3.f };
    const glm::vec3 target{ 4.f, -2.f, 8.f };
    Camera camera{};
    camera.SetViewTarget( position, target );

    const float distance = glm::length( target - position );
    ExpectVecNear( TransformPoint( camera.GetViewMatrix(), target ), { 0.f, 0.f, distance } );
}

TEST( CameraView, DoesNotStretchOrMirrorTheWorld )
{
    Camera camera{};
    camera.SetViewTarget( { 1.f, 2.f, 3.f }, { 4.f, -2.f, 8.f } );
    const glm::mat3 rotation{ camera.GetViewMatrix() };

    // A pure rotation: its transpose is its inverse and its determinant is 1
    ExpectMatNear( rotation * glm::transpose( rotation ), glm::mat3{ 1.f } );
    EXPECT_NEAR( glm::determinant( rotation ), 1.f, 1e-5f );
}

TEST( CameraView, ZeroRotationOnlyTranslates )
{
    const glm::vec3 position{ 3.f, -5.f, 2.f };
    Camera camera{};
    camera.SetViewYXZ( position, glm::vec3{ 0.f } );

    ExpectMatNear( camera.GetViewMatrix(), glm::translate( glm::mat4{ 1.f }, -position ) );
}

TEST( CameraView, YawLooksTheWayThePlayerWalks )
{
    // MovementController moves the player along ( sin(yaw), 0, cos(yaw) ) when W is held
    const float yaw = 0.7f;
    const glm::vec3 position{ 3.f, -5.f, 2.f };
    const glm::vec3 walkDirection{ glm::sin( yaw ), 0.f, glm::cos( yaw ) };

    Camera camera{};
    camera.SetViewYXZ( position, { 0.f, yaw, 0.f } );

    ExpectVecNear( TransformPoint( camera.GetViewMatrix(), position + walkDirection * 5.f ), { 0.f, 0.f, 5.f } );
}

TEST( CameraViewProjection, IsProjectionTimesView )
{
    Camera camera = MakePerspectiveCamera();
    camera.SetViewTarget( { 1.f, 2.f, 3.f }, { 4.f, -2.f, 8.f } );

    ExpectMatNear( camera.GetViewProjectionMatrix(), camera.GetProjectionMatrix() * camera.GetViewMatrix() );
}

// The view matrix should undo the camera's own transform, for any rotation

struct RotationCase
{
    std::string name;
    glm::vec3 rotation;
};

// Lets GoogleTest and CTest show the case name instead of raw bytes
void PrintTo( const RotationCase& rotationCase, std::ostream* os ) { *os << rotationCase.name; }

class CameraViewInverse : public ::testing::TestWithParam<RotationCase> {};

TEST_P( CameraViewInverse, ViewIsInverseOfCameraTransform )
{
    TransformComponent cameraTransform{};
    cameraTransform.translation = { 1.f, -2.f, 3.f };
    cameraTransform.rotation = GetParam().rotation;

    Camera camera{};
    camera.SetViewYXZ( cameraTransform.translation, cameraTransform.rotation );

    ExpectMatNear( camera.GetViewMatrix() * cameraTransform.mat4(), glm::mat4{ 1.f } );
}

INSTANTIATE_TEST_SUITE_P( Rotations, CameraViewInverse, ::testing::Values(
        RotationCase{ "None", { 0.f, 0.f, 0.f } },
        RotationCase{ "YawOnly", { 0.f, 1.2f, 0.f } },
        RotationCase{ "PitchOnly", { -0.6f, 0.f, 0.f } },
        RotationCase{ "RollOnly", { 0.f, 0.f, 0.9f } },
        RotationCase{ "AllAxes", { 0.3f, -2.1f, 0.5f } } ),
    []( const ::testing::TestParamInfo<RotationCase>& info ) { return info.param.name; } );
