uniform sampler2D u_Texture;
varying vec2 v_TexCoords;
varying vec4 v_Normal;
varying vec3 v_Position;


const vec3 directionalLight = normalize(-vec3(1.0, -1.0, -0.5));
uniform vec4 u_PositionOrDirection;
// composantes ambiantes
const vec3 lightAmbiantColor = vec3(0.5, 0.5, 0.5);
const vec3 materialAmbiantColor = vec3(0.2, 0.2, 0.2);
// composantes diffuses
const vec3 lightDiffuseColor = vec3(1.0, 1.0, 1.0);
const vec3 materialDiffuseColor = vec3(1.0, 1.0, 1.0);


void main(void)
{
	

    vec2 uv = vec2(v_TexCoords.x, 1.0 - v_TexCoords.y);
	// on a en fait passer des normales comme couleur
	//vec4 color = vec4(v_Normal.rgb * 0.5 + 0.5, 1.0);
	

	// ||u||*||v||*cos0 -> cos0 = U.V / ||u||*||v||
	// attention! U.V peut être négatif

	// la dricetion de v_Normal a pu être deformee par l'interpolation
	// il faut donc re-normaliser afin que la longueur du verteur soit correcte
	vec3 N = normalize(v_Normal.xyz);
	
	vec3 P = v_Position;
	vec3 L = vec3(-u_PositionOrDirection);
	if (u_PositionOrDirection.w > 0.0) {
		// Positionnel (point light ou spot light)
		L = normalize(u_PositionOrDirection.xyz - P);
	}
	
	float diffuseFactor = max(dot(N, directionalLight), 0.0);
	vec3 ambiantColor = lightAmbiantColor * materialAmbiantColor;
	vec3 diffuseColor = lightDiffuseColor * materialDiffuseColor;
	
	vec4 diffuse = vec4(diffuseFactor * diffuseColor, 1.0);
	vec4 specular = vec4(0.0);
	
	//gl_FragColor = diffuseFactor * texture2D(u_Texture, uv);
	//gl_FragColor = v_Color;
	
	gl_FragColor = vec4(ambiantColor, 1.0) * texture2D(u_Texture, uv)
					 + (diffuse * texture2D(u_Texture, uv) + specular);
					 
	gl_FragColor.a = 0.5;
}
