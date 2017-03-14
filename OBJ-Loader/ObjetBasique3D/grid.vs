in vec3 pos;

uniform mat4 u_WorldMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_CameraMatrix;

void main(void)
{
	gl_Position = u_ProjectionMatrix * u_WorldMatrix * u_CameraMatrix * vec4(pos, 1.0);
}