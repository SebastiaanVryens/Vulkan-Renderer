#pragma once
#include <cassert>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <cassert>

//Tait-Bryan angles, YZX
class Camera
{
public:
    Camera() = default;

    void SetOrthographicProjection(
        float left, float right,
        float bottom, float top,
        float near, float far ) 
    {
        m_ProjectionMatrix = glm::mat4{ 1.0f };
        m_ProjectionMatrix[ 0 ][ 0 ] = 2.f / ( right - left );
        m_ProjectionMatrix[ 1 ][ 1 ] = 2.f / ( bottom - top );
        m_ProjectionMatrix[ 2 ][ 2 ] = 1.f / ( far - near );
        m_ProjectionMatrix[ 3 ][ 0 ] = -( right + left ) / ( right - left );
        m_ProjectionMatrix[ 3 ][ 1 ] = -( bottom + top ) / ( bottom - top );
        m_ProjectionMatrix[ 3 ][ 2 ] = -near / ( far - near );
    }

    void SetPerspectiveProjection(
        float fov, float aspectRatio,
        float near, float far ) 
    {
        assert( glm::abs( aspectRatio ) > std::numeric_limits<float>::epsilon() );

        const float tanHalfFovy = tan( fov / 2.f );
        m_ProjectionMatrix = glm::mat4{ 0.0f };
        m_ProjectionMatrix[ 0 ][ 0 ] = 1.f / ( aspectRatio * tanHalfFovy );
        m_ProjectionMatrix[ 1 ][ 1 ] = 1.f / ( tanHalfFovy );
        m_ProjectionMatrix[ 2 ][ 2 ] = far / ( far - near );
        m_ProjectionMatrix[ 2 ][ 3 ] = 1.f;
        m_ProjectionMatrix[ 3 ][ 2 ] = -( far * near ) / ( far - near );
    }

    void SetViewDirection(
		const glm::vec3 pos,
		const glm::vec3 dir,
		const glm::vec3 up = glm::vec3{0.f, 1.f,0.0f})
	{
        const glm::vec3 w{ glm::normalize( dir ) };
        const glm::vec3 u{ glm::normalize( glm::cross( w, up ) ) };
        const glm::vec3 v{ glm::cross( w, u ) };

        m_ViewMatrix = glm::mat4{ 1.f };
        m_ViewMatrix[ 0 ][ 0 ] = u.x;
        m_ViewMatrix[ 1 ][ 0 ] = u.y;
        m_ViewMatrix[ 2 ][ 0 ] = u.z;
        m_ViewMatrix[ 0 ][ 1 ] = v.x;
        m_ViewMatrix[ 1 ][ 1 ] = v.y;
        m_ViewMatrix[ 2 ][ 1 ] = v.z;
        m_ViewMatrix[ 0 ][ 2 ] = w.x;
        m_ViewMatrix[ 1 ][ 2 ] = w.y;
        m_ViewMatrix[ 2 ][ 2 ] = w.z;
        m_ViewMatrix[ 3 ][ 0 ] = -glm::dot( u, pos );
        m_ViewMatrix[ 3 ][ 1 ] = -glm::dot( v, pos );
        m_ViewMatrix[ 3 ][ 2 ] = -glm::dot( w, pos );
	}

	void SetViewYXZ( 
        const glm::vec3 pos, 
        const glm::vec3 rot)
	{
        const float c3 = glm::cos( rot.z );
        const float s3 = glm::sin( rot.z );
        const float c2 = glm::cos( rot.x );
        const float s2 = glm::sin( rot.x );
        const float c1 = glm::cos( rot.y );
        const float s1 = glm::sin( rot.y );
        const glm::vec3 u{ ( c1 * c3 + s1 * s2 * s3 ), ( c2 * s3 ), ( c1 * s2 * s3 - c3 * s1 ) };
        const glm::vec3 v{ ( c3 * s1 * s2 - c1 * s3 ), ( c2 * c3 ), ( c1 * c3 * s2 + s1 * s3 ) };
        const glm::vec3 w{ ( c2 * s1 ), ( -s2 ), ( c1 * c2 ) };
        m_ViewMatrix = glm::mat4{ 1.f };
        m_ViewMatrix[ 0 ][ 0 ] = u.x;
        m_ViewMatrix[ 1 ][ 0 ] = u.y;
        m_ViewMatrix[ 2 ][ 0 ] = u.z;
        m_ViewMatrix[ 0 ][ 1 ] = v.x;
        m_ViewMatrix[ 1 ][ 1 ] = v.y;
        m_ViewMatrix[ 2 ][ 1 ] = v.z;
        m_ViewMatrix[ 0 ][ 2 ] = w.x;
        m_ViewMatrix[ 1 ][ 2 ] = w.y;
        m_ViewMatrix[ 2 ][ 2 ] = w.z;
        m_ViewMatrix[ 3 ][ 0 ] = -glm::dot( u, pos );
        m_ViewMatrix[ 3 ][ 1 ] = -glm::dot( v, pos );
        m_ViewMatrix[ 3 ][ 2 ] = -glm::dot( w, pos );
	}

    void SetViewTarget(
		const glm::vec3 pos,
		const glm::vec3 target,
		const glm::vec3 up = glm::vec3{ 0.f, 1.f, 0.0f })
	{
		SetViewDirection( pos, target - pos, up );
	}

	glm::mat4 GetViewProjectionMatrix() const
	{
		return m_ProjectionMatrix * m_ViewMatrix;
	}

    const glm::mat4& GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}
    const glm::mat4& GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

private:

    glm::mat4 m_ProjectionMatrix{ 1.f };
    glm::mat4 m_ViewMatrix{ 1.f };
};
