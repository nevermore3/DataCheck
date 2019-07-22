//
// Created by zhangxingang on 19-1-25.
//

#include "businesscheck/LaneCheck.h"
#include <geos/geom/GeometryFactory.h>
#include <businesscheck/LaneCheck.h>
#include <util/GeosObjUtil.h>

#include "util/CommonUtil.h"
#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {
        string LaneCheck::getId() {
            return id;
        }

        bool LaneCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            check_lane_divider_intersect(mapDataManager, errorOutput);
            check_lane_lane_intersect(mapDataManager, errorOutput);
            return true;
        }

        void LaneCheck::check_lane_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                                     shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &ptr_lane_groups = mapDataManager->laneGroups_;
            char zone[8] = {0};
            shared_ptr<DCError> ptr_error = nullptr;
            int total=0;
            bool check_kxs05003 = CheckItemValid(CHECK_ITEM_KXS_LANE_003);
            bool check_kxs05002 = CheckItemValid(CHECK_ITEM_KXS_LANE_002);
            for (const auto &lg : ptr_lane_groups) {
                if (lg.second->is_virtual_) {
                    continue;
                }
                auto lane_group_id = lg.first;
                auto ptr_lane_group = lg.second;
                bool is_positive = lg.second->direction_ == 1;

                // 获取divider
                auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, lane_group_id);

                auto ptr_lanes = CommonUtil::get_lanes_by_lg(mapDataManager, lane_group_id);

                if (!ptr_dividers.empty()) {
                    auto left_ptr_divider = ptr_dividers.front();
                    auto right_ptr_divider = ptr_dividers.back();
                    for (const auto &ptr_lane : ptr_lanes) {
                        total++;
                        CoordinateSequence *intersections_left = nullptr;
                        if (check_kxs05003 && lane_divider_intersect(mapDataManager, ptr_lane, left_ptr_divider,intersections_left)) {
                            shared_ptr<DCCoord>  coord;
                            if(intersections_left->size()>0){
                                GeosObjUtil::create_coordinate(left_ptr_divider->nodes_.front()->coord_, zone);
                                shared_ptr<DCCoord>  coord  = GeosObjUtil::get_coord(make_shared<geos::geom::Coordinate>(intersections_left->front()),zone);
                                ptr_error->coord = coord;
                            }
                            ptr_error = DCLaneError::createByKXS_05_003(ptr_lane->task_id_,DATA_TYPE_LANE+ptr_lane->task_id_+DATA_TYPE_LAST_NUM
                                    , DATA_TYPE_NODE,MODEL_NAME_DIVIDER, coord,ptr_lane->id_, left_ptr_divider->id_);

                            errorOutput->saveError(ptr_error);
                        }
                        CoordinateSequence *intersections_right = nullptr;
                        if (check_kxs05003 && lane_divider_intersect(mapDataManager, ptr_lane, right_ptr_divider,intersections_right)) {
                            shared_ptr<DCCoord>  coord;
                            if(intersections_right->size()>0){
                                GeosObjUtil::create_coordinate(left_ptr_divider->nodes_.front()->coord_, zone);
                                coord  = GeosObjUtil::get_coord(make_shared<geos::geom::Coordinate>(intersections_right->front()),zone);

                            }
                            ptr_error = DCLaneError::createByKXS_05_003(ptr_lane->task_id_,DATA_TYPE_LANE+ptr_lane->task_id_+DATA_TYPE_LAST_NUM
                                    , DATA_TYPE_NODE,MODEL_NAME_DIVIDER, coord,ptr_lane->id_, right_ptr_divider->id_);

                            errorOutput->saveError(ptr_error);
                        }

                        bool is_depart_or_merge = false;
                        // depart or merge
                        if (ptr_lane->laneNo_ == 1 && !is_positive) {
                            if (ptr_lane->leftDivider_->fromNodeId_ == ptr_lane->rightDivider_->fromNodeId_ ||
                                ptr_lane->leftDivider_->toNodeId_ == ptr_lane->rightDivider_->fromNodeId_ ||
                                ptr_lane->leftDivider_->toNodeId_ == ptr_lane->rightDivider_->toNodeId_) {
                                is_depart_or_merge = true;
                            }
                        } else {
                            if (ptr_lane->leftDivider_->fromNodeId_ == ptr_lane->rightDivider_->fromNodeId_ ||
                                ptr_lane->leftDivider_->toNodeId_ == ptr_lane->rightDivider_->toNodeId_) {
                                is_depart_or_merge = true;
                            }
                        }

                        if (is_depart_or_merge) {
                            if (lane_divider_intersects(mapDataManager, ptr_lane, ptr_lane->leftDivider_)) {
                                ptr_error = DCLaneError::createByKXS_05_002(ptr_lane->id_, ptr_lane->leftDivider_->id_);

                                ptr_error->taskId_ = ptr_lane->task_id_;
                                ptr_error->flag = ptr_lane->flag_;
                                ptr_error->dataKey_ = DATA_TYPE_LANE+ptr_lane->task_id_+DATA_TYPE_LAST_NUM;

                                errorOutput->saveError(ptr_error);
                            }

                            if (lane_divider_intersects(mapDataManager, ptr_lane, ptr_lane->rightDivider_)) {
                                ptr_error = DCLaneError::createByKXS_05_002(ptr_lane->id_,
                                                                            ptr_lane->rightDivider_->id_);

                                ptr_error->taskId_ = ptr_lane->task_id_;
                                ptr_error->flag = ptr_lane->flag_;
                                ptr_error->dataKey_ = DATA_TYPE_LANE+ptr_lane->task_id_+DATA_TYPE_LAST_NUM;

                                errorOutput->saveError(ptr_error);
                            }
                        }


                    }
                }
            }
            if(check_kxs05003){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LANE_003,total);
            }
        }

        bool LaneCheck::lane_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCLane> &ptr_lane,
                                               const shared_ptr<DCDivider> &ptr_divider,CoordinateSequence *intersections) {
            bool ret = false;
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);
            auto ptr_lane_line_string = CommonUtil::get_line_string(ptr_lane->coords_);
            if (ptr_lane_line_string && ptr_div_line_string) {

                ret = KDGeoUtil::isLineCross(ptr_lane_line_string.get(), ptr_div_line_string.get(),&intersections);
            }
            return ret;
        }

        bool LaneCheck::lane_divider_intersects(const shared_ptr<MapDataManager> &mapDataManager,
                                                const shared_ptr<DCLane> &ptr_lane,
                                                const shared_ptr<DCDivider> &ptr_divider) {
            bool ret = false;
            double lane_intersect_length = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::LANE_INTERSECT_LENGTH);
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);
            auto ptr_lane_line_string = CommonUtil::get_line_string(ptr_lane->coords_);
            if (ptr_lane_line_string && ptr_div_line_string) {
                CoordinateSequence *intersections = nullptr;
                if (KDGeoUtil::isLineCross(ptr_lane_line_string.get(), ptr_div_line_string.get(), &intersections)) {

                    if (intersections->size() > 2) {
                        ret = true;
                    } else if (intersections->size() == 2) {
                        const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
                        geos::geom::LineString *lineString = gf->createLineString(intersections);

                        double intersect_length = lineString->getLength();
                        if (intersect_length < lane_intersect_length) {
                            ret = true;
                        }
                    }
                }
            }
            return ret;
        }

        void LaneCheck::check_lane_lane_intersect(shared_ptr<MapDataManager> mapDataManager,
                                                  shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &ptr_lane_groups = mapDataManager->laneGroups_;

            shared_ptr<DCError> ptr_error = nullptr;
            for (const auto &lg : ptr_lane_groups) {
                check_lane_node(mapDataManager, errorOutput, lg.second);
                auto ptr_lanes = lg.second->lanes_;
                if (ptr_lanes.size() > 1) {
                    auto ptr_left_lane = ptr_lanes.front();
                    vector<shared_ptr<DCLane>> ptr_same_node_lanes;
                    ptr_same_node_lanes.emplace_back(ptr_left_lane);
                    for (int i = 1; i < ptr_lanes.size(); i++) {
                        if (ptr_left_lane && ptr_lanes[i]) {
                            // 如果首点相同
                            if (GeosObjUtil::is_same_coord(ptr_left_lane->coords_.front(),
                                                           ptr_lanes[i]->coords_.front())) {
                                ptr_same_node_lanes.emplace_back(ptr_lanes[i]);
                            } else {
                                // check
                                lane_intersects(mapDataManager, errorOutput, ptr_same_node_lanes);
                                ptr_same_node_lanes.clear();
                                ptr_left_lane = ptr_lanes[i];
                                ptr_same_node_lanes.emplace_back(ptr_left_lane);
                            }
                        }
                    }

                    ptr_same_node_lanes.clear();
                    ptr_left_lane = ptr_lanes.front();
                    ptr_same_node_lanes.emplace_back(ptr_left_lane);
                    for (int i = 1; i < ptr_lanes.size(); i++) {
                        if (ptr_left_lane && ptr_lanes[i]) {
                            // 如果首点相同
                            if (GeosObjUtil::is_same_coord(ptr_left_lane->coords_.back(),
                                                           ptr_lanes[i]->coords_.back())) {
                                ptr_same_node_lanes.emplace_back(ptr_lanes[i]);
                            } else {
                                // check
                                lane_intersects(mapDataManager, errorOutput, ptr_same_node_lanes);
                                ptr_same_node_lanes.clear();
                                ptr_left_lane = ptr_lanes[i];
                                ptr_same_node_lanes.emplace_back(ptr_left_lane);
                            }
                        }
                    }
                }
            }
        }

        void LaneCheck::lane_intersects(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<CheckErrorOutput> &errorOutput,
                                        const vector<shared_ptr<DCLane>> &ptr_lanes) {
            char zone[8] = {0};
            auto ptr_f_coord = GeosObjUtil::create_coordinate(ptr_lanes.front()->coords_.front(), zone);
            auto ptr_t_coord = GeosObjUtil::create_coordinate(ptr_lanes.front()->coords_.back(), zone);

            if (ptr_f_coord == nullptr) {
                return;
            }
            if (ptr_t_coord == nullptr) {
                return;
            }

            for (int i = 0; i < ptr_lanes.size() - 1; i++) {
                for (int j = i + 1; j < ptr_lanes.size(); j++) {
                    auto ptr_left_line = CommonUtil::get_line_string(ptr_lanes[i]->coords_);
                    auto ptr_right_line = CommonUtil::get_line_string(ptr_lanes[j]->coords_);
                    auto ptr_intersects_geos = ptr_left_line->intersection(ptr_right_line.get());
                    auto coord_size = ptr_intersects_geos->getCoordinates()->size();
                    if (coord_size > 1 && coord_size < 4) {
                        auto ptr_intersect_coords = ptr_intersects_geos->getCoordinates();
                        for (int k = 0; k < ptr_intersect_coords->size(); k++) {
                            // 如果不是第一个点
                            if (!GeosObjUtil::is_same_coord(*ptr_f_coord.get(), ptr_intersect_coords->getAt(k)) &&
                                !GeosObjUtil::is_same_coord(*ptr_t_coord.get(), ptr_intersect_coords->getAt(k))) {
                                auto ptr_error = DCLaneError::createByKXS_05_015(ptr_lanes[i]->id_, ptr_lanes[j]->id_);

                                ptr_error->taskId_ = ptr_lanes[i]->task_id_;
                                ptr_error->flag = ptr_lanes[i]->flag_;
                                ptr_error->dataKey_ = DATA_TYPE_LANE+ptr_lanes[i]->task_id_+DATA_TYPE_LAST_NUM;

                                errorOutput->saveError(ptr_error);
                            }
                        }
                    }
                }
            }

        }

        void LaneCheck::check_lane_node(shared_ptr<MapDataManager> mapDataManager,
                                        shared_ptr<CheckErrorOutput> errorOutput,
                                        shared_ptr<DCLaneGroup> ptr_lane_group) {
            for (const auto &ptr_lane : ptr_lane_group->lanes_) {
                vector<shared_ptr<NodeError>> ptr_error_nodes;
                auto first_node = ptr_lane->coords_.front();
                shared_ptr<NodeError> ptr_e_node = make_shared<NodeError>();
                ptr_e_node->index = 0;
                ptr_e_node->ptr_coord = first_node;
                ptr_error_nodes.emplace_back(ptr_e_node);
                for (int i = 1; i < ptr_lane->coords_.size(); i++) {
                    if (first_node->lng_ == ptr_lane->coords_.at(i)->lng_ &&
                        first_node->lat_ == ptr_lane->coords_.at(i)->lat_) {
                        shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                        ptr_cur_e_node->index = i;
                        ptr_cur_e_node->ptr_coord = ptr_lane->coords_.at(i);
                        ptr_error_nodes.emplace_back(ptr_cur_e_node);
                    } else {
                        if (ptr_error_nodes.size() > 1) {
                            auto ptr_error = DCLaneError::createByKXS_05_016(ptr_lane->id_, ptr_error_nodes);

                            ptr_error->taskId_ = ptr_lane->task_id_;
                            ptr_error->flag = ptr_lane->flag_;
                            ptr_error->dataKey_ = DATA_TYPE_LANE+ptr_lane->task_id_+DATA_TYPE_LAST_NUM;

                            errorOutput->saveError(ptr_error);
                        }
                        ptr_error_nodes.clear();
                        first_node = ptr_lane->coords_.at(i);
                        shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                        ptr_cur_e_node->index = i;
                        ptr_cur_e_node->ptr_coord = first_node;
                        ptr_error_nodes.emplace_back(ptr_cur_e_node);
                    }
                }
            }
        }
    }
}