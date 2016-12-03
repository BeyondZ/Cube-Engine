#ifndef TZW_EVENTMGR_H
#define TZW_EVENTMGR_H
#include <vector>
#include <deque>
#include <string>
#include "../EngineSrc/Math/vec2.h"
#include "../Engine/EngineDef.h"
#include <unordered_map>
namespace tzw {
class Node;
struct EventInfo
{
    int type;
    int keycode;
    unsigned int theChar;
    int arg;
    vec2 pos;
};
class EventListener;
class EventMgr
{
public:
    void addListener(EventListener * listener);
    void addListener(EventListener * listener, Node * node);
    void addFixedPiorityListener(EventListener * event);
    void addNodePiorityListener(Node * node,EventListener * event);

    void handleKeyPress(int keyCode);
    void handleKeyRelease(int keyCode);
    void handleMousePress(int button,vec2 pos);
    void handleCharInput(unsigned int theChar);
    void handleMouseRelease(int button,vec2 pos);
    void handleMouseMove(vec2 pos);
    void apply(float delta);
    void removeEventListener(EventListener * event);
    void sortFixedListener();
	void sortNodePiorityListener();
	void notifyListenerChange();
private:
    void visitNode(Node * node);
    void applyKeyPress(EventInfo & info);
    void applyKeyCharInput(EventInfo & info);
    void applyKeyRelease(EventInfo & info);
    void applyMousePress(EventInfo & info);
    void applyMouseRelease(EventInfo & info);
    void applyMouseMove(EventInfo & info);
private:
    std::vector<EventListener *>m_list;
    std::deque<EventListener *>m_NodePioritylist;
    std::deque<EventInfo>m_eventDeque;
    std::unordered_map<Node *, EventListener *> m_nodeListenerMap;
	bool m_isNeedSortNodeListener;
	virtual ~EventMgr();
    TZW_SINGLETON_DECL(EventMgr)
};

} // namespace tzw

#endif // TZW_EVENTMGR_H
