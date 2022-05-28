
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

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.