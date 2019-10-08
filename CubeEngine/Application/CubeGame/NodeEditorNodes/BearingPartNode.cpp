#include "BearingPartNode.h"
#include "CubeGame/BearPart.h"

namespace tzw
{
	BearingPartNode::BearingPartNode(BearPart * bear)
	{
		m_part = bear;
		BearingPartNode::syncName();
		addIn(u8"����");
	}

	void BearingPartNode::privateDraw()
	{
		handleNameEdit();
		//control the bearing turn direction
		ImGui::Text(u8"��ת����");
		auto click_left = false, click_right = false;
		click_left = ImGui::RadioButton(u8"��", !m_part->m_isFlipped);ImGui::SameLine(); click_right = ImGui::RadioButton(u8"��", m_part->m_isFlipped);
		
		if(click_left) 
		{
			m_part->m_isFlipped = false;
		}
		if(click_right) 
		{
			m_part->m_isFlipped = true;
		}
		if(click_left || click_right) 
		{
			m_part->updateFlipped();        
		}
		
		auto click_steer = false, click_bear = false;
		ImGui::Text(u8"�Ƿ�Ϊ���");
		click_steer = ImGui::RadioButton(u8"���", m_part->getIsSteering());ImGui::SameLine(); click_bear = ImGui::RadioButton(u8"��ͨ���", !m_part->getIsSteering());
		
		if(click_steer || click_bear)
		{
			m_part->setIsSteering(click_steer);
		}
		ImGui::Text(u8"�Ƿ����Ƕ�����");
		bool isCurrAngleLimit;
		float angle_low;
		float angle_high;
		m_part->getAngleLimit(isCurrAngleLimit, angle_low, angle_high);
		auto click_angle_yes = false, click_angle_no = false;
		click_angle_yes = ImGui::RadioButton(u8"����", isCurrAngleLimit);ImGui::SameLine(); click_angle_no = ImGui::RadioButton(u8"�ر�", !isCurrAngleLimit);
		if(isCurrAngleLimit)
		{
			bool isInput = false;
			ImGui::PushItemWidth(80);
			isInput |= ImGui::InputFloat(u8"��С�Ƕ�", &angle_low);
			isInput |= ImGui::InputFloat(u8"���Ƕ�", &angle_high);
			ImGui::PopItemWidth();
			if(isInput)
			{
				m_part->setAngleLimit(true, angle_low, angle_high);
			}
		}
		if(click_angle_yes || click_angle_no)
		{
			m_part->setAngleLimit(click_angle_yes, angle_low, angle_high);
		}
	}

	GamePart* BearingPartNode::getProxy()
	{
		return m_part;
	}

	void BearingPartNode::load(rapidjson::Value& partData)
	{
	}

	void BearingPartNode::dump(rapidjson::Value& partDocObj, rapidjson::Document::AllocatorType& allocator)
	{
		partDocObj.AddMember("Type", std::string("Resource"), allocator);
		partDocObj.AddMember("ResType", std::string("BearPart"), allocator);
		partDocObj.AddMember("ResUID", std::string(m_part->getGUID()), allocator);
		partDocObj.AddMember("UID", std::string(getGUID()), allocator);
	}

	void BearingPartNode::syncName()
	{
		char formatName[512];
		sprintf_s(formatName, 512, u8"��� %s",m_part->getName().c_str());
		name = formatName;
	}
}
