#ifndef TZW_NODE_H
#define TZW_NODE_H
#include "../Math/vec3.h"
#include "../Math/vec2.h"
#include <QMatrix4x4>
#include <QQuaternion>
#include <string>
#include <vector>
#include "External/TUtility/TUtility.h"
#include "../Scene/OctreeScene.h"
#include "../Action/ActionMgr.h"
namespace tzw {
/**
 * @brief The Node class 该类定义了一个节点类，用于控制变换
 *
 * Node支持层次结构，可以使用addChild将其它的Node加入成自己的子节点，子节点的变换参照： 父节点的变换矩阵* 子节点的变换矩阵
 * 大多数在引擎中能渲染的的实体都会继承该类
 */
class Scene;
class Node : public ActionMgr
{
public:
    enum class NodeType
    {
        NormalNode,
        Drawable3D,
        Drawable,
    };

    Node();
    virtual ~Node();
    static Node * create();
    QMatrix4x4 getTransform();
    QMatrix4x4 getLocalTransform();
    vec3 getPos() const;
    void setPos(const vec3 &pos);
    virtual void draw();
    virtual void update(float dt);
    vec3 getRotate() const;
    void setRotateE(const vec3 &rotate);
    void setRotateE(float x,float y,float z);
    vec3 getScale() const;
    void setScale(const vec3 &scale);
    void setScale(float x,float y,float z);
    QMatrix4x4 getTranslationMatrix();
    QMatrix4x4 getRotationMatrix();
    QMatrix4x4 getScalingMatrix();
    void visitPost(OctreeScene *scene);
    void addChild(Node * node);
    Node *getParent() const;
    std::string getName() const;
    Node *getChildByName(const std::string &name);
    void setName(const std::string &name);
    void setPos2D(float x,float y);
    void setPos2D(vec2 pos);
    vec2 getPos2D();
    vec3 getWorldPos();
    vec2 getWorldPos2D();
    bool getNeedToUpdate() const;
    void setNeedToUpdate(bool needToUpdate);
    virtual NodeType getNodeType();
    bool getIsDrawable() const;
    void setIsDrawable(bool isDrawable);
    void purgeAllChildren();
    bool getIsAccpectOCTtree() const;
    void setIsAccpectOCTtree(bool isAccpectOCTtree);
    void cacheTransform();
    virtual void visit();
    virtual void reCache();
    bool getIsValid() const;
    void setIsValid(bool isValid);
    void detachChild(Node * node);
    void removeFromParent();
    void setParent(Node *parent);
    vec3 getForward();
    int getPiority() const;
    void setPiority(int zOrder);
    void sortChildren();
    Scene *getScene() const;
    void setScene(Scene *scene);
    QQuaternion getRotateQ() const;
    void setRotateQ(const QQuaternion &rotateQ);
    unsigned int getGlobalPiority() const;
    void setGlobalPiority(unsigned int globalPiority);
protected:
    bool m_isDrawable;
    std::vector<Node *> m_children;
    bool m_isValid;
    Scene* m_scene;
    vec3 m_scale;
    QQuaternion m_rotateQ;
    vec3 m_pos;
    bool m_needToUpdate;
    QMatrix4x4 m_worldTransformCache;
    int m_piority;
    unsigned int m_globalPiority;
    bool m_isAccpectOCTtree;
    std::string m_name;
    Node * m_parent;
private:
    void removeAllChildren_r();
};

} // namespace tzw

#endif // TZW_NODE_H
