#version 130
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif


uniform sampler2D GrassTex;
uniform sampler2D DirtTex;
uniform sampler2D CliffTex;

uniform float uv_grass;
uniform float uv_cliff;
uniform float uv_dirt;
uniform float disturb_factor_near;
uniform float disturb_factor_far;
uniform float near_dist;
uniform float far_dist;
uniform float large_factor;
uniform vec4 TU_color;
uniform float TU_roughness;
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;
in vec3 v_worldPos;
in vec3 v_color;
in vec3 v_bc;
in vec3 v_mat;
//! [0]
float edgeFactor(){
	vec3 d = fwidth(v_bc);
	vec3 a3 = smoothstep(vec3(0.0), d*1.5, v_bc);
	return min(min(a3.x, a3.y), a3.z);
}


vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}


vec3 permute3(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v)
{
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute3( permute3( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}


vec3 powVec(vec3 data, float factor)
{
	return vec3(pow(data.x, factor), pow(data.y, factor), pow(data.z, factor));
}

vec4 multiUVMixed(sampler2D samp, vec2 UV)
{
	vec4 col = texture(samp, UV) * 0.4;
	col += texture(samp, UV * -0.5) * 0.3;
	col += texture(samp, UV * -0.25) * 0.3;
	return col;
}

vec4 triplanarSample(sampler2D sampler, float scaleFactor)
{
	float plateauSize = 0.25;
	float transitionSpeed = 1.0;
	vec3 blending = abs( normalize(v_normal));
	blending = clamp(blending - plateauSize, 0.0, 1.0);
	blending = powVec(blending, transitionSpeed);
	blending /= dot(blending, vec3(1.0, 1.0, 1.0));
	vec4 xaxis = texture( sampler, v_worldPos.yz * scaleFactor);
	vec4 yaxis = texture( sampler, v_worldPos.xz * scaleFactor);
	vec4 zaxis = texture( sampler, v_worldPos.xy * scaleFactor);
	return xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
}

vec4 outLineFilter(vec4 color)
{
	if(edgeFactor() < 0.3)
	{
		return vec4(1.0,0.0,0.0,1.0);
	}
	else
	{
		return color;
	}
}
float sum( vec3 v ) { return v.x+v.y+v.z; }

vec4 hash4( vec2 p ) { return fract(sin(vec4( 1.0+dot(p,vec2(37.0,17.0)), 
                                              2.0+dot(p,vec2(11.0,47.0)),
                                              3.0+dot(p,vec2(41.0,29.0)),
                                              4.0+dot(p,vec2(23.0,31.0))))*103.0); }
vec4 textureNoTile( sampler2D samp, in vec2 uv )
{
    ivec2 iuv = ivec2( floor( uv ) );
     vec2 fuv = fract( uv );

    // generate per-tile transform
    vec4 ofa = hash4( iuv + ivec2(0,0) );
    vec4 ofb = hash4( iuv + ivec2(1,0) );
    vec4 ofc = hash4( iuv + ivec2(0,1) );
    vec4 ofd = hash4( iuv + ivec2(1,1) );
    
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );

    // transform per-tile uvs
    ofa.zw = sign( ofa.zw-0.5 );
    ofb.zw = sign( ofb.zw-0.5 );
    ofc.zw = sign( ofc.zw-0.5 );
    ofd.zw = sign( ofd.zw-0.5 );
    
    // uv's, and derivatives (for correct mipmapping)
    vec2 uva = uv*ofa.zw + ofa.xy, ddxa = ddx*ofa.zw, ddya = ddy*ofa.zw;
    vec2 uvb = uv*ofb.zw + ofb.xy, ddxb = ddx*ofb.zw, ddyb = ddy*ofb.zw;
    vec2 uvc = uv*ofc.zw + ofc.xy, ddxc = ddx*ofc.zw, ddyc = ddy*ofc.zw;
    vec2 uvd = uv*ofd.zw + ofd.xy, ddxd = ddx*ofd.zw, ddyd = ddy*ofd.zw;
        
    // fetch and blend
    vec2 b = smoothstep( 0.25,0.75, fuv );
    
    return mix( mix( textureGrad( samp, uva, ddxa, ddya ), 
                     textureGrad( samp, uvb, ddxb, ddyb ), b.x ), 
                mix( textureGrad( samp, uvc, ddxc, ddyc ),
                     textureGrad( samp, uvd, ddxd, ddyd ), b.x), b.y );
}


vec4 textureNoTile2( sampler2D samp, in vec2 uv )
{
    vec2 p = floor( uv );
    vec2 f = fract( uv );
	
    // derivatives (for correct mipmapping)
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );
    
    // voronoi contribution
    vec4 va = vec4( 0.0 );
    float wt = 0.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2( float(i), float(j) );
        vec4 o = hash4( p + g );
        vec2 r = g - f + o.xy;
        float d = dot(r,r);
        float w = exp(-5.0*d );
        vec4 c = textureGrad( samp, uv + o.zw, ddx, ddy );
        va += w*c;
        wt += w;
    }
	
    // normalization
    return va/wt;
}

vec4 textureNoTile3( sampler2D samp, in vec2 x )
{
    // sample variation pattern    
    float k = texture( samp, 0.005*x ).x; // cheap (cache friendly) lookup    
    
    // compute index    
    float index = k*8.0;
    float i = floor( index );
    float f = fract( index );

    // offsets for the different virtual patterns    
    vec2 offa = sin(vec2(3.0,7.0)*(i+0.0)); // can replace with any other hash    
    vec2 offb = sin(vec2(3.0,7.0)*(i+1.0)); // can replace with any other hash    

    // compute derivatives for mip-mapping    
    vec2 dx = dFdx(x), dy = dFdy(x);
    
    // sample the two closest virtual patterns    
    vec3 cola = texture( samp, x + offa).rgb;
    vec3 colb = texture( samp, x + offb).rgb;

    // interpolate between the two virtual patterns    
    return vec4(mix( cola, colb, smoothstep(0.2,0.8,f-0.1*sum(cola-colb))), 1.0);
}

float getDistFactor(float start, float end, float dist)
{
	if (dist > end )
	{
		return 1.0;
	}
	if(dist < start)
	{
		return 0.0;
	}
	return (dist - start)/ (end - start);
}

vec3 noiseDisturb(vec3 color, float scaleFactor, float minVal, float maxVal)
{
	float noise = snoise(v_worldPos.xz * scaleFactor);
	noise = noise * 0.5 + 0.5;
	return color * mix(vec3(minVal, minVal, minVal), vec3(maxVal, maxVal, maxVal), noise);
}

vec4 getTerrainTex(sampler2D samp)
{
	vec3 detailTex = v_mat.x * triplanarSample(samp, 1.0 / uv_grass).xyz + v_mat.y * triplanarSample(CliffTex, 1.0 / uv_cliff).xyz + v_mat.z * triplanarSample(DirtTex, 1.0/ uv_dirt).xyz;
	detailTex = noiseDisturb(detailTex, disturb_factor_near, 0.95, 1.0); 
	vec3 largeTex = v_mat.x * triplanarSample(samp, 1.0 / (uv_grass * large_factor)).xyz + v_mat.y * triplanarSample(CliffTex, 1.0 / (uv_cliff * large_factor)).xyz + v_mat.z * triplanarSample(DirtTex, 1.0/ (uv_dirt * large_factor)).xyz;
	largeTex = noiseDisturb(largeTex, disturb_factor_far, 0.9, 1.0); 
	return vec4(mix(detailTex, largeTex, getDistFactor(near_dist, far_dist, -1 * v_position.z)), 1.0); 
}

vec4 texturePlain(sampler2D samp, in vec2 uv)
{
	return texture(samp, uv);
}

void main()
{
  gl_FragData[0] = vec4(pow(getTerrainTex(GrassTex).rgb, vec3(2.2)), 1.0);
  gl_FragData[1] = vec4(v_position,1.0);
  gl_FragData[2] = vec4(normalize(v_normal),1.0);
  gl_FragData[3] = vec4(TU_roughness,0.0,0.0,1.0);
}
//! [0]