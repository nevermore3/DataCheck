//
// Created by zhangxingang on 19-1-18.
//

#include <util/CommonUtil.h>

#include "util/CommonUtil.h"

namespace kd {
    namespace dc {
        shared_ptr<DCRoad> CommonUtil::get_road(shared_ptr<MapDataManager> mapDataManager, string road_id) {
            const auto& roads = mapDataManager->roads_;
            shared_ptr<DCRoad> ptr_road = nullptr;
            auto road_iter = roads.find(road_id);
            if (road_iter != roads.end()) {
                ptr_road = road_iter->second;
            }

            return ptr_road;
        }

        shared_ptr<DCLaneGroup>
        CommonUtil::get_lane_group(shared_ptr<MapDataManager> mapDataManager, string lane_group_id) {
            const auto& laneGroups = mapDataManager->laneGroups_;
            shared_ptr<DCLaneGroup> ptr_lane_group = nullptr;
            auto lane_group_iter = laneGroups.find(lane_group_id);
            if (lane_group_iter != laneGroups.end()) {
                ptr_lane_group = lane_group_iter->second;
            }

            return ptr_lane_group;
        }
    }
}