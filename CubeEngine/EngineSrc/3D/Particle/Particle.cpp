#include "Particle.h"
namespace tzw
{
	Particle::Particle(): size(1.0f), m_span(1.0f), m_curAge(0.0f), m_alpha(1.0)
	{
	}

	void Particle::step(float dt)
	{
		m_pos = m_velocity *dt + m_pos;
		m_curAge += dt;
	}

	bool Particle::isDead()
	{
		return m_curAge >= m_span;
	}
}
