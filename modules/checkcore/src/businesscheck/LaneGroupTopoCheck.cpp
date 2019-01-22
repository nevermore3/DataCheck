//
// Created by zhangxingang on 19-1-21.
//
#include <businesscheck/LaneGroupTopoCheck.h>

#include "businesscheck/LaneGroupTopoCheck.h"
#include "util/CommonUtil.h"

namespace kd {
    namespace dc {
        string LaneGroupTopoCheck::getId() {
            return id;
        }

        bool LaneGroupTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            bool ret = true;
            try {
                pre_divider_topo(mapDataManager);
                check_road_topo(mapDataManager, errorOutput);
            } catch (exception &e) {
                LOG(ERROR) << e.what();
                ret = false;
            }

            release(mapDataManager);
            return ret;
        }

        void LaneGroupTopoCheck::pre_divider_topo(shared_ptr<MapDataManager> mapDataManager) {
            const auto &dividers = mapDataManager->dividers_;
            for (const auto &div : dividers) {
                auto lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div.first);
                bool is_virtual = false;
                for (const auto &lg : lane_groups) {
                    if (is_virtual_lane_group(mapDataManager, lg)) {
                        is_virtual = true;
                        break;
                    }
                }

                // 过滤虚拟路口
                if (is_virtual) {
                    continue;
                }

                auto f_conn_dividers = CommonUtil::get_conn_divider(mapDataManager, div.second, true);
                if (lane_groups.size() == 1) {
                    string lane_group = *lane_groups.begin();
                    auto ptr_lane_group = CommonUtil::get_lane_group(mapDataManager, lane_group);

                    set<string> f_conn_lane_groups;
                    for (const auto &conn_div : f_conn_dividers) {
                        f_conn_lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, conn_div);
                        for (const auto &conn_lg : f_conn_lane_groups) {
                            if (is_virtual_lane_group(mapDataManager, conn_lg)) {
                                is_virtual = true;
                                break;
                            }

                            if (lane_group2_conn_lg_.find(make_pair(lane_group, conn_lg)) ==
                                lane_group2_conn_lg_.end()) {
                                lane_group2_conn_lg_.insert(make_pair(lane_group, conn_lg));
                            }
                        }
                        // 过滤虚拟路口
                        if (is_virtual) {
                            break;
                        }
                        divider2_conn_divider_.insert(make_pair(div.first, conn_div));
                    }

                } else {
                    for (const auto &conn_div : f_conn_dividers) {
                        divider2_conn_divider_.insert(make_pair(div.first, conn_div));
                    }
                }
            }
        }

        void LaneGroupTopoCheck::check_road_topo(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {
            for (const auto& conn_lg : lane_group2_conn_lg_) {
                bool check = false;
                auto pre_roads = CommonUtil::get_roads_by_lg(mapDataManager, conn_lg.first);
                auto lat_roads = CommonUtil::get_roads_by_lg(mapDataManager, conn_lg.second);

                if (!pre_roads.empty() && !lat_roads.empty()) {
                    // 判断道路是否连通
                    // 通过road id判断
                    for (const auto &pre : pre_roads) {
                        if (lat_roads.find(pre) != lat_roads.end()) {
                            // 连通的
                            check = true;
                            break;
                        }
                    }
                    // road node判断
                    if (!check) {
                         auto pre_road_nodes = get_road_nodes(mapDataManager, pre_roads, true);
                         auto lat_road_nodes = get_road_nodes(mapDataManager, lat_roads, false);
                         for (const auto &pre : pre_road_nodes) {
                             if (lat_road_nodes.find(pre) != lat_road_nodes.end()) {
                                 // 连通的
                                 check = true;
                                 break;
                             }
                         }
                    }
                } else {
                    // 不连通
                    check = false;
                }
                if (!check) {
                    // 错误输出
                    shared_ptr<DCError> ptr_error = DCLaneGroupTopoCheckError::createByKXS_04_001(conn_lg.first,
                                                                                                  conn_lg.second);
                    errorOutput->saveError(ptr_error);
                    int k = 0;
                }

            }
        }

        set<string> LaneGroupTopoCheck::get_road_nodes(const shared_ptr<MapDataManager> &mapDataManager,
                                                       const set<string> &roads, bool is_front) {
            set<string> ret_road_nodes_set;
            for (const auto &road : roads) {
                auto ptr_road = CommonUtil::get_road(mapDataManager, road);
                if (ptr_road) {
                    if (is_front) {
                        if (ret_road_nodes_set.find(ptr_road->f_node_id) == ret_road_nodes_set.end()) {
                            ret_road_nodes_set.insert(ptr_road->f_node_id);
                        }
                        if (ptr_road->direction_ == 1) {
                            if (ret_road_nodes_set.find(ptr_road->t_node_id) == ret_road_nodes_set.end()) {
                                ret_road_nodes_set.insert(ptr_road->t_node_id);
                            }
                        }
                    } else {
                        if (ret_road_nodes_set.find(ptr_road->t_node_id) == ret_road_nodes_set.end()) {
                            ret_road_nodes_set.insert(ptr_road->t_node_id);
                        }
                        if (ptr_road->direction_ == 1) {
                            if (ret_road_nodes_set.find(ptr_road->f_node_id) == ret_road_nodes_set.end()) {
                                ret_road_nodes_set.insert(ptr_road->f_node_id);
                            }
                        }
                    }
                } else {
                    LOG(ERROR) << "get road failed! road id :" << road;
                }
            }
            return ret_road_nodes_set;
        }

        bool LaneGroupTopoCheck::is_virtual_lane_group(const shared_ptr<MapDataManager> &mapDataManager,
                                                       const string &lane_group_id) {
            bool ret = false;
            auto ptr_lane_group = CommonUtil::get_lane_group(mapDataManager, lane_group_id);
            if (ptr_lane_group) {
                // 过滤路口
                if (ptr_lane_group->is_virtual_) {
                    virtual_lane_groups_.emplace_back(ptr_lane_group);
                    ret = true;
                }
            } else {
                LOG(ERROR) << "get lane group failed! lane group id : " << lane_group_id;
            }
            return ret;
        }

        void LaneGroupTopoCheck::release(shared_ptr<MapDataManager> mapDataManager) {
            mapDataManager->fnode_id2_dividers_maps_.clear();
            mapDataManager->tnode_id2_dividers_maps_.clear();
            mapDataManager->divider2_lane_groups_.clear();
        }
    }
}
