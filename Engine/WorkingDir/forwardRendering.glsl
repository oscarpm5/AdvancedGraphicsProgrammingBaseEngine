#ifdef FORWARD_PASS

struct Light
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;



layout( binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout( binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};


out vec2 vTexCoord;
out vec3 vPosition; //In WorldSpace
out vec3 vNormal; //In WorldSpace
out vec3 vViewDir; //In WorldSpace

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix*vec4(aPosition,1.0));
	vNormal = vec3(uWorldMatrix*vec4(aNormal,0.0));
	vViewDir = uCameraPosition - vPosition;

	gl_Position = uWorldViewProjectionMatrix*vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;



uniform sampler2D uTexture;

layout( binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location=0) out vec4 oColor;
layout(location=1) out vec4 oNormal;

void main()
{
	oColor = texture(uTexture,vTexCoord);
	oNormal = vec4(vNormal,1.0);
	//TODO this doesn't work
	
	vec3 lightColor = vec3(0.0);
	for(int i = 0; i<uLightCount; ++i)
	{
		Light l = uLight[i];

		if(l.type==0) //if directional light
		{
			float luminance = dot(normalize(vNormal),normalize(l.direction));
			lightColor += l.color * luminance;
		}
	}

	oColor =  vec4(oColor.xyz * lightColor,1.0);
	
}


#endif
#endif