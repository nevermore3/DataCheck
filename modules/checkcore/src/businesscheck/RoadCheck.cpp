//
// Created by zhangxingang on 19-1-25.
//

#include <businesscheck/RoadCheck.h>
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>


namespace kd {
    namespace dc {
        string RoadCheck::getId() {
            return id;
        }

        bool RoadCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            check_road_divider_intersect(mapDataManager, errorOutput);
            return true;
        }

        void RoadCheck::check_road_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                                     shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &ptr_lane_groups = mapDataManager->laneGroups_;

            shared_ptr<DCError> ptr_error = nullptr;
            for (const auto &lg : ptr_lane_groups) {
                auto lane_group_id = lg.first;
                auto ptr_lane_group = lg.second;

                auto roads = CommonUtil::get_roads_by_lg(mapDataManager, lane_group_id);
                auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, lane_group_id);

                if (!ptr_dividers.empty()) {
                    auto left_ptr_divider = ptr_dividers.front();
                    auto right_ptr_divider = ptr_dividers.back();
                    for (const auto &road_id : roads) {
                        auto ptr_road = CommonUtil::get_road(mapDataManager, road_id);
                        if (ptr_road) {
                            if (ptr_road->direction_ == 2) {
                                // 最左最右DIVIDER检查交点
                                if (road_divider_intersect(mapDataManager, ptr_road, lane_group_id, left_ptr_divider) ||
                                    road_divider_intersect(mapDataManager, ptr_road, lane_group_id,
                                                           right_ptr_divider)) {
                                    ptr_error = DCRoadCheckError::createByKXS_04_002(ptr_road->id_, lane_group_id);
                                    errorOutput->saveError(ptr_error);
                                }
                            } else if (ptr_road->direction_ == 1) {
                                // 只检查最右交点
                                if (road_divider_intersect(mapDataManager, ptr_road, lane_group_id,
                                                           right_ptr_divider)) {
                                    ptr_error = DCRoadCheckError::createByKXS_04_002(ptr_road->id_, lane_group_id);
                                    errorOutput->saveError(ptr_error);
                                }
                            }
                        } else {
                            LOG(ERROR) << "get road failed! road id : " << road_id;
                        }
                    }
                }
            }
        }

        bool RoadCheck::road_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCRoad> &ptr_road,
                                               const string &lane_group_id,
                                               const shared_ptr<DCDivider> &ptr_divider) {
            bool ret = false;
            auto ptr_road_line_string = get_road_line_string(mapDataManager, ptr_road, lane_group_id);
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);


            if (ptr_road_line_string && ptr_div_line_string) {
                CoordinateSequence *coor_seq = nullptr;
                ret = kd::automap::KDGeoUtil::isLineCross(ptr_road_line_string.get(),
                                                          ptr_div_line_string.get(), &coor_seq, 1);
//                ret = ptr_road_line_string->intersects(ptr_div_line_string.get());


                if (ret) {
                    auto f_node = ptr_div_line_string->getCoordinates()->front();
                    auto t_node = ptr_div_line_string->getCoordinates()->back();
                    bool is_same = true;
                    for (size_t i = 0; i < coor_seq->size(); i++) {
                        if (!GeosObjUtil::is_same_coord(coor_seq->getAt(i), f_node) &&
                            !GeosObjUtil::is_same_coord(coor_seq->getAt(i), t_node)) {
                            is_same = false;
                            break;
                        }
                    }
                    if (is_same) {
                        ret = false;
                    }
                }
            }
            return ret;
        }

        shared_ptr<geos::geom::LineString>
        RoadCheck::get_road_line_string(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCRoad> &ptr_road,
                                        const string &lane_group_id) {

            string road_id = ptr_road->id_;
            const auto &road2_lane_group_index = mapDataManager->road2LaneGroup2NodeIdxs_;
            auto road_node_index_iter = road2_lane_group_index.find(road_id);

            vector<shared_ptr<DCCoord>> road_nodes;
            if (road_node_index_iter != road2_lane_group_index.end()) {
                const unordered_map<string, std::pair<long, long>> &lane_group_index = road_node_index_iter->second;
                auto node_index_iter = lane_group_index.find(lane_group_id);
                if (node_index_iter != lane_group_index.end()) {
                    auto index_pair = node_index_iter->second;
                    if (index_pair.first >= 0 && index_pair.first < ptr_road->nodes_.size() &&
                        index_pair.second >= 0 && index_pair.second < ptr_road->nodes_.size()) {
                        if (index_pair.first < index_pair.second) {
                            for (long idx = index_pair.first; idx <= index_pair.second; idx++) {
                                road_nodes.emplace_back(ptr_road->nodes_.at(idx));
                            }
                        } else if (index_pair.first > index_pair.second) {
                            for (long idx = index_pair.second; idx <= index_pair.first; idx++) {
                                road_nodes.emplace_back(ptr_road->nodes_.at(idx));
                            }
                        }
                    }
                }
            }

            return CommonUtil::get_line_string(road_nodes);
        }
    }
}