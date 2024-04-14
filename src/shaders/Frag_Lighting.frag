#version 430

in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform sampler2D texImage;

uniform vec3 cameraPos;

const int MAX_LIGHTS = 2;
uniform vec4 lightPos;

uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;

uniform float lightConstantAttenuation ;
uniform float lightLinearAttenuation    ;
uniform float lightQuadraticAttenuation ;


uniform vec3 Ka = vec3( 1.0 );
uniform vec3 Kd = vec3( 1.0 );
uniform vec3 Ks = vec3( 1.0 );

uniform float Shininess;

vec4 BlinnPhong(vec3 Normal, vec3 Position, vec4 LightPos,vec3 LightAmbiance, vec3 LightDiffuse, vec3 LightSpecular)
{

	vec3 ToLight;
	float LightDistance=0.0;
	
	if ( LightPos.w == 0.0 )
	{
		ToLight	= LightPos.xyz;
	}
	else
	{
		ToLight	= LightPos.xyz - Position;
		LightDistance = length(ToLight);
	}
	ToLight = normalize(ToLight);

	float Attenuation = 1.0 / ( lightConstantAttenuation + lightLinearAttenuation * LightDistance + lightQuadraticAttenuation * LightDistance * LightDistance);
	
	vec3 Ambient = LightAmbiance * Ka;

	float DiffuseFactor = max(dot(ToLight,Normal), 0.0) * Attenuation;
	vec3 Diffuse = DiffuseFactor * LightDiffuse * Kd;
	
	// Spekuláris komponens
	vec3 viewDir = normalize( cameraPos - Position ); // A fragmentből a kamerába mutató vektor
	vec3 reflectDir = reflect( -ToLight, Normal ); // Tökéletes visszaverődés vektora
	
	// A spekuláris komponens a tökéletes visszaverődés iránya és a kamera irányától függ.
	// A koncentráltsága cos()^s alakban számoljuk, ahol s a fényességet meghatározó paraméter.
	// Szintén függ az attenuációtól.
	float SpecularFactor = pow(max( dot( viewDir, reflectDir) ,0.0), Shininess) * Attenuation;
	vec3 Specular = SpecularFactor*LightSpecular*Ks;

	// normal vector debug:
	// fs_out_col = vec4( normal * 0.5 + 0.5, 1.0 );
	return vec4( Ambient+Diffuse+Specular, 1.0 );
}

void main()
{
	vec3 normal = normalize( vs_out_norm );
	vec4 light = BlinnPhong(normal, vs_out_pos, lightPos, La, Ld, Ls);

	// normal vector debug
	// fs_out_col = vec4( normal * 0.5 + 0.5, 1.0 );

	fs_out_col = light * texture(texImage, vs_out_tex);
}
