//
// Created by zhangxingang on 19-6-17.
//

#include <util/CommonUtil.h>
#include <util/GeosObjUtil.h>
#include "businesscheck/LaneGroupCheck.h"
namespace kd {
    namespace dc {

        static const double DIVIDER_NODE_LENGTH = 10;

        LaneGroupCheck::~LaneGroupCheck() {

        }

        string LaneGroupCheck::getId() {
            return id_;
        }

        bool LaneGroupCheck::execute(shared_ptr<MapDataManager> data_manager,
                                     shared_ptr<CheckErrorOutput> error_output) {
            data_manager_ = data_manager;
            error_output_ = error_output;

            check_lanegroup_divider();

            check_divider();
            return false;
        }

        void LaneGroupCheck::check_lanegroup_divider() {
            const auto &divider2w_lane_groups = data_manager_->divider2_lane_groups_;
            shared_ptr<DCError> ptr_error = nullptr;
            for (auto div2_lg : divider2w_lane_groups) {
                bool check = false;
                if (div2_lg.second.size() == 2) {
                    // divider关联多个车道组
                    auto ptr_divider = CommonUtil::get_divider(data_manager_, div2_lg.first);
                    if (ptr_divider) {
                        // 如果是参考线
                        if (ptr_divider->dividerNo_ == 0) {
                            for (const auto &lg : div2_lg.second) {
                                auto ptr_road =
                                        CommonUtil::get_road_by_lg(data_manager_, lg);
                                if (ptr_road) {
                                    // 不是双向的
                                    if (ptr_road->direction_ != 1) {
                                        check = true;
                                        break;
                                    }
                                } else {
                                    LOG(ERROR) << "get_road_by_lg failed! lane group:" << lg;
                                }
                            }
                        } else {
                            check = true;
                        }
                    } else {
                        LOG(ERROR) << "get_divider failed! divider:" << div2_lg.first;
                    }
                } else {
                    check = div2_lg.second.size() != 1;
                }

                // 错误
                if (check) {
                    ptr_error = DCLaneGroupCheckError::createByKXS_03_004(div2_lg.first, div2_lg.second);
                    error_output_->saveError(ptr_error);
                }
            }
        }

        void LaneGroupCheck::check_divider() {
            const auto ptr_lane_groups = data_manager_->laneGroups_;
            for (const auto &lane_group : ptr_lane_groups) {
                auto ptr_dividers = CommonUtil::get_dividers_by_lg(data_manager_, lane_group.first);
                if (!ptr_dividers.empty()) {
//                    check_divider_no(mapDataManager, errorOutput, lane_group.first, ptr_dividers);
                    check_divider_length(lane_group.first, ptr_dividers);
                }
            }
        }

        void LaneGroupCheck::check_divider_no(shared_ptr<MapDataManager> mapDataManager,
                                                      shared_ptr<CheckErrorOutput> errorOutput, const string &lane_group,
                                                      const vector<shared_ptr<DCDivider>> &ptr_dividers) {
            bool is_check = false;

            auto ptr_lane_group = CommonUtil::get_lane_group(mapDataManager, lane_group);
            if (!ptr_lane_group->is_virtual_) {
                vector<shared_ptr<DCCoord>> divider_f_node_vecs;
                vector<shared_ptr<DCCoord>> divider_t_node_vecs;
                for (const auto &div : ptr_dividers) {
                    divider_f_node_vecs.emplace_back(div->nodes_.front()->coord_);
                    divider_t_node_vecs.emplace_back(div->nodes_.back()->coord_);
                }
                double temp_f_length = GeosObjUtil::get_length_of_coords(divider_f_node_vecs);
                double temp_t_length = GeosObjUtil::get_length_of_coords(divider_t_node_vecs);

                bool direction = ptr_lane_group->direction_ == 1;
                if (temp_f_length < ptr_dividers.size() * DIVIDER_NODE_LENGTH) {
                    if (check_divider_no(ptr_dividers, true, direction)) {
                        is_check = true;
                    }
                } else {
                    if (temp_t_length < ptr_dividers.size() * DIVIDER_NODE_LENGTH) {
                        if (check_divider_no(ptr_dividers, false, direction)) {
                            is_check = true;
                        }
                    }
                }



            }

            if (is_check) {
                shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_002(lane_group);
                errorOutput->saveError(ptr_error);
            }

        }

        bool LaneGroupCheck::check_divider_no(const vector<shared_ptr<DCDivider>> &ptr_dividers,
                                                      bool is_front, bool direction) {
            bool is_check = false;

            // 获取节点
            auto ptr_left_divider = ptr_dividers.front();
            auto ptr_left_divider_node = (is_front & direction) ? ptr_left_divider->nodes_.front()
                                                                : ptr_left_divider->nodes_.back();

            auto ptr_left_dis_node = CommonUtil::get_distance_node(ptr_left_divider, DIVIDER_NODE_LENGTH,
                                                                   (is_front & direction));
            if (ptr_left_divider->dividerNo_ == 0) {
                for (size_t index = 1; index < ptr_dividers.size(); index++) {
                    auto ptr_right_divider_node = is_front ? ptr_dividers[index]->nodes_.front()
                                                           : ptr_dividers[index]->nodes_.back();
//                    auto ptr_right_dis_node = CommonUtil::get_distance_node(ptr_dividers[index],
//                                                                            DIVIDER_NODE_LENGTH, is_front);

                    if (ptr_right_divider_node->id_ != ptr_left_divider_node->id_) {
                        if (is_front) {
                            if (CommonUtil::NodeOrentationOfDivider(ptr_left_divider_node, ptr_left_dis_node,
                                                                    ptr_right_divider_node) != -1) {
                                is_check = true;
                                break;
                            }
                        } else {
                            if (CommonUtil::NodeOrentationOfDivider(ptr_left_divider_node, ptr_left_dis_node,
                                                                    ptr_right_divider_node) != 1) {
                                is_check = true;
                                break;
                            }
                        }
                    } else {
                        auto ptr_right_dis_node = CommonUtil::get_distance_node(ptr_dividers[index],
                                                                                DIVIDER_NODE_LENGTH, is_front);
                        if (ptr_left_dis_node->id_ != ptr_right_dis_node->id_) {

                            if (is_front) {
                                if (CommonUtil::NodeOrentationOfDivider(ptr_left_divider_node, ptr_left_dis_node,
                                                                        ptr_right_dis_node) != -1) {
                                    is_check = true;
                                    break;
                                }
                            } else {
                                if (CommonUtil::NodeOrentationOfDivider(ptr_left_divider_node, ptr_left_dis_node,
                                                                        ptr_right_dis_node) != 1) {
                                    is_check = true;
                                    break;
                                }
                            }
                        }
                    }

                    ptr_left_divider_node = ptr_right_divider_node;
                    ptr_left_dis_node = CommonUtil::get_distance_node(ptr_dividers[index],
                                                                      DIVIDER_NODE_LENGTH, is_front);
                }
            } else {
                // 编号出错
                is_check = true;
            }

            return is_check;
        }

        void LaneGroupCheck::check_divider_length(const string &lane_group,
                                                  const vector<shared_ptr<DCDivider>> &ptr_dividers) {
            bool check = false;
            vector<string> check_dividers;
            // 读取配置
            double divider_length_ratio = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_LENGTH_RATIO);

            double total_length = 0;
            for (const auto &ptr_div : ptr_dividers) {
                total_length += ptr_div->len_;
            }
            double average_length = total_length / ptr_dividers.size();
            double ratio_length = divider_length_ratio * average_length;

            // 判断长度
            for (const auto &ptr_div : ptr_dividers) {
                if (fabs(ptr_div->len_ - average_length) > ratio_length) {
                    check = true;
                    check_dividers.emplace_back(ptr_div->id_);
                }
            }

            if (check) {
                shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_001(lane_group, check_dividers);
                error_output_->saveError(ptr_error);
            }
        }
    }
}