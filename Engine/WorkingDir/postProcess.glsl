///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SSAO

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

uniform mat4 uViewMatrix;//TODO consider passing this matrix as a uniform buffer
uniform mat4 uProjMatrix;//TODO consider passing this matrix as a uniform buffer

uniform sampler2D uNormalTexture;
uniform sampler2D uPosTexture;
uniform sampler2D uDepthTexture;

const int MAX_KERNEL_SIZE = 64;
uniform vec3 uKernel[MAX_KERNEL_SIZE];

layout(location=0) out vec4 oColor;



void main()
{
	float radius = 0.5;//TODO get this from c++ as a uniform
	float bias = 0.025;//TODO get this from c++ as a uniform

	float occlusion = 0.0;
	vec3 normalView =  vec3(normalize(uViewMatrix * vec4(texture(uNormalTexture,vTexCoord).xyz,0.0)));
	vec3 fragPosView = vec3(uViewMatrix * vec4(texture(uPosTexture,vTexCoord).xyz,1.0));

	
	//Create tangent to world basis
	vec3 tangent = cross(normalView,vec3(0,1,0));
	vec3 bitangent = cross(normalView,tangent);
	mat3 TBN = mat3(tangent,bitangent,normalView);

	for(int i =0; i<MAX_KERNEL_SIZE; ++i)
	{
		//get neighbour sample pos 
		vec3 offsetView = TBN * uKernel[i];
		vec3 samplePosView = fragPosView + offsetView*radius;

		//Transform sample to screen space to sample the neighbours texture depth
		vec4 offset = vec4(samplePosView,1.0);
		offset = uProjMatrix* offset; //from view to clip space
		offset.xyz /= offset.w; //perspective divide
		offset.xyz = offset.xyz*0.5+0.5; //transform to 0,1 range


		//sample depth
		float sampleDepth = vec3(uViewMatrix * vec4(texture(uPosTexture,offset.xy).xyz,1.0)).z;

		occlusion += (sampleDepth >= samplePosView.z + bias ? 1.0 : 0.0);  
	}

	oColor = vec4(1.0-occlusion/float(MAX_KERNEL_SIZE));
	
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