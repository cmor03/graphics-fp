#version 410 core

// Uniform inputs
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 materialColor;  

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 normalVec;

// Outputs to Fragment Shader
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 fragPos;  
layout(location = 2) out vec3 fragNormal; 
layout(location = 3) out vec3 color;

void main() {
    gl_Position = mvpMatrix * vec4(vPos, 1.0);
    fragPos = vPos;
    fragNormal = normalize(normalMatrix * normalVec);
    color = materialColor;
    texCoord = inTexCoord;
}
