#include "PhysicsHingeConstraint.h"
#include "BulletDynamics/ConstraintSolver/btHingeConstraint.h"

namespace tzw
{
	PhysicsHingeConstraint::PhysicsHingeConstraint(btHingeConstraint * constraint)
		: m_constraint(constraint)
	{
	}

btTypedConstraint* PhysicsHingeConstraint::constraint() const
{
	return m_constraint;
}

void PhysicsHingeConstraint::setConstraint(btTypedConstraint* constraint)
{
	m_constraint = static_cast<btHingeConstraint *>(constraint);
}

void PhysicsHingeConstraint::enableAngularMotor(bool enableMotor, float targetVelocity, float maxMotorImpulse)
{
  if (m_constraint) 
  {
	//let the rigidBody wake up
	m_constraint->getRigidBodyA().activate();
	m_constraint->getRigidBodyB().activate();
	m_constraint->enableAngularMotor(enableMotor, targetVelocity, maxMotorImpulse);
  }

}

void PhysicsHingeConstraint::setLimit(float low, float high, float _softness, float _biasFactor, float _relaxationFactor)
{
  if (m_constraint) 
  {
	//let the rigidBody wake up
	m_constraint->getRigidBodyA().activate();
	m_constraint->getRigidBodyB().activate();
	m_constraint->setLimit(low, high,  _softness,  _biasFactor,  _relaxationFactor);
  }
}
}
