#include "GamePart.h"
#include "Island.h"
#include "BearPart.h"

namespace tzw
{

	Drawable3D * GamePart::getNode() const
	{
		return m_node;
	}

	void GamePart::setNode(Drawable3D * node)
	{
		m_node = node;
	}

	PhysicsShape * GamePart::getShape()
	{
		return m_shape;
	}

	Attachment * GamePart::findProperAttachPoint(Ray ray, vec3 &attachPosition, vec3 &Normal, vec3 & up)
	{
		float minDist = 99999999.0;
		int resultIndx = -1.0;
		for (auto i = 0; i < getAttachmentCount(); i++) 
		{
			auto attach = getAttachment(i);
			vec3 hitPointLocal;
			if(attach->isHit(ray, hitPointLocal)) 
			{
				if (hitPointLocal.distance(ray.origin()) < minDist) 
				{
					resultIndx = i;
					minDist = hitPointLocal.distance(ray.origin());
				}
	        }
		}
		if (resultIndx >= 0) 
		{
			auto attachPtr = getAttachmentInfo(resultIndx, attachPosition, Normal, up);
			return attachPtr;
		}
		return nullptr;
	}

	void GamePart::attachTo(Attachment * attach)
	{
		vec3 attachPosition,  Normal,  up;
		attach->getAttachmentInfo(attachPosition, Normal, up);
		//we use m_attachment[0]
		Normal = Normal * -1;
		auto selfAttah = getFirstAttachment();
		vec3 right = vec3::CrossProduct(Normal, up);
		Matrix44 transformForAttachPoint;
		auto data = transformForAttachPoint.data();
		data[0] = right.x;
		data[1] = right.y;
		data[2] = right.z;
		data[3] = 0.0;

		data[4] = up.x;
		data[5] = up.y;
		data[6] = up.z;
		data[7] = 0.0;

		data[8] = -Normal.x;
		data[9] = -Normal.y;
		data[10] = -Normal.z;
		data[11] = 0.0;

		data[12] = attachPosition.x;
		data[13] = attachPosition.y;
		data[14] = attachPosition.z;
		data[15] = 1.0;


		Matrix44 attachmentTrans;
		data = attachmentTrans.data();
		auto rightForAttach = vec3::CrossProduct(selfAttah->m_normal, selfAttah->m_up);
		data[0] = rightForAttach.x;
		data[1] = rightForAttach.y;
		data[2] = rightForAttach.z;
		data[3] = 0.0;

		data[4] = selfAttah->m_up.x;
		data[5] = selfAttah->m_up.y;
		data[6] = selfAttah->m_up.z;
		data[7] = 0.0;

		//use invert
		data[8] = -selfAttah->m_normal.x;
		data[9] = -selfAttah->m_normal.y;
		data[10] = -selfAttah->m_normal.z;
		data[11] = 0.0;

		data[12] = selfAttah->m_pos.x;
		data[13] = selfAttah->m_pos.y;
		data[14] = selfAttah->m_pos.z;
		data[15] = 1.0;

		auto result = transformForAttachPoint * attachmentTrans.inverted();
		Quaternion q;
		q.fromRotationMatrix(&result);
		m_node->setPos(result.getTranslation());
		m_node->setRotateQ(q);
	}

	Matrix44 GamePart::attachToFromOtherIsland(Attachment * attach)
	{
		auto islandMatrixInverted = m_parent->m_node->getLocalTransform().inverted();
		vec3 attachPosition,  Normal,  up;
		attach->getAttachmentInfoWorld(attachPosition, Normal, up);
		//transform other island attachment to our island
		attachPosition = islandMatrixInverted.transformVec3(attachPosition);
		Normal = islandMatrixInverted.transofrmVec4(vec4(Normal, 0.0)).toVec3();
		vec3 InvertedNormal = Normal * -1;
		attachPosition = attachPosition + Normal * 0.01f;
		up = islandMatrixInverted.transofrmVec4(vec4(up, 0.0)).toVec3();
		//we use m_attachment[0]
		auto selfAttah = getFirstAttachment();
        if (attach->m_parent->isConstraint()) 
		{
			static_cast<GameConstraint *>(attach->m_parent)->m_a = selfAttah;
        	selfAttah->m_connected = attach;
        }
		vec3 right = vec3::CrossProduct(InvertedNormal, up);
		Matrix44 transformForAttachPoint;
		auto data = transformForAttachPoint.data();
		data[0] = right.x;
		data[1] = right.y;
		data[2] = right.z;
		data[3] = 0.0;

		data[4] = up.x;
		data[5] = up.y;
		data[6] = up.z;
		data[7] = 0.0;

		data[8] = -InvertedNormal.x;
		data[9] = -InvertedNormal.y;
		data[10] = -InvertedNormal.z;
		data[11] = 0.0;

		data[12] = attachPosition.x;
		data[13] = attachPosition.y;
		data[14] = attachPosition.z;
		data[15] = 1.0;


		Matrix44 attachmentTrans;
		data = attachmentTrans.data();
		auto rightForAttach = vec3::CrossProduct(selfAttah->m_normal, selfAttah->m_up);
		vec3 normalForAttach = selfAttah->m_normal;
		data[0] = rightForAttach.x;
		data[1] = rightForAttach.y;
		data[2] = rightForAttach.z;
		data[3] = 0.0;

		data[4] = selfAttah->m_up.x;
		data[5] = selfAttah->m_up.y;
		data[6] = selfAttah->m_up.z;
		data[7] = 0.0;

		//use invert
		data[8] = -normalForAttach.x;
		data[9] = -normalForAttach.y;
		data[10] = -normalForAttach.z;
		data[11] = 0.0;

		data[12] = selfAttah->m_pos.x;
		data[13] = selfAttah->m_pos.y;
		data[14] = selfAttah->m_pos.z;
		data[15] = 1.0;

		auto result = transformForAttachPoint * attachmentTrans.inverted();
		Quaternion q;
		q.fromRotationMatrix(&result);
		m_node->setPos(result.getTranslation());
		m_node->setRotateQ(q);
		m_node->reCache();
		return result;
	}

	Matrix44 GamePart::attachToFromOtherIslandAlterSelfIsland(Attachment* attach, Attachment * ownAttachment)
	{
		vec3 attachPosition,  Normal,  up;
		attach->getAttachmentInfoWorld(attachPosition, Normal, up);
		vec3 InvertedNormal = Normal * -1;
		attachPosition = attachPosition + Normal * 0.01f;
		//we use m_attachment[0]
		Attachment * selfAttah = nullptr;
		if (!ownAttachment) 
		{
			selfAttah = getFirstAttachment();  
		}
		else 
		{
            selfAttah = ownAttachment;
		}
        if (attach->m_parent->isConstraint()) 
		{
			static_cast<GameConstraint *>(attach->m_parent)->m_a = selfAttah;
        	selfAttah->m_connected = attach;
        }
		vec3 right = vec3::CrossProduct(InvertedNormal, up);
		Matrix44 attachOuterWorldMat;
		auto data = attachOuterWorldMat.data();
		data[0] = right.x;
		data[1] = right.y;
		data[2] = right.z;
		data[3] = 0.0;

		data[4] = up.x;
		data[5] = up.y;
		data[6] = up.z;
		data[7] = 0.0;

		data[8] = -InvertedNormal.x;
		data[9] = -InvertedNormal.y;
		data[10] = -InvertedNormal.z;
		data[11] = 0.0;

		data[12] = attachPosition.x;
		data[13] = attachPosition.y;
		data[14] = attachPosition.z;
		data[15] = 1.0;


		Matrix44 attachmentTrans;
		data = attachmentTrans.data();
		auto rightForAttach = vec3::CrossProduct(selfAttah->m_normal, selfAttah->m_up);
		vec3 normalForAttach = selfAttah->m_normal;
		data[0] = rightForAttach.x;
		data[1] = rightForAttach.y;
		data[2] = rightForAttach.z;
		data[3] = 0.0;

		data[4] = selfAttah->m_up.x;
		data[5] = selfAttah->m_up.y;
		data[6] = selfAttah->m_up.z;
		data[7] = 0.0;

		//use invert
		data[8] = -normalForAttach.x;
		data[9] = -normalForAttach.y;
		data[10] = -normalForAttach.z;
		data[11] = 0.0;

		data[12] = selfAttah->m_pos.x;
		data[13] = selfAttah->m_pos.y;
		data[14] = selfAttah->m_pos.z;
		data[15] = 1.0;

		auto result = attachOuterWorldMat * attachmentTrans.inverted() * getNode()->getLocalTransform().inverted();

		Quaternion q;
		q.fromRotationMatrix(&result);
		m_parent->m_node->setPos(result.getTranslation());
		m_parent->m_node->setRotateQ(q);
		m_parent->m_node->reCache();
		return result;
	}

	Attachment* GamePart::getFirstAttachment()
	{
		return nullptr;
	}

	Attachment* GamePart::getBottomAttachment()
	{
		return getFirstAttachment();
	}

	Attachment* GamePart::getTopAttachment()
	{
		return getFirstAttachment();
	}

	Attachment* GamePart::getAttachment(int index)
	{
		return nullptr;
	}

	int GamePart::getAttachmentCount()
	{
		return 0;
	}

	Attachment* GamePart::getAttachmentInfo(int index, vec3& pos, vec3& N, vec3& up)
	{
		auto mat = m_node->getLocalTransform();
		auto atta = getAttachment(index);
		vec4 a_pos = vec4(atta->m_pos, 1.0);
		vec4 a_n = vec4(atta->m_normal, 0.0);
		vec4 a_up = vec4(atta->m_up, 0.0);
		pos = mat.transofrmVec4(a_pos).toVec3();
		N = mat.transofrmVec4(a_n).toVec3();
		up = mat.transofrmVec4(a_up).toVec3();
		return atta;
	}

	GamePart::~GamePart()
	{
		
		m_node->removeFromParent();
		delete m_node;
	}

	int GamePart::getType()
	{
		return GAME_PART_NOT_VALID;
	}

	float GamePart::getMass()
	{
		return 1.0f;
	}

	void GamePart::load(rapidjson::Value& partData)
	{
		setGUID(partData["UID"].GetString());
		getNode()->setPos(vec3(partData["pos"][0].GetDouble(),partData["pos"][1].GetDouble(), partData["pos"][2].GetDouble()));
		auto q = Quaternion(partData["rotate"][0].GetDouble(),partData["rotate"][1].GetDouble(), partData["rotate"][2].GetDouble(), partData["rotate"][3].GetDouble());
		getNode()->setRotateQ(q);
		if (partData.HasMember("attachList"))
		{
			auto& attachList = partData["attachList"];
			for(unsigned int i =0; i < attachList.Size(); i ++)
			{
				auto &attachData = attachList[i];
				std::string guid_str = attachData["UID"].GetString();
				getAttachment(i)->setGUID(guid_str.c_str());
				getAttachment(i)->m_connectedGUID = attachData["to"].GetString();
			}
        }
	}

	bool GamePart::isConstraint()
	{
		return false;
	}

	vec3 GamePart::getWorldPos()
	{
		return m_node->getWorldPos();
	}
}
