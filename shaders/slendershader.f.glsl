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

// Random function for fun effects
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    // Get texture color including alpha
    vec4 texColor = texture(textureMap, texCoord);

    // Discard fragments with low alpha
    if(texColor.a < 0.1) {
        discard;
    }

    vec3 ambientReflection = vec3(0.1);  // Some small ambient light

    // Material color
    vec3 materialColor = color;
    if (color.x == -1) {
        materialColor = texColor.rgb;  // Use RGB from texture
    }

    // Add a bit of randomness to the color for fun
    materialColor *= vec3(rand(texCoord * 10.0), rand(texCoord * 20.0), rand(texCoord * 30.0));

    // Directional Light
    vec3 directionalNormalizedLightDirection = normalize(-lightDirection);
    float diff = max(dot(fragNormal, directionalNormalizedLightDirection), 0.0);
    vec3 directionalDiffuse = directionalLightColor * diff * materialColor * 2.0; // Exaggerate for fun

    // Specular for Directional Light
    vec3 viewDir = normalize(viewVector - fragPos);
    vec3 reflectDir = reflect(-directionalNormalizedLightDirection, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0); // Make it shinier
    vec3 directionalSpecular = directionalLightColor * spec * materialColor;

    // Point Light
    vec3 pointLightDir = normalize(pointLightPosition - fragPos);
    float pointDiff = max(dot(fragNormal, pointLightDir), 0.0);
    vec3 pointDiffuse = pointLightColor * pointDiff * materialColor;

    // Specular for Point Light
    vec3 pointReflectDir = reflect(-pointLightDir, fragNormal);
    float pointSpec = pow(max(dot(viewDir, pointReflectDir), 0.0), 4.0);
    vec3 pointSpecular = pointLightColor * pointSpec * materialColor;

    // Attenuation for Point Light
    float distance = length(pointLightPosition - fragPos);
    float attenuation = 1.0 / (1.0 + 0.05 * distance * distance); // Over-the-top attenuation

    // Sparkle effect
    float sparkle = step(0.95, rand(texCoord * 50.0)) * 0.8; // Random sparkles

    // Combine
    vec3 result = ambientReflection * materialColor + 0.2 * (directionalDiffuse + directionalSpecular) + attenuation * 8.0 * (pointDiffuse + pointSpecular);
    result += vec3(sparkle); // Add sparkles

    fragColorOut = vec4(result, texColor.a);
}