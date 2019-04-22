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

            shared_ptr<DCError> ptr_error = nullptr;
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

                        if (lane_divider_intersect(mapDataManager, ptr_lane, left_ptr_divider)) {
                            ptr_error = DCLaneError::createByKXS_05_003(ptr_lane->id_, left_ptr_divider->id_);
                            errorOutput->saveError(ptr_error);
                        }
                        if (lane_divider_intersect(mapDataManager, ptr_lane, right_ptr_divider)) {
                            ptr_error = DCLaneError::createByKXS_05_003(ptr_lane->id_, right_ptr_divider->id_);
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
                                errorOutput->saveError(ptr_error);
                            }

                            if (lane_divider_intersects(mapDataManager, ptr_lane, ptr_lane->rightDivider_)) {
                                ptr_error = DCLaneError::createByKXS_05_002(ptr_lane->id_,
                                                                            ptr_lane->rightDivider_->id_);
                                errorOutput->saveError(ptr_error);
                            }
                        }


                    }
                }
            }
        }

        bool LaneCheck::lane_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCLane> &ptr_lane,
                                               const shared_ptr<DCDivider> &ptr_divider) {
            bool ret = false;
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);
            auto ptr_lane_line_string = CommonUtil::get_line_string(ptr_lane->coords_);
            if (ptr_lane_line_string && ptr_div_line_string) {
                ret = KDGeoUtil::isLineCross(ptr_lane_line_string.get(), ptr_div_line_string.get());
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
                                auto ptr_error = DCLaneError::createByKXS_05_013(ptr_lanes[i]->id_, ptr_lanes[j]->id_);
                                errorOutput->saveError(ptr_error);
                            }
                        }
                    }
                }
            }

        }
    }
}