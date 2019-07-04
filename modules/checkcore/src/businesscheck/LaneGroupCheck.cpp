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
    if (nullptr == data_manager) {
        return false;
    }

    set_data_manager(data_manager);
    set_error_output(error_output);

    if (CheckItemValid(CHECK_ITEM_KXS_LG_001)) {
        Check_kxs_03_001();
    }

    if (CheckItemValid(CHECK_ITEM_KXS_LG_003)) {
        Check_kxs_03_003();
    }

    if (CheckItemValid(CHECK_ITEM_KXS_LG_004)) {
        Check_kxs_03_004();
    }

    return false;
}

void LaneGroupCheck::Check_kxs_03_004() {
    const auto &divider2_lane_groups = data_manager()->divider2_lane_groups_;
    shared_ptr<DCError> ptr_error = nullptr;
    string taskid,flag,dataKey;
    for (auto div2_lg : divider2_lane_groups) {
        bool check = false;
        if (div2_lg.second.size() == 2) {
            // divider关联多个车道组
            auto ptr_divider = CommonUtil::get_divider(data_manager(), div2_lg.first);
            if (ptr_divider) {
                // 如果是参考线
                if (ptr_divider->dividerNo_ == 0) {
                    for (const auto &lg : div2_lg.second) {
                        auto ptr_road =
                                CommonUtil::get_road_by_lg(data_manager(), lg);
                        if (ptr_road) {
                            // 不是双向的
                            if (ptr_road->direction_ != 1) {

                                taskid = ptr_road->task_id_;
                                flag = ptr_road->flag_;
                                dataKey = DATA_TYPE_LANE + taskid + DATA_TYPE_LAST_NUM;
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

            ptr_error->flag = flag;
            ptr_error->dataKey_ = dataKey;
            ptr_error->taskId_ = taskid;
            ptr_error->coord = nullptr;
            error_output()->saveError(ptr_error);
        }
    }
}

void LaneGroupCheck::Check_kxs_03_003() {
    const auto &divider_maps = data_manager()->dividers_;
    const auto &divider2_lane_groups = data_manager()->divider2_lane_groups_;
    for (const auto &divider : divider_maps) {
        if (divider2_lane_groups.find(divider.first) == divider2_lane_groups.end()) {
            auto ptr_error = DCLaneGroupCheckError::createByKXS_03_003(divider.second);
            error_output()->saveError(ptr_error);
        }
    }

    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = CHECK_ITEM_KXS_LG_003;
    checkItemInfo->totalNum = data_manager()->dividers_.size();
    error_output()->addCheckItemInfo(checkItemInfo);
}

void LaneGroupCheck::Check_kxs_03_001() {

    const auto& ptr_lane_groups = data_manager()->laneGroups_;
    for (const auto &lane_group : ptr_lane_groups) {
        auto ptr_dividers = CommonUtil::get_dividers_by_lg(data_manager(), lane_group.first);
        if (!ptr_dividers.empty()) {

//                    check_divider_no(mapDataManager, errorOutput, lane_group.first, ptr_dividers);
            check_divider_length(lane_group.first, ptr_dividers);
        }
    }

    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = CHECK_ITEM_KXS_LG_001;
    checkItemInfo->totalNum = data_manager()->laneGroups_.size();
    error_output()->addCheckItemInfo(checkItemInfo);
}

void LaneGroupCheck::check_divider_no(shared_ptr<MapDataManager> mapDataManager,
                                      shared_ptr<CheckErrorOutput> errorOutput, const string &lane_group,
                                      const vector<shared_ptr<DCDivider>> &ptr_dividers) {
    bool is_check = false;
    string taskId;
    string flag;
    string dataKey;
    if(ptr_dividers.size()>0){
        taskId = ptr_dividers[0]->task_id_;
        flag = ptr_dividers[0]->flag_;
        dataKey = DATA_TYPE_LANE + taskId + DATA_TYPE_LAST_NUM;
    }

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

        ptr_error->taskId_ = taskId;
        ptr_error->flag = flag;
        ptr_error->dataKey_ = dataKey;
        ptr_error->coord = nullptr;
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

    string taskId;
    string flag;
    string dataKey;
    if(ptr_dividers.size()>0){
        taskId = ptr_dividers[0]->task_id_;
        flag = ptr_dividers[0]->flag_;
        dataKey = DATA_TYPE_LANE + taskId + DATA_TYPE_LAST_NUM;
    }
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
        ptr_error->taskId_ = taskId;
        ptr_error->flag = flag;
        ptr_error->dataKey_ = dataKey;
        ptr_error->coord = make_shared<DCCoord>();
        ptr_error->coord->lat_=0;
        ptr_error->coord->lng_=0;
        ptr_error->coord->z_=0;
        error_output()->saveError(ptr_error);
    }
}
}
}