//Plasma Psychedelic FINAL
#version 150

uniform vec2 uResolution;
uniform float ciElapsedSeconds;

uniform sampler2D uTxNoise;
uniform sampler2D uTxGrad;

in vec2 vTexCoord0;
out vec4 outColor;

void main()
{
    vec2 uv = vTexCoord0 * vec2(0.5625,1.0) * 0.5;
    
    vec2 uvbase = vec2(uv.x+ciElapsedSeconds*0.1, uv.y);
    float base = texture(uTxNoise, uvbase).r;
    
    vec2 uvmid = vec2(uv.x-ciElapsedSeconds*0.0667, uv.y);
    float mid = texture(uTxNoise, uvmid).g;
    
    vec2 uvdet = vec2(sin(uv.x-ciElapsedSeconds*0.01), uv.y+ciElapsedSeconds*0.1);
    float detail = texture(uTxNoise, uvdet).b;
    
    vec2 uvovl = vec2(uv.x+cos(ciElapsedSeconds*0.1), sin(uv.y-ciElapsedSeconds*0.04));
    float ovl = texture(uTxNoise, uvovl).a;
    
    float comp = base+mid;
    comp = comp-detail;
    comp *= ovl;
    comp = clamp(comp, 0.0, 1.0);
    vec3 grad = texture(uTxGrad, vec2(cos(comp-ciElapsedSeconds),comp+sin(ciElapsedSeconds*0.5))).rgb;
    outColor = vec4(grad,1.0);
}