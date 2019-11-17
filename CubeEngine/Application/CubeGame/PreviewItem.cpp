#include "PreviewItem.h"
#include "CubeGame/PartSurfaceMgr.h"
#include "Texture/TextureMgr.h"
#include "GamePart.h"
#include "GameItem.h"
#include "LiftPart.h"
#include "BuildingSystem.h"
#include "Scene/SceneMgr.h"
namespace tzw
{
	PreviewItem::PreviewItem():m_sphere(nullptr), m_previewPart(nullptr),m_previewAngle(0), m_previewIsland(new Island(vec3(0, 0, 0)))
	{
	}

	GamePart* PreviewItem::getPreviewPart() const
	{
		return m_previewPart;
	}

	void PreviewItem::setPreviewPart(GamePart* const previewPart)
	{
		m_previewPart = previewPart;
	}

	Island* PreviewItem::getPreviewIsland() const
	{
		return m_previewIsland;
	}

	void PreviewItem::setPreviewIsland(Island* const previewIsland)
	{
		m_previewIsland = previewIsland;
	}

	float PreviewItem::getPreviewAngle() const
	{
		return m_previewAngle;
	}

	void PreviewItem::setPreviewAngle(const float previewAngle)
	{
		m_previewAngle = previewAngle;
	}

	void PreviewItem::setPreviewItem(GameItem* item)
	{
		if(m_previewPart)
		{
			m_previewIsland->remove(m_previewPart);
			delete m_previewPart;
			m_previewPart = nullptr;
		}
		if(m_sphere)
		{
			m_sphere->removeFromParent();
			delete m_sphere;
			m_sphere = nullptr;
		}
		if(item->m_name == "Lift")
		{
			auto lift = new LiftPart();
			m_previewPart = lift;
			auto m_material = Material::createFromTemplate("ModelRimLight");
			lift->setMaterial(m_material);
			//m_previewGamePart->setPos(wherePos);
		}
		else if(item->m_name == "Digger")
		{
			auto m_material = Material::createFromTemplate("ModelRimLight");
			m_sphere = new SpherePrimitive(1.0, 25);
			g_GetCurrScene()->addNode(m_sphere);
			m_sphere->setMaterial(m_material);
		}
		else 
		{

			auto gamePart = new GamePart();
			gamePart->initFromItem(item);
			m_previewIsland->insert(gamePart);
			m_previewPart = gamePart;
			auto m_material = Material::createFromTemplate("ModelRimLight");
			m_previewPart->getNode()->setMaterial(m_material);
			tlog("create preview part");
        }
	}

	void PreviewItem::handlePreview(GameItem* currSelected, vec3 camPos, vec3 playerForwardDir)
	{
		bool isNoSelected = !currSelected;
		bool isNoNeedPreviewItem = currSelected && (currSelected->m_type == GamePartType::SPECIAL_PART_PAINTER);
		bool isInXray = BuildingSystem::shared()->isIsInXRayMode(); //͸��ģʽ�²���ҪԤ����ֱ�ӷ���
		if(isNoSelected || isNoNeedPreviewItem || isInXray)
		{
			hidePreviewPart();
			return;
		}
		auto part = BuildingSystem::shared()->rayTestPart(camPos, playerForwardDir, 10.0);

		switch (currSelected->m_type)
		{
			case GamePartType::GAME_PART_LIFT:
		{
			//�Ѿ���������������ûʲô��˵�ģ�ֱ�Ӽ�������
			if(BuildingSystem::shared()->getLift()) 
			{
				hidePreviewPart();
				return;
			}
			//ָ��һ��������׼����������״̬
			if(part)
			{
				hidePreviewPart();
			}
			else //��ͨ״̬
			{
				auto resultPos = BuildingSystem::shared()->hitTerrain(camPos, playerForwardDir, 10);
				if (resultPos.y > -99999)
				{
					showPreviewPart();
					static_cast<LiftPart *>(m_previewPart)->setPos(resultPos);

					//���ɶ������״̬�������Ӹ������е�������һ��
					if(!BuildingSystem::shared()->getStoreIslandGroup().empty())
					{
						std::vector<Island * > storeIslandGroup;
						BuildingSystem::shared()->getIslandsByGroup(BuildingSystem::shared()->getStoreIslandGroup(), storeIslandGroup);
						BuildingSystem::shared()->tempPlace(storeIslandGroup[0], static_cast<LiftPart *>(m_previewPart));
						////readjust
						auto attach = BuildingSystem::shared()->replaceToLiftIslands(storeIslandGroup[0]->m_islandGroup);
						BuildingSystem::shared()->replaceToLift(attach->m_parent->m_parent, attach, static_cast<LiftPart *>(m_previewPart));
					}
				}
				else
				{
					hidePreviewPart();
				}
			}
            }
			break;
            case GamePartType::SPECIAL_PART_DIGGER:
			{
				auto resultPos = BuildingSystem::shared()->hitTerrain(camPos, playerForwardDir, 10);
				m_sphere->setPos(resultPos);
            }
			
			break;

			default:
				if(part)
				{
					if(m_previewPart)
					{
						vec3 p,n,u;
						auto attach = part->findProperAttachPoint(Ray(camPos, playerForwardDir),p,n,u);
						if(attach && !attach->m_connected && !currSelected->isSpecialFunctionItem())
						{
							showPreviewPart();
							m_previewPart->adjustToOtherIslandByAlterSelfPart(attach, m_previewPart->getFirstAttachment(), m_previewAngle);
						}else
						{
							hidePreviewPart();
						}
					}
				}
				else
				{
					hidePreviewPart();
				}
			break;
		}
		if(currSelected->m_type != GamePartType::GAME_PART_LIFT) // no lift preview part
		{

		}
		else //lift, lift is so fucking special
		{
			//�Ѿ���������������ûʲô��˵�ģ�ֱ�Ӽ�������
			if(BuildingSystem::shared()->getLift()) 
			{
				hidePreviewPart();
				return;
			}
			//ָ��һ��������׼����������״̬
			if(part)
			{
				hidePreviewPart();
			}
			else //��ͨ״̬
			{
				auto resultPos = BuildingSystem::shared()->hitTerrain(camPos, playerForwardDir, 10);
				if (resultPos.y > -99999)
				{
					showPreviewPart();
					static_cast<LiftPart *>(m_previewPart)->setPos(resultPos);

					//���ɶ������״̬�������Ӹ������е�������һ��
					if(!BuildingSystem::shared()->getStoreIslandGroup().empty())
					{
						std::vector<Island * > storeIslandGroup;
						BuildingSystem::shared()->getIslandsByGroup(BuildingSystem::shared()->getStoreIslandGroup(), storeIslandGroup);
						BuildingSystem::shared()->tempPlace(storeIslandGroup[0], static_cast<LiftPart *>(m_previewPart));
						////readjust
						auto attach = BuildingSystem::shared()->replaceToLiftIslands(storeIslandGroup[0]->m_islandGroup);
						BuildingSystem::shared()->replaceToLift(attach->m_parent->m_parent, attach, static_cast<LiftPart *>(m_previewPart));
					}
				}
				else
				{
					hidePreviewPart();
				}
			}
		}
	}

	void PreviewItem::hidePreviewPart()
	{
		if(!m_previewPart && !m_sphere) return;
		if(m_sphere)
		{
			m_sphere->setIsVisible(false);
		}
		if(m_previewPart) 
		{
			if(m_previewPart->getType() == GamePartType::GAME_PART_LIFT)
			{
				static_cast<LiftPart *>(m_previewPart)->setIsVisible(false);
			}else
			{
				m_previewPart->getNode()->setIsVisible(false);
			}
		}

	}

	void PreviewItem::showPreviewPart()
	{
		if(!m_previewPart) return;
		if(m_previewPart->getType() == GamePartType::GAME_PART_LIFT)
		{
			static_cast<LiftPart *>(m_previewPart)->setIsVisible(true);
		}else
		{
			m_previewPart->getNode()->setIsVisible(true);
		}
	}
}
