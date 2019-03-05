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
            static shared_ptr<geos::geom::Coordinate> create_coordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                       char *zone0, bool is_z = false);

            static shared_ptr<geos::geom::Coordinate> create_coordinate(const shared_ptr<DCCoord> &ptr_node,
                                                                       char *zone0, bool is_z = false);

            static shared_ptr<DCCoord> get_coord(const shared_ptr<geos::geom::Coordinate> &ptr_coord,
                                                                 char *zone0, bool is_z = false);

            /**
             * 获取点集合长度
             * @param ptr_coords
             * @return
             */
            static double get_length_of_coords(const vector<shared_ptr<DCCoord>> &ptr_coords);

            /**
             * 获取点集合长度
             * @param ptr_coords
             * @return
             */
            static double get_length_of_coords(const vector<shared_ptr<geos::geom::Coordinate>> &ptr_coords);

            /**
             * 判断是否是相同点
             * @param coord1
             * @param coord2
             * @return
             */
            static bool is_same_coord(Coordinate coord1, Coordinate coord2, double precise = 0.0001);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
