#include "vec4.h"

namespace tzw {

vec4::vec4()
    :x(0),y(0),z(0),w(1.0)
{

}

vec4::vec4(float x, float y, float z, float w)
    :x(x),y(y),z(z),w(w)
{

}

vec4::vec4(vec3 v, float w)
    :x(v.x),y(v.y),z(v.z),w(w)
{

}

vec3 vec4::toVec3()
{
    return vec3(x,y,z);
}

vec4 vec4::operator +(const vec4 &other)
{
    return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

} // namespace tzw

