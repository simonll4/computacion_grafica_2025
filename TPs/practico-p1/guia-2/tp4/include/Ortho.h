#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline glm::mat4 orthoAspect(float width, float height, float globalScale = 0.8f)
{
    const float aspect = (height > 0.f) ? (width / height) : 1.0f;
    float left, right, bottom, top;

    if (aspect >= 1.0f)
    {
        left = -aspect * globalScale;
        right = aspect * globalScale;
        bottom = -1.0f * globalScale;
        top = 1.0f * globalScale;
    }
    else
    {
        left = -1.0f * globalScale;
        right = 1.0f * globalScale;
        bottom = -1.0f / aspect * globalScale;
        top = 1.0f / aspect * globalScale;
    }

    return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
}
