
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef SIMPLE_BLUR

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

uniform sampler2D uBlurInputTexture;
uniform int uKernelHalfSize;

layout(location=0) out vec4 oColor;

void main()
{    
	vec2 texelSize = 1.0 / vec2(textureSize(uBlurInputTexture, 0));

	float result = 0.0;
    for (int x = -uKernelHalfSize; x < uKernelHalfSize; ++x) 
    {
        for (int y = -uKernelHalfSize; y < uKernelHalfSize; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(uBlurInputTexture, vTexCoord + offset).r;
        }
    }

	int kernelSize = uKernelHalfSize * 2;
	result = result / float(kernelSize*kernelSize);
	oColor = vec4(result,result,result,1.0);
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