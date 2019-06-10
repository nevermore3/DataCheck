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
            check_road_node_height(mapDataManager, errorOutput);
            check_road_node(mapDataManager, errorOutput);
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

        void RoadCheck::check_road_node_height(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            double heightchange = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER);

            const auto &ptr_roads_map = mapDataManager->roads_;
            for (const auto &road : ptr_roads_map) {
                auto ptr_road = road.second;
                if (!ptr_road->valid_) {
                    continue;
                }
                size_t nodeCount = ptr_road->nodes_.size();
                if (nodeCount < 2)
                    continue;

                vector<NodeCheck> error_index_pair;
                bool check = false;

                for (int i = 1; i < nodeCount; i++) {

                    auto node1 = ptr_road->nodes_[i - 1];
                    auto node2 = ptr_road->nodes_[i];

                    //间距判断
                    double distance = kd::automap::KDGeoUtil::distanceLL(node1->lng_, node1->lat_,
                                                                         node2->lng_, node2->lat_);

                    //坡度判断
                    double slopLimit = distance * heightchange;
                    double realDeltaZ = node1->z_ - node2->z_;
                    if (fabs(realDeltaZ) > slopLimit) {
                        check = true;
                        NodeCheck node_check{};
                        node_check.pre_index = i - 1;
                        node_check.index = i;
                        node_check.diff_height = realDeltaZ;
                        node_check.distance = distance;
                        error_index_pair.emplace_back(node_check);
                    }
                }
                if (check) {
                    shared_ptr<DCRoadCheckError> error =
                            DCRoadCheckError::createByKXS_04_003(ptr_road->id_, error_index_pair);
                    errorOutput->saveError(error);
                }
            }
        }

        void RoadCheck::check_road_node(shared_ptr<MapDataManager> mapDataManager,
                                        shared_ptr<CheckErrorOutput> errorOutput) {
            for (auto road_iter : mapDataManager->roads_) {
                auto ptr_road = road_iter.second;
                if (ptr_road) {
                    // 检查结点是否重复
                    check_road_node_repeat(errorOutput, ptr_road);

                    // 检查结点间角度是否过大出现拐点
                    check_road_node_angle(errorOutput, ptr_road);

                    // 结点间距检查
                    check_road_node_distance(errorOutput, ptr_road);
                }
            }
        }

        void RoadCheck::check_road_node_repeat(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road) {
            vector<shared_ptr<NodeError>> ptr_error_nodes;
            auto first_node = ptr_road->nodes_.front();
            shared_ptr<NodeError> ptr_e_node = make_shared<NodeError>();
            ptr_e_node->index = 0;
            ptr_e_node->ptr_coord = first_node;
            ptr_error_nodes.emplace_back(ptr_e_node);
            for (int i = 1; i < ptr_road->nodes_.size(); i++) {
                if (first_node->lng_ == ptr_road->nodes_.at(i)->lng_ &&
                    first_node->lat_ == ptr_road->nodes_.at(i)->lat_) {
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = ptr_road->nodes_.at(i);
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                } else {
                    if (ptr_error_nodes.size() > 1) {
                        auto ptr_error = DCRoadCheckError::createByKXS_04_006(ptr_road->id_, ptr_error_nodes);
                        errorOutput->saveError(ptr_error);
                    }
                    ptr_error_nodes.clear();
                    first_node = ptr_road->nodes_.at(i);
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = first_node;
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                }
            }

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCRoadCheckError::createByKXS_04_006(ptr_road->id_, ptr_error_nodes);
                errorOutput->saveError(ptr_error);
            }
            ptr_error_nodes.clear();
        }

        void RoadCheck::check_road_node_angle(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road) {
            if (ptr_road->nodes_.size() > 2) {
                shared_ptr<DCCoord> ptr_coord1 = ptr_road->nodes_[0];
                shared_ptr<DCCoord> ptr_coord2 = ptr_road->nodes_[1];

                vector<shared_ptr<NodeError>> check_road_indexs;
                for (int i = 2; i < ptr_road->nodes_.size(); i++) {
                    shared_ptr<DCCoord> ptr_coord3 = ptr_road->nodes_[i];
                    if (!CommonUtil::CheckCoordAngle(ptr_coord1, ptr_coord2, ptr_coord3)) {
                        shared_ptr<NodeError> ptr_error_node = make_shared<NodeError>();
                        ptr_error_node->ptr_coord = ptr_coord2;
                        ptr_error_node->index = i - 1;
                        check_road_indexs.emplace_back(ptr_error_node);
                    }

                    ptr_coord1 = ptr_coord2;
                    ptr_coord2 = ptr_coord3;
                }

                if (check_road_indexs.size() > 1) {
                    auto ptr_error = DCRoadCheckError::createByKXS_04_007(ptr_road->id_, check_road_indexs);
                    errorOutput->saveError(ptr_error);
                }
            }
        }

        void RoadCheck::check_road_node_distance(shared_ptr<CheckErrorOutput> errorOutput,
                                                 shared_ptr<DCRoad> ptr_road) {
            double road_node_dis = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::ROAD_NODE_DISTANCE);
            vector<shared_ptr<NodeError>> ptr_error_nodes;
            auto first_node = ptr_road->nodes_.front();
            shared_ptr<NodeError> ptr_e_node = make_shared<NodeError>();
            ptr_e_node->index = 0;
            ptr_e_node->ptr_coord = first_node;
            ptr_error_nodes.emplace_back(ptr_e_node);
            for (int i = 1; i < ptr_road->nodes_.size(); i++) {
                double node_dis = GeosObjUtil::get_length_of_node(first_node, ptr_road->nodes_.at(i));
                if (node_dis > 0 && node_dis < road_node_dis) {
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = ptr_road->nodes_.at(i);
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                } else {
                    if (ptr_error_nodes.size() > 1) {
                        auto ptr_error = DCRoadCheckError::createByKXS_04_008(ptr_road->id_, ptr_error_nodes);
                        errorOutput->saveError(ptr_error);
                    }
                    ptr_error_nodes.clear();
                    first_node = ptr_road->nodes_.at(i);
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = first_node;
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                }
            }

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCRoadCheckError::createByKXS_04_008(ptr_road->id_, ptr_error_nodes);
                errorOutput->saveError(ptr_error);
            }
            ptr_error_nodes.clear();
        }
    }
}