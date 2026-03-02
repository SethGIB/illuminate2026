#version 330

uniform vec2 uResolution;
uniform float uTime;

uniform sampler2D baseTx;
uniform sampler2D layer0Tx;
uniform sampler2D layer1Tx;
uniform sampler2D layer2Tx;
uniform sampler2D gradTx;

out vec4 outColor;

/*
{
	layer2Tx = Overlay, .5
	layer1Tx = Color Burn .2
	layer0Tx = Normal 0.7
} Screen .9
baseTx = Normal .48
*/

float blendOverlay(float base, float blend, float opacity)
{
	return (base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend))) * opacity + base * (1.0f-opacity);
}

float blendColorBurn(float base, float blend, float opacity)
{
	return ((blend==0.0)?blend:max((1.0-((1.0-base)/blend)),0.0)) * opacity + base * (1.0f-opacity);
}

float blendScreen(float base, float blend, float opacity)
{
	return (1.0-((1.0-base)*(1.0-blend))) * opacity + base * (1.0f-opacity);
}
float kL2Alpha = 0.5f;
float kL1Alpha = 0.2f;
float kL0Alpha = 0.7f;
float kGrpAlpha = 0.9f;
float kBaseAlpha = 0.48f;

void main()
{
    vec2 uv = gl_FragCoord.xy/uResolution;
    float t_l2 = texture(layer2Tx, vec2(uv.x+sin((0.01-uTime)*0.115f),uv.y-cos(0.002-uTime*0.077))).r;
    float t_l1 = texture(layer1Tx, vec2(uv.x, uv.y+uTime*0.06)).r;
    float t_l0 = texture(layer0Tx, vec2(uv.x, uv.y+uTime*-0.04)).r;
    float t_b = texture(baseTx, vec2(uv.x-cos(uTime*0.15f),uv.y)).r;
    
	float t_brn = blendColorBurn(t_l0*kL0Alpha, t_l1, kL1Alpha);
	float t_ovl = blendOverlay(t_brn, t_l2, kL2Alpha);
	float t_scrn = blendScreen(t_b*kBaseAlpha, t_ovl, kGrpAlpha);
			
    outColor = texture(gradTx, vec2(t_scrn+sin(uTime*0.5), t_scrn-cos(uTime*0.26633)));
}