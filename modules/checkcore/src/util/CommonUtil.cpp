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
        CommonUtil::get_lane_group(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id) {
            const auto& laneGroups = mapDataManager->laneGroups_;
            shared_ptr<DCLaneGroup> ptr_lane_group = nullptr;
            auto lane_group_iter = laneGroups.find(lane_group_id);
            if (lane_group_iter != laneGroups.end()) {
                ptr_lane_group = lane_group_iter->second;
            }

            return ptr_lane_group;
        }

        shared_ptr<DCDivider> CommonUtil::get_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                      const string &divider) {
            const auto &dividers = mapDataManager->dividers_;
            shared_ptr<DCDivider> ptr_divider = nullptr;
            auto divider_iter = dividers.find(divider);
            if (divider_iter != dividers.end()) {
                ptr_divider = divider_iter->second;
            }

            return ptr_divider;
        }

        shared_ptr<DCRoad> CommonUtil::get_road_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                      const string &lane_group_id) {
            auto ptr_lane_group = get_lane_group(mapDataManager, lane_group_id);
            if (ptr_lane_group) {
                return ptr_lane_group->road_;
            }
            return nullptr;
        }

        set<string> CommonUtil::get_conn_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                 const shared_ptr<DCDivider> &ptr_divider, bool is_front) {
            set<string> ret_divider_ids;
            const auto &node_id2_dividers_maps_ = is_front ? mapDataManager->tnode_id2_dividers_maps_ :
                                                  mapDataManager->fnode_id2_dividers_maps_;
            string node_id = is_front ? ptr_divider->nodes_.front()->id_ : ptr_divider->nodes_.back()->id_;
            auto node_iter = node_id2_dividers_maps_.find(node_id);
            if (node_iter != node_id2_dividers_maps_.end()) {
                for (const auto &ptr_div : node_iter->second) {
                    ret_divider_ids.insert(ptr_div->id_);
                }
            }
            return ret_divider_ids;
        }

        set<string> CommonUtil::get_lane_groups_by_divider(shared_ptr<MapDataManager> mapDataManager,
                                                           string divider_id) {
            set<string> ret_lane_groups;
            const auto &divider2_lane_groups = mapDataManager->divider2_lane_groups_;
            auto lane_group_iter = divider2_lane_groups.find(divider_id);
            if (lane_group_iter != divider2_lane_groups.end()) {
                ret_lane_groups.insert(lane_group_iter->second.begin(), lane_group_iter->second.end());
            }

            return ret_lane_groups;
        }

        set<string>
        CommonUtil::get_roads_by_lg(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id) {
            set<string> ret_road_set;
            const auto &lane_group2_roads_maps = mapDataManager->lane_group2_roads_maps_;
            auto lane_group_iter = lane_group2_roads_maps.find(lane_group_id);
            if (lane_group_iter != lane_group2_roads_maps.end()) {
                ret_road_set.insert(lane_group_iter->second.begin(), lane_group_iter->second.end());
            }
            return ret_road_set;
        }
    }
}