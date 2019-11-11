#include "CubePlayer.h"
#include "EngineSrc/Scene/SceneMgr.h"
#include "EngineSrc/3D/Model/Model.h"
#include "GameConfig.h"
#include "GameWorld.h"
#include "EngineSrc/Event/EventMgr.h"
#include <iostream>
#include "3D/Primitive/CubePrimitive.h"
#include "3D/Primitive/SpherePrimitive.h"
#include "3D/Sky.h"
#include "Projectile.h"

#include "EngineSrc/Collision/PhysicsMgr.h"
#include "BuildingSystem.h"
#include "ItemMgr.h"
#include "AssistDrawSystem.h"
#include "3D/Particle/ParticleEmitter.h"
#include "3D/Particle/ParticleInitPosModule.h"
#include "3D/Particle/ParticleInitSizeModule.h"
#include "3D/Particle/ParticleInitVelocityModule.h"
#include "3D/Particle/ParticleInitAlphaModule.h"
#include "3D/Particle/ParticleUpdateAlphaModule.h"
#include "3D/Particle/ParticleInitLifeSpanModule.h"
#include "3D/Particle/ParticleUpdateSizeModule.h"
#include "3D/Particle/ParticleUpdateColorModule.h"
#include "Shader/ShaderMgr.h"

namespace tzw
{

	CubePlayer::CubePlayer(Node* mainRoot):m_previewAngle(0)
	{
		ItemMgr::shared();
		initSlots();
		m_previewGamePart = nullptr;
		m_paintGun = new PaintGun();
		m_paintGun->color = vec3(1, 1, 1);
		m_currMode = Mode::MODE_DEFORM_SPHERE;
		GUISystem::shared()->addObject(this);
		FPSCamera* camera = FPSCamera::create(g_GetCurrScene()->defaultCamera());
		camera->setCamPos(vec3(5, 30.0, -5));
		mainRoot->addChild(camera);
		g_GetCurrScene()->setDefaultCamera(camera);
		m_camera = camera;
		camera->reCache();
		m_camera->setIsEnableGravity(true);
		m_currSelectItemIndex = 0;


		m_orbitcamera = OrbitCamera::create(g_GetCurrScene()->defaultCamera());
		mainRoot->addChild(m_orbitcamera);

		auto pos = getPos();
		oldPosX = pos.x / ((MAX_BLOCK + 1) * BLOCK_SIZE);
		oldPosZ = (-1.0f * pos.z) / ((MAX_BLOCK + 1) * BLOCK_SIZE);
		Sky::shared()->setCamera(m_camera);
		EventMgr::shared()->addFixedPiorityListener(this);

		m_enableGravity = true;
		m_currPointPart = nullptr;
		m_previewIsland = new Island(vec3(0, 0, 0));
	}

	FPSCamera* CubePlayer::camera() const
	{
		return m_camera;
	}

	void CubePlayer::setCamera(FPSCamera* camera)
	{
		m_camera = camera;
	}

	vec3 CubePlayer::getPos()
	{
		return m_camera->getPos();
	}

	void CubePlayer::setPos(vec3 newPos)
	{
		m_camera->setPos(newPos);
	}

	void CubePlayer::logicUpdate(float dt)
	{
		static float theTime = 0.0f;
		//vec3 oldPos = m_gunModel->getPos();
		float offset = 0.002;
		float freq = 1;
		if (m_camera->getIsMoving())
		{
			offset = 0.006;
			freq = 6;
		}
		theTime += freq * dt;
		// m_gunModel->setPos(vec3(oldPos.x, -0.2 + sinf(theTime) * offset, oldPos.z));
		if (checkIsNeedUpdateChunk())
		{
			GameWorld::shared()->loadChunksAroundPlayer();
		}

		GamePart * part = nullptr;
		if(BuildingSystem::shared()->isIsInXRayMode())
		{
			part = BuildingSystem::shared()->rayTestPartXRay(getPos(), m_camera->getTransform().forward(), 10.0);
		}else
		{
			part = BuildingSystem::shared()->rayTestPart(getPos(), m_camera->getTransform().forward(), 10.0);
		}
		
		if(part)
		{

			if(m_previewGamePart)
			{
				vec3 p,n,u;
				auto attach = part->findProperAttachPoint(Ray(getPos(), m_camera->getTransform().forward()),p,n,u);
				if(attach)
				{
					m_previewGamePart->getNode()->setIsVisible(true);
					m_previewGamePart->adjustToOtherIslandByAlterSelfPart(attach, m_previewGamePart->getFirstAttachment(), m_previewAngle);
				}else
				{
					m_previewGamePart->getNode()->setIsVisible(false);
				}
				
			}
			if(m_currPointPart != part)
			{
				//part->highLight();
				//if(m_currPointPart)
				//{
				//	m_currPointPart->unhighLight();
				//}
				m_currPointPart = part;
				updateCrossHairTipsInfo();
			}
		}else
		{
			if(m_previewGamePart)
			{
				m_previewGamePart->getNode()->setIsVisible(false);
            }
			if(m_currPointPart)
			{
				//m_currPointPart->unhighLight();
				m_currPointPart = nullptr;
				updateCrossHairTipsInfo();
			}
		}
	}

	bool CubePlayer::checkIsNeedUpdateChunk()
	{
		auto pos = getPos();
		int posX = pos.x / ((MAX_BLOCK + 1) * BLOCK_SIZE);
		int posZ = (-1.0f * pos.z) / ((MAX_BLOCK + 1) * BLOCK_SIZE);
		if (posX != oldPosX || posZ != oldPosZ)
		{
			oldPosX = posX;
			oldPosZ = posZ;
			return true;
		}
		return false;
	}
	static PhysicsRigidBody *wheelFrontLeft = nullptr;
	static PhysicsRigidBody *wheelFrontRight = nullptr;

	static PhysicsHingeConstraint * constraint1 = nullptr;
	static PhysicsHingeConstraint * constraint2 = nullptr;
	bool CubePlayer::onKeyPress(int keyCode)
	{
		switch (keyCode)
		{		
			case TZW_KEY_T:
			{	
			}
			break;
		default:
		  break;
		}
		return false;
	}

	bool CubePlayer::onKeyRelease(int keyCode)
	{
		//if (MainMenu::shared()->isVisible()) return false;
		switch (keyCode)
		{
	    case TZW_KEY_ESCAPE:
	        MainMenu::shared()->closeCurrentWindow();
	        break;
		case TZW_KEY_Q:
			{
				m_enableGravity = !m_enableGravity;
				m_camera->setIsEnableGravity(m_enableGravity);
			}
			break;
            case TZW_KEY_U:
			{
				MainMenu::shared()->setWindowShow(WindowType::PAINTER, true);
            }
			break;
		case TZW_KEY_T:
			{
				auto toggleXray = !BuildingSystem::shared()->isIsInXRayMode();
				BuildingSystem::shared()->setIsInXRayMode(toggleXray);
				AssistDrawSystem::shared()->setIsShowAssistInfo(toggleXray);
			}
			break;
		case TZW_KEY_J:
			{
				BuildingSystem::shared()->replaceToLiftByRay(getPos(), m_camera->getForward(), 15);
			}
			break;
		default:
			break;
		}
		return false;
	}

	bool CubePlayer::onMousePress(int button, vec2 thePos)
	{
		if (MainMenu::shared()->isVisible())
				return false;
		//switch(button) 
		//{
		//case 0://left mouse
		//	handleItemPrimaryUse(m_itemSlots[m_currSelectItemIndex]);
		//	break;
		//case 1://right mouse
		//	handleItemSecondaryUse(m_itemSlots[m_currSelectItemIndex]);
		//	break;
		//default: break;
		//}
		return true;
	}

	void CubePlayer::modeSwitch(Mode newMode)
	{
		m_currMode = newMode;
	}

	void CubePlayer::initSlots()
	{
		m_itemSlots.push_back(ItemMgr::shared()->getItem("Block"));
		m_itemSlots.push_back(ItemMgr::shared()->getItem("Wheel"));
		m_itemSlots.push_back(ItemMgr::shared()->getItem("Bearing"));
		m_itemSlots.push_back(ItemMgr::shared()->getItem("TerrainFormer"));
	}

	vec3 CubePlayer::getForward() const
	{
		return m_camera->getForward();
	}

	void CubePlayer::attachCamToGamePart(GamePart * part)
	{
		m_camera->removeFromParent();
		m_camera->setEnableFPSFeature(false);
		//m_camera->setIsEnableGravity(false);
		//part->getNode()->addChild(m_camera);
		//m_camera->setPos(0, 0, 0);
		//m_camera->setRotateE(0, 0, 0);
		//m_camera->reCache();

		m_orbitcamera->resetDirection();
		m_orbitcamera->setFocusNode(part->getNode());
		g_GetCurrScene()->setDefaultCamera(m_orbitcamera);
	}

	void CubePlayer::attachCamToWorld()
	{
		m_camera->setIsEnableGravity(true);
		m_camera->setEnableFPSFeature(true);
		GameWorld::shared()->getMainRoot()->addChild(m_camera);
		g_GetCurrScene()->setDefaultCamera(m_camera);
	}

	void CubePlayer::removePartByAttach(Attachment* attach)
	{
		BuildingSystem::shared()->removePartByAttach(attach);
		m_currPointPart = nullptr;
		updateCrossHairTipsInfo();
	}

	void CubePlayer::removePart(GamePart* part)
	{
		BuildingSystem::shared()->removePart(part);
		m_currPointPart = nullptr;
		updateCrossHairTipsInfo();
	}

	void CubePlayer::removeAllBlocks()
	{
		BuildingSystem::shared()->removeAll();
		m_currPointPart = nullptr;
	}

	void CubePlayer::updateCrossHairTipsInfo()
	{
		auto label = MainMenu::shared()->getCrossHairTipsInfo();
		if(!label) return;
		if(!m_currPointPart) 
		{
			label->setIsVisible(false);
			return;
        }
		label->setIsVisible(true);
		switch(m_currPointPart->getType())
		{
		case GamePartType::GAME_PART_LIFT: label->setString(u8"(E) �ؾ������"); break;
		case GamePartType::GAME_PART_CONTROL: label->setString(u8"(E) ��ʻ\n(F) �ڵ�༭��");break;
		case GamePartType::GAME_PART_THRUSTER: label->setString(u8"(E) �������");break;
		case GamePartType::GAME_PART_CANNON: label->setString(u8"(E) �������");break;
		case GamePartType::GAME_PART_BEARING: label->setString(u8"(E) �������\n(F) ��������");break;
		case GamePartType::GAME_PART_SPRING: label->setString(u8"(E) �������");break;
		default:
			label->setIsVisible(false);
			break;
		}
	}

	void CubePlayer::openCurrentPartInspectMenu()
	{
		if(m_currPointPart && m_currPointPart->isNeedDrawInspect())
		{
			MainMenu::shared()->openInspectWindow(m_currPointPart);
		}
	}

	GamePart* CubePlayer::getCurrPointPart()
	{
		return m_currPointPart;
	}

	PaintGun* CubePlayer::getPaintGun()
	{
		return m_paintGun;
	}

	void CubePlayer::paint()
	{
		auto part = BuildingSystem::shared()->rayTestPart(getPos(), m_camera->getTransform().forward(), 10.0);
		part->getNode()->setColor(vec4(m_paintGun->color, 1.0));
	}

	void CubePlayer::setCurrSelected(std::string itemName)
	{
		if(itemName.empty()) return;
		if(itemName == "Lift") return;
		if(m_previewGamePart)
		{
			m_previewIsland->remove(m_previewGamePart);
			delete m_previewGamePart;
		}
		auto gamePart = new GamePart();
		gamePart->initFromItemName(itemName);
		m_previewIsland->insert(gamePart);
		m_previewGamePart = gamePart;
	    auto m_material = Material::createFromTemplate("ModelRimLight");
		//auto texture =  TextureMgr::shared()->getByPath("Texture/metalgrid3-ue/metalgrid3_basecolor.png");
		//m_material->setTex("DiffuseMap", texture);

		//auto metallicTexture =  TextureMgr::shared()->getByPath("Texture/metalgrid3-ue/metalgrid3_metallic.png");
		//m_material->setTex("MetallicMap", metallicTexture);

		//auto roughnessTexture =  TextureMgr::shared()->getByPath("Texture/metalgrid3-ue/metalgrid3_roughness.png");
		//m_material->setTex("RoughnessMap", roughnessTexture);

		//auto normalMapTexture =  TextureMgr::shared()->getByPath("Texture/metalgrid3-ue/metalgrid3_normal-dx.png");
		//m_material->setTex("NormalMap", normalMapTexture);
		m_previewGamePart->getNode()->setMaterial(m_material);
		m_previewIsland->m_node->addChild(m_previewGamePart->getNode());
		tlog("create preview part");
	}

	void CubePlayer::setPreviewAngle(float angle)
	{
		m_previewAngle = angle;
		tlog("%f", angle);
	}

	float CubePlayer::getPreviewAngle() const
	{
		return m_previewAngle;
	}
} // namespace tzw
