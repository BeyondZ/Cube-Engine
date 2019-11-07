#pragma once
#include "BearPart.h"
#include "GamePart.h"
#include "noise/module/perlin.h"

namespace tzw
{
	class Island;
	class ParticleEmitter;
	class ThrusterPart : public GamePart
	{
	public:
		ThrusterPart();
		explicit ThrusterPart(std::string itemName);
		void initAttachments();
		Attachment * getAttachmentInfo(int index, vec3&pos, vec3&N, vec3&up);
		Attachment * getFirstAttachment() override;
		Attachment * getBottomAttachment() override;
		Attachment * getAttachment(int index) override;
		int getAttachmentCount() override;
		void generateName() override;
		GamePartType getType() override;
		BearPart * m_bearPart[6];
		void toggle(int openSignal);
		void updateForce(float dt);
		GameNodeEditorNode * getGraphNode() const;
		virtual ~ThrusterPart();
		void drawInspect() override;
		bool isNeedDrawInspect() override;
		float getThrusterForce();
		void setThrusterForce(float thruster);
    private:
		int m_isOpen;
		float m_topRadius, m_bottomRadius, m_height;
		float m_phase, m_scale, m_t;
		vec3 m_phaseV3;
		float m_dir_t;
		float m_thrusterForce;
		GameNodeEditorNode * m_graphNode;
		ParticleEmitter * emitter;
		noise::module::Perlin flatNoise;
	};
}
