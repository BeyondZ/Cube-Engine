layout(set = 0, binding = 1) uniform sampler2D DiffuseMap;

layout(location = 0) out vec4 out_Color;
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 v_texcoord;
void main() 
{
  out_Color = texture(DiffuseMap, v_texcoord) * fragColor;//vec4( fragColor.rgb, fragColor.a);
}
