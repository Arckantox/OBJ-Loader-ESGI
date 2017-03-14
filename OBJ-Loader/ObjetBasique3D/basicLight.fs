#ifndef GL_ES
#version 110
#endif

uniform sampler2D u_Texture;
varying vec2 v_TexCoords;

varying vec3 v_Normal;		// normal dans le monde calculee dans le VS
varying vec3 v_Position;	// position dans le monde calculee dans le VS

//const vec3 directionalLight = normalize(-vec3(1.0, 1.0, -1.0));
// si W = 0 (X,Y,Z) est interprete comme un vecteur directeur
// si W = 1 (X,Y,Z) est interprete comme une position
uniform vec4 u_PositionOrDirection;

//
// TODO: passer les valeurs de couleurs en tant qu'uniformes
//

// composantes ambiantes
const vec3 lightAmbientColor = vec3(0.5, 0.5, 0.5);
const vec3 materialAmbientColor = vec3(0.5, 0.5, 0.0);
// composantes diffuses
const vec3 lightDiffuseColor = vec3(0.7, 0.6, 0.0);
const vec3 materialDiffuseColor = vec3(0.8, 0.0, 0.0);

void main(void)
{
	vec2 uv = vec2(v_TexCoords.x, 1.0-v_TexCoords.y);
	// on a en fait passer des normales comme couleur
	// ces valeurs sont dans le domaine [-1, +1]
	// pour les utiliser comme couleur on converti vers [0, 1]
	//vec4 color = vec4(v_Normal.xyz * 0.5 + 0.5, 1.0);
	
	// ||u||*||v||*cos0 -> cos0 = U.V / ||u||*||v||
	// attention! U.V peut etre negatif 

	// la direction de v_Normal a pu etre deformee par l'interpolation
	// il faut re-normaliser afin que la longueur du vecteur soit correcte.
	
	// rappel: v_Normal et v_Position sont exprimes dans le repere du monde. 
	vec3 N = normalize(v_Normal);
	vec3 P = v_Position;
	vec3 L = vec3(-u_PositionOrDirection);
	if (u_PositionOrDirection.w > 0.0) {
		// Positionnel (point light ou spot light)
		L = normalize(u_PositionOrDirection.xyz - P);
	}
	// facteur d'intensite qui suis la loi du cosinus de Lambert
	float diffuseFactor = max(dot(N, L), 0.0);

	vec3 ambientColor = lightAmbientColor * materialAmbientColor;
	vec3 diffuseColor = lightDiffuseColor * materialDiffuseColor;

	vec4 diffuse = vec4(diffuseFactor * diffuseColor, 1.0);
	vec4 specular = vec4(0.0);

	gl_FragColor = vec4(ambientColor, 1.0) * texture2D(u_Texture, uv)
					 + (diffuse * texture2D(u_Texture, uv) + specular);
	
	gl_FragColor.a = 0.5;
}
