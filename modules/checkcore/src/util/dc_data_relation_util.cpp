//
// Created by zhangxingang on 19-8-15.
//

#include <util/dc_data_relation_util.h>
#include <util/CommonUtil.h>
#include <mvg/Coordinates.hpp>
#include <util/GeosObjUtil.h>

namespace kd {
    namespace dc {
        bool DCDataRelationUtil::BuildTopoLaneGroup(shared_ptr<MapDataManager> data_manager,
                                                    unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups) {
            unordered_map<long, vector<shared_ptr<DCDivider>>> ordered_lanegroup_;
            for (auto lane_group : data_manager->laneGroups_) {

                vector<shared_ptr<DCDivider>> dividers = CommonUtil::get_dividers_by_lg(data_manager, lane_group.first);

                ordered_lanegroup_.emplace(stol(lane_group.first), dividers);
            }

            unordered_map<long, set<long>> divider_lanegroup_maps;
            for (auto lane_group_it : data_manager->divider2_lane_groups_) {
                set<long> temp_lane_groups;
                for (auto lane_group : lane_group_it.second) {
                    temp_lane_groups.emplace(stol(lane_group));
                }
                divider_lanegroup_maps.emplace(stol(lane_group_it.first), temp_lane_groups);
            }

            //建立空的拓扑lanegroup，同时构建车道线之间的方向关系
            for (auto lgit : ordered_lanegroup_) {
                shared_ptr<TopoLaneGroup> topo_lane_group = make_shared<TopoLaneGroup>();
                topo_lane_group->id_ = lgit.first;
                topo_lane_group->dividers_ = lgit.second;

                //构建车道线之间的方向关系，以便后面生成车道中心线或道路使用
                if (topo_lane_group->BuildDividerDirectionInfo()) {
                    topo_lane_groups.emplace(make_pair(topo_lane_group->id_, topo_lane_group));
                }
            }

            //建立节点与车道线的关联关系
            unordered_map<long, shared_ptr<TopoDividerNodeExt>> topo_divider_nodes_;
            for (auto lgit : ordered_lanegroup_) {
                for (const auto &divider : lgit.second) {
                    buildDivider2NodeTopo(divider, divider->nodes_.front(), true, topo_divider_nodes_);
                    buildDivider2NodeTopo(divider, divider->nodes_.back(), false, topo_divider_nodes_);
                }
            }

            //建立所有分组之间的关联关系
            for (auto laneGroup : ordered_lanegroup_) {
                shared_ptr<TopoLaneGroup> topo_lanegroup = GetTopoLaneGroup(topo_lane_groups, laneGroup.first); //此处肯定会有
                // 如果数据存在问题，会出现nullptr，从而引起崩溃问题
                if (topo_lanegroup == nullptr) {
                    LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << laneGroup.first;
                    continue;
                }

                //根据相邻的两个车道线，查找其相邻的车道组
                set<long> from_groups, to_groups;
                const vector<shared_ptr<DCDivider>> &dividers = laneGroup.second;
                for (int i = 0; i < dividers.size() - 1; i++) {
                    getLaneGroupRef(i, dividers.size(), topo_lanegroup, topo_lane_groups, from_groups, to_groups,
                                    topo_divider_nodes_, divider_lanegroup_maps);
                }

                for (auto lg : from_groups) {
                    shared_ptr<TopoLaneGroup> from_topo_lg = GetTopoLaneGroup(topo_lane_groups, lg); //此处肯定会有
                    if (from_topo_lg != nullptr) {
                        topo_lanegroup->from_lanegroups_.emplace_back(from_topo_lg);
                    } else {
                        LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << laneGroup.first;
                    }
                }

                for (auto lg : to_groups) {
                    shared_ptr<TopoLaneGroup> to_topo_lg = GetTopoLaneGroup(topo_lane_groups, lg); //此处肯定会有
                    if (to_topo_lg != nullptr) {
                        topo_lanegroup->to_lanegroups_.emplace_back(to_topo_lg);
                    } else {
                        LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << laneGroup.first;
                    }
                }
            }

            //数据关联一致性检查，需要互为from/to
            for (auto laneGroup : ordered_lanegroup_) {
                shared_ptr<TopoLaneGroup> topo_lanegroup = GetTopoLaneGroup(topo_lane_groups, laneGroup.first); //此处肯定会有
                // 如果数据存在问题，会出现nullptr，从而引起崩溃问题
                if (topo_lanegroup == nullptr) {
                    LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << laneGroup.first;
                    continue;
                }

                for(shared_ptr<TopoLaneGroup> from_topo_lg : topo_lanegroup->from_lanegroups_){
                    if(!from_topo_lg->FindTopoLaneGroup(topo_lanegroup, false)){
                        from_topo_lg->AppendTopoLaneGroup(topo_lanegroup, false);
                    }
                }

                for(shared_ptr<TopoLaneGroup> to_topo_lg : topo_lanegroup->to_lanegroups_){
                    if(!to_topo_lg->FindTopoLaneGroup(topo_lanegroup, true)){
                        to_topo_lg->AppendTopoLaneGroup(topo_lanegroup, true);
                    }
                }
            }

            //标记出所有的普通分组、出口分组、入口分组
            for (auto laneGroup : ordered_lanegroup_) {
                shared_ptr<TopoLaneGroup> topo_lanegroup = topo_lane_groups[laneGroup.first];
                //此处肯定会有
                if (topo_lanegroup== nullptr) {
                    continue;
                }
                if (topo_lanegroup->conn_type_ == CONNT_INTERSECT) {
                    continue;
                }

                int from_lg_count = topo_lanegroup->from_lanegroups_.size();
                int to_lg_count = topo_lanegroup->to_lanegroups_.size();

                //根据联通关系对车道组进行分类
                if ((from_lg_count == to_lg_count && from_lg_count == 1) || //from, to 各连一个
                    (from_lg_count == 1 && to_lg_count == 0) ||   //from 连一个， to 无
                    (from_lg_count == 0 && to_lg_count == 1) ||   //from 无， to连一个
                    (from_lg_count == 0 && to_lg_count == 0)) {   //两天都无分组关联，如连接虚拟路口的分组
                    topo_lanegroup->conn_type_ = CONNT_NORMAL;
                } else if (from_lg_count >= 2 && to_lg_count <= 1) {
                    if (checkLaneGroupIntersect(topo_lanegroup->from_lanegroups_)) {
                        topo_lanegroup->conn_type_ = CONNT_MERGE_INTERSECT;
                    } else {
                        topo_lanegroup->conn_type_ = CONNT_MERGE;
                    }
                } else if (from_lg_count <= 1 && to_lg_count >= 2) {
                    if (checkLaneGroupIntersect(topo_lanegroup->to_lanegroups_)) {
                        topo_lanegroup->conn_type_ = CONNT_SPLIT_INTERSECT;
                    } else {
                        topo_lanegroup->conn_type_ = CONNT_SPLIT;
                    }
                } else if (from_lg_count >= 2 && to_lg_count >= 2) {
                    if (checkLaneGroupIntersect(topo_lanegroup->from_lanegroups_) &&
                            checkLaneGroupIntersect(topo_lanegroup->to_lanegroups_)) {
                        topo_lanegroup->conn_type_ = CONNT_MULTI_INTERSECT;
                    } else if (checkLaneGroupIntersect(topo_lanegroup->from_lanegroups_)) {
                        topo_lanegroup->conn_type_ = CONNT_SPLIT;
                    } else if (checkLaneGroupIntersect(topo_lanegroup->to_lanegroups_)) {
                        topo_lanegroup->conn_type_ = CONNT_MERGE;
                    } else {
                        topo_lanegroup->conn_type_ = CONNT_MULTI;
                        LOG(WARNING) << "lane group " << laneGroup.first << " type is CONNT_MULTI.";
                    }
                } else {
                    LOG(WARNING) << "unhandle lanegroup type " << laneGroup.first << ", from_group size " << from_lg_count
                                 << ", to_group size " << to_lg_count << endl;
                    return false;
                }
            }

            //找出所有的双向车道组
            for (auto divgroupit : divider_lanegroup_maps) {
                if (divgroupit.second.size() > 1) {
                    for (long lgid : divgroupit.second) {
                        shared_ptr<TopoLaneGroup> topo_lanegroup = GetTopoLaneGroup(topo_lane_groups, lgid);
                        if (topo_lanegroup) {
                            if (topo_lanegroup->conn_type_ == CONNT_INTERSECT) {
                                topo_lanegroup->conn_type_ = CONNT_DUAL_INTERSECT;
                            } else {
                                topo_lanegroup->conn_type_ = CONNT_DUAL;
                            }
                        }
                    }
                }
            }

            // 分组关系排序
            for (auto laneGroup : ordered_lanegroup_) {
                shared_ptr<TopoLaneGroup> topo_lanegroup = GetTopoLaneGroup(topo_lane_groups, laneGroup.first);
                if (topo_lanegroup == nullptr) {
                    LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << laneGroup.first;
                    continue;
                }

                if (!orderLaneGroups(topo_lanegroup, true, topo_lane_groups)) {
                    return false;
                }

                if (!orderLaneGroups(topo_lanegroup, false, topo_lane_groups)) {
                    return false;
                }
            }

            return false;
        }

        void DCDataRelationUtil::buildDivider2NodeTopo(shared_ptr<DCDivider> divider, shared_ptr<DCDividerNode> node,
                                                       bool from_or_to,
                                                       unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_) {
            shared_ptr<TopoDividerNodeExt> topo_node = nullptr;
            auto topo_node_it = topo_divider_nodes_.find(stol(node->id_));
            if (topo_node_it == topo_divider_nodes_.end()) {
                topo_node = make_shared<TopoDividerNodeExt>();
                topo_node->node_ = node;
                topo_divider_nodes_.insert(make_pair(stol(node->id_), topo_node));
            } else {
                topo_node = topo_node_it->second;
            }

            if (topo_node == nullptr) {
                LOG(ERROR) << "create or find topo node error.";
                return;
            }

            long direction = divider->direction_;
            if (direction == 1) {
                topo_node->from_dividers_.insert(make_pair(stol(divider->id_), divider));
                topo_node->to_dividers_.insert(make_pair(stol(divider->id_), divider));
            } else {
                if (from_or_to) {
                    topo_node->from_dividers_.insert(make_pair(stol(divider->id_), divider));
                } else {
                    topo_node->to_dividers_.insert(make_pair(stol(divider->id_), divider));
                }
            }
        }

        shared_ptr<TopoLaneGroup>
        DCDataRelationUtil::GetTopoLaneGroup(unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups,
                                             long lane_group) {
            auto lgit = topo_lane_groups.find(lane_group);
            if (lgit != topo_lane_groups.end()) {
                return lgit->second;
            }
            return nullptr;
        }

        void
        DCDataRelationUtil::getLaneGroupRef(int left_div_index, int right_div_limit, shared_ptr<TopoLaneGroup> topo_lg,
                                            unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                            set<long> &from_groups, set<long> &to_groups,
                                            unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                            unordered_map<long, set<long>> &divider_lanegroup_maps_) {
            shared_ptr<DCDividerNode> left_start_node = topo_lg->GetDividerNormalNode(left_div_index, true);
            shared_ptr<DCDividerNode> left_end_node = topo_lg->GetDividerNormalNode(left_div_index, false);

            for (int right_div_index = left_div_index + 1; right_div_index < right_div_limit; right_div_index++) {
                shared_ptr<DCDividerNode> right_start_node = topo_lg->GetDividerNormalNode(right_div_index, true);
                shared_ptr<DCDividerNode> right_end_node = topo_lg->GetDividerNormalNode(right_div_index, false);

                set<long> expect_div_ids;
                expect_div_ids.insert(stol(topo_lg->dividers_[left_div_index]->id_));
                expect_div_ids.insert(stol(topo_lg->dividers_[right_div_index]->id_));

                if (left_start_node && right_start_node) {
                    long left_from_node_id = stol(left_start_node->id_);
                    long right_from_node_id = stol(right_start_node->id_);
                    if(left_from_node_id != right_from_node_id){
                        getLanegroupRefAccordFromNode(topo_lg, topo_lanegroups, left_from_node_id, right_from_node_id, from_groups,
                                                      topo_divider_nodes_, divider_lanegroup_maps_,expect_div_ids);
                    }
                }

                if (left_end_node && right_end_node) {
                    long left_to_node_id = stol(left_end_node->id_);
                    long right_to_node_id = stol(right_end_node->id_);
                    if(left_to_node_id != right_to_node_id){
                        getLanegroupRefAccordToNode(topo_lg, topo_lanegroups, left_to_node_id, right_to_node_id, to_groups,
                                                    topo_divider_nodes_, divider_lanegroup_maps_,expect_div_ids);
                    }
                }
            }
        }

        void DCDataRelationUtil::getLanegroupRefAccordFromNode(shared_ptr<TopoLaneGroup> topo_lg,
                                                               unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                               long nodeId1, long nodeId2, set<long> &groups,
                                                               unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                                               unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                               const set<long> &expect_div_ids) {
            auto divnode1it = topo_divider_nodes_.find(nodeId1);
            auto divnode2it = topo_divider_nodes_.find(nodeId2);
            if (divnode1it == topo_divider_nodes_.end() || divnode1it == topo_divider_nodes_.end()) {
                return;
            }

            shared_ptr<TopoDividerNodeExt> node1 = divnode1it->second;
            shared_ptr<TopoDividerNodeExt> node2 = divnode2it->second;

            findGroupAccrodToDividers(topo_lg, topo_lanegroups, node1, node2, groups,
                                      divider_lanegroup_maps_, expect_div_ids, true);
        }

        void DCDataRelationUtil::getLanegroupRefAccordToNode(shared_ptr<TopoLaneGroup> topo_lg,
                                                             unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                             long nodeId1, long nodeId2, set<long> &groups,
                                                             unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                                             unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                             const set<long> &expect_div_ids) {
            auto divnode1it = topo_divider_nodes_.find(nodeId1);
            auto divnode2it = topo_divider_nodes_.find(nodeId2);
            if (divnode1it == topo_divider_nodes_.end() || divnode1it == topo_divider_nodes_.end()) {
                return;
            }

            shared_ptr<TopoDividerNodeExt> node1 = divnode1it->second;
            shared_ptr<TopoDividerNodeExt> node2 = divnode2it->second;

            findGroupAccrodToDividers(topo_lg, topo_lanegroups, node1, node2, groups,
                                      divider_lanegroup_maps_, expect_div_ids, false);
        }

        void DCDataRelationUtil::findGroupAccrodToDividers(shared_ptr<TopoLaneGroup> topo_lg,
                                                           unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                           shared_ptr<TopoDividerNodeExt> node1,
                                                           shared_ptr<TopoDividerNodeExt> node2, set<long> &groups,
                                                           unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                           const set<long> &expect_div_ids, bool is_from) {
            if (is_from) {
                if(node1->to_dividers_.size() == 0 || node2->to_dividers_.size() == 0){
                    return;
                }
            } else {
                if(node1->from_dividers_.size() == 0 || node2->from_dividers_.size() == 0){
                    return;
                }
            }

            set<long> group1, group2;

            if (is_from) {
                findGroupAccordDivider(topo_lg, topo_lanegroups, node1->to_dividers_, node1->from_dividers_, group1,
                                       divider_lanegroup_maps_, expect_div_ids);
                findGroupAccordDivider(topo_lg, topo_lanegroups, node2->to_dividers_, node2->from_dividers_, group2,
                                       divider_lanegroup_maps_, expect_div_ids);
            } else {
                findGroupAccordDivider(topo_lg, topo_lanegroups, node1->from_dividers_, node1->to_dividers_, group1,
                                       divider_lanegroup_maps_, expect_div_ids);
                findGroupAccordDivider(topo_lg, topo_lanegroups, node2->from_dividers_, node2->to_dividers_, group2,
                                       divider_lanegroup_maps_, expect_div_ids);
            }


            bool insert = false;
            for (long groupid : group1) {
                auto groupit = group2.find(groupid);
                if (groupit != group2.end()) {
                    insert = true;
                    groups.insert(groupid);
                }
            }

            if (!insert) {
                groups.insert(group1.begin(), group1.end());
                groups.insert(group2.begin(), group2.end());
            }
        }

        void DCDataRelationUtil::findGroupAccordDivider(shared_ptr<TopoLaneGroup> topo_lg,
                                                        unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                        map<long, shared_ptr<DCDivider>> &dividers,
                                                        map<long, shared_ptr<DCDivider>> &conn_dividers,
                                                        set<long> &groups,
                                                        unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                        const set<long> &expect_div_ids) {
            for (auto itemit : dividers) {
                long divid = itemit.first;

                if(expect_div_ids.find(divid) != expect_div_ids.end()){
                    //车道线为双向时，需要排除掉自身
                    continue;
                }

                auto div_lg_it = divider_lanegroup_maps_.find(divid);
                if (div_lg_it != divider_lanegroup_maps_.end()) {
                    set<long> groups_temp = div_lg_it->second;
                    if (groups_temp.size() > 1) {
                        // 判断是否参考线连接，并判断方向
                        int conn_direction = 0;
                        for (const auto &divider : topo_lg->dividers_) {
                            auto conn_iter = conn_dividers.find(stol(divider->id_));
                            if (conn_iter != conn_dividers.end()) {
                                shared_ptr<DCDivider> conn_divider = conn_iter->second;
                                if (conn_divider->direction_ == 1) {
                                    // 判断起终点确定方向
                                    if (conn_divider->nodes_.front()->id_ == itemit.second->nodes_.back()->id_ ||
                                        conn_divider->nodes_.back()->id_ == itemit.second->nodes_.front()->id_) {
                                        conn_direction = 1;
                                    } else {
                                        conn_direction = 2;
                                    }
                                }
                            }
                        }

                        for (long groupId : groups_temp) {
                            //查找通行方向一致的组
                            shared_ptr<TopoLaneGroup> conn_topo_lg = GetTopoLaneGroup(topo_lanegroups, groupId);
                            if (conn_topo_lg) {
                                // 参考线方向相同
                                if (conn_direction == 1) {
                                    if (!(topo_lg->dir_infos_.front() ^ conn_topo_lg->dir_infos_.front())) {
                                        groups.insert(groupId);
                                    }
                                } else if (conn_direction == 2) {
                                    if (topo_lg->dir_infos_.front() ^ conn_topo_lg->dir_infos_.front()) {
                                        groups.insert(groupId);
                                    }
                                } else {
                                    groups.insert(groupId);
                                }
                            } else {
                                LOG(ERROR) << "GetTopoLaneGroup failed! lane group : " << groupId;
                            }

                        }
                    } else {
                        for (long groupId : groups_temp) {
                            groups.insert(groupId);
                        }
                    }
                }
            }
        }

        bool DCDataRelationUtil::checkLaneGroupIntersect(const vector<shared_ptr<TopoLaneGroup>> &topo_lane_groups) {
            bool ret = true;
            for (auto topo_lg : topo_lane_groups) {
                if (topo_lg->conn_type_ != CONNT_INTERSECT && topo_lg->conn_type_ != CONNT_DUAL_INTERSECT) {
                    ret = false;
                }
            }

            return ret;
        }

        bool DCDataRelationUtil::orderLaneGroups(shared_ptr<TopoLaneGroup> cur_topolg, bool start,
                                                 unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups) {
            vector<pair<shared_ptr<TopoLaneGroup>, int>> lanespanes;

            if (start) {
                if (!cur_topolg->from_lanegroups_.empty()) {
                    for (auto from_topo_lg : cur_topolg->from_lanegroups_) {
                        int weight = cur_topolg->GetLaneSpanWeight(from_topo_lg, start);
                        lanespanes.emplace_back(make_pair(from_topo_lg, weight));
                    }

                    //排序
                    sort(lanespanes.begin(), lanespanes.end(),
                         [](pair<shared_ptr<TopoLaneGroup>,int> & firstRec, pair<shared_ptr<TopoLaneGroup>,int> & secondRec) {
                             if (firstRec.second < secondRec.second) {
                                 return true;
                             } else {
                                 return false;
                             };
                         });
                    cur_topolg->from_lanegroups_.clear();

                    for (auto lanespane_it : lanespanes) {
                        cur_topolg->from_lanegroups_.emplace_back(lanespane_it.first);
                    }

                    for (auto from_topo_lg : cur_topolg->from_lanegroups_) {
                        double start_angle = getLaneGroupDirection(from_topo_lg, true);
                        double end_angle = getLaneGroupDirection(from_topo_lg, false);
                        long position = getLaneGroupPosition(start_angle, end_angle);
                        cur_topolg->f_lane_group_position_.emplace(from_topo_lg->id_, position);
                    }

                }
            } else {
                if (!cur_topolg->to_lanegroups_.empty()) {
                    for (auto to_topo_lg : cur_topolg->to_lanegroups_) {
                        int weight = cur_topolg->GetLaneSpanWeight(to_topo_lg, start);
                        lanespanes.emplace_back(make_pair(to_topo_lg, weight));
                    }

                    //排序
                    sort(lanespanes.begin(), lanespanes.end(),
                         [](pair<shared_ptr<TopoLaneGroup>,int> & firstRec, pair<shared_ptr<TopoLaneGroup>,int> & secondRec) {
                             if (firstRec.second < secondRec.second) {
                                 return true;
                             } else {
                                 return false;
                             };
                         });
                    cur_topolg->to_lanegroups_.clear();

                    for (auto lanespane_it : lanespanes) {
                        cur_topolg->to_lanegroups_.emplace_back(lanespane_it.first);
                    }

                    for (auto to_topo_lg : cur_topolg->to_lanegroups_) {
                        double start_angle = getLaneGroupDirection(to_topo_lg, true);
                        double end_angle = getLaneGroupDirection(to_topo_lg, false);
                        long position = getLaneGroupPosition(start_angle, end_angle);
                        cur_topolg->t_lane_group_position_.emplace(to_topo_lg->id_, position);
                    }
                };
            }

            return true;
        }

        double DCDataRelationUtil::getLaneGroupDirection(shared_ptr<TopoLaneGroup> topo_lg, bool is_start) {
            double ref_dir = 0;

            vector<shared_ptr<DCCoord>> leftNodes, rightNodes;
            if (topo_lg->dividers_.empty()) {
                return ref_dir;
            }

            for (auto node : topo_lg->dividers_.front()->nodes_) {
                shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                ptr_coord->x_ = node->coord_.x_;
                ptr_coord->y_ = node->coord_.y_;
                ptr_coord->z_ = node->coord_.z_;
                leftNodes.emplace_back(ptr_coord);
            }

            for (auto node : topo_lg->dividers_.back()->nodes_) {
                shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                ptr_coord->x_ = node->coord_.x_;
                ptr_coord->y_ = node->coord_.y_;
                ptr_coord->z_ = node->coord_.z_;
                rightNodes.emplace_back(ptr_coord);
            }

            char zone[4];
            shared_ptr<DCCoord> ref_node = leftNodes.front();
            kd::automap::Coordinates::utmZone(ref_node->y_, ref_node->x_, zone);

            //获得两边车道线的参考方向

            double left_ref_dir =
                    GeosObjUtil::GetAngle(leftNodes, is_start, ANGLE_REF_DIST_, zone);
            double right_ref_dir =
                    GeosObjUtil::GetAngle(rightNodes, is_start, ANGLE_REF_DIST_, zone);

            //计算平均方向
            double total_dir = left_ref_dir + right_ref_dir;
            if (fabs(left_ref_dir - right_ref_dir) > M_PI) {
                total_dir += 2 * M_PI;
            }

            ref_dir = total_dir / 2.0;
            if (ref_dir > 2 * M_PI) {
                ref_dir -= 2 * M_PI;
            }

            return ref_dir;
        }

        long DCDataRelationUtil::getLaneGroupPosition(double start_angle, double end_angle) {
            long ret = 0;

            double start_edge_angle = start_angle + kd::automap::PI;

            if (start_edge_angle > 2 * kd::automap::PI) {
                start_edge_angle -= 2 * kd::automap::PI;
            }

            if (start_angle < start_edge_angle) {
                // left
                if (start_angle < end_angle && end_angle < start_edge_angle) {
                    double sub = end_angle - start_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_MIDDLE;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_LEFT;
                    } else {
                        ret = POSITION_RETURN;
                    }
                } else if (end_angle <= start_angle) {
                    // right
                    double sub = start_angle - end_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_MIDDLE;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_RIGHT;
                    } else {
                        ret = POSITION_RETURN;
                    }
                } else {
                    // right
                    double sub = end_angle - start_edge_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_RETURN;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_RIGHT;
                    } else {
                        ret = POSITION_MIDDLE;
                    }
                }
            } else {
                // right
                if (start_edge_angle < end_angle && end_angle < start_angle) {
                    double sub = start_angle - end_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_MIDDLE;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_RIGHT;
                    } else {
                        ret = POSITION_RETURN;
                    }
                } else if (end_angle <= start_edge_angle) {
                    // left
                    double sub = start_edge_angle - end_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_RETURN;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_LEFT;
                    } else {
                        ret = POSITION_MIDDLE;
                    }
                } else {
                    double sub = end_angle - start_angle;
                    if (sub < kd::automap::PI / 6) {
                        ret = POSITION_MIDDLE;
                    } else if (kd::automap::PI / 6 < sub && sub < kd::automap::PI * 5 / 6){
                        ret = POSITION_LEFT;
                    } else {
                        ret = POSITION_RETURN;
                    }
                }
            }

            return ret;
        }
    }
}
