#ifndef TZW_QTGLWIDGET_H
#define TZW_QTGLWIDGET_H


#include <time.h>
#include <string>
#include "../Engine/EngineDef.h"
#include "../Math/vec2.h"
namespace tzw {
class AbstractDevice
{
public:
    void keyPressEvent(std::string theCode);
    void keyReleaseEvent(std::string theCode);
    void mousePressEvent(int buttonCode, vec2 pos);
    void mouseReleaseEvent(int buttonCode, vec2 pos);
    void mouseMoveEvent(vec2 pos);
    void init(int width, int height);
    void resizeGL(int w, int h);
    void paintGL();
    clock_t m_oldTicks,m_nowTicks;
    bool m_isFirstFrame;
    TZW_SINGLETON_DECL(AbstractDevice)
};

} // namespace tzw

#endif // TZW_QTGLWIDGET_H
