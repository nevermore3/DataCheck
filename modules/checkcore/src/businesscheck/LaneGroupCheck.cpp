
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
            preCheck();
            BuildLaneGroup2Lanes();

            if (CheckItemValid(CHECK_ITEM_KXS_LG_001)) {
                Check_kxs_03_001();
            }

            if (CheckItemValid(CHECK_ITEM_KXS_LG_003)) {
                Check_kxs_03_003();
            }

            if (CheckItemValid(CHECK_ITEM_KXS_LG_004)) {
                Check_kxs_03_004();
            }
            if (CheckItemValid(CHECK_ITEM_KXS_LG_028)) {
                check_kxs_03_028();
            }
            if (CheckItemValid(CHECK_ITEM_KXS_LG_029)) {
                check_kxs_03_029();
            }
            clearMeomery();
            return true;
        }
        void LaneGroupCheck::preCheck(){
            data_manager()->initRelation(kRLaneGroup);
            data_manager()->initRelation(kLaneConnectivity);
            data_manager()->initRelation(kLaneGroup);
            BuildLaneGroup2Lanes();
            BuildLaneConn();
        }

        void LaneGroupCheck::clearMeomery(){
            data_manager()->clearData(kRLaneGroup);
            data_manager()->clearData(kLaneConnectivity);
            data_manager()->clearData(kLaneGroup);
            map_lg_id_to_lanes.clear();
            map_lane_id_to_lanes.clear();
            map_lane_id_to_lg_id.clear();
        }

        void LaneGroupCheck::Check_kxs_03_004() {

            const auto &divider2_lane_groups = data_manager()->divider2_lane_groups_;
            shared_ptr<DCError> ptr_error = nullptr;
            string taskid, flag, dataKey;
            for (auto div2_lg : divider2_lane_groups) {
                bool check = false;
                if (div2_lg.second.size() == 2) {
                    // divider关联多个车道组
                    auto ptr_divider = CommonUtil::get_divider(data_manager(), div2_lg.first);
                    if (ptr_divider) {
                        string divider_id = ptr_divider->id_;
                        // 如果是参考线
                        if (ptr_divider->dividerNo_ == 0) {
                            // 不是双向的
                            if (ptr_divider->direction_ != 1) {
                                taskid = ptr_divider->task_id_;
                                flag = ptr_divider->flag_;
                                dataKey = DATA_TYPE_LANE + taskid + DATA_TYPE_LAST_NUM;
                                check = true;
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
                    ptr_error->coord = make_shared<DCCoord>();
                    ptr_error->coord->x_ = 0;
                    ptr_error->coord->y_ = 0;
                    ptr_error->coord->z_ = 0;
                    error_output()->saveError(ptr_error);
                }
            }
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LG_004;
            checkItemInfo->totalNum = divider2_lane_groups.size();
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void LaneGroupCheck::Check_kxs_03_003() {
            const auto &divider_maps = data_manager()->dividers_;
            set<string> divider_ids;
            for (auto divider:divider_maps) {
                divider_ids.insert(divider.first);
            }
            const auto &laneGroups = data_manager()->laneGroups_;
            for (const auto &lane_group_item : laneGroups) {
                auto lane_group = lane_group_item.second;
                for (auto lane : lane_group->lanes_) {
                    string left_div_id = lane->leftDivider_->id_;
                    string right_div_id = lane->rightDivider_->id_;
                    if (divider_ids.find(left_div_id) != divider_ids.end()) {
                        divider_ids.erase(left_div_id);
                    }

                    if (divider_ids.find(right_div_id) != divider_ids.end()) {
                        divider_ids.erase(right_div_id);
                    }
                }
            }

            if (divider_ids.size() > 0) {
                for (auto divider_id:divider_ids) {
                    auto ptr_error = DCLaneGroupCheckError::createByKXS_03_003(divider_maps.find(divider_id)->second);
                    error_output()->saveError(ptr_error);
                }
            }

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LG_003;
            checkItemInfo->totalNum = divider_maps.size();
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void LaneGroupCheck::Check_kxs_03_001() {

            const auto &ptr_lane_groups = data_manager()->laneGroups_;
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
            if (ptr_dividers.size() > 0) {
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
            if (ptr_dividers.size() > 0) {
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
                ptr_error->coord->y_ = 0;
                ptr_error->coord->x_ = 0;
                ptr_error->coord->z_ = 0;
                error_output()->saveError(ptr_error);
            }
        }
        void LaneGroupCheck::check_kxs_03_028(){

            ///车道组是否属于虚拟路口检查
            vector<string> lane_group_ids;
            auto lanegroup = data_manager()->laneGroups_;
            for(auto groupItem:lanegroup) {
                auto laneGroup = groupItem.second;
                long is_vir = laneGroup->is_virtual_;

                for (auto laneitem:laneGroup->lanes_) {
                    bool findErr = false;
                    if(laneitem->laneNo_ == 1){
                        ///最左侧车道两条DIV都需要遍历
                        auto leftDa = laneitem->leftDivider_->atts_;
                        for(auto da:leftDa){
                            findErr = checkDaTypeAndVirtual(da->type_,da->virtual_,is_vir);
                            if(findErr){
                                shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_028(laneGroup,da->id_,laneitem->leftDivider_->nodes_[0]->coord_);
                                error_output()->saveError(ptr_error);
                                break;
                            }
                        }

                    }
                    if(!findErr) {
                        ///检查Lane的右侧车道线
                        auto rightDa = laneitem->rightDivider_->atts_;
                        for (auto da:rightDa) {
                            findErr = checkDaTypeAndVirtual(da->type_, da->virtual_, is_vir);
                            if(findErr){
                                shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_028(laneGroup,da->id_,laneitem->leftDivider_->nodes_[0]->coord_);
                                error_output()->saveError(ptr_error);
                                break;
                            }
                        }
                    }
                    if(findErr){
                        break;
                    }
                }
            }


            if(CheckItemValid(CHECK_ITEM_KXS_LG_028)) {
                shared_ptr<CheckItemInfo> checkItem_028 = make_shared<CheckItemInfo>();
                checkItem_028->checkId = CHECK_ITEM_KXS_LG_028;
                checkItem_028->totalNum = lanegroup.size();
                error_output()->addCheckItemInfo(checkItem_028);
            }

        }

        void LaneGroupCheck::check_kxs_03_029(){

            auto lane_group = data_manager()->getKxfData(kLaneGroup);
            auto lane_conn = data_manager()->getKxfData(kLaneConnectivity);
            set<long> vir_lg;
            ///收集虚拟组
            for(auto it:lane_group){
                if(it.second->getPropertyLong(IS_VIR)!=1){
                    continue;
                }
                vir_lg.insert(it.first);
            }

            for(auto it:vir_lg){
                ///虚拟组下的Lane
                auto lg_lanes = map_lg_id_to_lanes.find(it);
                if(lg_lanes==map_lg_id_to_lanes.end()){
                    continue;
                }
                for(auto lane_id : lg_lanes->second){
                    ///lane连接的lane
                    auto lane_to_lanes = map_lane_id_to_lanes.find(lane_id);
                    if(lane_to_lanes != map_lane_id_to_lanes.end()){
                        for(auto to_lane_id:lane_to_lanes->second){
                           long lg_id = map_lane_id_to_lg_id.find(to_lane_id)->second;
                            auto it_lg = lane_group.find(lg_id);
                            if(it_lg!= lane_group.end()){
                                auto lg = it_lg->second;
                                if(lg->getPropertyLong(IS_VIR) == 1){
                                    shared_ptr<DCError> ptr_error = DCLaneGroupCheckError::createByKXS_03_029(it,lg_id);
                                    error_output()->saveError(ptr_error);
                                }
                            }
                        }
                    }
                }
            }

            if(CheckItemValid(CHECK_ITEM_KXS_LG_029)) {
                shared_ptr<CheckItemInfo> checkItem_029 = make_shared<CheckItemInfo>();
                checkItem_029->checkId = CHECK_ITEM_KXS_LG_029;
                checkItem_029->totalNum = map_lane_id_to_lanes.size();
                error_output()->addCheckItemInfo(checkItem_029);
            }

        }
        bool LaneGroupCheck::checkDaTypeAndVirtual(long type_,long virtual_,long is_vir_){
            if(( type_ == 6 || virtual_ == 1) && is_vir_ != 1){
                return true;
            }
            return false;
        }
        void LaneGroupCheck::BuildLaneGroup2Lanes(){
            auto r_lane_group = data_manager()->getKxfData(kRLaneGroup);
            for(auto it:r_lane_group){
                long lg_id = it.second->getPropertyLong(LG_ID);
                long lane_id = it.second->getPropertyLong(LANE_ID);
                auto lg_info = map_lg_id_to_lanes.find(lg_id);
                if(map_lg_id_to_lanes.find(lg_id) == map_lg_id_to_lanes.end()){
                    set<long> lanes;
                    lanes.emplace(lane_id);
                    map_lg_id_to_lanes.insert(make_pair(lg_id,lanes));
                }else{
                    lg_info->second.insert(lane_id);
                }
                if(map_lane_id_to_lg_id.find(lane_id) == map_lane_id_to_lg_id.end()){
                    map_lane_id_to_lg_id.insert(make_pair(lane_id,lg_id));
                }
            }
        }
        void LaneGroupCheck::BuildLaneConn(){
            auto lane_conn = data_manager()->getKxfData(kLaneConnectivity);
            for(auto it:lane_conn){
                long flane_id = it.second->getPropertyLong(FLANE_ID);
                long tlane_id = it.second->getPropertyLong(TLANE_ID);
                auto lg_info = map_lane_id_to_lanes.find(flane_id);
                if(map_lane_id_to_lanes.find(flane_id) == map_lane_id_to_lanes.end()){
                    set<long> lanes;
                    lanes.emplace(tlane_id);
                    map_lane_id_to_lanes.insert(make_pair(flane_id,lanes));
                }else{
                    lg_info->second.insert(tlane_id);
                }
            }
        }
    }
}