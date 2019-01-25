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
                check_lane_topo(mapDataManager, errorOutput);
            } catch (exception &e) {
                LOG(ERROR) << e.what();
                ret = false;
            }

            release(mapDataManager);
            return ret;
        }

        void LaneGroupTopoCheck::pre_divider_topo(shared_ptr<MapDataManager> mapDataManager) {
            const auto lane_connectivitys = mapDataManager->laneConnectivitys_;
            for (const auto &lane_conn : lane_connectivitys) {
                pre_lane_conn_pair_.insert(make_pair(to_string(lane_conn.second->fLaneId_),
                                                     to_string(lane_conn.second->tLaneId_)));
            }
            const auto &dividers = mapDataManager->dividers_;
            for (const auto &div : dividers) {
                auto lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div.first);
                for (const auto &lg : lane_groups) {
                    if (!is_virtual_lane_group(mapDataManager, lg)) {
                        auto ptr_lg = CommonUtil::get_lane_group(mapDataManager, lg);
                        get_conn_lane_groups(mapDataManager, ptr_lg);
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
                    // 虚拟组
                    if (virtual_lane_groups_maps_.find(lane_group_id) == virtual_lane_groups_maps_.end()) {
                        virtual_lane_groups_maps_.insert(make_pair(lane_group_id, ptr_lane_group));
                    }
                    ret = true;
                }
            } else {
                LOG(ERROR) << "get lane group failed! lane group id : " << lane_group_id;
            }
            return ret;
        }

        bool LaneGroupTopoCheck::is_virtual_lane_group(const shared_ptr<DCLaneGroup> &ptr_lane_group) {
            bool ret = false;
            // 过滤路口
            if (ptr_lane_group->is_virtual_) {
                // 虚拟组
                if (virtual_lane_groups_maps_.find(ptr_lane_group->id_) == virtual_lane_groups_maps_.end()) {
                    virtual_lane_groups_maps_.insert(make_pair(ptr_lane_group->id_, ptr_lane_group));
                }
                ret = true;
            }
            return ret;
        }


        void LaneGroupTopoCheck::release(shared_ptr<MapDataManager> mapDataManager) {
            mapDataManager->fnode_id2_dividers_maps_.clear();
            mapDataManager->tnode_id2_dividers_maps_.clear();
            mapDataManager->divider2_lane_groups_.clear();
        }

        void LaneGroupTopoCheck::check_lane_topo(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {
            set<string> tag_f_lanes;
            for (const auto& conn_lg : lane_group2_conn_lg_) {
                // 过lane group 获取全部通divider
                auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, conn_lg.first);
                bool is_conn = true;
                if (ptr_dividers.size() > 1) {
                    auto left_divider_idx = 0;
                    auto right_divider_idx = (left_divider_idx + 1);

                    while (right_divider_idx < ptr_dividers.size()) {
                        bool is_normal = false;
                        bool is_depart = false;

                        // pre组有分离点
                        if (ptr_dividers[left_divider_idx]->fromNodeId_ ==
                            ptr_dividers[right_divider_idx]->fromNodeId_) {
                            right_divider_idx++;
                            is_depart = true;
                        }
                        if ((right_divider_idx + 1) < ptr_dividers.size()) {
                            if (ptr_dividers[right_divider_idx]->fromNodeId_ ==
                                    ptr_dividers[right_divider_idx + 1]->fromNodeId_) {
                                right_divider_idx++;
                                is_depart = true;
                            }
                        }
                        auto left_conn_dividers = get_conn_dividers(mapDataManager,
                                                                    ptr_dividers[left_divider_idx]->id_);

                        while (left_conn_dividers.empty()) {
                            left_divider_idx++;
                            right_divider_idx = left_divider_idx + 1;
                            if (right_divider_idx < ptr_dividers.size()) {
                                left_conn_dividers = get_conn_dividers(mapDataManager,
                                                                       ptr_dividers[left_divider_idx]->id_);
                            } else {
                                break;
                            }
                        }
                        auto right_conn_dividers = get_conn_dividers(mapDataManager,
                                                                     ptr_dividers[right_divider_idx]->id_);
                        while (right_conn_dividers.empty()) {
                            right_divider_idx++;
                            if (right_divider_idx < ptr_dividers.size()) {
                                right_conn_dividers = get_conn_dividers(mapDataManager,
                                                                       ptr_dividers[right_divider_idx]->id_);
                            } else {
                                break;
                            }
                        }

                        if (left_conn_dividers.empty() || right_conn_dividers.empty()) {
                            break;
                        }

                        auto divider_no2_ptr_divider = get_conn_ptr_dividers(mapDataManager,
                                                                             left_conn_dividers,
                                                                             right_conn_dividers);
//                        auto divider_no2_ptr_divider = get_conn_ptr_dividers(mapDataManager,
//                                                                             ptr_dividers[left_divider_idx],
//                                                                             ptr_dividers[right_divider_idx],
//                                                                             is_normal);

                        if (!divider_no2_ptr_divider.empty()) {
                            auto pre_ptr_lanes = CommonUtil::get_lanes_between_dividers(mapDataManager,
                                                                                        ptr_dividers[left_divider_idx],
                                                                                        ptr_dividers[right_divider_idx]);

                            auto lat_left_ptr_divider = divider_no2_ptr_divider.front();
                            auto lat_right_ptr_divider = divider_no2_ptr_divider.back();
                            auto lat_ptr_lanes = CommonUtil::get_lanes_between_dividers(mapDataManager,
                                                                                        lat_left_ptr_divider,
                                                                                        lat_right_ptr_divider);

                            if (left_conn_dividers.size() == 1 && right_conn_dividers.size() == 1 && !is_depart) {
                                if (!is_lane_conn(mapDataManager, pre_ptr_lanes, lat_ptr_lanes)) {
                                    is_conn = false;
                                }
                            } else {
                                if (!is_lane_conn(mapDataManager, pre_ptr_lanes, lat_ptr_lanes, tag_f_lanes)) {
                                    is_conn = false;
                                }
                            }

                        }

                        left_divider_idx = right_divider_idx;
                        right_divider_idx++;
                    }
                } else {
                    LOG(ERROR) << "divider size < 2, lane group :" << conn_lg.first;
                }

                if (!is_conn) {
                    shared_ptr<DCError> ptr_error = DCLaneGroupTopoCheckError::createByKXS_05_001(conn_lg.first,
                                                                                                  conn_lg.second);
                    errorOutput->saveError(ptr_error);
                }
            }
        }


        void LaneGroupTopoCheck::get_conn_lane_groups(shared_ptr<MapDataManager> mapDataManager,
                                                      const shared_ptr<DCLaneGroup> &ptr_lane_group) {
            set<string> ret_conn_lane_groups;
            auto insert_lane_group_times = [&](map<string, int> &lane_group2_times, const string &div) {
                auto conn_lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div);
                for (auto lg : conn_lane_groups) {
                    auto times_iter = lane_group2_times.find(lg);
                    if (times_iter != lane_group2_times.end()) {
                        times_iter->second++;
                    } else {
                        int time = 1;
                        lane_group2_times.insert(make_pair(lg, time));
                    }
                }
            };

            auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, ptr_lane_group->id_);

            map<string, int> lane_group2_times;
            for (const auto &ptr_div : ptr_dividers) {
                if (ptr_div->dividerNo_ == 0 && ptr_div->direction_ == 1) {
                    bool is_front = (ptr_lane_group->direction_ == 1);
                    auto con_dividers = CommonUtil::get_ref_conn_divider(mapDataManager, ptr_lane_group->id_,
                                                                         ptr_div, is_front);
                    for (const auto &div : con_dividers) {
                        insert_divider2_conn_divider(ptr_div->id_, div);
                        insert_lane_group_times(lane_group2_times, div);
                    }
                } else {
                    auto con_dividers = CommonUtil::get_conn_divider(mapDataManager, ptr_div, true);
                    for (const auto &div : con_dividers) {
                        insert_divider2_conn_divider(ptr_div->id_, div);
                        insert_lane_group_times(lane_group2_times, div);
                    }
                }
            }

            for (const auto &lg2_time : lane_group2_times) {
                if (lg2_time.second > 1) {
                    ret_conn_lane_groups.insert(lg2_time.first);
                    if (!is_virtual_lane_group(mapDataManager, lg2_time.first)) {
                        lane_group2_conn_lg_.insert(make_pair(ptr_lane_group->id_, lg2_time.first));
                    }
                }
            }
        }

        void LaneGroupTopoCheck::insert_divider2_conn_divider(string divider, string con_divider) {
            auto divider2_conn_iter = divider2_conn_dividers_maps_.find(divider);
            if (divider2_conn_iter != divider2_conn_dividers_maps_.end()) {
                divider2_conn_iter->second.insert(con_divider);
            } else {
                set<string> conn_dividers;
                conn_dividers.insert(con_divider);
                divider2_conn_dividers_maps_.insert(make_pair(divider, conn_dividers));
            }
        }

        vector<shared_ptr<DCDivider>>
        LaneGroupTopoCheck::get_conn_dividers(const shared_ptr<MapDataManager> &mapDataManager, string divider) {
            vector<shared_ptr<DCDivider>> ret_ptr_dividers;
            auto divider2_conn_iter = divider2_conn_dividers_maps_.find(divider);
            if (divider2_conn_iter != divider2_conn_dividers_maps_.end()) {
                for (const auto &div : divider2_conn_iter->second) {
                    auto ptr_divider = CommonUtil::get_divider(mapDataManager, div);
                    if (ptr_divider) {
                        ret_ptr_dividers.emplace_back(ptr_divider);
                    } else {
                        LOG(ERROR) << "get divider failed! divider : " << div;
                    }
                }
            }
            return ret_ptr_dividers;
        }

        vector<shared_ptr<DCDivider>>
        LaneGroupTopoCheck::get_conn_ptr_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                  const vector<shared_ptr<DCDivider>> &left_ptr_dividers,
                                                  const vector<shared_ptr<DCDivider>> &right_ptr_dividers) {
            vector<shared_ptr<DCDivider>> ret_ptr_dividers;

            map<long, shared_ptr<DCDivider>> divider_no2_ptr_divider;
            for (const auto &div : left_ptr_dividers) {
                divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
            }
            for (const auto &div : right_ptr_dividers) {
                divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
            }
            for (const auto &div_no_iter : divider_no2_ptr_divider) {
                ret_ptr_dividers.emplace_back(div_no_iter.second);
            }
            return ret_ptr_dividers;
        }

        bool LaneGroupTopoCheck::is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes,
                                              set<string> &tag_f_lane) {
            bool ret = true;
            for (const auto &f_lane : lat_ptr_lanes) {
                for (const auto &t_lane : pre_ptr_lanes) {
                    if (tag_f_lane.find(f_lane->id_) == tag_f_lane.end()) {
                        if (pre_lane_conn_pair_.find(make_pair(f_lane->id_, t_lane->id_)) ==
                            pre_lane_conn_pair_.end()) {
                            ret = false;
                        }
                        tag_f_lane.insert(f_lane->id_);
                    }
                }
            }

            return ret;
        }

        bool LaneGroupTopoCheck::is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes) {
            bool ret = true;
            if (pre_ptr_lanes.size() == lat_ptr_lanes.size()) {
                auto pre_ptr_lanes_iter = pre_ptr_lanes.begin();
                auto lat_ptr_lanes_iter = lat_ptr_lanes.begin();
                while (pre_ptr_lanes_iter != pre_ptr_lanes.end() && lat_ptr_lanes_iter != lat_ptr_lanes.end()) {
                    auto f_lane = *lat_ptr_lanes_iter;
                    auto t_lane = *pre_ptr_lanes_iter;
                    if (pre_lane_conn_pair_.find(make_pair(f_lane->id_, t_lane->id_)) == pre_lane_conn_pair_.end()) {
                        ret = false;
                    }
                    pre_ptr_lanes_iter++;
                    lat_ptr_lanes_iter++;
                }
            } else {
                ret = false;
            }


            return ret;
        }
    }
}
