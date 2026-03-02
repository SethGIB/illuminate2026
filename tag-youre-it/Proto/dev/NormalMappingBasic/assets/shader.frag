#version 150

uniform sampler2D	uCamMap, uGradMap, uDiffuseMap, uNormalMap, uARMMap;
uniform vec3		uLightLocViewSpace;
uniform float ciElapsedSeconds;

// inputs passed from the vertex shader
in vec4		VertexViewSpace;
in vec3		NormalViewSpace, TangentViewSpace, BitangentViewSpace;
in vec2		TexCoord0;

// output a single color
out vec4			oColor;

#include "./blend_modes.glsl"

void main()
{
	//Wall Setup 
	vec2 wallUvs = vec2(TexCoord0.x * 1.667, TexCoord0.y);
	vec2 camUvs = vec2(TexCoord0.x, TexCoord0.y);

	// normal setup
	vec3 vMappedNormal = texture(uNormalMap, wallUvs).rgb * 2.0 - 1.0;
	vec3 normal = normalize((TangentViewSpace * vMappedNormal.x) + (BitangentViewSpace * vMappedNormal.y) + (-NormalViewSpace * vMappedNormal.z));
	vec3 vToCamera = normalize( -VertexViewSpace.xyz );
	vec3 light = normalize( uLightLocViewSpace - VertexViewSpace.xyz );
	vec3 reflect = normalize(-reflect(light, normal));

	//diffuse and support contribs
	vec3 diffSample = texture( uDiffuseMap, wallUvs ).rgb;
	vec3 armTerms = texture(uARMMap, wallUvs).rgb;

	//Per-pixel lighting setup
	// calculate diffuse term
	float nDotL = clamp(max( dot(-normal, light), 0.0 ), 0.1, 1.0);

	// calculate specular term
	float specPow = 10.25 *  (1.0-armTerms.g);
	float specStr = 0.88 *  (1.0-armTerms.r);
	float specTerm = clamp( pow( max( dot(reflect, vToCamera), 0.0), specPow ), 0.0, 1.0);

	//setup final diffuse term
	vec4 camSample = texture(uCamMap, camUvs);

	//lookup fx first
	vec3 t_lum = vec3(0.299, 0.587, 0.114);
	float l_cam = pow(dot(camSample.rgb, t_lum), 2.5);
	vec3 lutColor = texture(uGradMap, vec2(l_cam+sin(ciElapsedSeconds*0.075), 0.0)).rgb;

	vec3 additive = blendVividLight( diffSample, lutColor );
	vec3 compedDiff = mix( diffSample, additive, camSample.a );
	vec3 vDiffuseColor = vec3( nDotL ) * compedDiff * armTerms.r;
	vec3 vSpecularColor = vec3( specTerm ) * specStr;
	
	oColor.rgb = (vDiffuseColor + vSpecularColor).rgb;
	oColor.a = 1.0;
}