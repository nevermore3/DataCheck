//
// Created by zhangxingang on 19-1-25.
//

#include "businesscheck/LaneCheck.h"
#include <geos/geom/GeometryFactory.h>
#include <businesscheck/LaneCheck.h>
#include <util/GeosObjUtil.h>

#include "util/CommonUtil.h"
#include "util/KDGeoUtil.hpp"
#include "util/CommonCheck.h"
#include <shp/ShpData.hpp>

using namespace kd::automap;

namespace kd {
    namespace dc {

        LaneCheck::LaneCheck(string fileName) : SCHCheck(fileName) {}

        string LaneCheck::getId() {
            return id;
        }

        void LaneCheck::SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager){
            map_data_manager_ = mapDataManager;
        }

        bool LaneCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            SetMapDataManager(mapDataManager);

            check_lane_divider_intersect(mapDataManager, errorOutput);

            check_lane_lane_intersect(mapDataManager, errorOutput);

            // 检查车道中心线 节点间角度
            check_lane_nodes_angle(mapDataManager, errorOutput);

            // 有拓扑关系的车道中心线夹角检查
            check_lane_angle(mapDataManager, errorOutput);

            // 车道中心线曲率检查
            CurvatureValueCheck(errorOutput);

            // 相邻HD_LANE_SCH点之间距离不超过1.3m
            CheckAdjacentNodeDistance(errorOutput);

            // 每一Lane的形状点周围1.5米内必有一个关联该Lane的HD_LANE_SCH
            LaneRelevantLaneSCH(errorOutput);

            //HD_LANE_SCH点离关联的LANE的垂直距离不超过10cm
            LaneSCHVerticalDistance(errorOutput);
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
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_015;
            size_t  total = 0;
            const auto &ptr_lane_groups = mapDataManager->laneGroups_;

            shared_ptr<DCError> ptr_error = nullptr;
            for (const auto &lg : ptr_lane_groups) {
                check_lane_node(mapDataManager, errorOutput, lg.second);
                auto ptr_lanes = lg.second->lanes_;
                total += ptr_lanes.size();
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
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
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
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_016;
            size_t total = 0;
            for (const auto &ptr_lane : ptr_lane_group->lanes_) {
                total += ptr_lane->coords_.size();
                vector<shared_ptr<NodeError>> ptr_error_nodes;
                auto first_node = ptr_lane->coords_.front();
                shared_ptr<NodeError> ptr_e_node = make_shared<NodeError>();
                ptr_e_node->index = 0;
                ptr_e_node->ptr_coord = first_node;
                ptr_error_nodes.emplace_back(ptr_e_node);
                for (int i = 1; i < ptr_lane->coords_.size(); i++) {
                    if (first_node->x_ == ptr_lane->coords_.at(i)->x_ &&
                        first_node->y_ == ptr_lane->coords_.at(i)->y_) {
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
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        //同一条车道中心线上连续三个节点构成的夹角（绝对值）不能小于165度 (可配置)
        void LaneCheck::check_lane_nodes_angle(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            double angleThreshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_NODE_ANGLE);
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_017;
            size_t total = mapDataManager->lanes_.size();
            for (const auto &lane : mapDataManager->lanes_) {
                string laneId = lane.first;
                vector<shared_ptr<DCCoord>> coords = lane.second->coords_;

                vector<shared_ptr<NodeCompareError>> errorArray = CommonCheck::AngleCheck(coords, angleThreshold);
                if (!errorArray.empty()) {
                    auto error = DCLaneError::createByKXS_05_017(laneId, errorArray);
                    errorOutput->saveError(error);
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        void LaneCheck::check_lane_angle(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_018;
            size_t  total = mapDataManager->laneConnectivitys_.size();

            double angleThreshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_ANGLE);
            for (const auto &connectNode : mapDataManager->laneConnectivitys_) {
                long fromLaneID = connectNode.second->fLaneId_;
                long toLaneID = connectNode.second->tLaneId_;

                shared_ptr<DCLane>fromLane = mapDataManager->lanes_[to_string(fromLaneID)];
                shared_ptr<DCLane>toLane = mapDataManager->lanes_[to_string(toLaneID)];

                if (fromLane == nullptr || toLane == nullptr) {
                    continue;
                }
                shared_ptr<DCCoord> previous = fromLane->coords_[fromLane->coords_.size() - 2];
                shared_ptr<DCCoord> current = toLane->coords_[0];
                shared_ptr<DCCoord> next = toLane->coords_[1];
                double angle = 0;
                if (!CommonUtil::CheckCoordAngle(previous, current, next, angleThreshold, angle)) {
                    //angle = angle * 180 / kd::automap::PI;
                    auto error = DCLaneError::createByKXS_05_018(fromLaneID, toLaneID, angle);
                    errorOutput->saveError(error);
                }

            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }



        // 每一Lane的形状点周围1.5米内必有一个关联该Lane的HD_LANE_SCH
        void LaneCheck::LaneRelevantLaneSCH(shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_020;
            size_t total = 0;

            double threshold = 1.5;
            /*
             * 1、找到一条Lane对象
             * 2、在map_obj_schs中找到对应 该Lane对象的属性点集合
             * 3、遍历该Lane对象的形点
             * 3、按照属性点中的obj_index索引，第一个遍历到的属性点就是离该形点最近的点
             */
            auto lanes = map_data_manager_->lanes_;
            for (const auto &lane : lanes) {
                long laneID = stol(lane.first);
                if (map_obj_schs_.find(laneID) == map_obj_schs_.end()) {
                    continue;
                }
                auto schNodes = map_obj_schs_[laneID];
                total += lane.second->coords_.size();
                // 关联同一个Lane对象的属性点的索引
                size_t j = 0;
                for (size_t i = 0; i < lane.second->coords_.size(); i++) {
                    while (j < schNodes.size()) {
                        if (schNodes[j]->obj_index_ == i) {
                            // 找到第一个和Lane形点关联的属性点
                            double distance = GeosObjUtil::get_length_of_node(lane.second->coords_[i], schNodes[j]->coord_);
                            if (distance > threshold) {
                                auto error = DCLaneError::createByKXS_05_020(laneID, i, lane.second->coords_[i], 1);
                                errorOutput->saveError(error);
                            }

                            // Lane的起点和终点之处（buffer20cm）必有一个关联该Lane的HD_LANE_SCH
                            if (i == 0 || i == lane.second->coords_.size() - 1) {
                                if (distance > 0.2) {
                                    auto error = DCLaneError::createByKXS_05_020(laneID, i,
                                                                                 lane.second->coords_[i], 2);
                                    errorOutput->saveError(error);
                                }
                            }

                            j++;
                            break;
                        }
                        j++;
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);

        }


        void LaneCheck::LaneSCHVerticalDistance(shared_ptr<CheckErrorOutput> &errorOutput) {

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LANE_022;
            size_t total = 0;
            for (const auto &laneSCH : map_obj_schs_) {
                long laneID = laneSCH.first;
                total += laneSCH.second.size();
                string strLaneID = to_string(laneID);
                if (map_data_manager_->lanes_.find(strLaneID) == map_data_manager_->lanes_.end()) {
                    continue;
                }
                auto lane = map_data_manager_->lanes_[strLaneID];
                for (const auto &node : laneSCH.second) {
                    shared_ptr<geos::geom::Point> point = GeosObjUtil::CreatePoint(node->coord_);
                    double distance = GeosObjUtil::GetVerticleDistance(lane->line_, point);
                    if (distance > 0.1) {
                        auto error = DCLaneError::createByKXS_05_022(node->index_, node->coord_);
                        errorOutput->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

    }
}