#version 410 core

uniform sampler2D textureMap;
uniform vec3 lightDirection;
uniform vec3 directionalLightColor;
uniform vec3 pointLightPosition;
uniform vec3 pointLightColor;
uniform vec3 viewVector;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 color;

// Fragment Output
out vec4 fragColorOut;

void main() {
    vec3 ambientReflection = vec3(0.1);  // Some small ambient light

    // Material color
    vec3 materialColor = color;
    if (color.x == -1) {
        materialColor = vec3(texture(textureMap, texCoord));
    }

    // Directional Light
    vec3 directionalNormalizedLightDirection = normalize(-lightDirection);
    float diff = max(dot(fragNormal, directionalNormalizedLightDirection), 0.0);
    vec3 directionalDiffuse = directionalLightColor * diff * materialColor;

    // Specular for Directional Light
    vec3 viewDir = normalize(viewVector - fragPos);
    vec3 reflectDir = reflect(-directionalNormalizedLightDirection, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);
    vec3 directionalSpecular = directionalLightColor * spec * materialColor;

    // Point Light
    vec3 pointLightDir = normalize(pointLightPosition - fragPos);
    float pointDiff = max(dot(fragNormal, pointLightDir), 0.0);
    vec3 pointDiffuse = pointLightColor * pointDiff * materialColor;

    // Specular for Point Light
    vec3 pointReflectDir = reflect(-pointLightDir, fragNormal);
    float pointSpec = pow(max(dot(viewDir, pointReflectDir), 0.0), 2.0);
    vec3 pointSpecular = pointLightColor * pointSpec * materialColor;

    // Attenuation for Point Light
    float distance = length(pointLightPosition - fragPos);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);




    // Combine
    vec3 result = ambientReflection * materialColor + (directionalDiffuse + directionalSpecular) + attenuation * (pointDiffuse + pointSpecular);
    fragColorOut = vec4(result, 1.0);
}
