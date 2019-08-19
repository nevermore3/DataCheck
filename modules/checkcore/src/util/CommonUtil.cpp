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
#include <geos/geom/Point.h>
#include <DataCheckConfig.h>
#include "util/GeosObjUtil.h"
#include "util/KDGeoUtil.hpp"

using namespace geos::geom;
using namespace kd::automap;

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


        set<shared_ptr<DCDivider>> CommonUtil::get_ref_conn_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                                    const string &lg,
                                                                    const shared_ptr<DCDivider> &ptr_divider,
                                                                    bool is_front) {
            set<shared_ptr<DCDivider>> ret_dividers;
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
                        ret_dividers.insert(ptr_div);
                    }
                }
            }
            return ret_dividers;
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
                                               const shared_ptr<DCDivider> &right_divider, bool same) {
            vector<shared_ptr<DCLane>> ret_ptr_lanes;
            string lane_group;
            if (is_same_lane_group(mapDataManager, left_divider, right_divider, lane_group)) {
                if (left_divider->id_ != right_divider->id_ && left_divider->dividerNo_ < right_divider->dividerNo_) {

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
            } else {
                if (!same) {
                    auto left_lane_group = get_lane_groups_by_divider(mapDataManager, left_divider->id_);
                    auto right_lane_group = get_lane_groups_by_divider(mapDataManager, right_divider->id_);
                    if (!left_lane_group.empty()) {
                        auto left_dividers = get_dividers_by_lg(mapDataManager, *left_lane_group.begin());
                        if (!left_dividers.empty()) {
                            auto left_lanes = get_lanes_between_dividers(mapDataManager, left_divider,
                                                                         left_dividers.at(left_dividers.size() - 1), true);
                            ret_ptr_lanes.insert(ret_ptr_lanes.end(), left_lanes.begin(), left_lanes.end());
                        }
                    }

                    if (!right_lane_group.empty()) {
                        auto right_dividers = get_dividers_by_lg(mapDataManager, *right_lane_group.begin());
                        if (!right_dividers.empty()) {
                            auto right_lanes = get_lanes_between_dividers(mapDataManager, right_dividers.at(0),
                                                                          right_divider, true);
                            ret_ptr_lanes.insert(ret_ptr_lanes.end(), right_lanes.begin(), right_lanes.end());
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
            } else {
                LOG(ERROR) << "is_same_lane_group error! lane_group : " << lane_group;
            }
            return ret;
        }

        double CommonUtil::get_length_between_divider_nodes(const shared_ptr<DCDividerNode> &divider_node1,
                                                            const shared_ptr<DCDividerNode> &divider_node2) {
            vector<shared_ptr<DCCoord>> ptr_coords;
            if (divider_node1 && divider_node2) {
                shared_ptr<DCCoord> ptr_coord1 = divider_node1->coord_;
                shared_ptr<DCCoord> ptr_coord2 = divider_node2->coord_;

                ptr_coords.emplace_back(ptr_coord1);
                ptr_coords.emplace_back(ptr_coord2);
                return GeosObjUtil::get_length_of_coords(ptr_coords);
            }
            return -1;
        }

        shared_ptr<geos::geom::LineString> CommonUtil::get_line_string(const vector<shared_ptr<DCCoord>> &nodes) {
            shared_ptr<geos::geom::LineString> ret_road_line_string = nullptr;

            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (const auto &node : nodes) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(node->y_, node->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->z_));
            }

            if (cl->size() >= 2) {
                const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
                geos::geom::LineString *lineString = gf->createLineString(cl);
                if (lineString) {
                    ret_road_line_string.reset(lineString);
                }
            }

            return ret_road_line_string;
        }

        shared_ptr<geos::geom::LineString>
        CommonUtil::get_divider_line_string(const vector<shared_ptr<DCDividerNode>> &nodes) {
            shared_ptr<geos::geom::LineString> ret_road_line_string = nullptr;

            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (const auto &node : nodes) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(node->coord_.lat_, node->coord_.lng_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->coord_->z_));
            }

            if (cl->size() >= 2) {
                const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
                geos::geom::LineString *lineString = gf->createLineString(cl);
                if (lineString) {
                    ret_road_line_string.reset(lineString);
                }
            }

            return ret_road_line_string;
        }

        bool CommonUtil::check_dividers_same_direction(const shared_ptr<DCDivider> &left_divider,
                                                       const shared_ptr<DCDivider> &right_divider) {
            //todo： 应该取更多的点进行检查
            shared_ptr<DCDividerNode> s_node = left_divider->nodes_.front();
            shared_ptr<DCDividerNode> t_node = left_divider->nodes_.back();

            shared_ptr<DCDividerNode> ref_s_node = right_divider->nodes_.front();
            shared_ptr<DCDividerNode> ref_t_node = right_divider->nodes_.back();

            bool sameDir = calLaneSameDir(s_node->coord_.x_, s_node->coord_.y_,
                                          t_node->coord_.x_, t_node->coord_.y_,
                                          ref_s_node->coord_.x_, ref_s_node->coord_.y_,
                                          ref_t_node->coord_.x_, ref_t_node->coord_.y_);

            return sameDir;
        }

        bool CommonUtil::calLaneSameDir(double firstNode1X, double firstNode1Y, double firstNode2X, double firstNode2Y,
                                        double secondNode1X, double secondNode1Y, double secondNode2X,
                                        double secondNode2Y, double angleLimit) {
            double thetaFirst = geo::geo_util::calcAngle(firstNode1X, firstNode1Y, firstNode2X, firstNode2Y);
            double thetaSencond = geo::geo_util::calcAngle(secondNode1X, secondNode1Y, secondNode2X, secondNode2Y);

            double angleDiff = fabs(thetaFirst - thetaSencond);
            if (angleDiff > PI) {
                angleDiff = 2 * PI - angleDiff;
            }
            double angle =  180/PI * angleDiff;
            bool isSameDir = true;
            if(angle > angleLimit){
                isSameDir = false;
            }
            return isSameDir;
        }

        double CommonUtil::get_min_distance_from_divider(const shared_ptr<DCDividerNode> &divider_node,
                                                         const shared_ptr<DCDivider> &divider) {
            char zone0[8] = {0};

            auto ptr_coordinate = GeosObjUtil::create_coordinate(divider_node, zone0);
            double PtA[2] = {0};
            PtA[0] = ptr_coordinate->x;
            PtA[1] = ptr_coordinate->y;
            auto *Line = new double[divider->nodes_.size() * 2];
            for (int i = 0; i < divider->nodes_.size(); i++) {
                auto ptr_tmp_coordinate = GeosObjUtil::create_coordinate(divider->nodes_[i], zone0);
                Line[i * 2] = ptr_tmp_coordinate->x;
                Line[i * 2 + 1] = ptr_tmp_coordinate->y;
            }
            double PtB[2], PtC[4];
            int index = 0;
            double dis = geo::geo_util::pt2LineDist(Line, static_cast<int>(divider->nodes_.size() * 2),
                                                    PtA, PtB, PtC, index);
            return dis;
        }

        long CommonUtil::GetMaxDividerNo(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group) {
            map<int, shared_ptr<DCDivider>> dividerNo2Divder;
            auto dividers = get_dividers_by_lg(mapDataManager, lane_group);
            long max_no = 0;
            for (const auto &div : dividers) {
                auto no = div->dividerNo_;
                if (no > max_no) {
                    max_no = no;
                }
            }
            return max_no;
        }

        shared_ptr<DCDividerNode> CommonUtil::get_distance_node(shared_ptr<DCDivider> ptr_divider,
                                                                double length, bool is_front) {
            shared_ptr<DCDividerNode> ret_ptr_node = nullptr;
            if (ptr_divider && !ptr_divider->nodes_.empty()) {
                vector<shared_ptr<DCCoord>> divider_node_vecs;
                if (is_front) {
                    ret_ptr_node = ptr_divider->nodes_.back();
                    divider_node_vecs.emplace_back(ptr_divider->nodes_.front()->coord_);
                    for (int i = 1; i < ptr_divider->nodes_.size(); i++) {
                        divider_node_vecs.emplace_back(ptr_divider->nodes_[i]->coord_);
                        double temp_length = GeosObjUtil::get_length_of_coords(divider_node_vecs);
                        if (temp_length > length) {
                            ret_ptr_node = ptr_divider->nodes_[i];
                            break;
                        }
                    }
                } else {
                    ret_ptr_node = ptr_divider->nodes_.front();
                    divider_node_vecs.emplace_back(ptr_divider->nodes_.back()->coord_);
                    for (int i = static_cast<int>(ptr_divider->nodes_.size() - 2); i >= 0; i--) {
                        divider_node_vecs.emplace_back(ptr_divider->nodes_[i]->coord_);
                        double temp_length = GeosObjUtil::get_length_of_coords(divider_node_vecs);
                        if (temp_length > length) {
                            ret_ptr_node = ptr_divider->nodes_[i];
                            break;
                        }
                    }
                }
            }

            return ret_ptr_node;
        }

        int CommonUtil::NodeOrentationOfDivider(shared_ptr<DCDividerNode> f_ptr_node,
                                                shared_ptr<DCDividerNode> t_ptr_node,
                                                shared_ptr<DCDividerNode> ptr_node) {
            if (f_ptr_node == nullptr || t_ptr_node == nullptr || ptr_node == nullptr) {
                return -2;
            }
            char zone[8] = {0};
            auto f_coord = GeosObjUtil::create_coordinate(f_ptr_node, zone);
            auto t_coord = GeosObjUtil::create_coordinate(t_ptr_node, zone);
            auto coord = GeosObjUtil::create_coordinate(ptr_node, zone);

            return KDGeoUtil::calPTOrentationOfLine(f_coord->x, f_coord->y, t_coord->x, t_coord->y, coord->x, coord->y);
        }

        bool CommonUtil::CheckCoordValid(DCCoord coord) {
            bool ret = true;
            if (__isnan(coord.y_) || __isnan(coord.x_) || __isnan(coord.z_)) {
                ret = false;
            }
            if (-180 > coord.x_ || coord.y_ > 180) {
                ret = false;
            }
            if (-90 > coord.y_ || coord.y_ > 90) {
                ret = false;
            }
            return ret;
        }

        bool CommonUtil::CheckCoordValid(shared_ptr<DCCoord> coord) {
            bool ret = true;
            if (__isnan(coord->y_) || __isnan(coord->x_) || __isnan(coord->z_)) {
                ret = false;
            }
            if (-180 > coord->x_ || coord->y_ > 180) {
                ret = false;
            }
            if (-90 > coord->y_ || coord->y_ > 90) {
                ret = false;
            }
            return ret;
        }

        bool CommonUtil::CheckCoordAngle(shared_ptr<DCCoord> ptr_coord1, shared_ptr<DCCoord> ptr_coord2,
                                         shared_ptr<DCCoord> ptr_coord3, double angle_threthold, double &angle) {
            char zone[8] = {0};
            auto ptr_utm_coord1 = GeosObjUtil::create_coordinate(ptr_coord1, zone);
            auto ptr_utm_coord2 = GeosObjUtil::create_coordinate(ptr_coord2, zone);
            auto ptr_utm_coord3 = GeosObjUtil::create_coordinate(ptr_coord3, zone);
            double diff_angle = KDGeoUtil::getAngleDiff(ptr_utm_coord1,
                                                        ptr_utm_coord2,
                                                        ptr_utm_coord2,
                                                        ptr_utm_coord3);


            if (fabs(angle_threthold) < 0.001) {
                angle_threthold = kd::automap::PI / 2;
            } else {
                angle_threthold = kd::automap::PI * angle_threthold / 180;
            }
            if (fabs(diff_angle) > angle_threthold) {
                angle = diff_angle;
                return false;
            }
            return true;
        }
    }
}