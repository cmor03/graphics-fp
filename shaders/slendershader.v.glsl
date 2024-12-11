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
vec3 bezier(float time) {
    float T = fract(time / 15);
    vec3 P0 = vec3(-200, -200, 100);
    vec3 P1 = vec3(25, -379, 0);
    vec3 P2 = vec3(116, 330, 500);
    vec3 P3 = vec3(200, -200, -200);
    vec3 bezierPoint = (pow((1 - T),3)) * P0
        + (3*T*pow((1 - T),2)) * P1
        + (3*pow(T,2)*(1 - T)) * P2
        + (pow(T,3)) * P3;
    return bezierPoint;
}

void main() {

    // Add a bezier distortion to the vertex position
    vec3 bezierPos = vPos;
    vec3 bezierDistortion = normalize(bezier(time)) * 0.1;
    bezierPos += bezierDistortion;
    // Assign the final position to the output
    gl_Position = mvpMatrix * vec4(bezierPos, 1.0);

    // Output distorted attributes
    fragPos = bezierPos;
    fragNormal = normalize(normalMatrix * normalVec);

    // Color changes dynamically based on position and time
    color = materialColor;

    // Add some bezier distortion to texture coordinates
    texCoord = inTexCoord + vec2(bezierDistortion);
}
