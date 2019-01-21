//
// Created by zhangxingang on 19-1-17.
//

#include <businesscheck/LaneGroupCheck.h>
#include <util/CommonUtil.h>

#include "businesscheck/LaneGroupCheck.h"

namespace kd {
    namespace dc {

        string LaneGroupCheck::getId() {
            return id;
        }

        bool LaneGroupCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                     shared_ptr<CheckErrorOutput> errorOutput) {
            check_lanegroup_road(mapDataManager, errorOutput);

            check_lanegroup_divider(mapDataManager, errorOutput);
            return false;
        }

        void LaneGroupCheck::check_lanegroup_road(shared_ptr<MapDataManager> mapDataManager,
                                                  shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &road2LaneGroup2NodeIdxs = mapDataManager->road2LaneGroup2NodeIdxs_;
            const auto &roads = mapDataManager->roads_;
            for (const auto &road2_lg_idx:road2LaneGroup2NodeIdxs) {
                shared_ptr<DCRoad> ptr_road = CommonUtil::get_road(mapDataManager, road2_lg_idx.first);
                const auto &lg_idx = road2_lg_idx.second;
                vector<LGNodeIndex> pos_dir_lg_vec;
                vector<LGNodeIndex> neg_dir_lg_vec;
                for (const auto &node_idx : lg_idx) {
                    const string &lane_group_id = node_idx.first;
                    bool lg_dir = true;
                    auto lang_group = CommonUtil::get_lane_group(mapDataManager, lane_group_id);
                    if (lang_group) {
                        if (lang_group->direction_ != 1) {
                            lg_dir = false;
                        }
                    } else {
                        LOG(ERROR) << "get_lane_group failed! lane groud:" << lane_group_id;
                    }
                    const pair<long, long> &ft_node_pair = node_idx.second;
                    LGNodeIndex lg_node_index(lane_group_id, road2_lg_idx.first, ft_node_pair.first,
                                              ft_node_pair.second);
                    if (lg_dir) {
                        pos_dir_lg_vec.emplace_back(lg_node_index);
                    } else {
                        neg_dir_lg_vec.emplace_back(lg_node_index);
                    }
                }
                if (!pos_dir_lg_vec.empty()) {
                    check_road_node_index(pos_dir_lg_vec, ptr_road, true, errorOutput);
                } else {
                    //
                    LOG(ERROR) << "lanegroup关联road索引缺失";
                }
                if (ptr_road) {
                    // 双向道路
                    if (ptr_road->direction_ == 1) {
                        if (!neg_dir_lg_vec.empty()) {
                            check_road_node_index(neg_dir_lg_vec, ptr_road, false, errorOutput);
                        } else {
                            //
                            LOG(ERROR) << "lanegroup关联road索引缺失";
                        }
                    }
                } else {
                    LOG(ERROR) << "ptr_road failed! road:" << road2_lg_idx.first;
                }
            }
        }

        void LaneGroupCheck::check_road_node_index(vector<LGNodeIndex> lg_node_index_vec, shared_ptr<DCRoad> ptr_road,
                                                   bool is_positive, shared_ptr<CheckErrorOutput> errorOutput) {
            // 检查是否存在交叉
            if (is_positive) {
                sort(lg_node_index_vec.begin(), lg_node_index_vec.end());
            } else {
                sort(lg_node_index_vec.begin(), lg_node_index_vec.end(), [](const LGNodeIndex &lg_node_idx1,
                                                                            const LGNodeIndex &lg_node_idx2) {
                    return lg_node_idx1.f_idx > lg_node_idx2.f_idx;
                });
            }
            shared_ptr<DCError> ptr_error;
            if (ptr_road) {
                // 检查索引点是否铺满
                long min_index = 0;
                long max_index = 0;
                if (is_positive) {
                    min_index = lg_node_index_vec.front().f_idx;
                    max_index = lg_node_index_vec.back().t_idx;
                    if (min_index != 0) {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(ptr_road->id_, 0, is_positive);
                        errorOutput->saveError(ptr_error);
                    }
                    if (max_index != ptr_road->nodes_.size() - 1) {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(ptr_road->id_,
                                                                              ptr_road->nodes_.size() - 1, is_positive);
                        errorOutput->saveError(ptr_error);
                    }
                } else {
                    min_index = lg_node_index_vec.back().t_idx;
                    max_index = lg_node_index_vec.front().f_idx;
                    if (min_index != 0) {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(ptr_road->id_, 0, is_positive);
                        errorOutput->saveError(ptr_error);
                    }
                    if (max_index != ptr_road->nodes_.size() - 1) {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(ptr_road->id_,
                                                                              ptr_road->nodes_.size() - 1,
                                                                              is_positive);
                        errorOutput->saveError(ptr_error);
                    }
                }
            }

            auto pre_iter = lg_node_index_vec.begin();
            auto lat_iter = ++lg_node_index_vec.begin();

            while (lat_iter != lg_node_index_vec.end()) {
                if (lat_iter->f_idx > pre_iter->t_idx) {
                    if (is_positive) {
                        // lanegroup没有铺满整条道路
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(pre_iter->road_id, pre_iter->t_idx,
                                                                              lat_iter->f_idx, is_positive);
                    } else {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_006(pre_iter->road_id, pre_iter->lanegroup_id,
                                                                              pre_iter->f_idx, pre_iter->t_idx,
                                                                              lat_iter->lanegroup_id, lat_iter->f_idx,
                                                                              lat_iter->t_idx, is_positive);
                    }

                    errorOutput->saveError(ptr_error);
                } else if (lat_iter->f_idx < pre_iter->t_idx) {
                    if (is_positive) {
                        // 出现交叉
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_006(pre_iter->road_id, pre_iter->lanegroup_id,
                                                                              pre_iter->f_idx, pre_iter->t_idx,
                                                                              lat_iter->lanegroup_id, lat_iter->f_idx,
                                                                              lat_iter->t_idx, is_positive);
                    } else {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(pre_iter->road_id, pre_iter->t_idx,
                                                                              lat_iter->f_idx, is_positive);
                    }

                    errorOutput->saveError(ptr_error);
                } else {
                    // 正常
                }
                pre_iter = lat_iter;
                lat_iter++;
            }
        }

        void LaneGroupCheck::check_lanegroup_divider(shared_ptr<MapDataManager> mapDataManager,
                                                     shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &divider2w_lane_groups = mapDataManager->divider2_lane_groups_;
            shared_ptr<DCError> ptr_error = nullptr;
            for (auto div2_lg : divider2w_lane_groups) {
                bool check = false;
                if (div2_lg.second.size() == 2) {
                    // divider关联多个车道组
                    auto ptr_divider = CommonUtil::get_divider(mapDataManager, div2_lg.first);
                    if (ptr_divider) {
                        // 如果是参考线
                        if (ptr_divider->dividerNo_ == 0) {
                            for (const auto &lg : div2_lg.second) {
                                auto ptr_road = CommonUtil::get_road_by_lg(mapDataManager, lg);
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
                    errorOutput->saveError(ptr_error);
                }
            }
        }
    }
}