#version 330

uniform vec2 uResolution;
uniform float uTime;

uniform sampler2D tech00Tx; //voronoi-like
uniform sampler2D tech01Tx; //skinny
uniform sampler2D tech02Tx; //voronoi
uniform sampler2D techGradTx;

out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy/uResolution;
    
    float tech00 = texture(tech00Tx, vec2(uv.x,uv.y+sin(gl_FragCoord.x*0.001-uTime*0.04))).r;
    float tech01 = texture(tech02Tx, vec2(uv.x-cos(gl_FragCoord.y*0.0033+uTime*0.07),uv.y)).r;
    float mask = tech00*tech01;

    float uvx = mask+cos(uTime*0.025);
    float uvy = mask-sin(uTime*0.01);
    float m2 = texture(tech01Tx, vec2(uvx,uvy)).r;
    
    outColor = texture(techGradTx, vec2(m2+uTime*0.09,m2-uTime*0.12));
    //outColor = vec4(vec3(mask), vec2(m2));
}