#ifndef TZW_CUBEPRIMITIVE_H
#define TZW_CUBEPRIMITIVE_H
#include "../../Interface/Drawable3D.h"
#include "../../Mesh/Mesh.h"
namespace tzw {

class CubePrimitive : public Drawable3D
{
public:
    CubePrimitive(float width, float depth, float height);
    virtual void submitDrawCmd();
    bool intersectBySphere(const t_Sphere &sphere, std::vector<vec3> &hitPoint);
protected:
    Material * m_tech;
    void initMesh();
    Mesh * m_mesh;
    float m_width, m_depth, m_height;
};

} // namespace tzw

#endif // TZW_CUBEPRIMITIVE_H
