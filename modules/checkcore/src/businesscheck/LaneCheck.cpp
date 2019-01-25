//
// Created by zhangxingang on 19-1-25.
//

#include "businesscheck/LaneCheck.h"

#include <geos/geom/GeometryFactory.h>

#include "util/CommonUtil.h"

namespace kd {
    namespace dc {
        string LaneCheck::getId() {
            return id;
        }

        bool LaneCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            check_lane_divider_intersect(mapDataManager, errorOutput);
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
                if (lg.first == "59340043") {
                    int l = 0;
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
                ret = ptr_lane_line_string->intersects(ptr_div_line_string.get());
            }
            return ret;
        }

        bool LaneCheck::lane_divider_intersects(const shared_ptr<MapDataManager> &mapDataManager,
                                                const shared_ptr<DCLane> &ptr_lane,
                                                const shared_ptr<DCDivider> &ptr_divider) {
            bool ret = false;
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);
            auto ptr_lane_line_string = CommonUtil::get_line_string(ptr_lane->coords_);
            if (ptr_lane_line_string && ptr_div_line_string) {
                if (ptr_lane_line_string->intersects(ptr_div_line_string.get())) {
                    auto intersections = ptr_lane_line_string->intersection(ptr_div_line_string.get());

                    if (intersections->getNumPoints() > 2) {
                        ret = true;
                    } else if (intersections->getNumPoints() == 2) {
                        const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
                        geos::geom::LineString *lineString = gf->createLineString(intersections->getCoordinates());

                        double intersect_length = lineString->getLength();
                        if (intersect_length < 50) {
                            ret = true;
                        }
                    }
                }
            }
            return ret;
        }
    }
}