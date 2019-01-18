//
// Created by zhangxingang on 19-1-18.
//

#ifndef AUTOHDMAP_DATACHECK_COMMONUTIL_H
#define AUTOHDMAP_DATACHECK_COMMONUTIL_H

#include <data/DataManager.h>
#include "data/DividerGeomModel.h"
#include "CommonInclude.h"

namespace kd {
    namespace dc {
        class CommonUtil {
        public:
            /**
             * 获取road
             * @param mapDataManager
             * @param road_id
             * @return 不存在返回null
             */
            static shared_ptr<DCRoad> get_road(shared_ptr<MapDataManager> mapDataManager, string road_id);

            /**
             * 获取lane group
             * @param mapDataManager
             * @param lane_group_id
             * @return 不存在返回null
             */
            static shared_ptr<DCLaneGroup> get_lane_group(shared_ptr<MapDataManager> mapDataManager, string lane_group_id);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_COMMONUTIL_H
