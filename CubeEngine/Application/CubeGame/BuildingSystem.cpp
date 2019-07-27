#include "BuildingSystem.h"
#include "3D/Primitive/CubePrimitive.h"
#include <algorithm>
#include "Scene/SceneMgr.h"
#include "Collision/PhysicsCompoundShape.h"
#include "Collision/PhysicsMgr.h"
#include "CylinderPart.h"
#include "3D/Primitive/CylinderPrimitive.h"
#include "Chunk.h"
#include "ControlPart.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "Utility/file/Tfile.h"
#include "Base/Log.h"
#include "Base/GuidMgr.h"


namespace tzw
{
const float bearingGap = 0.00;
TZW_SINGLETON_IMPL(BuildingSystem);
BuildingSystem::BuildingSystem()
{
}

void BuildingSystem::createNewToeHold(vec3 pos)
{
	auto newIsland = new Island(pos);
	m_IslandList.push_back(newIsland);
	auto part = new BlockPart();
	newIsland->m_node->addChild(part->getNode());
	newIsland->insert(part);
}

void BuildingSystem::placePartNextToBearing(Attachment * attach, int type)
{
	auto bearing = attach->m_bearPart;
	vec3 pos, n, up;
	attach->getAttachmentInfoWorld(pos, n, up);
	auto island = new Island(pos + n * 0.5);
	m_IslandList.push_back(island);
	GamePart * part = nullptr;
	switch (type)
	{
	case 0:
		part = new BlockPart();
		break;
	case 1:
		part = new CylinderPart();
		break;
	}
	island->m_node->addChild(part->getNode());
	island->insert(part);
	part->attachToFromOtherIsland(attach, bearing);
}

void BuildingSystem::placePartByHit(vec3 pos, vec3 dir, float dist, int type)
{
	std::vector<GamePart *> tmp;
	//search island
	for (auto island : m_IslandList)
	{
		
		for (auto iter : island->m_partList)
		{
			tmp.push_back(iter);
		}
	}
	std::sort(tmp.begin(), tmp.end(), [&](GamePart * left, GamePart * right)
	{
		float distl = left->getNode()->getWorldPos().distance(pos);
		float distr = right->getNode()->getWorldPos().distance(pos);
		return distl < distr;
	}
	);
		for (auto iter : tmp)
		{
			auto island = iter->m_parent;
			auto node = iter->getNode();
			auto invertedMat = node->getTransform().inverted();
			vec4 dirInLocal = invertedMat * vec4(dir, 0.0);
			vec4 originInLocal = invertedMat * vec4(pos, 1.0);

			auto r = Ray(originInLocal.toVec3(), dirInLocal.toVec3());
			RayAABBSide side;
			vec3 hitPoint;
			auto isHit = r.intersectAABB(node->localAABB(), &side, hitPoint);
			GamePart * newPart = nullptr;
			if (isHit)
			{
				if(type == 0)
				{
					newPart = new BlockPart();
				}
				else if (type == 1)
				{
					newPart = new CylinderPart();
				}
				vec3 attachPos, attachNormal, attachUp;
				auto attach = iter->findProperAttachPoint(r, attachPos, attachNormal,attachUp);
				if(attach)
				{
					if(!attach->m_bearPart)
					{
						newPart->attachTo(attach);
					}
					else // have some bearing?
					{
						placePartNextToBearing(attach, type);
						//todo
					}
				}
			return;
			}
		}
		//any island intersect can't find, so we need create a new toehold
		createNewToeHold(pos + dir * 5.0f);
}

void BuildingSystem::removePartByAttach(Attachment* attach)
{
	if (attach)
	{
		auto part = attach->m_parent;
		auto island = part->m_parent;
		if (!attach->m_bearPart)
		{
			part->getNode()->removeFromParent();
			island->remove(part);
			delete part;
		}
		else // have some bearing?
		{
			attach->m_bearPart->m_node->removeFromParent();
			m_bearList.erase(m_bearList.find(attach->m_bearPart));
			attach->m_bearPart = nullptr;
		}
	}
}

void BuildingSystem::placeGamePart(GamePart* part, vec3 pos)
{
	auto newIsland = new Island(pos);
	m_IslandList.push_back(newIsland);
	newIsland->m_node->addChild(part->getNode());
	newIsland->insert(part);
}

void BuildingSystem::attachGamePartToBearing(GamePart* part,Attachment * attach)
{
	auto bearing = attach->m_bearPart;
	vec3 pos, n, up;
	attach->getAttachmentInfoWorld(pos, n, up);
	
	auto island = new Island(pos + n * 0.5);
	attach->m_parent->m_parent->addNeighbor(island);
	island->addNeighbor(attach->m_parent->m_parent);
	m_IslandList.push_back(island);
	island->m_node->addChild(part->getNode());
	island->insert(part);
	part->attachToFromOtherIsland(attach, bearing);
}

void BuildingSystem::attachGamePartNormal(GamePart* part, Attachment* attach)
{
	if (attach->m_parent->getType() == GAME_PART_LIFT) 
	{
		auto liftPart = dynamic_cast<LiftPart *>(attach->m_parent);
		vec3 pos, n, up;
		attach->getAttachmentInfoWorld(pos, n, up);
		auto newIsland = new Island(pos);
		m_IslandList.push_back(newIsland);
		newIsland->m_node->addChild(part->getNode());
		newIsland->insert(part);
		part->attachToFromOtherIsland(attach, nullptr);
		liftPart->setEffectedIsland(newIsland);
		
	} else
	{
		part->attachTo(attach);
		auto island = attach->m_parent->m_parent;
		island->m_node->addChild(part->getNode());
		island->insert(part);
	}
}

void BuildingSystem::terrainForm(vec3 pos, vec3 dir, float dist, float value, float range)
{
	std::vector<Drawable3D *> list;
	AABB aabb;
	aabb.update(vec3(pos.x - 10, pos.y - 10, pos.z - 10));
	aabb.update(vec3(pos.x + 10, pos.y + 10, pos.z + 10));
	g_GetCurrScene()->getRange(&list, aabb);
	if (!list.empty())
	{
		Drawable3DGroup group(&list[0], list.size());
		Ray ray(pos, dir);
		vec3 hitPoint;
		auto chunk = static_cast<Chunk *>(group.hitByRay(ray, hitPoint));
		if (chunk)
		{
			chunk->deformSphere(hitPoint, value, range);
		}
	}
}

vec3 BuildingSystem::hitTerrain(vec3 pos, vec3 dir, float dist)
{
	std::vector<Drawable3D *> list;
	AABB aabb;
	aabb.update(vec3(pos.x - 10, pos.y - 10, pos.z - 10));
	aabb.update(vec3(pos.x + 10, pos.y + 10, pos.z + 10));
	g_GetCurrScene()->getRange(&list, aabb);
	if (!list.empty())
	{
		Drawable3DGroup group(&list[0], list.size());
		Ray ray(pos, dir);
		vec3 hitPoint;
		auto chunk = static_cast<Chunk *>(group.hitByRay(ray, hitPoint));
		if (chunk)
		{
			return hitPoint;
		}
	}
	return vec3(999, 999, 999);
}

void BuildingSystem::placeLiftPart(vec3 wherePos)
{
	auto part = new LiftPart();
	//TODO

	//auto newIsland = new Island(wherePos);
	//m_IslandList.insert(newIsland);
	//newIsland->m_node->addChild(part->getNode());
	//newIsland->insert(part);
	part->getNode()->setPos(wherePos);
	g_GetCurrScene()->addNode(part->getNode());
	m_liftPart = part;
}

void BuildingSystem::setCurrentControlPart(GamePart* controlPart)
{
	auto part = dynamic_cast<ControlPart *> (controlPart);
	if(part) 
	{
		part->setActivate(true);
		m_controlPart = part;
	}
}

ControlPart* BuildingSystem::getCurrentControlPart() const
{
	return m_controlPart;
}

GamePart* BuildingSystem::createPart(int type)
{
	GamePart * resultPart = nullptr;
	switch(type)
	{
    case 0:
		resultPart = new BlockPart();
		break;
    case 1:
		resultPart = new CylinderPart();
		break;
    case 2:
		resultPart = new LiftPart();
		break;
    case 3: 
		{
			auto control_part =  new ControlPart();
			m_controlPart = control_part;
			resultPart = m_controlPart;
    	}
		break;
	default: ;
	}
	return resultPart;
}

BearPart * BuildingSystem::placeBearingToAttach(Attachment* attachment)
{
	auto bear = new BearPart();
	bear->m_b = attachment;
	attachment->m_bearPart = bear;
	m_bearList.insert(bear);
//create a indicate model
	auto cylinderIndicator = new CylinderPrimitive(0.15, 0.15, 0.1);
	cylinderIndicator->setColor(vec4(1.0, 1.0, 0.0, 0.0));
	auto mat = attachment->getAttachmentInfoMat44();
	Quaternion q;
	q.fromRotationMatrix(&mat);
	cylinderIndicator->setPos(mat.getTranslation());
	cylinderIndicator->setRotateQ(q);
	cylinderIndicator->reCache();
	bear->m_node = cylinderIndicator;
	bear->updateFlipped();
	attachment->m_parent->getNode()->addChild(cylinderIndicator);
	return bear;
}

SpringPart* BuildingSystem::placeSpringToAttach(Attachment* attachment)
{
	auto bear = new SpringPart();
	bear->m_b = attachment;
	//attachment->m_bearPart = bear;
	//m_bearList.insert(bear);
//create a indicate model
	auto cylinderIndicator = new CylinderPrimitive(0.15, 0.15, 0.1);
	cylinderIndicator->setColor(vec4(1.0, 1.0, 0.0, 0.0));
	auto mat = attachment->getAttachmentInfoMat44();
	Quaternion q;
	q.fromRotationMatrix(&mat);
	cylinderIndicator->setPos(mat.getTranslation());
	cylinderIndicator->setRotateQ(q);
	cylinderIndicator->reCache();
	bear->m_node = cylinderIndicator;
	attachment->m_parent->getNode()->addChild(cylinderIndicator);
	return bear;
}

Island* BuildingSystem::createIsland(vec3 pos)
{
	auto island = new Island(pos);
	m_IslandList.push_back(island);
	return island;
}

Attachment* BuildingSystem::rayTest(vec3 pos, vec3 dir, float dist)
{
	std::vector<GamePart *> tmp;
	//search island
	for (auto island : m_IslandList)
	{
		
		for (auto iter : island->m_partList)
		{
			tmp.push_back(iter);
		}
	}
	//add extra lift part
	tmp.push_back(m_liftPart);
	std::sort(tmp.begin(), tmp.end(), [&](GamePart * left, GamePart * right)
	{
		float distl = left->getNode()->getWorldPos().distance(pos);
		float distr = right->getNode()->getWorldPos().distance(pos);
		return distl < distr;
	}
	);
	for (auto iter : tmp)
	{
		auto island = iter->m_parent;
		auto node = iter->getNode();
		auto invertedMat = node->getTransform().inverted();
		vec4 dirInLocal = invertedMat * vec4(dir, 0.0);
		vec4 originInLocal = invertedMat * vec4(pos, 1.0);

		auto r = Ray(originInLocal.toVec3(), dirInLocal.toVec3());
		RayAABBSide side;
		vec3 hitPoint;
		auto isHit = r.intersectAABB(node->localAABB(), &side, hitPoint);
		GamePart * newPart = nullptr;
		if (isHit)
		{
			vec3 attachPos, attachNormal, attachUp;
			auto attach = iter->findProperAttachPoint(r, attachPos, attachNormal,attachUp);
			if(attach)
			{
				return attach;
			}
		}
	}
	//any island intersect can't find, return null
	return nullptr;
}

Island* BuildingSystem::rayTestIsland(vec3 pos, vec3 dir, float dist)
{
	auto attach = rayTest(pos, dir, dist);
	if(attach)
		return attach->m_parent->m_parent;
    else
		return nullptr;
}

LiftPart* BuildingSystem::getLift() const
{
	return m_liftPart;
}

ControlPart* BuildingSystem::getControlPart()
{
	return m_controlPart;
}

void BuildingSystem::dump()
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Value islandList(rapidjson::kArrayType);
	for (auto island : m_IslandList)
	{
		rapidjson::Value islandObject(rapidjson::kObjectType);
		island->dump(islandObject, doc.GetAllocator());
		islandList.PushBack(islandObject, doc.GetAllocator());
	}
	doc.AddMember("islandList", islandList, doc.GetAllocator());

	rapidjson::Value constraintList(rapidjson::kArrayType);
	for(auto bearing :m_bearList)
	{
		rapidjson::Value bearingObj(rapidjson::kObjectType);
		bearing->dump(bearingObj, doc.GetAllocator());
		constraintList.PushBack(bearingObj, doc.GetAllocator());
		
	}
	doc.AddMember("constraintList", constraintList, doc.GetAllocator());
	rapidjson::StringBuffer buffer;
	auto file = fopen("./test_island.txt", "w");
	char writeBuffer[65536];
	rapidjson::FileWriteStream stream(file, writeBuffer, sizeof(writeBuffer));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(stream);
	writer.SetIndent('\t', 1);
	doc.Accept(writer);
	fclose(file);
}

void BuildingSystem::load()
{
	
	rapidjson::Document doc;
	std::string filePath = "./test_island.txt";
	auto data = Tfile::shared()->getData(filePath, true);
	doc.Parse<rapidjson::kParseDefaultFlags>(data.getString().c_str());
	if (doc.HasParseError())
	{
		tlog("[error] get json data err! %s %d offset %d\n", filePath.c_str(), doc.GetParseError(), doc.GetErrorOffset());
		exit(0);
	}
	//island
	if (doc.HasMember("islandList"))
	{
		auto& items = doc["islandList"];
		for (unsigned int i = 0; i < items.Size(); i++)
		{
			auto& item = items[i];
			auto newIsland = new Island(vec3());
			m_IslandList.push_back(newIsland);
			newIsland->load(item);
		}
	}
	auto island = m_IslandList[0];
	replaceToLift(island);
	//constraint
	if (doc.HasMember("constraintList"))
	{
		auto& items = doc["constraintList"];
		for (unsigned int i = 0; i < items.Size(); i++)
		{
			auto& item = items[i];
			auto GUID = item["from"].GetString();
			auto fromAttach = reinterpret_cast<Attachment*>(GUIDMgr::shared()->get(GUID));
			placeBearingToAttach(fromAttach);
			if(item.HasMember("to"))
			{
				auto GUID = item["to"].GetString();
				auto toAttach = reinterpret_cast<Attachment*>(GUIDMgr::shared()->get(GUID));
				auto bearing = fromAttach->m_bearPart;
				fromAttach->m_parent->m_parent->addNeighbor(toAttach->m_parent->m_parent);
				toAttach->m_parent->m_parent->addNeighbor(fromAttach->m_parent->m_parent);
				toAttach->m_parent->attachToFromOtherIslandAlterSelfIsland(fromAttach, toAttach);
				bearing->m_a = toAttach;
			}
		}
	}

	printf("aaaaaaa");
}


void BuildingSystem::placeBearingByHit(vec3 pos, vec3 dir, float dist)
{
	std::vector<GamePart *> tmp;
	for (auto island : m_IslandList)
	{
		
		for (auto iter : island->m_partList)
		{
			tmp.push_back(iter);
		}
	}
	std::sort(tmp.begin(), tmp.end(), [&](GamePart * left, GamePart * right)
	{
		float distl = left->getNode()->getWorldPos().distance(pos);
		float distr = right->getNode()->getWorldPos().distance(pos);
		return distl < distr;
	}
	);
	for (auto iter : tmp)
	{
		auto node = iter->getNode();
		auto invertedMat = node->getTransform().inverted();
		auto dirInLocal = invertedMat * vec4(dir, 0.0);
		auto originInLocal = invertedMat * vec4(pos, 1.0);
		const auto r = Ray(originInLocal.toVec3(), dirInLocal.toVec3());
		vec3 a,b,c;
		auto attachment = iter->findProperAttachPoint(r, a, b, c);
		if (attachment && !attachment->m_bearPart)
		{
			//connect a BearPart
			auto bear = new BearPart();
			bear->m_b = attachment;
			attachment->m_bearPart = bear;
			m_bearList.insert(bear);

//			create a indicate model
			auto cylinderIndicator = new CylinderPrimitive(0.15, 0.15, 0.1);
			cylinderIndicator->setColor(vec4(1.0, 1.0, 0.0, 0.0));
			auto mat = attachment->getAttachmentInfoMat44();
			Quaternion q;
			q.fromRotationMatrix(&mat);
			cylinderIndicator->setPos(mat.getTranslation());
			cylinderIndicator->setRotateQ(q);
			cylinderIndicator->reCache();
			bear->m_node = cylinderIndicator;
			bear->updateFlipped();
			iter->getNode()->addChild(cylinderIndicator);

			break;
		}
	}
}

void BuildingSystem::flipBearingByHit(vec3 pos, vec3 dir, float dist)
{

	std::vector<GamePart *> tmp;
	//search island
	for (auto island : m_IslandList)
	{
		
		for (auto iter : island->m_partList)
		{
			tmp.push_back(iter);
		}
	}
	std::sort(tmp.begin(), tmp.end(), [&](GamePart * left, GamePart * right)
	{
		float distl = left->getNode()->getWorldPos().distance(pos);
		float distr = right->getNode()->getWorldPos().distance(pos);
		return distl < distr;
	}
	);
	for (auto iter : tmp)
	{
		auto island = iter->m_parent;
		auto node = iter->getNode();
		auto invertedMat = node->getTransform().inverted();
		vec4 dirInLocal = invertedMat * vec4(dir, 0.0);
		vec4 originInLocal = invertedMat * vec4(pos, 1.0);

		auto r = Ray(originInLocal.toVec3(), dirInLocal.toVec3());
		RayAABBSide side;
		vec3 hitPoint;
		auto isHit = r.intersectAABB(node->localAABB(), &side, hitPoint);
		GamePart * newPart = nullptr;
		if (isHit)
		{
			vec3 attachPos, attachNormal, attachUp;
			auto attach = iter->findProperAttachPoint(r, attachPos, attachNormal,attachUp);
			if(attach)
			{
				if(attach->m_bearPart)
				{
					attach->m_bearPart->m_isFlipped = ! attach->m_bearPart->m_isFlipped;
					attach->m_bearPart->updateFlipped();
				}
			}
			return;
		}
	}
}

void BuildingSystem::placeItem(GameItem * item, vec3 pos, vec3 dir, float dist)
{
	if (item->m_type >= 0)
	{
		placePartByHit(pos, dir, dist, item->m_type);
	}
	else
	{
		placeBearingByHit(pos, dir, dist);
	}
}


void BuildingSystem::dropFromLift()
{
	//each island, we create a rigid
	for (auto island : m_IslandList)
	{
		island->enablePhysics(true);
	}

	for (auto bear : m_bearList)
	{
		bear->enablePhysics(true);
	}
	if(m_liftPart) 
	{
		//m_liftPart->getNode()->removeFromParent();
		//m_liftPart->m_parent->remove(m_liftPart);
	}

}

void BuildingSystem::replaceToLiftByRay(vec3 pos, vec3 dir, float dist)
{
	//disable physics and put them back to lift position
	for (auto island : m_IslandList)
	{
		island->enablePhysics(false);
	}
	for (auto bear : m_bearList)
	{
		bear->enablePhysics(false);
	}

	//put them back to the lift

	auto island = rayTestIsland(pos, dir, dist);
	replaceToLift(island);
}

void BuildingSystem::replaceToLift(Island* island)
{
	if(island) 
	{
		vec3 attachPos, n, up;
		auto attach = m_liftPart->getFirstAttachment();
		attach->getAttachmentInfoWorld(attachPos, n, up);

		island->m_partList[0]->attachToFromOtherIslandAlterSelfIsland(attach, island->m_partList[0]->getBottomAttachment());

		m_liftPart->setEffectedIsland(island);

		for(auto part : island->m_partList) 
		{
			int count = part->getAttachmentCount();
			for(int i = 0; i < count; i++)
			{
				auto attach = part->getAttachment(i);
				if(attach->m_bearPart)
				{
					Attachment * other = nullptr;
					if(attach->m_bearPart->m_a == attach)
					{
						other = attach->m_bearPart->m_b;
					}else
					{
						other = attach->m_bearPart->m_a;
					}
					other->m_parent->attachToFromOtherIslandAlterSelfIsland(attach, other);
				}
			}

		}
	}
}

//toDo
void BuildingSystem::findPiovtAndAxis(Attachment * attach, vec3 hingeDir,  vec3 & pivot, vec3 & asix)
{
	auto part = static_cast<BlockPart * >(attach->m_parent);
	auto island = part->m_parent;
	auto islandInvertedMatrix = island->m_node->getLocalTransform().inverted();
	
	auto transform = part->getNode()->getLocalTransform();
	auto normalInIsland = transform.transofrmVec4(vec4(attach->m_normal, 0.0)).toVec3();

	pivot = transform.transofrmVec4(vec4(attach->m_pos + attach->m_normal * 0.05, 1.0)).toVec3();
	asix = islandInvertedMatrix.transofrmVec4(vec4(hingeDir, 0.0)).toVec3();
}

void BuildingSystem::tmpMoveWheel(bool isOpen)
{
	for(auto constrain : m_constrainList)
	{
		printf("move Wheel\n");
		constrain->enableAngularMotor(isOpen, 10, 100);
	}
}

}
