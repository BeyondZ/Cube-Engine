#pragma once
class btCollisionShape;
namespace tzw
{
class vec3;
class AABB;

class PhysicsShape
	{
	public:
		PhysicsShape();
		void initBoxShape(AABB aabb);
		void initBoxShape(vec3 size);
		void initSphereShape(float radius);
		void initCylinderShape(float width, float height, float radius);
	btCollisionShape * getRawShape();
	private:
		btCollisionShape * m_shape;
	};
}
