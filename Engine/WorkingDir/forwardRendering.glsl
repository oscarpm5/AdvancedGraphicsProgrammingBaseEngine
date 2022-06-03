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

layout( binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};


out vec2 vTexCoord;
out vec3 vPosition; //In WorldSpace
out vec3 vNormal; //In WorldSpace

void main()
{
	vPosition = vec3(uWorldMatrix*vec4(aPosition,1.0));
	vNormal = vec3(uWorldMatrix*vec4(normalize(aNormal),0.0));
	vTexCoord = aTexCoord;

	gl_Position = uWorldViewProjectionMatrix*vec4(aPosition,1.0);

}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;



uniform sampler2D uTexture;

layout( binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location=0) out vec4 oColor;

void main()
{
	vec3 albedo = texture(uTexture,vTexCoord).xyz;	
	vec3 normal = normalize(vNormal);	
	vec3 position = vPosition;	
	
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

		
		oColor =  vec4(albedo * lightColor,1.0);	
}


#endif
#endif