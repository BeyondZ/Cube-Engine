#include "GameWorld.h"
#include "GameMap.h"
#include "EngineSrc/Event/EventMgr.h"
#include "TUtility/TUtility.h"
#include "time.h"
namespace tzw {
GameWorld *GameWorld::m_instance = nullptr;
GameWorld *GameWorld::shared()
{
    if(!m_instance)
    {
        m_instance = new GameWorld();
    }
    return m_instance;
}


void GameWorld::createWorld(Scene *scene, int width, int height,float ratio)
{
    m_scene = scene;
    GameMap::shared()->init(ratio);
    //int block_width = width / MAX_BLOCK;
    //int block_height = height /MAX_BLOCK;
    m_width = width;
    m_height = height;
    for(int i = -1 * width/2;i<=width/2;i++)
    {
        for(int j=-1 * height/2;j<=height/2;j++)
        {
            for(int k= -1 *WORLD_HEIGHT/2; k<=WORLD_HEIGHT/2;k++)
            {
                auto chunkA = new Chunk(i,k,j);
                m_mainRoot->addChild(chunkA);
                m_chunkList.push_back(chunkA);
                m_chunkArray[i + width/2][j + height/2][k + WORLD_HEIGHT/2] = chunkA;
            }
        }
    }
    loadChunksAroundPlayer();
}

vec3 GameWorld::worldToGrid(vec3 world)
{
    return vec3((world.x+1) / BLOCK_SIZE, (world.y+1) / BLOCK_SIZE, (world.z+1) / BLOCK_SIZE);
}

vec3 GameWorld::gridToChunk(vec3 grid)
{
    return vec3(grid.x / MAX_BLOCK, grid.y / MAX_BLOCK, grid.z / MAX_BLOCK);
}

Block *GameWorld::getBlockByGrid(int x, int y, int z)
{
    vec3 chunkPos = gridToChunk(vec3(x,y,z));
    Chunk * chunk = getChunkByChunk(chunkPos.x,chunkPos.y,chunkPos.z);
    if(chunk)
    {
        return chunk->getBlock(x - int(chunkPos.x)*MAX_BLOCK,y-int(chunkPos.y)*MAX_BLOCK,z - int(chunkPos.z)*MAX_BLOCK);
    }
    return nullptr;
}

Player *GameWorld::getPlayer() const
{
    return m_player;
}

void GameWorld::setPlayer(Player *player)
{
    m_player = player;
}

Chunk *GameWorld::getOrCreateChunk(int x, int y, int z)
{
    auto chunk = getChunkByChunk(x,y,z);
    if(!chunk)
    {
        chunk = createChunk(x,y,z);
    }
    return chunk;
}

void GameWorld::onFrameUpdate(float delta)
{
    if(m_currentState != GameState::OnPlay)
        return;
    static float timer = 0;
    timer += delta;
    if (timer >= 5.0f)
    {

        //loadChunksAroundPlayer();
        timer = 0.0f;
    }
}

Chunk *GameWorld::createChunk(int x, int y, int z)
{
    auto chunkA = new Chunk(x,0,z);
    m_scene->addNode(chunkA);
    m_chunkList.push_back(chunkA);
    return chunkA;
}

void GameWorld::loadBlockSheet()
{
    m_blockSheet = new TextureAtlas("./Res/User/CubeGame/texture/dest/blocks.json");
    m_blockSheet->texture()->setFilter(Texture::FilterType::Nearest);
}

TextureAtlas *GameWorld::getBlockSheet() const
{
    return m_blockSheet;
}

void GameWorld::setBlockSheet(TextureAtlas *blockSheet)
{
    m_blockSheet = blockSheet;
}

void GameWorld::startGame()
{
    unloadGame();
    GameWorld::shared()->loadBlockSheet();
    crossHair = Sprite::create("./Res/User/CubeGame/texture/GUI/cross_hair.png");
    auto size = crossHair->getContentSize();
    crossHair->setPos2D(Engine::shared()->windowWidth()/2 - size.x/2,Engine::shared()->windowHeight()/2 - size.y/2);
    m_mainRoot->addChild(crossHair);
    GameMap::shared()->setMapType(GameMap::MapType::Noise);
    GameMap::shared()->setMaxHeight(10);
    auto player = new Player(m_mainRoot);
    GameWorld::shared()->setPlayer(player);
    GameWorld::shared()->createWorld(SceneMgr::shared()->currentScene(),10,10,0.05);
    Cube* skybox = Cube::create("./Res/User/CubeGame/texture/SkyBox/left.jpg","./Res/User/CubeGame/texture/SkyBox/right.jpg",
                 "./Res/User/CubeGame/texture/SkyBox/top.jpg","./Res/User/CubeGame/texture/SkyBox/bottom.jpg",
                 "./Res/User/CubeGame/texture/SkyBox/back.jpg","./Res/User/CubeGame/texture/SkyBox/front.jpg");
    skybox->setIsAccpectOCTtree(false);
    skybox->setScale(80,80,80);
    m_mainRoot->addChild(skybox);
    m_currentState = GameState::OnPlay;
}

bool tzw::GameWorld::onKeyPress(int keyCode)
{
    switch (keyCode) {

    case Qt::Key_Escape:
        this->toggleMainMenu();
        break;
    default:
        break;
    }
}

void GameWorld::loadChunksAroundPlayer()
{
    std::vector<Chunk*> m_tempArray = m_activedChunkList;
    auto pos = m_player->getPos();
    pos = gridToChunk(worldToGrid(pos));
    int posX = pos.x + m_width/2;
    int posY = pos.z + m_height/2;
    for(int i =posX -4;i<=posX + 4;i++)
    {
        for(int j =posY -4;j<=posY + 4;j++)
        {
            for (int k = -1 * WORLD_HEIGHT/2;k<= WORLD_HEIGHT/2;k++)
            {
                auto targetChunk = m_chunkArray[i][j][k + WORLD_HEIGHT/2];
                targetChunk->load();
                auto findResult = std::find(m_tempArray.begin(),m_tempArray.end(),targetChunk);
                if(findResult!= m_tempArray.end())
                {
                    m_tempArray.erase(findResult);
                }else
                {
                    m_activedChunkList.push_back(m_chunkArray[i][j][k + WORLD_HEIGHT/2]);
                }
            }
        }
    }
    for(Chunk* i:m_tempArray)
    {
        i->unload();
    }
}

Chunk *GameWorld::getChunkByChunk(int x, int y, int z)
{
    if(x + m_width/2 >=0 && x + m_width/2 <m_width && z + m_height/2 >=0 && z + m_height/2 <m_height
            && y + WORLD_HEIGHT/2 >=0 && y + WORLD_HEIGHT/2 < WORLD_HEIGHT)
    {
        return m_chunkArray[x + m_width/2][z + m_height/2][y + WORLD_HEIGHT/2];
    }
    else
    {
        return nullptr;
    }
}

GameWorld::GameWorld()
{
    EventMgr::shared()->addFixedPiorityListener(this);
    memset(m_chunkArray,CUBE_MAP_SIZE * CUBE_MAP_SIZE *WORLD_HEIGHT * sizeof(Chunk *),0);
    m_currentState = GameState::MainMenu;
    m_mainMenu = new MainMenu();
    SceneMgr::shared()->currentScene()->addNode(m_mainMenu);
    m_mainRoot = new Node();
    SceneMgr::shared()->currentScene()->addNode(m_mainRoot);
}

MainMenu *GameWorld::getMainMenu() const
{
    return m_mainMenu;
}

void GameWorld::toggleMainMenu()
{
    m_mainMenu->toggle();
}

void GameWorld::unloadGame()
{
    m_mainRoot->purgeAllChildren();
    m_activedChunkList.clear();
    m_chunkList.clear();
    memset(m_chunkArray,CUBE_MAP_SIZE * CUBE_MAP_SIZE *WORLD_HEIGHT * sizeof(Chunk *),0);
}

GameWorld::GameState GameWorld::getCurrentState() const
{
    return m_currentState;
}

void GameWorld::setCurrentState(const GameState &currentState)
{
    m_currentState = currentState;
}

Node *GameWorld::getMainRoot() const
{
    return m_mainRoot;
}

void GameWorld::setMainRoot(Node *mainRoot)
{
    m_mainRoot = mainRoot;
}

} // namespace tzw

