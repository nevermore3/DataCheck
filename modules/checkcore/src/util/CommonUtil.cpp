//
// Created by zhangxingang on 19-1-18.
//

#include <util/CommonUtil.h>
#include <mvg/Coordinates.hpp>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/LineString.h>
#include "geos/geom/Point.h"
#include <geom/geo_util.h>

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


        set<string> CommonUtil::get_ref_conn_divider(const shared_ptr<MapDataManager> &mapDataManager, const string &lg,
                                                     const shared_ptr<DCDivider> &ptr_divider, bool is_front) {
            set<string> ret_divider_ids;
            const auto &node_id2_dividers_maps_ = mapDataManager->node_id2_dividers_maps_;
            string node_id = is_front ? ptr_divider->nodes_.front()->id_ : ptr_divider->nodes_.back()->id_;
            auto node_iter = node_id2_dividers_maps_.find(node_id);
            if (node_iter != node_id2_dividers_maps_.end()) {
                for (const auto &ptr_div : node_iter->second) {
                    if (ptr_div->id_ != ptr_divider->id_) {
                        // 过滤一个组内的
                        auto lane_groups = get_lane_groups_by_divider(mapDataManager, ptr_div->id_);
                        if (lane_groups.size() == 1 && *lane_groups.begin() == lg) {
                            continue;
                        }
                        ret_divider_ids.insert(ptr_div->id_);
                    }
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

        vector<shared_ptr<DCLane>>
        CommonUtil::get_lanes_by_lg(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id) {
            const auto &lane_groups = mapDataManager->laneGroups_;
            auto lane_group_iter = lane_groups.find(lane_group_id);
            if (lane_group_iter != lane_groups.end()) {
                return lane_group_iter->second->lanes_;
            } else {
                LOG(ERROR) << "get lane group failed! lane group id : " << lane_group_id;
            }
            return vector<shared_ptr<DCLane>>();
        }

        vector<shared_ptr<DCDivider>>
        CommonUtil::get_dividers_by_lg(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id) {
            vector<shared_ptr<DCDivider>> ret_ptr_dividers;
            set<string> tag_divider;
            const auto &ptr_lanes = get_lanes_by_lg(mapDataManager, lane_group_id);
            bool is_first = true;
            for (const auto &lane : ptr_lanes) {
                if (is_first) {
                    if (tag_divider.find(lane->leftDivider_->id_) == tag_divider.end()) {
                        ret_ptr_dividers.emplace_back(lane->leftDivider_);
                        tag_divider.insert(lane->leftDivider_->id_);
                    }
                    if (tag_divider.find(lane->rightDivider_->id_) == tag_divider.end()) {
                        ret_ptr_dividers.emplace_back(lane->rightDivider_);
                        tag_divider.insert(lane->rightDivider_->id_);
                    }
                    is_first = false;
                } else {
                    if (tag_divider.find(lane->rightDivider_->id_) == tag_divider.end()) {
                        ret_ptr_dividers.emplace_back(lane->rightDivider_);
                        tag_divider.insert(lane->rightDivider_->id_);
                    }
                }
            }
            return ret_ptr_dividers;
        }

        vector<shared_ptr<DCLane>>
        CommonUtil::get_lanes_between_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCDivider> &left_divider,
                                               const shared_ptr<DCDivider> &right_divider) {
            vector<shared_ptr<DCLane>> ret_ptr_lanes;
            if (left_divider->id_ != right_divider->id_ && left_divider->dividerNo_ < right_divider->dividerNo_) {
                string lane_group;
                if (is_same_lane_group(mapDataManager, left_divider, right_divider, lane_group)) {
                    const auto &ptr_lanes = get_lanes_by_lg(mapDataManager, lane_group);
                    for (const auto &lane : ptr_lanes) {
                        auto lane_left_divider = lane->leftDivider_;
                        auto lane_right_divider = lane->rightDivider_;

                        if (left_divider->dividerNo_ <= lane_left_divider->dividerNo_ &&
                            lane_right_divider->dividerNo_ <= right_divider->dividerNo_) {
                            ret_ptr_lanes.emplace_back(lane);
                        }
                    }

                }
            }

            return ret_ptr_lanes;
        }

        bool CommonUtil::is_same_lane_group(const shared_ptr<MapDataManager> &mapDataManager,
                                            const shared_ptr<DCDivider> &left_divider,
                                            const shared_ptr<DCDivider> &right_divider, string &lane_group) {
            bool ret = false;
            auto left_lane_groups = get_lane_groups_by_divider(mapDataManager, left_divider->id_);
            auto right_lane_groups = get_lane_groups_by_divider(mapDataManager, right_divider->id_);
            if (left_lane_groups.size() == 1 && right_lane_groups.size() == 1) {
                if (*left_lane_groups.begin() == *right_lane_groups.begin()) {
                    lane_group = *left_lane_groups.begin();
                    ret = true;
                }
            } else if (left_lane_groups.size() == 2 && right_lane_groups.size() == 1) {
                for (const auto &lg : left_lane_groups) {
                    if (lg == *right_lane_groups.begin()) {
                        lane_group = *right_lane_groups.begin();
                        ret = true;
                    }
                }
            }
            return ret;
        }

        double CommonUtil::get_length_of_coords(const vector<shared_ptr<DCCoord>> &ptr_coords) {
            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
            for(const auto &ptr_coord : ptr_coords){
                double X0, Y0;
                char zone0[8] = {0};
                double x = ptr_coord->lng_;
                double y = ptr_coord->lat_;
                double z = ptr_coord->z_;
                kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
                cl->add(geos::geom::Coordinate(X0, Y0, z));
            }
            if (cl->size() < 2) {
                return -1;
            }
            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
            geos::geom::LineString *linesString = gf->createLineString(cl);
            if (linesString) {
                double len = linesString->getLength();
                return len;
            }
            return -1;
        }

        double CommonUtil::get_length_between_divider_nodes(const shared_ptr<DCDividerNode> &divider_node1,
                                                            const shared_ptr<DCDividerNode> &divider_node2) {
            vector<shared_ptr<DCCoord>> ptr_coords;
            if (divider_node1 && divider_node2) {
                shared_ptr<DCCoord> ptr_coord1 = make_shared<DCCoord>(divider_node1->coord_);
                shared_ptr<DCCoord> ptr_coord2 = make_shared<DCCoord>(divider_node2->coord_);

                ptr_coords.emplace_back(ptr_coord1);
                ptr_coords.emplace_back(ptr_coord2);
                return get_length_of_coords(ptr_coords);
            }
            return -1;
        }
    }
}