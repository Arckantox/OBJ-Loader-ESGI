
varying vec2 v_TexCoords;
varying vec4 v_Normal;

uniform sampler2D u_Texture;

const vec3 directionalLight = normalize(-vec3(1.0, -1.0, -0.5));

// composantes ambiantes
const vec3 lightAmbiantColor = vec3(0.0, 0.5, 0.0);
const vec3 materialAmbiantColor = vec3(1.0, 0.5, 1.0);
// composantes diffuses
const vec3 lightDiffuseColor = vec3(1.0, 0.0, 0.2);
const vec3 materialDiffuseColor = vec3(0.8, 0.0, 1.0);


void main(void)
{
	vec3 diffuseColor = lightDiffuseColor * materialDiffuseColor;

    vec2 uv = vec2(v_TexCoords.x, 1.0 - v_TexCoords.y);
	// on a en fait passer des normales comme couleur
	//vec4 color = vec4(v_Normal.rgb * 0.5 + 0.5, 1.0);
	

	// ||u||*||v||*cos0 -> cos0 = U.V / ||u||*||v||
	// attention! U.V peut être négatif

	// la dricetion de v_Normal a pu être deformee par l'interpolation
	// il faut donc re-normaliser afin que la longueur du verteur soit correcte
	vec3 normal = normalize(v_Normal.xyz);
	float diffuseFactor = max(dot(normal, directionalLight), 0.0);

	gl_FragColor = diffuseFactor * texture2D(u_Texture, uv);
	//gl_FragColor = v_Color;
}
