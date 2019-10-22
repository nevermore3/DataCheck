

#include <businesscheck/LaneGroupRelationCheck.h>
#include <util/CommonUtil.h>
#include <util/GeosObjUtil.h>

#include "businesscheck/LaneGroupRelationCheck.h"

namespace kd {
    namespace dc {

        static const double DIVIDER_NODE_LENGTH = 10;

        string LaneGroupRelationCheck::getId() {
            return id;
        }

        bool LaneGroupRelationCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                             shared_ptr<CheckErrorOutput> errorOutput) {
            check_lanegroup_road(mapDataManager, errorOutput);

            release(mapDataManager);

            return true;
        }

        void LaneGroupRelationCheck::check_lanegroup_road(shared_ptr<MapDataManager> mapDataManager,
                                                  shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &road2LaneGroup2NodeIdxs = mapDataManager->road2LaneGroup2NodeIdxs_;
            const auto &roads = mapDataManager->roads_;
            int total = 0;
            for (const auto &road2_lg_idx:road2LaneGroup2NodeIdxs) {
                shared_ptr<DCRoad> ptr_road = CommonUtil::get_road(mapDataManager, road2_lg_idx.first);
                const auto &lg_idx = road2_lg_idx.second;
                vector<LGNodeIndex> pos_dir_lg_vec;
                vector<LGNodeIndex> neg_dir_lg_vec;
                for (const auto &node_idx : lg_idx) {
                    total++;
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
                    check_road_node_index(pos_dir_lg_vec, ptr_road, true, mapDataManager, errorOutput);
                } else {
                    //
                    LOG(ERROR) << "lanegroup关联road索引缺失";
                }
                if (ptr_road) {
                    // 双向道路
                    if (ptr_road->direction_ == 1) {
                        if (!neg_dir_lg_vec.empty()) {
                            check_road_node_index(neg_dir_lg_vec, ptr_road, false, mapDataManager, errorOutput);
                        } else {
                            //
                            LOG(ERROR) << "lanegroup关联road索引缺失";
                        }
                    }
                } else {
                    LOG(ERROR) << "ptr_road failed! road:" << road2_lg_idx.first;
                }
            }
            if(CheckItemValid(CHECK_ITEM_KXS_LG_005)){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LG_005,total);
            }
            if(CheckItemValid(CHECK_ITEM_KXS_LG_006)){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LG_006,total);
            }

        }

        void LaneGroupRelationCheck::check_road_node_index(vector<LGNodeIndex> lg_node_index_vec, shared_ptr<DCRoad> ptr_road,
                                                   bool is_positive, shared_ptr<MapDataManager> mapDataManager,
                                                   shared_ptr<CheckErrorOutput> errorOutput) {
            // 检查是否存在交叉
            if (is_positive) {
                sort(lg_node_index_vec.begin(), lg_node_index_vec.end(), [](const LGNodeIndex &lg_node_idx1,
                                                                            const LGNodeIndex &lg_node_idx2) {
                    return lg_node_idx1.f_idx < lg_node_idx2.f_idx;
                });
            } else {
                sort(lg_node_index_vec.begin(), lg_node_index_vec.end(), [](const LGNodeIndex &lg_node_idx1,
                                                                            const LGNodeIndex &lg_node_idx2) {
                    return lg_node_idx1.f_idx > lg_node_idx2.f_idx;
                });
            }
            shared_ptr<DCError> ptr_error;
            if (CheckItemValid(CHECK_ITEM_KXS_LG_005)&& ptr_road) {
                // 检查索引点是否铺满
                check_index_fill_all(lg_node_index_vec, ptr_road, is_positive, errorOutput);
            }

            auto pre_iter = lg_node_index_vec.begin();
            auto lat_iter = ++lg_node_index_vec.begin();

            while (lat_iter != lg_node_index_vec.end()) {
                if (lat_iter->f_idx > pre_iter->t_idx) {
                    if (!is_positive) {
                        auto ptr_lane_group = CommonUtil::get_lane_group(mapDataManager, lat_iter->lanegroup_id);
                        if (CheckItemValid(CHECK_ITEM_KXS_LG_006)&& (ptr_lane_group && ptr_lane_group->is_virtual_ != 1)) {
                            ptr_error = DCLaneGroupCheckError::createByKXS_03_006(pre_iter->road_id, pre_iter->lanegroup_id,
                                                                                  pre_iter->f_idx, pre_iter->t_idx,
                                                                                  lat_iter->lanegroup_id, lat_iter->f_idx,
                                                                                  lat_iter->t_idx,ptr_road->task_id_, is_positive);

                        }
                    }

                    errorOutput->saveError(ptr_error);
                } else if (lat_iter->f_idx < pre_iter->t_idx) {
                    if (is_positive) {
                        // 出现交叉
                        auto ptr_lane_group = CommonUtil::get_lane_group(mapDataManager, lat_iter->lanegroup_id);
                        if (CheckItemValid(CHECK_ITEM_KXS_LG_006) && (ptr_lane_group && ptr_lane_group->is_virtual_ != 1)) {
                            ptr_error = DCLaneGroupCheckError::createByKXS_03_006(pre_iter->road_id,
                                                                                  pre_iter->lanegroup_id,
                                                                                  pre_iter->f_idx, pre_iter->t_idx,
                                                                                  lat_iter->lanegroup_id,
                                                                                  lat_iter->f_idx,
                                                                                  lat_iter->t_idx,ptr_road->task_id_, is_positive);
                        }
                    }

                    errorOutput->saveError(ptr_error);
                } else {
                    // 正常
                }
                pre_iter = lat_iter;
                lat_iter++;
            }
        }

        void LaneGroupRelationCheck::check_index_fill_all(vector<LGNodeIndex> lg_node_index_vec, shared_ptr<DCRoad> ptr_road,
                                                  bool is_positive, shared_ptr<CheckErrorOutput> errorOutput) {
            shared_ptr<DCError> ptr_error;
            int min_index = 0;
            int max_index = 0;

            int *road_index = new int[sizeof(int) * ptr_road->nodes_.size()];
            if (road_index != nullptr) {

                memset(road_index, 0, sizeof(int) * ptr_road->nodes_.size());
                for (const auto &node_index : lg_node_index_vec) {
                    if (is_positive) {
                        min_index = node_index.f_idx;
                        max_index = node_index.t_idx;
                    } else {
                        min_index = node_index.t_idx;
                        max_index = node_index.f_idx;
                    }
                    for (int i = min_index; i <= max_index; i++) {
                        if (0 <= i && i < ptr_road->nodes_.size()) {
                            if (road_index[i] == 0) {
                                road_index[i] = 1;
                            }
                        }
                    }
                }

                for (int i = 0; i < ptr_road->nodes_.size(); i++) {
                    if (road_index[i] == 0) {
                        ptr_error = DCLaneGroupCheckError::createByKXS_03_005(ptr_road->id_, i, is_positive);
                        ptr_error->taskId_ = ptr_road->task_id_;
                        ptr_error->flag = ptr_road->flag_;
                        ptr_error->dataKey_ = DATA_TYPE_LANE + ptr_road->task_id_ + DATA_TYPE_LAST_NUM;
                        ptr_error->coord = ptr_road->nodes_[i];
                        shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(ptr_road->nodes_[i]);
                        errNodeInfo->dataType = DATA_TYPE_NODE;
                        errNodeInfo->dataLayer = MODEL_NAME_ROAD;
                        ptr_error->errNodeInfo.emplace_back(errNodeInfo);

                        errorOutput->saveError(ptr_error);
                    }
                }
            }

            if (road_index != nullptr) {
                delete[] road_index;
                road_index = nullptr;
            }
        }

        void LaneGroupRelationCheck::release(shared_ptr<MapDataManager> mapDataManager) {
            mapDataManager->road2LaneGroup2NodeIdxs_.clear();
        }
    }
}