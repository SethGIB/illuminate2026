#version 150

uniform vec2 uResolution;
uniform float ciElapsedSeconds;

uniform sampler2D uTxBase;
uniform sampler2D uTxMid;
uniform sampler2D uTxCoarse;
uniform sampler2D uTxFine;
uniform sampler2D uTxGrad;

in vec2 vTexCoord0;
out vec4 outColor;

void main()
{
    //vec2 uv = gl_FragCoord.xy/uResolution;
    vec2 uv = vTexCoord0 * vec2(0.5625,1.0);
	float baseTx = texture(uTxBase, vec2(uv.x+cos(ciElapsedSeconds*0.1),uv.y-sin(ciElapsedSeconds*0.0445))).r;
	float midTx = texture(uTxMid, vec2(uv.x-sin(ciElapsedSeconds*0.2),uv.y+cos(ciElapsedSeconds*0.056))).r;
	float crsTx = texture(uTxCoarse, vec2(uv.x-cos(ciElapsedSeconds*0.07),uv.y+sin(ciElapsedSeconds*0.11))).r;
	float finTx = texture(uTxFine, vec2(uv.x+sin(ciElapsedSeconds*0.033),uv.y-cos(ciElapsedSeconds*0.05))).r;
    
    float t1 = clamp((midTx-baseTx),0,1);
    float t2 = clamp((finTx-crsTx),0,1);
    float mask = clamp(t1+t2,0,1);
    
    float muvx = mask;//-cos(gl_FragCoord.y*0.03-uTime*0.024);
  
    float muvy = mask+(sin(gl_FragCoord.x*0.012-cos(ciElapsedSeconds*1.8))-cos(gl_FragCoord.y*0.02+sin(ciElapsedSeconds*1.2)))*ciElapsedSeconds*0.00005;
    outColor = texture(uTxGrad, vec2(muvx,muvy));
}