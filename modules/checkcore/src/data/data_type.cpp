//
// Created by zhangxingang on 19-8-15.
//

#include <data/data_type.h>
#include <util/CommonUtil.h>
#include <util/GeosObjUtil.h>

namespace kd {
    namespace dc {
        bool TopoLaneGroup::BuildDividerDirectionInfo() {
            //找出本组内的正向车道线
            shared_ptr<DCDivider> positiveDiv = nullptr;
            for (auto divider : dividers_) {
                long direction = divider->direction_;
                if (direction == 2) {
                    positiveDiv = divider;
                    break;
                }
            }

            if (positiveDiv == nullptr) {
                LOG(ERROR) << "lanegroup " << id_ << " has no positive divider. need to be optimize.";
                return false;
            }

            dir_ref_div_ = positiveDiv;

            for (int i = 0; i < dividers_.size(); i++) {

                shared_ptr<DCDivider> check_div = dividers_[i];
                long direction = check_div->direction_;
                if (direction == 2) {
                    dir_infos_.push_back(true);
                } else {
                    bool dir_same = CommonUtil::check_dividers_same_direction(dividers_[i], dir_ref_div_);
                    dir_infos_.push_back(dir_same);
                }
            }

            return true;
        }

        shared_ptr<DCDividerNode> TopoLaneGroup::GetDividerNormalNode(int index, bool start) {
            shared_ptr<DCDivider> div = dividers_[index];
            bool dirSame = dir_infos_[index];
            if (dirSame) {
                return start ? div->nodes_.front() : div->nodes_.back();
            } else {
                return start ? div->nodes_.back() : div->nodes_.front();
            }
        }


        bool TopoLaneGroup::FindTopoLaneGroup(const shared_ptr<TopoLaneGroup> tlg_check, bool from_or_to){
            if(from_or_to){
                for(shared_ptr<TopoLaneGroup> tlg : from_lanegroups_){
                    if(tlg->id_ == tlg_check->id_){
                        return true;
                    }
                }
            }else{
                for(shared_ptr<TopoLaneGroup> tlg : to_lanegroups_){
                    if(tlg->id_ == tlg_check->id_){
                        return true;
                    }
                }
            }
            return false;
        }

        void TopoLaneGroup::AppendTopoLaneGroup(const shared_ptr<TopoLaneGroup> tlg, bool from_or_to){
            if(from_or_to){
                from_lanegroups_.emplace_back(tlg);
            }else{
                to_lanegroups_.emplace_back(tlg);
            }
        }
        int TopoLaneGroup::Relation(const shared_ptr<TopoLaneGroup> tlg) {
            if (tlg == nullptr) {
                return -1;
            }

            //TODO 本处是最简单的逻辑判断，正常应该有很多的场景
            shared_ptr<DCDivider> org_front_div = dividers_.front();
            shared_ptr<DCDivider> org_back_div = dividers_.back();

            shared_ptr<DCDivider> dst_front_div = tlg->dividers_.front();
            shared_ptr<DCDivider> dst_back_div = tlg->dividers_.back();

            bool conn_rel1 = DividerIsConcurrentNode(org_back_div, dst_front_div);
            bool conn_rel2 = DividerIsConcurrentNode(org_front_div, dst_back_div);

            if (conn_rel1 && !conn_rel2) {
                return 0;
            } else if (conn_rel2 && !conn_rel1) {
                return 1;
            } else {
                return -1;
            }
        }

        bool TopoLaneGroup::DividerIsConcurrentNode(const shared_ptr<DCDivider> src_div, const shared_ptr<DCDivider> dst_div) {
            if (src_div == nullptr || dst_div == nullptr) {
                return false;
            }

            long src_start_node = stol(src_div->nodes_.front()->id_);
            long src_end_node = stol(src_div->nodes_.back()->id_);

            long dst_start_node = stol(dst_div->nodes_.front()->id_);
            long dst_end_node = stol(dst_div->nodes_.back()->id_);

            if (src_start_node == dst_start_node ||
                src_start_node == dst_end_node ||
                src_end_node == dst_start_node ||
                src_end_node == dst_end_node) {
                return true;
            }

            return false;
        }
        int TopoLaneGroup::GetLaneSpanWeight(const shared_ptr<TopoLaneGroup> tlg, bool from_or_to) {
            //获得要计算的车道线的起点和终点
            shared_ptr<DCDividerNode> fromNode = tlg->GetDividerNormalNode(0, true);
            shared_ptr<DCDividerNode> toNode = tlg->GetDividerNormalNode(0, false);

            //获得本组对应的车道线的信息
            int divIndex = from_or_to ? GetNodeIndex(stol(toNode->id_), from_or_to, true) :
                           GetNodeIndex(stol(fromNode->id_), from_or_to, true);

            double directionWeight = 0.0;
            if (divIndex == -1) {
//        LOG(ERROR) << "GetDirectionWeight error. not find ref divider.";
                return directionWeight;
            }

            shared_ptr<DCDivider> div = dividers_[divIndex];
            bool div_dir = dir_infos_[divIndex];

            if (div->nodes_.size() < 2) {
                LOG(ERROR) << "GetDirectionWeight error. less than 2 nodes in divider :"
                           << div->id_;
                return directionWeight;
            }

            DCDividerNode diff_node;
            if (div_dir) {
                GeosObjUtil::GetDiffPoint(div.get(), 10.0, from_or_to, &diff_node);
            } else {
                GeosObjUtil::GetDiffPoint(div.get(), 10.0, !from_or_to, &diff_node);
            }

            if (from_or_to) {

                //  from ----->------                // center line left , angle is negative
                //                    \
        //                     to  -------- diff --->-------
                //                    /
                //  ------->---------               // center line right, angle is positive

                directionWeight = kd::automap::KDGeoUtil::getAngleDiff(diff_node.coord_->x_, diff_node.coord_->y_,
                                                                       toNode->coord_->x_, toNode->coord_->y_,
                                                                       diff_node.coord_->x_, diff_node.coord_->y_,
                                                                       fromNode->coord_->x_, fromNode->coord_->y_);

                int relation = kd::automap::KDGeoUtil::calPTOrentationOfLine(toNode->coord_->x_, toNode->coord_->y_,
                                                                             diff_node.coord_->x_, diff_node.coord_->y_,
                                                                             fromNode->coord_->x_, fromNode->coord_->y_);
                if (relation == 1) {
                    directionWeight *= -1;
                }


            } else {

                //                           ------->------ to    // center line left , angle is negative
                //                         /
                //  ----- diff ---->---- from
                //                         \
        //                           ------->---------  // center line right, angle is positive


                directionWeight = kd::automap::KDGeoUtil::getAngleDiff(diff_node.coord_->x_, diff_node.coord_->y_,
                                                                       fromNode->coord_->x_, fromNode->coord_->y_,
                                                                       diff_node.coord_->x_, diff_node.coord_->y_,
                                                                       toNode->coord_->x_, toNode->coord_->y_);

                int relation = kd::automap::KDGeoUtil::calPTOrentationOfLine(diff_node.coord_->x_, diff_node.coord_->y_,
                                                                             fromNode->coord_->x_, fromNode->coord_->y_,
                                                                             toNode->coord_->x_, toNode->coord_->y_);
                if (relation == 1) {
                    directionWeight *= -1;
                }
            }

            return directionWeight;
        }

        int TopoLaneGroup::GetNodeIndex(long node_id, bool start, bool left_to_right, bool ignore_same) {
            if (left_to_right) {
                for (int i = 0; i < dividers_.size(); i++) {
                    shared_ptr<DCDividerNode> node = GetDividerNormalNode(i, start);
                    if (node != nullptr && stol(node->id_) == node_id) {
                        if (ignore_same) {//如果是忽略掉相同车道，则继续要查找到
                            for (int j = i + 1; j < dividers_.size(); j++) {
                                shared_ptr<DCDividerNode> node = GetDividerNormalNode(j, start);
                                if (node != nullptr && stol(node->id_) == node_id) {
                                    i = j;
                                } else {
                                    break;
                                }
                            }
                        }

                        return i;
                    }
                }
            } else {
                for (int i = dividers_.size() - 1; i >= 0; i--) {
                    shared_ptr<DCDividerNode> node = GetDividerNormalNode(i, start);
                    if (node != nullptr && stol(node->id_) == node_id) {
                        if (ignore_same) {//如果是忽略掉相同车道，则继续要查找到
                            for (int j = i - 1; j >= 0; j--) {
                                shared_ptr<DCDividerNode> node = GetDividerNormalNode(j, start);
                                if (node != nullptr && stol(node->id_) == node_id) {
                                    i = j;
                                } else {
                                    break;
                                }
                            }
                        }
                        return i;
                    }
                }
            }

            return -1;
        }

    }
}