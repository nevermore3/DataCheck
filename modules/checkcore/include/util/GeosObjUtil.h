//
// Created by zhangxingang on 19-2-18.
//

#ifndef AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
#define AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
#include <data/DataManager.h>
#include "data/DividerGeomModel.h"
#include "CommonInclude.h"

using namespace geos::geom;

namespace kd {
    namespace dc {
        class GeosObjUtil {
        public :
            static shared_ptr<geos::geom::Coordinate> CreateCoordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                       char *zone0, bool is_z = false);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
