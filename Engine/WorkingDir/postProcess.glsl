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
uniform sampler2D uDepthTexture;
uniform sampler2D uRandomVecTexture;

const int MAX_KERNEL_SIZE = 64;
uniform vec3 uKernel[MAX_KERNEL_SIZE];
uniform vec2 uNoiseScale;
uniform vec2 uViewportSize;

layout(location=0) out vec4 oColor;


//Returns fragment pos in view space
//Params
//d: depth (from depth buffer)
//l: left
//r: right
//b: bottom
//t: top
//n: znear
//f: zfar
//v: viewport size
vec3 ReconstructPixelPos(float d, float l, float r, float b, float t, float n, float f, vec2 v)
{
		float xndc = gl_FragCoord.x/v.x * 2.0 -1.0;
		float yndc = gl_FragCoord.y/v.y * 2.0 -1.0;
		float zndc = d*2.0-1.0;
		float zeye = 2*f*n/(zndc*(f-n)-(f+n));//pixel coords to NDC
		float xeye = -zeye*(xndc*(r-l)+(r+l))/(2.0*n);
		float yeye = -zeye*(yndc*(t-b)+(t+b))/(2.0*n);
		vec3 eyecoords = vec3(xeye,yeye,zeye);
		return eyecoords;
}

//Returns fragment pos in view space
//Params
//d: depth (from depth buffer)
//projectionMatrixInv: inverse of the projection matrix
//v: viewport size
vec3 ReconstructPixelPos(float d,mat4 projectionMatrixInv, vec2 v)
{
		float xndc = gl_FragCoord.x/v.x * 2.0 -1.0;
		float yndc = gl_FragCoord.y/v.y * 2.0 -1.0;
		float zndc = d*2.0-1.0;
		vec4 posNDC = vec4(xndc,yndc,zndc,1.0);
		vec4 posView = projectionMatrixInv * posNDC;

		return posView.xyz / posView.w;
}

//Returns fragment pos in view space
//Params
//
//projectionMatrixInv: inverse of the projection matrix
//v: viewport size
vec3 ReconstructPixelPos(vec2 uv,mat4 projectionMatrixInv, vec2 v)
{
		float d = texture(uDepthTexture,uv).x;
		float xndc = uv.x/v.x * 2.0 -1.0;
		float yndc = uv.y/v.y * 2.0 -1.0;
		float zndc = d*2.0-1.0;
		vec4 posNDC = vec4(xndc,yndc,zndc,1.0);
		vec4 posView = projectionMatrixInv * posNDC;

		return posView.xyz / posView.w;
}



void main()
{
	float radius = 1.0;//TODO get this from c++ as a uniform
	float bias = 0.0;//TODO get this from c++ as a uniform

	float occlusion = 0.0;

	vec3 randomVec = texture(uRandomVecTexture, vTexCoord*uNoiseScale).xyz; 
	vec3 normalView =  vec3(uViewMatrix * vec4(texture(uNormalTexture,vTexCoord).xyz,0.0));
	
	//Create tangent to world basis
	vec3 tangent = normalize(randomVec-normalView*dot(randomVec,normalView));
	vec3 bitangent = cross(normalView,tangent);
	mat3 TBN = mat3(tangent,bitangent,normalView);

	mat4 invProjMat = inverse(uProjMatrix);
	vec3 fragPosView = ReconstructPixelPos(texture(uDepthTexture,vTexCoord).x,invProjMat,uViewportSize);



	for(int i =0; i<MAX_KERNEL_SIZE; ++i)
	{
		//get neighbour sample pos 
		vec3 offsetView = TBN * uKernel[i];
		vec3 samplePosView = fragPosView + offsetView*radius;

		//Transform sample to screen space to sample the neighbours texture depth
		vec4 offset = uProjMatrix * vec4(samplePosView,1.0);//from view to clip space 
		offset.xyz /= offset.w; //perspective divide
		offset.xyz = offset.xyz*0.5+0.5; //transform to 0,1 range


		//sample new pos
		vec3 samplePosNew = ReconstructPixelPos(offset.xy,invProjMat,uViewportSize);

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosView.z - samplePosNew.z));
		occlusion += (samplePosNew.z >= samplePosView.z + bias ? 1.0 : 0.0)*rangeCheck;  
	}

	oColor = vec4(vec3(1.0-occlusion/float(MAX_KERNEL_SIZE)),1.0);

	//oColor = vec4(randomVec,1.0);	
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