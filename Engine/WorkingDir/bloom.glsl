
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef BRIGHTEST_PIXELS

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;


out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;

uniform sampler2D uColorTexture;
uniform float uThreshold;

layout(location=0) out vec4 oColor;

void main()
{    
	vec3 luminances = vec3(0.2126,0.7152,0.0722);
	vec4 texel =texture2D(uColorTexture, vTexCoord);
	float luminance = dot(luminances,texel.rgb);
	luminance = max(0.0,luminance - uThreshold);
	texel.rgb *= sign(luminance);
	texel.a = 1.0;
	oColor = texel;
}


#endif
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef BLUR

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;


out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;

uniform sampler2D uColorTexture;
uniform vec2 uDirection;
uniform int uLOD;

layout(location=0) out vec4 oColor;

void main()
{    
	vec2 texSize = textureSize(uColorTexture,uLOD);
	vec2 texelSize = 1.0/texSize;
	vec2 margin1 = texelSize *0.5;
	vec2 margin2 = vec2(1.0) - margin1;
	oColor = vec4(0.0);

	vec2 directionFragCoord = gl_FragCoord.xy * uDirection;
	int coord = int(directionFragCoord.x + directionFragCoord.y);
	vec2 directionTexSize = texSize * uDirection;
	int size = int(directionTexSize.x + directionTexSize.y);
	int kernelRadius = 24;
	int kernelBegin = -min(kernelRadius,coord);
	int kernelEnd = min(kernelRadius,size - coord);
	float weight = 0.0;

	for(int i = kernelBegin; i <= kernelEnd; ++i)
	{
		float currentWeight = smoothstep(float(kernelRadius),0.0,float(abs(i)));
		vec2 finalTexCoords = vTexCoord + i * uDirection * texelSize;
		finalTexCoords = clamp(finalTexCoords,margin1,margin2);
		oColor += textureLod(uColorTexture,finalTexCoords,uLOD)*currentWeight;
		weight += currentWeight;
	}

	oColor = oColor/weight;

}


#endif
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef BLOOM

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;


out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;

uniform sampler2D uColorTexture;
uniform int uMaxLOD;

layout(location=0) out vec4 oColor;

void main()
{    
	oColor = vec4(0.0);
	for(int lod = 0; lod <uMaxLOD; ++lod)
	{
		oColor+= textureLod(uColorTexture,vTexCoord,lod);
	}
	oColor.a = 1.0;
}


#endif
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.