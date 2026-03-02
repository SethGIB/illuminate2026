#version 150

uniform vec2 uResolution;
uniform float ciElapsedSeconds;

uniform sampler2D uTxNoise;
uniform sampler2D uTxGrad;

in vec2 vTexCoord0;
out vec4 outColor;

float blendDarken(float base, float blend, float opacity) {
	return (min(blend,base) * opacity + base * (1.0 - opacity));
}

float blendDifference(float base, float blend, float opacity) {
	return (abs(base-blend) * opacity + base * (1.0 - opacity));
}

void main()
{
    vec2 uv = vTexCoord0 * vec2(0.5625,1.0) * 0.25;
    
    vec2 uvBase = vec2(uv.x+sin(ciElapsedSeconds*0.03), uv.y-cos(ciElapsedSeconds*0.05));
    float base = texture(uTxNoise, uvBase).r; //layer 0
    
    vec2 uvGrid = vec2(uv.x+cos(ciElapsedSeconds*0.06), uv.y-sin(ciElapsedSeconds*0.03));
    float grid = texture(uTxNoise, uvGrid).g; //layer 1
    
    vec2 uvDet0 = vec2(uv.x-cos(ciElapsedSeconds*0.04), uv.y+sin(ciElapsedSeconds*0.02));
    float det0 = texture(uTxNoise, uvDet0).b; //layer 2
    
    vec2 uvDet1 = vec2(uv.x-sin(ciElapsedSeconds*0.02667), uv.y+cos(ciElapsedSeconds*0.0334));
    float det1 = texture(uTxNoise, uvDet1).a; //layer 3
    
    float layer0 = blendDifference(base, grid, 1.0f);                  
    float layer2 = blendDarken(layer0, det0, 1.0f);
    float mask = blendDifference(layer2, det1, 1.0f);
    
    outColor = texture(uTxGrad, vec2(sin(mask+ciElapsedSeconds*0.2),cos(mask-ciElapsedSeconds*0.32)));
}