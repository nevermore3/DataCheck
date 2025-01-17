
#include <businesscheck/LaneGroupTopoCheck.h>

#include "businesscheck/LaneGroupTopoCheck.h"
#include "util/CommonUtil.h"
#include "util/dc_data_relation_util.h"

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
                if (CheckItemValid(CHECK_ITEM_KXS_ROAD_001) && mapDataManager->is_auto_road) {
                    check_road_topo(mapDataManager, errorOutput);
                }
                if(CheckItemValid(CHECK_ITEM_KXS_LANE_001)){
                    check_lane_topo(mapDataManager, errorOutput);
                }
                if(CheckItemValid(CHECK_ITEM_KXS_LG_027)) {
                    check_lane_group_depart_merge(mapDataManager, errorOutput);
                }
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
            // 过滤重复组
            set<string> tag_lane_group;
            for (const auto &div : dividers) {
                auto lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div.first);
                for (const auto &lg : lane_groups) {
                    if (tag_lane_group.find(lg) == tag_lane_group.end()) {
                        if (!is_virtual_lane_group(mapDataManager, lg)) {
                            auto ptr_lg = CommonUtil::get_lane_group(mapDataManager, lg);
                            get_conn_lane_groups(mapDataManager, ptr_lg);
                        }
                        tag_lane_group.insert(lg);
                    }
                }
            }

            for (auto topo_lg : mapDataManager->topo_lane_groups_) {
                if (topo_lg.second) {
                    for (auto from_lg : topo_lg.second->from_lanegroups_) {
                        lane_group2_conn_lg_.emplace(make_pair(to_string(topo_lg.first), to_string(from_lg->id_)));
                    }
                }
            }
        }

        void LaneGroupTopoCheck::check_road_topo(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {




            for (const auto &conn_lg : lane_group2_conn_lg_) {
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
                    auto onepair = lane_group2_conn_lg_.begin();
                    auto div_v = CommonUtil::get_dividers_by_lg(mapDataManager,conn_lg.first);
                    string taskid="";
                    if(div_v.size()>0){
                        taskid = div_v.front()->task_id_;
                    }
                    shared_ptr<DCError> ptr_error = DCLaneGroupTopoCheckError::createByKXS_04_001(conn_lg.first,
                                                                                                  conn_lg.second);
                    ptr_error->taskId_ = taskid;
                    errorOutput->saveError(ptr_error);
                }

            }

            errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_001,lane_group2_conn_lg_.size());
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

        bool LaneGroupTopoCheck::is_double_lane_group(shared_ptr<MapDataManager> mapDataManager,
                                                      const string &lane_group_id) {
            auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, lane_group_id);
            if (ptr_dividers.front()->direction_ == 1) {
                return true;
            }
            return false;
        }


        void LaneGroupTopoCheck::release(shared_ptr<MapDataManager> mapDataManager) {
            mapDataManager->fnode_id2_dividers_maps_.clear();
            mapDataManager->tnode_id2_dividers_maps_.clear();
            mapDataManager->divider2_lane_groups_.clear();
        }

        void LaneGroupTopoCheck::check_lane_topo(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {
            set<string> tag_f_lanes;
            for (const auto &conn_lg : lane_group2_conn_lg_) {
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
                        auto left_conn_dividers = get_conn_dividers(mapDataManager, conn_lg.second,
                                                                    ptr_dividers[left_divider_idx]->id_);

                        while (left_conn_dividers.empty()) {
                            left_divider_idx++;
                            right_divider_idx = left_divider_idx + 1;
                            if (right_divider_idx < ptr_dividers.size()) {
                                left_conn_dividers = get_conn_dividers(mapDataManager, conn_lg.second,
                                                                       ptr_dividers[left_divider_idx]->id_);
                            } else {
                                break;
                            }
                        }
                        if (right_divider_idx < 0 || right_divider_idx >= ptr_dividers.size()) {
                            continue;
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
                        if (right_divider_idx < 0 || right_divider_idx >= ptr_dividers.size()) {
                            continue;
                        }

                        if (left_conn_dividers.empty() || right_conn_dividers.empty()) {
                            break;
                        }

                        auto divider_no2_ptr_divider = get_conn_ptr_dividers(mapDataManager,
                                                                             left_conn_dividers,
                                                                             right_conn_dividers);

                        if (!divider_no2_ptr_divider.empty()) {
                            auto pre_ptr_lanes = CommonUtil::get_lanes_between_dividers(mapDataManager,
                                                                                        ptr_dividers[left_divider_idx],
                                                                                        ptr_dividers[right_divider_idx]);

                            auto lat_left_ptr_divider = divider_no2_ptr_divider.back();
                            auto lat_right_ptr_divider = divider_no2_ptr_divider.back();
                            auto lat_ptr_lanes = CommonUtil::get_lanes_between_dividers(mapDataManager,
                                                                                        lat_left_ptr_divider,
                                                                                        lat_right_ptr_divider);
                            if (pre_ptr_lanes.size() == lat_ptr_lanes.size()) {
//                            if (left_conn_dividers.size() == 1 && right_conn_dividers.size() == 1 && !is_depart) {
                                if (!is_lane_conn(mapDataManager, pre_ptr_lanes, lat_ptr_lanes)) {
                                    is_conn = false;
                                }
                            } else {
                                if (is_depart) {
                                    if (!is_lane_conn(mapDataManager, pre_ptr_lanes, lat_ptr_lanes, tag_f_lanes)) {
                                        is_conn = false;
                                    }
                                } else {
                                    if (!is_lane_conn_case(mapDataManager, pre_ptr_lanes, lat_ptr_lanes, tag_f_lanes)) {
                                        is_conn = false;
                                    }
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
            errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LANE_001,lane_group2_conn_lg_.size());
        }


        void LaneGroupTopoCheck::get_conn_lane_groups(shared_ptr<MapDataManager> mapDataManager,
                                                      const shared_ptr<DCLaneGroup> &ptr_lane_group) {
            set<string> ret_conn_lane_groups;
            auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, ptr_lane_group->id_);

            map<string, int> lane_group2_times;
            // key:lane group id value:divider关联对
            map<string, vector<pair<string, string>>> lane_group2_div2_div;
            for (const auto &ptr_div : ptr_dividers) {
                if (ptr_div->dividerNo_ == 0 && ptr_div->direction_ == 1) {
//                    bool is_front = (ptr_lane_group->direction_ == 1);
                    bool is_front = CommonUtil::check_dividers_same_direction(ptr_dividers[0],
                                                                              ptr_dividers[ptr_dividers.size() - 1]);
                    auto con_dividers = CommonUtil::get_ref_conn_divider(mapDataManager, ptr_lane_group->id_,
                                                                         ptr_div, is_front);
                    set<string> lane_group_tag;
                    for (const auto &div : con_dividers) {
                        auto conn_lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div->id_);
                        // 每次查找的DIVIDER连接组，最多添加一次
                        for (auto lg : conn_lane_groups) {
                            if (lane_group_tag.find(lg) == lane_group_tag.end()) {
                                auto times_iter = lane_group2_times.find(lg);
                                if (times_iter != lane_group2_times.end()) {
                                    times_iter->second++;
                                } else {
                                    int time = 1;
                                    lane_group2_times.insert(make_pair(lg, time));
                                }
                                auto lg2_div_iter = lane_group2_div2_div.find(lg);
                                if (lg2_div_iter != lane_group2_div2_div.end()) {
                                    lg2_div_iter->second.emplace_back(make_pair(ptr_div->id_, div->id_));
                                } else {
                                    vector<pair<string, string>> vec_div2_div;
                                    vec_div2_div.emplace_back(make_pair(ptr_div->id_, div->id_));
                                    lane_group2_div2_div.insert(make_pair(lg, vec_div2_div));
                                }
                                lane_group_tag.insert(lg);
                            }
                        }
                    }
                } else {
                    auto con_dividers = CommonUtil::get_ref_conn_divider(mapDataManager, ptr_lane_group->id_,
                                                                         ptr_div, true);
                    // 是否包含双向参考线
                    bool double_ref_div = false;
                    for (const auto &div : con_dividers) {
                        if (div->dividerNo_ == 0 && div->direction_ == 1) {
                            double_ref_div = true;
                            break;
                        }
                    }

                    if (is_double_lane_group(mapDataManager, ptr_lane_group->id_)) {
                        double_ref_div = true;
                    }
                    for (const auto &div : con_dividers) {
                        auto conn_lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, div->id_);
                        if (conn_lane_groups.size() == 1 && !double_ref_div) {
                            auto conn_lg = *conn_lane_groups.begin();
                            if (!is_double_lane_group(mapDataManager, conn_lg)) {
                                auto conn_lg_max_no = CommonUtil::GetMaxDividerNo(mapDataManager, conn_lg);
                                bool state = false;
                                // 如果不是最外侧DIVIDER连接最内侧DIVIDER
                                if (ptr_div->dividerNo_ == ptr_dividers.size() - 1 &&
                                    div->dividerNo_ != 0) {
                                    state = true;
                                } else if (ptr_div->dividerNo_ == 0 &&
                                           div->dividerNo_ != conn_lg_max_no &&
                                           div->dividerNo_ != 0) {
                                    state = true;
                                } else if (ptr_div->dividerNo_ != ptr_dividers.size() - 1 &&
                                           ptr_div->dividerNo_ != 0 &&
                                           div->dividerNo_ == 0) {
                                    state = true;
                                } else if (ptr_div->dividerNo_ != 0 &&
                                           div->dividerNo_ == conn_lg_max_no) {
                                    state = true;
                                }

                                if (state) {
                                    if (!is_virtual_lane_group(ptr_lane_group) &&
                                        !is_virtual_lane_group(mapDataManager, conn_lg)) {
//                                        lane_group2_conn_lg_.insert(make_pair(ptr_lane_group->id_, conn_lg));
                                        insert_divider2_conn_divider(ptr_div->id_, div->id_);
                                    }
                                }
                            }
                        }
                        for (auto lg : conn_lane_groups) {
                            auto times_iter = lane_group2_times.find(lg);
                            if (times_iter != lane_group2_times.end()) {
                                times_iter->second++;
                            } else {
                                int time = 1;
                                lane_group2_times.insert(make_pair(lg, time));
                            }
                            auto lg2_div_iter = lane_group2_div2_div.find(lg);
                            if (lg2_div_iter != lane_group2_div2_div.end()) {
                                lg2_div_iter->second.emplace_back(make_pair(ptr_div->id_, div->id_));
                            } else {
                                vector<pair<string, string>> vec_div2_div;
                                vec_div2_div.emplace_back(make_pair(ptr_div->id_, div->id_));
                                lane_group2_div2_div.insert(make_pair(lg, vec_div2_div));
                            }
                        }
                    }
                }
            }

            for (const auto &lg2_time : lane_group2_times) {
                if (lg2_time.second > 1) {
                    ret_conn_lane_groups.insert(lg2_time.first);
                    if (!is_virtual_lane_group(mapDataManager, lg2_time.first)) {
//                        lane_group2_conn_lg_.insert(make_pair(ptr_lane_group->id_, lg2_time.first));
                        auto lg2_div_iter = lane_group2_div2_div.find(lg2_time.first);
                        if (lg2_div_iter != lane_group2_div2_div.end()) {
                            for (auto div2_div : lg2_div_iter->second) {
                                insert_divider2_conn_divider(div2_div.first, div2_div.second);
                            }
                        }
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
        LaneGroupTopoCheck::get_conn_dividers(const shared_ptr<MapDataManager> &mapDataManager, string lane_group,
                                              string divider) {
            vector<shared_ptr<DCDivider>> ret_ptr_dividers;
            auto divider2_conn_iter = divider2_conn_dividers_maps_.find(divider);
            if (divider2_conn_iter != divider2_conn_dividers_maps_.end()) {
                for (const auto &div : divider2_conn_iter->second) {
                    auto ptr_divider = CommonUtil::get_divider(mapDataManager, div);
                    auto lane_groups = CommonUtil::get_lane_groups_by_divider(mapDataManager, ptr_divider->id_);
                    for (const auto &lg : lane_groups) {
                        if (lg == lane_group) {
                            if (ptr_divider) {
                                ret_ptr_dividers.emplace_back(ptr_divider);
                            } else {
                                LOG(ERROR) << "get divider failed! divider : " << div;
                            }
                        }
                    }
                }
            }
            return ret_ptr_dividers;
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

            map<long, shared_ptr<DCDivider>> left_divider_no2_ptr_divider;
            map<long, shared_ptr<DCDivider>> right_divider_no2_ptr_divider;
            if (left_ptr_dividers.size() > 1 && right_ptr_dividers.size() == 1) {
                auto right_lane_group = CommonUtil::get_lane_groups_by_divider(mapDataManager,
                                                                               right_ptr_dividers.front()->id_);
                for (const auto &div : left_ptr_dividers) {
                    auto left_lane_group = CommonUtil::get_lane_groups_by_divider(mapDataManager,
                                                                                  div->id_);
                    for (const auto &lg : left_lane_group) {
                        if (right_lane_group.find(lg) != right_lane_group.end()) {
                            left_divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
                        }
                    }
                }
                for (const auto &div : right_ptr_dividers) {
                    right_divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
                }
            } else {
                for (const auto &div : left_ptr_dividers) {
                    left_divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
                }
                for (const auto &div : right_ptr_dividers) {
                    right_divider_no2_ptr_divider.insert(make_pair(div->dividerNo_, div));
                }

            }

            for (const auto &div_no_iter : left_divider_no2_ptr_divider) {
                ret_ptr_dividers.emplace_back(div_no_iter.second);
            }

            for (const auto &div_no_iter : right_divider_no2_ptr_divider) {
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


        bool LaneGroupTopoCheck::is_lane_conn_case(const shared_ptr<MapDataManager> &mapDataManager,
                                                   const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                                                   const vector<shared_ptr<DCLane>> &lat_ptr_lanes,
                                                   set<string> &tag_f_lane) {
            bool ret = true;
            for (const auto &f_lane : lat_ptr_lanes) {
                bool flag = false;
                for (const auto &t_lane : pre_ptr_lanes) {
                    if (tag_f_lane.find(f_lane->id_) == tag_f_lane.end()) {
                        if (pre_lane_conn_pair_.find(make_pair(f_lane->id_, t_lane->id_)) !=
                            pre_lane_conn_pair_.end()) {
                            flag = true;
                            tag_f_lane.insert(f_lane->id_);
                            break;
                        }
                    } else {
                        flag = true;
                    }
                }
                if (!flag) {
                    ret = false;
                }
            }

            return ret;
        }

        void LaneGroupTopoCheck::check_lane_group_depart_merge(shared_ptr<MapDataManager> mapDataManager,
                                                               shared_ptr<CheckErrorOutput> errorOutput) {
            // 通行方向后面的在前
            map<string, set<string>> f_lane_group2_conn_lg;
            // 通行方向前面的在前
            map<string, set<string>> t_lane_group2_conn_lg;
            for (const auto &pairLg : lane_group2_conn_lg_) {
                if(virtual_lane_groups_maps_.find(pairLg.first)!=virtual_lane_groups_maps_.end()){
                    continue;
                }

                auto f_lg_iter = f_lane_group2_conn_lg.find(pairLg.first);

                if (f_lg_iter != f_lane_group2_conn_lg.end()) {
                    f_lg_iter->second.insert(pairLg.second);
                } else {
                    set<string> conn_lg_set;
                    conn_lg_set.insert(pairLg.second);
                    f_lane_group2_conn_lg.insert(make_pair(pairLg.first, conn_lg_set));
                }
                auto t_lg_iter = t_lane_group2_conn_lg.find(pairLg.second);
                if (t_lg_iter != t_lane_group2_conn_lg.end()) {
                    t_lg_iter->second.insert(pairLg.first);
                } else {
                    set<string> conn_lg_set;
                    conn_lg_set.insert(pairLg.first);
                    t_lane_group2_conn_lg.insert(make_pair(pairLg.second, conn_lg_set));
                }
            }
            auto onepair = lane_group2_conn_lg_.begin();
            vector<shared_ptr<DCDivider>> div_v = CommonUtil::get_dividers_by_lg(mapDataManager,onepair->first);
            string taskid="";
            if(div_v.size()>0){
                taskid = div_v[0]->task_id_;
            }

            for (auto f_lg2_conn_lg : f_lane_group2_conn_lg) {
                // 存在进入车道
                if (f_lg2_conn_lg.second.size() > 1) {
                    auto t_lg2_conn_lg_iter = t_lane_group2_conn_lg.find(f_lg2_conn_lg.first);
                    if (t_lg2_conn_lg_iter != t_lane_group2_conn_lg.end()) {
                        if (t_lg2_conn_lg_iter->second.size() > 1) {
                            // 错误输出
                            shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_027(
                                    f_lg2_conn_lg.first);
                            ptr_error->taskId_ = taskid;
                            errorOutput->saveError(ptr_error);
                        }
                    }
                }
            }

            errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LG_027,lane_group2_conn_lg_.size());
        }
    }
}
