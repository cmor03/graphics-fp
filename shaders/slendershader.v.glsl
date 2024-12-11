#version 410 core

// Uniform inputs
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 materialColor;
uniform float time;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 normalVec;

// Outputs to Fragment Shader
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 color;

// Minimal Wave Function
float wave(vec3 pos, float time) {
    return sin(pos.x * 2.0 + time) * 0.02 + cos(pos.y * 2.0 + time) * 0.02;
}

void main() {

    // Apply only minimal distortion to the vertex position
    vec3 adjustedPos = vPos;
    adjustedPos.z += wave(vPos, time/120.0);

    // Remove spin for better stability

    // Assign the final position to the output
    gl_Position = mvpMatrix * vec4(adjustedPos, 1.0);

    // Output attributes with minimal modification
    fragPos = adjustedPos;
    fragNormal = normalize(normalMatrix * normalVec);

    // Subtle color variation based on position
    color = materialColor;

    // Static texture coordinates
    texCoord = inTexCoord;
}
