#include "BearPart.h"
#include "Interface/Drawable3D.h"
#include "3D/Primitive/CylinderPrimitive.h"
#include "Texture/TextureMgr.h"
#include "NodeEditorNodes/BearingPartNode.h"
#include "MainMenu.h"
#include "GamePart.h"
#include "Island.h"
#include "Collision/PhysicsMgr.h"
#include "Utility/math/TbaseMath.h"
#include "Rendering/Renderer.h"
#include "BuildingSystem.h"
#include "3D/Model/Model.h"
#include "Base/TranslationMgr.h"

// #include "EngineSrc/Collision/Physics6DOFConstraint.h"
namespace tzw
{
	float blockSize = 0.05;
BearPart::BearPart()
{
	m_a = nullptr;
	m_b = nullptr;
	m_isFlipped = false;
	m_node = nullptr;
	m_constrain = nullptr;
	

	m_isSteering = false;
	m_isAngleLimit = false;
	m_angleLimitLow = -30.0f;
	m_angleLimitHigh = 30.0f;
	//forward backward
	addAttachment(new Attachment(vec3(0.0, 0.0, blockSize / 2.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0) ,this));
	addAttachment(new Attachment(vec3(0.0, 0.0, -blockSize / 2.0), vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0) ,this));
	BearPart::generateName();
	
	auto nodeEditor = MainMenu::shared()->getNodeEditor();
	m_graphNode = new BearingPartNode(this);
	nodeEditor->addNode(m_graphNode);
	m_xrayMat = Material::createFromTemplate("PartXRay");
	
	// create a indicate model
	auto cylinderIndicator = new CylinderPrimitive(0.15, 0.15, 0.1);
	cylinderIndicator->setColor(vec4(1.0, 1.0, 1.0, 1.0));
	m_node = cylinderIndicator;
	m_xrayMat->setTex("DiffuseMap", cylinderIndicator->getTopBottomMaterial()->getTex("diffuseMap"));
	cylinderIndicator->onSubmitDrawCommand = [cylinderIndicator, this](RenderCommand::RenderType passType)
	{
		if(BuildingSystem::shared()->isIsInXRayMode())
		{
			RenderCommand command(cylinderIndicator->getMesh(), this->m_xrayMat, passType);
			cylinderIndicator->setUpCommand(command);
			command.setRenderState(RenderFlag::RenderStage::AFTER_DEPTH_CLEAR);
			Renderer::shared()->addRenderCommand(command);

			RenderCommand command2(cylinderIndicator->getTopBottomMesh(), this->m_xrayMat, passType);
			cylinderIndicator->setUpCommand(command2);
			command2.setRenderState(RenderFlag::RenderStage::AFTER_DEPTH_CLEAR);
			Renderer::shared()->addRenderCommand(command2);
		}
	};
}

BearPart::BearPart(std::string itemName)
{
	m_a = nullptr;
	m_b = nullptr;
	m_isFlipped = false;
	m_node = nullptr;
	m_constrain = nullptr;
	

	m_isSteering = false;
	m_isAngleLimit = false;
	m_angleLimitLow = -30.0f;
	m_angleLimitHigh = 30.0f;


	GamePart::initFromItemName(itemName);
	
	BearPart::generateName();
	
	auto nodeEditor = MainMenu::shared()->getNodeEditor();
	m_graphNode = new BearingPartNode(this);
	nodeEditor->addNode(m_graphNode);
	m_xrayMat = Material::createFromTemplate("PartXRay");
	
	auto cylinderIndicator = static_cast<Model *> (m_node);
	m_xrayMat->setTex("DiffuseMap", cylinderIndicator->getMat(0)->getTex("DiffuseMap"));
	cylinderIndicator->onSubmitDrawCommand = [cylinderIndicator, this](RenderCommand::RenderType passType)
	{
		if(BuildingSystem::shared()->isIsInXRayMode())
		{
			RenderCommand command(cylinderIndicator->getMesh(0), this->m_xrayMat, passType);
			cylinderIndicator->setUpCommand(command);
			command.setRenderState(RenderFlag::RenderStage::AFTER_DEPTH_CLEAR);
			Renderer::shared()->addRenderCommand(command);
		}
	};
}

	void BearPart::updateFlipped()
{
	if(!m_node) return;
	auto model = static_cast<Model *>(m_node);
	Texture * tex;
	if(m_isFlipped)
	{
		tex = TextureMgr::shared()->getByPath("Blocks/Bearing/diffuse_inverted.png");
	}
	else
	{
		tex = TextureMgr::shared()->getByPath("Blocks/Bearing/diffuse.png");
	}
	model->getMat(0)->setTex("DiffuseMap", tex);
	m_xrayMat->setTex("DiffuseMap", tex);
}

int BearPart::getAttachmentCount()
{
	return 2;
}

BearPart::~BearPart()
{
	auto nodeEditor = MainMenu::shared()->getNodeEditor();
	nodeEditor->removeNode(m_graphNode);
	delete m_graphNode;
	if(m_constrain) 
	{
		PhysicsMgr::shared()->removeConstraint(m_constrain);
		delete m_constrain;
	}
	
}

GameNodeEditorNode* BearPart::getGraphNode() const
{
	return m_graphNode;
}

void BearPart::load(rapidjson::Value& constraintData)
{
	m_isFlipped = constraintData["isFlipped"].GetBool();
	updateFlipped();
    static_cast<ResNode *> (m_graphNode)->syncName();
}

void BearPart::setIsSteering(bool isSteering)
{
	m_isSteering = isSteering;
	if(m_constrain && m_isEnablePhysics)
	{
		if(m_isSteering)
		{
			m_constrain->enableAngularMotor(true, 0.0f, 10000.0f);
		}
		else 
		{
			m_constrain->enableAngularMotor(false, 0.0f, 10000.0f);
		}
	}

}

bool BearPart::getIsSteering() const
{
	return m_isSteering;
}

void BearPart::setAngleLimit(bool isAngleLimit, float low, float high)
{
	m_isAngleLimit = isAngleLimit;
	m_angleLimitLow = low;
	m_angleLimitHigh = high;
	if(m_constrain && m_isEnablePhysics)
	{
		if(m_isAngleLimit)
		{
			m_constrain->setLimit(m_angleLimitLow* (TbaseMath::PI_OVER_180), m_angleLimitHigh * (TbaseMath::PI_OVER_180));
		}
		else 
		{
			m_constrain->setLimit(0, 0);
		}
	}
}

void BearPart::getAngleLimit(bool& isAngleLimit, float& low, float& high) const
{
	isAngleLimit = m_isAngleLimit;
	low = m_angleLimitLow;
	high = m_angleLimitHigh;
}

void BearPart::generateName()
{
	char formatName[512];
	sprintf_s(formatName, 512, TRC(u8"��� %s"),genShortName().c_str());
	setName(formatName);
}

	GamePartType BearPart::getType()
	{
		return GamePartType::GAME_PART_BEARING;
	}

	void BearPart::enableAngularMotor(bool enableMotor, float targetVelocity, float maxMotorImpulse)
	{
		if(m_constrain)
		{
			float factor = m_isFlipped?-1.0f:1.0f;
			m_constrain->enableAngularMotor(enableMotor, targetVelocity * factor, maxMotorImpulse);
		}
	}

	void BearPart::drawInspect()
	{
		if(drawInspectNameEdit())
		{
			static_cast<ResNode *>(m_graphNode)->syncName();
		}
		//control the bearing turn direction
		ImGui::Text(TRC(u8"��ת����"));
		auto click_left = false, click_right = false;
		click_left = ImGui::RadioButton(TRC(u8"��"), !m_isFlipped);ImGui::SameLine(); click_right = ImGui::RadioButton(TRC(u8"��"), m_isFlipped);
		
		if(click_left) 
		{
			m_isFlipped = false;
		}
		if(click_right) 
		{
			m_isFlipped = true;
		}
		if(click_left || click_right) 
		{
			updateFlipped();        
		}
		
		auto click_steer = false, click_bear = false;
		ImGui::Text(TRC(u8"�Ƿ�Ϊ���"));
		click_steer = ImGui::RadioButton(TRC(u8"���"), getIsSteering());ImGui::SameLine(); click_bear = ImGui::RadioButton(TRC(u8"��ͨ���"), !getIsSteering());
		
		if(click_steer || click_bear)
		{
			setIsSteering(click_steer);
		}
		ImGui::Text(TRC(u8"�Ƿ����Ƕ�����"));
		bool isCurrAngleLimit;
		float angle_low;
		float angle_high;
		getAngleLimit(isCurrAngleLimit, angle_low, angle_high);
		auto click_angle_yes = false, click_angle_no = false;
		click_angle_yes = ImGui::RadioButton(TRC(u8"����"), isCurrAngleLimit);ImGui::SameLine(); click_angle_no = ImGui::RadioButton(TRC(u8"�ر�"), !isCurrAngleLimit);
		if(isCurrAngleLimit)
		{
			bool isInput = false;
			ImGui::PushItemWidth(80);
			isInput |= ImGui::InputFloat(TRC(u8"��С�Ƕ�"), &angle_low);
			isInput |= ImGui::InputFloat(TRC(u8"���Ƕ�"), &angle_high);
			ImGui::PopItemWidth();
			if(isInput)
			{
				setAngleLimit(true, angle_low, angle_high);
			}
		}
		if(click_angle_yes || click_angle_no)
		{
			setAngleLimit(click_angle_yes, angle_low, angle_high);
		}

	}
	static Matrix44 groupMatNode(vec3 pos, vec3 normal, vec3 up, Matrix44 reservedMat)
	{
	Matrix44 mat;
	auto data = mat.data();
	vec3 right = vec3::CrossProduct(normal * -1, up);
	data[0] = right.x;
	data[1] = right.y;
	data[2] = right.z;
	data[3] = 0.0;

	data[4] = up.x;
	data[5] = up.y;
	data[6] = up.z;
	data[7] = 0.0;

	data[8] = normal.x;
	data[9] = normal.y;
	data[10] = normal.z;
	data[11] = 0.0;

	data[12] = pos.x;
	data[13] = pos.y;
	data[14] = pos.z;
	data[15] = 1.0;

	return reservedMat * mat;
	}
	bool BearPart::isNeedDrawInspect()
	{
		return true;
	}

	void BearPart::updateConstraintState()
	{

		if(m_constrain)
		{
			if(m_isEnablePhysics)
			{
				PhysicsMgr::shared()->removeConstraint(m_constrain);
			}
			delete m_constrain;
		}
		m_constrain = nullptr;
		auto attachA = m_a;
		auto attachB = m_b;
		if (attachA && attachB) 
		{
			auto partA = attachA->m_parent;
			auto partB = attachB->m_parent;

			vec3 worldPosA, worldNormalA, worldUpA;
			attachA->getAttachmentInfoWorld(worldPosA, worldNormalA, worldUpA);
			vec3 worldPosB, worldNormalB, worldUpB;
			attachB->getAttachmentInfoWorld(worldPosB, worldNormalB, worldUpB);
			vec3 hingeDir = (worldPosB - worldPosA).normalized();
			vec3 pivotA, pivotB, axisA, axisB;
			findPiovtAndAxis(attachA, hingeDir, pivotA, axisA);
			findPiovtAndAxis(attachB, hingeDir, pivotB, axisB);
				Matrix44 frameInA;
		vec3 pos, n, up;
		attachA->getAttachmentInfoWorld(pos, n, up);
		attachA->getAttachmentInfoWorld(pos, n, up);
		frameInA = groupMatNode(pos, n, up, partA->m_parent->m_node->getTransform().inverted());
		Matrix44 frameInB;
		frameInB = groupMatNode(pos, n, up, partB->m_parent->m_node->getTransform().inverted());
		
		//auto constrain = PhysicsMgr::shared()->create6DOFConstraint(partA->m_parent->m_rigid, partB->m_parent->m_rigid, frameInA, frameInB);
		auto constrain = PhysicsMgr::shared()->createHingeConstraint(partA->m_parent->m_rigid, partB->m_parent->m_rigid, pivotA, pivotB, axisA, axisB, false);
			m_constrain = constrain;
			//constrain->makeUpBearing();
		}
		if(m_isEnablePhysics)
		{
			if(m_constrain)
			{
				PhysicsMgr::shared()->addConstraint(m_constrain);
			}
		}
	}

	void BearPart::createConstraintImp()
	{
		auto attachA = m_a;
		auto attachB = m_b;
		auto partA = attachA->m_parent;
		auto partB = attachB->m_parent;

		vec3 worldPosA, worldNormalA, worldUpA;
		attachA->getAttachmentInfoWorld(worldPosA, worldNormalA, worldUpA);
		vec3 worldPosB, worldNormalB, worldUpB;
		attachB->getAttachmentInfoWorld(worldPosB, worldNormalB, worldUpB);
		vec3 hingeDir = (worldPosB - worldPosA).normalized();
		vec3 pivotA, pivotB, axisA, axisB;
		findPiovtAndAxis(attachA, hingeDir, pivotA, axisA);
		findPiovtAndAxis(attachB, hingeDir, pivotB, axisB);
		Matrix44 frameInA;
		vec3 pos, n, up;
		attachA->getAttachmentInfoWorld(pos, n, up);
		frameInA = groupMatNode(pos, n, up, partA->m_parent->m_node->getTransform().inverted());
		Matrix44 frameInB;
		frameInB = groupMatNode(pos, n, up, partB->m_parent->m_node->getTransform().inverted());
		//auto constrain = PhysicsMgr::shared()->create6DOFConstraint(partA->m_parent->m_rigid, partB->m_parent->m_rigid, frameInA, frameInB);
		auto constrain = PhysicsMgr::shared()->createHingeConstraint(partA->m_parent->m_rigid, partB->m_parent->m_rigid, pivotA, pivotB, axisA, axisB, false);
		m_constrain = constrain;
		//constrain->makeUpBearing();
	}

	void BearPart::findPiovtAndAxis(Attachment * attach, vec3 hingeDir,  vec3 & pivot, vec3 & asix)
{
	auto part = attach->m_parent;
	auto island = part->m_parent;
	auto islandInvertedMatrix = island->m_node->getLocalTransform().inverted();
	
	auto transform = part->getNode()->getLocalTransform();
	auto normalInIsland = transform.transofrmVec4(vec4(attach->m_normal, 0.0)).toVec3();

	pivot = transform.transofrmVec4(vec4(attach->m_pos /*+ attach->m_normal * (blockSize / 2.0)*/, 1.0)).toVec3();
	asix = islandInvertedMatrix.transofrmVec4(vec4(hingeDir, 0.0)).toVec3();
}

void BearPart::enablePhysics(bool isEnable)
{
	GameConstraint::enablePhysics(isEnable);
	if(isEnable) 
	{
		if(!m_constrain) 
		{
			auto attachA = m_a;
			auto attachB = m_b;
			if (attachA && attachB) 
			{
				createConstraintImp();
				PhysicsMgr::shared()->addConstraint(m_constrain);
			}
		}
		else 
		{
			PhysicsMgr::shared()->addConstraint(m_constrain);
		}
		if(m_isSteering)
		{
			m_constrain->enableAngularMotor(true, 0.0f, 10000.0f);
		}

		if(m_isAngleLimit)
		{
			m_constrain->setLimit(m_angleLimitLow* (TbaseMath::PI_OVER_180), m_angleLimitHigh * (TbaseMath::PI_OVER_180));
		}
	}
	else 
	{
		if(m_constrain) 
		{
			PhysicsMgr::shared()->removeConstraint(m_constrain);
		}
	}
}

void BearPart::dump(rapidjson::Value& partData, rapidjson::Document::AllocatorType& allocator)
{
	GameConstraint::dump(partData, allocator);
	partData.AddMember("Type", "Bearing", allocator);
	
	partData.AddMember("from", std::string(m_b->getGUID()), allocator);

	partData.AddMember("isSteering", getIsSteering(), allocator);

	bool isLimit;
	float low,high;
	getAngleLimit(isLimit, low, high);
	partData.AddMember("isAngleLimit", isLimit, allocator);
	partData.AddMember("AngleLimitLow", low, allocator);
	partData.AddMember("AngleLimitHigh", high, allocator);
	if(m_a)
	{
		partData.AddMember("to", std::string(m_a->getGUID()), allocator);
	}
	dumpAttach(partData, allocator);
	partData.AddMember("UID", std::string(getGUID()), allocator);
	partData.AddMember("isFlipped", m_isFlipped, allocator);
}

Attachment* BearPart::getFirstAttachment()
{
	return m_attachment[0];
}

Attachment* BearPart::getAttachment(int index)
{
	return m_attachment[index];
}
}

