#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float exposure;
uniform bool bloom;
uniform float glTime;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D lyrics;


// glitch effect based on this code : https://www.shadertoy.com/view/lsfGD2
float sat( float t ) {
	return clamp( t, 0.0, 1.0 );
}

vec2 sat( vec2 t ) {
	return clamp( t, 0.0, 1.0 );
}

float remap  ( float t, float a, float b ) {
	return sat( (t - a) / (b - a) );
}

float linterp( float t ) {
	return sat( 1.0 - abs( 2.0*t - 1.0 ) );
}

vec3 spectrum_offset( float t ) {
    float t0 = 3.0 * t - 1.5;
	return clamp( vec3( -t0, 1.0-abs(t0), t0), 0.0, 1.0);
}

//note: [0;1]
float rand( vec2 n ) {
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

//note: [-1;1]
float srand( vec2 n ) {
	return rand(n) * 2.0 - 1.0;
}

float mytrunc( float x, float num_levels )
{
	return floor(x*num_levels) / num_levels;
}
vec2 mytrunc( vec2 x, float num_levels )
{
	return floor(x*num_levels) / num_levels;
}

void main()
{   
    if(!bloom)
    {
        vec3 hdrColor = texture(scene, TexCoords).rgb; 
        vec3 bloomS = texture(bloomBlur, TexCoords).rgb; 
        FragColor = vec4(hdrColor, 1.0);
        return;
    }
    //const float gamma = 2.2;
    const float gamma = 1.4;
    
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

	//THE FOLLOWING CODE IS NOT REALLY MINE https://www.shadertoy.com/view/lsfGD2
	const float glitchStart = 29.863;
	float GLITCH;
	if(glTime < glitchStart)
	{
		GLITCH = 0.0;
	}
	else{
		GLITCH = min((.003 * (glTime - glitchStart)), .2);
	}

    vec2 uv = TexCoords;

    float gnm = sat( GLITCH );
	float rnd0 = rand( mytrunc( vec2(glTime, glTime), 6.0 ) );
	float r0 = sat((1.0-gnm)*0.7 + rnd0);
	float rnd1 = rand( vec2(mytrunc( uv.x, 10.0*r0 ), glTime) );
	float r1 = 0.5 - 0.5 * gnm + rnd1;
	r1 = 1.0 - max(0.0, ((r1<1.0) ? r1 : 0.9999999));
	float rnd2 = rand( vec2(mytrunc(uv.y, 40.0*r1), glTime));
	float r2 = sat( rnd2 );

	float rnd3 = rand(vec2(mytrunc( uv.y, 10.0*r0 ), glTime));
	float r3 = (1.0-sat(rnd3+0.8)) - 0.1;

	float pxrnd = rand(uv + glTime);

	float ofs = 0.05 * r2 * GLITCH * ( rnd0 > 0.5 ? 1.0 : -1.0 );
	ofs += 0.5 * pxrnd * ofs;

    uv.y += 0.1 * r3 * GLITCH;

    const int NUM_SAMPLES = 10;
    const float RCP_NUM_SAMPLES_F = 1.0 / float(NUM_SAMPLES);
    
	vec4 sum = vec4(0.0);
	vec3 wsum = vec3(0.0);
	for( int i=0; i<NUM_SAMPLES; ++i )
	{
		float t = float(i) * RCP_NUM_SAMPLES_F;
		uv.x = sat( uv.x + ofs * t );
		vec4 samplecol = texture(scene, uv, -10.0 );
		vec3 s = spectrum_offset( t );
		samplecol.rgb = samplecol.rgb * s;
		sum += samplecol;
		wsum += s;
	}
	sum.rgb /= wsum;
	sum.a *= RCP_NUM_SAMPLES_F;

	FragColor.a = sum.a;
	FragColor.rgb = sum.rgb; // * outcol0.a;
	hdrColor = vec3(FragColor);
	hdrColor += bloomColor * .5; // additive blending
    //FragColor = vec4(hdrColor, 1.0);
    //return; // not sure if gamma makes it better
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
	// brighten the scene slowly as it goes on
	float brightenSpeed = .004;
	float end = 59.0f;
	if(glTime > end)
	{
		brightenSpeed = .010;
	}
	FragColor += vec4(1) * glTime * brightenSpeed;




	vec2 texOff = vec2(0);
	vec2 newTexOff = vec2(texOff.x, 2 - texOff.y);
	vec2 lCoords = vec2((TexCoords.x + newTexOff.x) * .25, (TexCoords.y + newTexOff.y) * (1.0 / 3.0));
	// do some sort of bounds checking to make sure
	vec4 tcol = texture(lyrics, lCoords);
	// FragColor += tcol;
}  