#version 150

uniform sampler2D uWallDiffuseMap; //0
uniform sampler2D uWallNormalMap; //1
uniform sampler2D uWallARMMap; //2
uniform sampler2D uGradMap; //3
uniform sampler2D uCamRGBMap; //4
uniform usampler2D uCamDepthMap; //5

uniform vec2 uTexScale;
uniform vec3 uLightLocViewSpace;
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
	vec2 wallUvs = vec2(TexCoord0.x * uTexScale.x, TexCoord0.y * uTexScale.y);
	vec2 camUvs = vec2(TexCoord0.x, TexCoord0.y);

	// normal setup
	vec3 vMappedNormal = texture(uWallNormalMap, wallUvs).rgb * 2.0 - 1.0;
	vec3 normal = normalize((TangentViewSpace * vMappedNormal.x) + (BitangentViewSpace * vMappedNormal.y) + (-NormalViewSpace * vMappedNormal.z));
	vec3 vToCamera = normalize( -VertexViewSpace.xyz );
	vec3 light = normalize( uLightLocViewSpace - VertexViewSpace.xyz );
	vec3 reflect = normalize(-reflect(light, normal));

	//diffuse and support contribs
	vec3 wallDiffuse = texture( uWallDiffuseMap, wallUvs ).rgb;
	vec3 wallARM = texture(uWallARMMap, wallUvs).rgb;

	//Per-pixel lighting setup
	// calculate diffuse term
	float nDotL = clamp(max( dot(vec3(normal.x, -normal.y, -normal.z), light), 0.0 ), 0.1, 1.0);

	// calculate specular term
	float specPow = 3.25 *  (1.0-wallARM.g);
	float specStr = 0.667 *  (1.0-wallARM.r);
	float specTerm = clamp( pow( max( dot(reflect, vToCamera), 0.0), specPow ), 0.0, 1.0);

	//setup final diffuse term
	vec4 camRGBSample = texture(uCamRGBMap, camUvs);
	uvec4 camDepthSample = texture(uCamDepthMap, camUvs);
	float interpDepth = 0.0f;

	uint minDepth = 10u;
	if (camDepthSample.r > minDepth)
	{
		interpDepth = 1.0f;
	}
	//lookup fx first
	vec3 lumaWeights = vec3(0.299f, 0.587f, 0.114f);
	float camRGBtoGray = pow(dot(camRGBSample.rgb, lumaWeights), 2.5);
	vec3 lutColor = texture(uGradMap, vec2(camRGBtoGray+sin(ciElapsedSeconds*0.075), 0.0)).rgb;

	//vec3 additive = blendVividLight( wallDiffuse, lutColor );
	//vec3 compedDiff = mix( wallDiffuse, additive, interpDepth );
	//vec3 vDiffuseColor = vec3( nDotL ) * compedDiff * wallARM.r;
	vec3 vDiffuseColor = vec3( nDotL ) * wallDiffuse * wallARM.r;
	vec3 vSpecularColor = vec3( specTerm ) * specStr;
	
	oColor.rgb = (vDiffuseColor + vSpecularColor).rgb;
	oColor.a = 1.0;
}