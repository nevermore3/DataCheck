//
// Created by zhangxingang on 19-2-18.
//

#include <util/GeosObjUtil.h>
#include <mvg/Coordinates.hpp>


namespace kd {
    namespace dc {
        shared_ptr<geos::geom::Coordinate> GeosObjUtil::CreateCoordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                         char *zone0, bool is_z) {
            shared_ptr<geos::geom::Coordinate> ret = nullptr;
            double X0, Y0;
            double x = ptr_node->coord_.lng_;
            double y = ptr_node->coord_.lat_;
            double z = ptr_node->coord_.z_;
            kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
            if (is_z) {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0);
            } else {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0, z);
            }
            return ret;
        }
    }
}
