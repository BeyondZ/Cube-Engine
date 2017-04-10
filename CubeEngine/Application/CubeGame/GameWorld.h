#ifndef TZW_CHUNKMGR_H
#define TZW_CHUNKMGR_H

#include "EngineSrc/CubeEngine.h"
#include "./Chunk.h"
#include "CubePlayer.h"
#include "MainMenu.h"
#include "GameConfig.h"
#include <set>
namespace tzw {

class GameWorld : public EventListener
{
public:
    enum class GameState
    {
        MainMenu,
        OnPlay,
    };

    static GameWorld * shared();
    void createWorld(Scene* scene, int blockWitdh, int depth, int height, float ratio);
    vec3 worldToGrid(vec3 world);
    vec3 gridToChunk(vec3 grid);
    Chunk * getChunk(int x,int y,int z);
    CubePlayer *getPlayer() const;
    void setPlayer(CubePlayer *player);
    Chunk * getOrCreateChunk(int x,int y, int z);
    virtual void onFrameUpdate(float delta);
    Chunk * createChunk(int x,int y,int z);
    void setBlockSheet(TextureAtlas *blockSheet);
    bool isTheSurfaceBlock();
    void startGame();
    virtual bool onKeyPress(int keyCode);
    MainMenu *getMainMenu() const;
    void toggleMainMenu();
    void unloadGame();
    GameState getCurrentState() const;
    void setCurrentState(const GameState &currentState);

    Node *getMainRoot() const;
    void setMainRoot(Node *mainRoot);
	void loadChunksAroundPlayer();
	tzw::vec3 getMapOffset() const;
	void setMapOffset(tzw::vec3 val);
private:
    Node * m_mainRoot;
    GameState m_currentState;
    vec3 m_mapOffset;
    int m_width, m_depth, m_height;
    Chunk * m_chunkArray[128][128][16];
    Scene  * m_scene;
    CubePlayer * m_player;
    Sprite * crossHair;
    std::vector<Chunk*> m_chunkList;
    static GameWorld *m_instance;
    GameWorld();
    std::set<Chunk*> m_activedChunkList;
    MainMenu * m_mainMenu;
	virtual ~GameWorld();
};

} // namespace tzw

#endif // TZW_CHUNKMGR_H
