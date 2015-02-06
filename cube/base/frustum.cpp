#include "frustum.h"
#include <QMatrix4x4>
#include "base/camera.h"
bool Frustum::initFrustum(Camera* camera)
{
    _initialized = true;
    createPlane(camera);
    return true;
}

bool Frustum::isOutOfFrustum(const AABB& aabb) const
{
    if (_initialized)
    {
        QVector3D point;

        int plane = _clipZ ? 6 : 4;
        for (int i = 0; i < plane; i++)
        {
            const QVector3D& normal = _plane[i].getNormal();
            point.setX (normal.x() < 0 ? aabb.max ().x (): aabb.min ().x ());
            point.setY( normal.y() < 0 ? aabb.max().y (): aabb.min ().y ());
            point.setZ (normal.z() < 0 ? aabb.max ().z (): aabb.min ().z ());

            if (_plane[i].getSide(point) == PointSide::FRONT_PLANE )
                return true;
        }
    }
    return false;
}

void Frustum::createPlane( Camera* camera)
{
     QMatrix4x4 mat = camera->getProjection () * camera->getViewMatrix ();
    //ref http://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/
    //extract frustum plane
     float * m =mat.data ();
    _plane[0].initPlane(-QVector3D(m[3] + m[0], m[7] + m[4], m[11] + m[8]), (m[15] + m[12]));//left
    _plane[1].initPlane(-QVector3D(m[3] - m[0], m[7] - m[4], m[11] - m[8]), (m[15] - m[12]));//right
    _plane[2].initPlane(-QVector3D(m[3] + m[1], m[7] + m[5], m[11] + m[9]), (m[15] + m[13]));//bottom
    _plane[3].initPlane(-QVector3D(m[3] - m[1], m[7] - m[5], m[11] - m[9]), (m[15] - m[13]));//top
    _plane[4].initPlane(-QVector3D(m[3] + m[2], m[7] + m[6], m[11] + m[10]), (m[15] + m[14]));//near
    _plane[5].initPlane(-QVector3D(m[3] - m[2], m[7] - m[6], m[11] - m[10]), (m[15] - m[14]));//far
}
