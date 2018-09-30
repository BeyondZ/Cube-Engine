#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 TU_mvpMatrix;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;
varying vec3 v_normal;

//! [0]
void main()
{
    // Calculate vertex position in screen space
    gl_Position = TU_mvpMatrix * vec4(a_position,1.0);

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
    v_texcoord.y = 1.0 - v_texcoord.y;
    v_normal = a_normal;
}
//! [0]
