///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

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

uniform sampler2D uTexture;
uniform int isDepth;

layout(location=0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture,vTexCoord);	
	if(isDepth==1)
	{
		float depth = texture(uTexture,vTexCoord).x;	
		oColor = vec4(depth,depth,depth,1.0);
	}
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

#ifdef SHOW_TEXTURED_MESH

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
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBitangent;


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

//////////////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;


layout( binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};


out vec3 vPosition; //In WorldSpace
out vec3 vNormal; //In WorldSpace
out vec2 vTexCoord;

void main()
{
	vPosition = vec3(uWorldMatrix*vec4(aPosition,1.0));
	vNormal = vec3(uWorldMatrix*vec4(normalize(aNormal),0.0));
	vTexCoord = aTexCoord;

	gl_Position = uWorldViewProjectionMatrix*vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;


uniform sampler2D uTexture;

layout(location=0) out vec4 oAlbedo;
layout(location=1) out vec4 oNormal;
layout(location=2) out vec4 oPosition;


void main()
{
	oAlbedo = texture(uTexture,vTexCoord);
	oNormal = vec4(normalize(vNormal),1.0);
	oPosition = vec4(vPosition,1.0);
	
}


#endif
#endif


//////////////////////////////////////////////////////////////////////////////

#ifdef LIGHTING_PASS

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
uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uPosition;
uniform sampler2D uSSAO;
uniform bool uSSAOActive;

layout( binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};


layout(location=0) out vec4 oRadiance;

void main()
{

	vec3 albedo = texture(uAlbedo,vTexCoord).xyz;	
	vec3 normal = texture(uNormal,vTexCoord).xyz;	
	vec3 position = texture(uPosition,vTexCoord).xyz;	
	
	vec3 viewDir = normalize(uCameraPosition -position);

	
	vec3 lightColor = vec3(0.0,0.0,0.0);
	for(int i = 0; i<uLightCount; ++i)
	{
		
			Light l = uLight[i];

			vec3 reflectDir = reflect(l.direction,normal);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);


			if(l.type==0) //if directional light
			{
				float luminance = max(dot(normalize(normal),normalize(-l.direction)),0.0f);
				lightColor += l.color * (luminance + spec);
			}

			if(l.type==1)
			{
				float dist = length(position-l.position);
				float attenuation = clamp(1.0/ (dist*dist),0.0,1.0);

				float luminance = max(dot(normalize(normal),normalize(l.position - position)),0.0);

				lightColor += l.color * attenuation *(luminance  + spec);
			}

		
	}

	if(uSSAOActive==true)
	{
		oRadiance =  vec4(albedo * lightColor*texture(uSSAO,vTexCoord).rgb,1.0);
	}
	else
	{
		oRadiance =  vec4(albedo * lightColor,1.0);
	}




	//oRadiance = albedo*normal*position;	
}


#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef LIGHT_MESH_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;

layout( binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
	vec3 color;
};

void main()
{
	gl_Position = uWorldViewProjectionMatrix*vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
layout( binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
	vec3 color;
};

layout(location=0) out vec4 oColor;

void main()
{
	oColor = vec4(color,1.0);
}


#endif
#endif
