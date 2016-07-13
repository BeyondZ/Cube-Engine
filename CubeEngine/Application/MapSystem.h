#ifndef TZWS_MAPSYSTEM_H
#define TZWS_MAPSYSTEM_H
#include "../EngineSrc/Engine/EngineDef.h"

#include "MapCell.h"
#include <vector>
namespace tzwS {

class MapSystem
{
public:
    MapSystem();
    void init();
    void createMapFromData();

    unsigned int width() const;
    void setWidth(unsigned int width);

    unsigned int height() const;
    void setHeight(unsigned int height);
    void addCell(int x,int y,MapCell cell);
    void addSettlement(int x,int y,Settlement * settlement);
    void initGraphics();
    unsigned int cellGraphicsSize() const;
    void setCellGraphicsSize(unsigned int cellGraphicsSize);
    void update();
    Settlement * getSettlementByName(std::string theName);
private:
    MapCell * m_map;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_cellGraphicsSize;
    std::vector<Settlement *> m_settlementList;
    TZW_SINGLETON_DECL(MapSystem);
};

} // namespace tzwS

#endif // TZWS_MAPSYSTEM_H
