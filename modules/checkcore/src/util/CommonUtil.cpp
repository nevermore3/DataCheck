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

        shared_ptr<DCDivider> CommonUtil::get_divider(shared_ptr<MapDataManager> mapDataManager, string divider) {
            const auto &dividers = mapDataManager->dividers_;
            shared_ptr<DCDivider> ptr_divider = nullptr;
            auto divider_iter = dividers.find(divider);
            if (divider_iter != dividers.end()) {
                ptr_divider = divider_iter->second;
            }

            return ptr_divider;
        }

        shared_ptr<DCRoad> CommonUtil::get_road_by_lg(shared_ptr<MapDataManager> mapDataManager, string lane_group_id) {
            auto ptr_lane_group = get_lane_group(mapDataManager, lane_group_id);
            if (ptr_lane_group) {
                return ptr_lane_group->road_;
            }
            return nullptr;
        }
    }
}