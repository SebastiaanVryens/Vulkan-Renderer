#include "Input.h"
#include "TestHelpers.h"

#include <algorithm>
#include <vector>

// Tests for the gravity, jumping and ground collision in MovementController.
// In this engine +Y points down (models are flipped when loaded), so a smaller Y means higher up.

namespace
{
    constexpr float kDeltaTime = 1.f / 60.f;
    constexpr float kGravity = 9.8f;         // MovementController's default
    constexpr float kPlayerHeight = 0.4f;    // MovementController's default
    constexpr float kFloorY = -1.f;
    constexpr float kRestY = kFloorY - kPlayerHeight;   // Where the player stands on the floor

    // A flat 20 x 20 floor made of two triangles
    std::vector<glm::vec3> MakeFloor( float y )
    {
        return {
            { -10.f, y, -10.f }, { 10.f, y, -10.f }, { 10.f, y, 10.f },
            { -10.f, y, -10.f }, { 10.f, y, 10.f }, { -10.f, y, 10.f },
        };
    }

    class PhysicsTest : public ::testing::Test
    {
    protected:
        PhysicsTest()
            : m_Controller( MakeFloor( kFloorY ) )
        {
            // Without input, UpdatePhysics never reads the window, so the tests can pass nullptr
            m_Controller.CanMoveWithInput( false );
            m_Player.m_Transform.translation = { 0.f, kRestY - 1.f, 0.f };   // 1 unit in the air
        }

        void Step()
        {
            m_Controller.UpdatePhysics( nullptr, kDeltaTime, m_Player );
        }

        void Simulate( float seconds )
        {
            const int steps = static_cast<int>( seconds / kDeltaTime );
            for ( int i = 0; i < steps; i++ )
            {
                Step();
            }
        }

        // Simulates and returns how far above the floor the player got
        float SimulateAndTrackHighestPoint( float seconds )
        {
            float highestY = Height();
            const int steps = static_cast<int>( seconds / kDeltaTime );
            for ( int i = 0; i < steps; i++ )
            {
                Step();
                highestY = std::min( highestY, Height() );
            }
            return kRestY - highestY;
        }

        float Height() const { return m_Player.m_Transform.translation.y; }

        MovementController m_Controller;
        GameObject m_Player = GameObject::Create();
    };
}

TEST_F( PhysicsTest, FallsWithGravity )
{
    const float startY = Height();
    const float time = 0.25f;

    Simulate( time );

    // Distance fallen is 1/2 g t^2, give or take the error of stepping 60 times a second
    EXPECT_NEAR( Height() - startY, 0.5f * kGravity * time * time, 0.05f );
}

TEST_F( PhysicsTest, LandsOnFloorAtPlayerHeight )
{
    Simulate( 2.f );

    EXPECT_NEAR( Height(), kRestY, 1e-4f );
}

TEST_F( PhysicsTest, StaysOnFloorOnceLanded )
{
    Simulate( 2.f );
    Simulate( 3.f );

    EXPECT_NEAR( Height(), kRestY, 1e-4f );
}

TEST_F( PhysicsTest, FlyingIgnoresGravity )
{
    const float startY = Height();
    m_Controller.Fly( true );

    Simulate( 1.f );

    EXPECT_FLOAT_EQ( Height(), startY );
}

TEST_F( PhysicsTest, JumpRisesThenLandsBackOnFloor )
{
    const float jumpSpeed = 3.f;
    Simulate( 2.f );

    m_Controller.Jump( m_Player, jumpSpeed );
    const float jumpHeight = SimulateAndTrackHighestPoint( 2.f );

    // A jump at speed v peaks at v^2 / 2g. Stepping per frame loses about one frame of rise
    EXPECT_NEAR( jumpHeight, jumpSpeed * jumpSpeed / ( 2.f * kGravity ), jumpSpeed * kDeltaTime );
    EXPECT_NEAR( Height(), kRestY, 1e-4f );
}

TEST_F( PhysicsTest, BouncesOffFloorThenSettles )
{
    // Each bounce launches at bounceStrength * jumpStrength (3), and the strength drops by 1 per bounce
    m_Controller.SetBounceStrength( 2.f );

    // Fall until the first contact with the floor
    for ( int i = 0; i < 600 && Height() < kRestY - 1e-3f; i++ )
    {
        Step();
    }
    ASSERT_GE( Height(), kRestY - 1e-3f ) << "never reached the floor";

    const float bounceHeight = SimulateAndTrackHighestPoint( 5.f );

    EXPECT_GT( bounceHeight, 1.f );   // The first bounce launches at 6: about 1.8 units high
    EXPECT_NEAR( Height(), kRestY, 1e-4f );
}
