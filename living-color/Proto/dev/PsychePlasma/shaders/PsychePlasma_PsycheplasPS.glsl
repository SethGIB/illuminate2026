#version 330

uniform vec2 uResolution;
uniform float uTime;

uniform sampler2D uTxBase;
uniform sampler2D uTxMid;
uniform sampler2D uTxDetail;
uniform sampler2D uTxOvl;
uniform sampler2D uTxGradient;

out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy/uResolution;
    
    vec2 uvbase = vec2(uv.x+uTime*0.1, uv.y);
    float base = texture(uTxBase, uvbase).r;
    
    vec2 uvmid = vec2(uv.x-uTime*0.0667, uv.y);
    float mid = texture(uTxMid, uvmid).g;
    
    vec2 uvdet = vec2(sin(uv.x-uTime*0.01), uv.y+uTime*0.1);
    float detail = texture(uTxDetail, uvdet).b;
    
    vec2 uvovl = vec2(uv.x+cos(uTime*0.1), sin(uv.y-uTime*0.04));
    float ovl = texture(uTxOvl, uvovl).b;
    
    float comp = base+mid;
    comp = comp-detail;
    comp *= ovl;
    comp = clamp(comp, 0.0, 1.0);
    vec3 grad = texture(uTxGradient, vec2(cos(comp-uTime),comp+sin(uTime*0.5))).rgb;
    outColor = vec4(grad,1.0);
}