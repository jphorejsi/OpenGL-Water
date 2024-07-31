#version 330 core

in vec4 clipSpace;
out vec4 FragColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

void main()
{
	vec2 ndc = (clipSpace.xy/clipSpace.w)/2.0f + 0.5f;
	vec2 refractTexCoords = vec2(ndc.x, ndc.y);
	vec2 reflectTexCoords = vec2(ndc.x, 1.0-ndc.y);

	vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
	vec4 refractColor = texture(refractionTexture, refractTexCoords);

	//FragColor = mix(reflectColor, refractColor, 0.5);
	FragColor = reflectColor;
}