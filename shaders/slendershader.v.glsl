#version 410 core

// Uniform inputs
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 materialColor;
uniform int time;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 normalVec;

// Outputs to Fragment Shader
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 color;

// Wacky Wave Function
float wave(vec3 pos, float time) {
    return 0.02*(sin(pos.x * 10.0 + time/20) * 0.2 + cos(pos.y * 10.0 + time/20) * 0.2);
}

void main() {

    // Add a wavy distortion to the vertex position
    vec3 wavyPos = vPos;
    wavyPos.z += wave(vPos, time);
    wavyPos.xy += vec2(sin(vPos.y * 10.0 + time), cos(vPos.x * 10.0 + time)) * 0.1;

    // Spin the object around its origin
    float angle = time * 0.5; // Adjust rotation speed
    mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    wavyPos.xy = rotation * wavyPos.xy;

    // Assign the final position to the output
    gl_Position = mvpMatrix * vec4(wavyPos, 1.0);

    // Output distorted attributes
    fragPos = wavyPos;
    fragNormal = normalize(normalMatrix * normalVec);

    // Color changes dynamically based on position and time
    color = materialColor * abs(sin(wavyPos.x * 5.0 + time)) * vec3(0.5, 1.0, 1.5);

    // Add some wavy distortion to texture coordinates for fun
    texCoord = inTexCoord + vec2(sin(time + inTexCoord.y * 10.0), cos(time + inTexCoord.x * 10.0)) * 0.05;
}
