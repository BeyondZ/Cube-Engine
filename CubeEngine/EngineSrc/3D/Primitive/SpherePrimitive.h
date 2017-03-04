#ifndef TZW_SPHERE_PRIMITIVE_H
#define TZW_SPHERE_PRIMITIVE_H


#include "../../Interface/Drawable3D.h"
#include "../../Mesh/Mesh.h"

namespace tzw
{
	class SpherePrimitive  : public Drawable3D
	{
	public:
		SpherePrimitive(float radius, int resolution);
		virtual void submitDrawCmd();
		bool intersectBySphere(const t_Sphere &sphere, std::vector<vec3> &hitPoint);
		vec3 pointOnSurface(float u, float v);
		vec3 getWorldPos(vec3 localPos);
	protected:
		
		void initMesh();
		
		virtual void checkCollide(ColliderEllipsoid * package);
		Mesh * m_mesh;
		float m_radius;
		int m_resolution;
	};

}



#endif