#version 330

uniform vec2 uResolution;
uniform float uTime;

uniform sampler2D txBase;
uniform sampler2D txMid;
uniform sampler2D txCrs;
uniform sampler2D txFin;
uniform sampler2D txGrad;

out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy/uResolution;
    
	float baseTx = texture(txBase, vec2(uv.x+cos(uTime*0.1),uv.y-sin(uTime*0.0445))).r;
	float midTx = texture(txMid, vec2(uv.x-sin(uTime*0.2),uv.y+cos(uTime*0.056))).r;
	float crsTx = texture(txCrs, vec2(uv.x-cos(uTime*0.07),uv.y+sin(uTime*0.11))).r;
	float finTx = texture(txFin, vec2(uv.x+sin(uTime*0.033),uv.y-cos(uTime*0.05))).r;
    
    float t1 = clamp((midTx-baseTx),0,1);
    float t2 = clamp((finTx-crsTx),0,1);
    float mask = clamp(t1+t2,0,1);
    
    float muvx = mask;//-cos(gl_FragCoord.y*0.03-uTime*0.024);
  
    float muvy = mask+(sin(gl_FragCoord.x*0.012-cos(uTime*1.8))-cos(gl_FragCoord.y*0.02+sin(uTime*1.2)))*uTime*0.00005;
    outColor = texture(txGrad, vec2(muvx,muvy));
}