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
             * 获取lane group关联的道路
             * @param mapDataManager
             * @param lane_group
             * @return 不存在返回null
             */
            static shared_ptr<DCRoad> get_road_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                     const string &lane_group_id);

            /**
             * 获取lane group关联的道路，可能是多条道路
             * @param mapDataManager
             * @param lane_group_id
             * @return
             */
            static set<string> get_roads_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                              const string &lane_group_id);

            /**
             * 获取lane group
             * @param mapDataManager
             * @param lane_group_id
             * @return 不存在返回null
             */
            static shared_ptr<DCLaneGroup> get_lane_group(const shared_ptr<MapDataManager> &mapDataManager,
                                                          const string &lane_group_id);

            /**
             * 获取lane group
             * @param mapDataManager
             * @param divider_id
             * @return
             */
            static set<string> get_lane_groups_by_divider(shared_ptr<MapDataManager> mapDataManager, string divider_id);

            /**
             * 获取divider
             * @param mapDataManager
             * @param divider
             * @return 不存在返回null
             */
            static shared_ptr<DCDivider> get_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                     const string &divider);

            /**
             * 获取连接的divider
             * @param mapDataManager
             * @param divider
             * @param is_front 是否是当前首点连接
             * @return
             */
            static set<string> get_conn_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCDivider> &ptr_divider, bool is_front);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_COMMONUTIL_H
