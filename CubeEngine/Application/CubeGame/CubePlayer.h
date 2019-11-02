#ifndef TZW_PLAYER_H
#define TZW_PLAYER_H

#include "Game/FPSCamera.h"
#include "EngineSrc/3D/Model/Model.h"
#include "2D/GUISystem.h"
#include "Game/OrbitCamera.h"
namespace tzw
{
	struct Attachment;
	class GameItem;
	class GamePart;
	class CubePlayer : public Node, public EventListener, public IMGUIObject
	{
	public:
		enum class Mode
		{
			MODE_DEFORM_SPHERE,
			MODE_PLACE_CUBE,
			MODE_PLACE_SPHERE,
		};

		explicit CubePlayer(Node* mainRoot);
		FPSCamera* camera() const;
		void setCamera(FPSCamera* camera);
		vec3 getPos();
		void setPos(vec3 newPos);
		void logicUpdate(float dt) override;
		bool checkIsNeedUpdateChunk();
		bool onKeyPress(int keyCode) override;
		bool onKeyRelease(int keyCode) override;
		bool onMousePress(int button, vec2 pos) override;
		void modeSwitch(Mode newMode);
		void initSlots();
		void handleItemPrimaryUse(GameItem * item);
		void handleItemSecondaryUse(GameItem * item);
		vec3 getForward() const;
		void attachCamToGamePart(GamePart * part);
		void attachCamToWorld();
		void removePartByAttach(Attachment* attach);
		void setCurrPointPart(GamePart * part);
		void removeAllBlocks();
		void updateCrossHairTipsInfo();
	private:
		std::vector<GameItem * > m_itemSlots;
		Mode m_currMode;
		FPSCamera* m_camera;
		OrbitCamera * m_orbitcamera;
		Model* m_gunModel;
		int oldPosX;
		int oldPosZ;
		int m_currSelectItemIndex;
		bool m_enableGravity;
		float m_seatAngle = 0.0;
		GamePart * m_currPointPart;
	};
} // namespace tzw

#endif // TZW_PLAYER_H
