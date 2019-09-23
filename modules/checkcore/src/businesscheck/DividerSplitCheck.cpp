//
// Created by ubuntu on 19-9-3.
//

#include "businesscheck/DividerSplitCheck.h"
#include "CommonDefine.h"
#include "mvg/Coordinates.hpp"
namespace kd {
    namespace dc {
        string DividerSplitCheck::getId() {
            return id;
        }

        bool DividerSplitCheck::execute(shared_ptr<MapDataManager> data_manager,
                                        shared_ptr<CheckErrorOutput> error_output) {
            set_data_manager(data_manager);
            set_error_output(error_output);

            PreCheck();

            CheckSplitLocationAndBuildSpatialIndex();

//            WriteAllSplit();

            CheckIntersectSplitDivider();

            RemoveConnectedSplitPoint();

            CheckOverlapSplitDivider();

            TraceDivider();

            ExcludeTasksBorderSplit();

            CheckEmergencyStandbyLaneSplitDivider();

            ExportErrSplitNode();

//            ShowStatisticsInfo();

            EndCheck();
            return !HasErrors();

        }
        bool DividerSplitCheck::HasErrors() {
            if (error_nums_.empty()) {
                return false;
            }
            return true;
        }
        void DividerSplitCheck::PreCheck() {
            divider_node_quadtree_ = std::make_shared<geos::index::quadtree::Quadtree>();
            output_details_ = false;
        }

        void DividerSplitCheck::EndCheck() {
            divider_node_quadtree_ = nullptr;
            split_node_info_map_.clear();
            common_node_info_list_.clear();
            divider_info_map_.clear();
        }


        void DividerSplitCheck::ShowStatisticsInfo() {
            LOG(INFO) << id_
                      << "\r\n"
                      << " : [" << CHECK_DIV_SPLIT_LOCATION_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_LOCATION_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_OVERLAP << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_OVERLAP) << "]"
                      << " : [" << CHECK_DIV_SPLIT_INTERSECT << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_INTERSECT) << "]"
                      << " : [" << CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV) << "]"
                      << " : [" << CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE) << "]"
                      << " : [" << CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER) << "]"
                      << "\r\n"
                      << " : [" << CHECK_DIV_SPLIT_DIRECTION_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DIRECTION_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_R_LINE_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_R_LINE_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_TOLLFLAG_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_TOLLFLAG_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_VIRTUAL_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_COLOR_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_COLOR_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_TYPE_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_TYPE_ERR) << "]"
                      << "\r\n"
                      << " : [" << CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_OVERLAY_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_OVERLAY_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_MATERIAL_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_MATERIAL_ERR) << "]"
                      << " : [" << CHECK_DIV_SPLIT_DA_WIDTH_ERR << ":"
                      << GetErrorNum(CHECK_DIV_SPLIT_DA_WIDTH_ERR) << "]";
        }

        int DividerSplitCheck::GetErrorNum(const string &check_no) {
            auto itemit = error_nums_.find(check_no);
            if (itemit != error_nums_.end()) {
                return itemit->second;
            }
            return 0;
        }

        void DividerSplitCheck::CheckSplitLocationAndBuildSpatialIndex() {



            ///所有节点建立空间索引,split点加入待检测列表
            const auto &divider_data = data_manager()->dividers_;
            for (auto it : divider_data) {

//        const auto &das_in_divider = preprop_data_manager_->getDividerDAs(it.first);
                auto divider = std::static_pointer_cast<DCDivider>(it.second);

                ///判断split点是否位于divider中间
                size_t node_size = divider->nodes_.size();
                if (node_size <= 1) {
                    continue;
                }
                for (int i = 1; i < node_size - 2; ++i) {
                    shared_ptr<DCDividerNode> node_ptr =
                            std::static_pointer_cast<DCDividerNode>(divider->nodes_[i]);
                    long split = node_ptr->isSplit_;
                    long taskId = stol(node_ptr->task_id_);
                    if (0 != split && Isvalid(taskId)) {
                        stringstream errss;
                        errss << "[DIV_ID:" << divider->id_ << ", NODE_ID:"
                              << node_ptr->id_ << "]";
//                        ErrorOutput(CHECK_DIV_SPLIT_LOCATION_ERR,
//                                    CHECK_DIV_SPLIT_LOCATION_ERR_DESC, ERROR_FATAL,
//                                    taskId, errss.str(), node_ptr);
                    }
                }

                ///首点加入索引
                shared_ptr<DCDividerNode> start_node =
                        std::static_pointer_cast<DCDividerNode>(divider->nodes_.front());
                long start_split = start_node->isSplit_;
                shared_ptr<DividerNodeInfo> split_info_ptr = make_shared<DividerNodeInfo>();
                split_info_ptr->geom_ptr = GeosObjUtil::CreatePoint(start_node->coord_);
                split_info_ptr->divider_ptr = divider;
                split_info_ptr->node_ptr = start_node;
                split_info_ptr->task_id = stol(start_node->task_id_);
                if (start_split) {
                    split_info_ptr->is_split = true;
                    split_info_ptr->is_start = true;
                    split_node_info_map_.insert(make_pair(stol(start_node->id_), split_info_ptr));
                } else {
                    split_info_ptr->is_start = true;
                    split_info_ptr->is_split = false;
                }
                common_node_info_list_.push_back(split_info_ptr);
                divider_node_quadtree_->insert(split_info_ptr->geom_ptr->getEnvelopeInternal(), split_info_ptr.get());

                ///尾点加入索引
                shared_ptr<DCDividerNode> end_node =
                        std::static_pointer_cast<DCDividerNode>(divider->nodes_.back());
                long end_split = end_node->isSplit_;
                split_info_ptr = make_shared<DividerNodeInfo>();
                split_info_ptr->geom_ptr = GeosObjUtil::CreatePoint(end_node->coord_);
                split_info_ptr->divider_ptr = divider;
                split_info_ptr->node_ptr = end_node;
                split_info_ptr->task_id = stol(end_node->task_id_);
                if (end_split) {
                    split_info_ptr->is_start = false;
                    split_info_ptr->is_split = true;
                    split_node_info_map_.insert(make_pair(stol(end_node->id_), split_info_ptr));
                } else {
                    split_info_ptr->is_start = false;
                    split_info_ptr->is_split = false;
                }
                common_node_info_list_.push_back(split_info_ptr);
                divider_node_quadtree_->insert(split_info_ptr->geom_ptr->getEnvelopeInternal(), split_info_ptr.get());
            }
        }

        std::shared_ptr<DividerGeomInfo> DividerSplitCheck::CreateDividerGeomInfo(
                DividerNodeInfo *ptr, bool make_connect) {
            std::shared_ptr<DividerGeomInfo> divider_info_ptr =
                    std::make_shared<DividerGeomInfo>();
            divider_info_ptr->divider_ptr = ptr->divider_ptr;
            if (ptr->is_start) {
                divider_info_ptr->is_start_split = ptr->is_split;
                divider_info_ptr->start_ptr = ptr->geom_ptr;
                divider_info_ptr->is_start_connect = make_connect;
            } else {
                divider_info_ptr->is_end_split = ptr->is_split;
                divider_info_ptr->end_ptr = ptr->geom_ptr;
                divider_info_ptr->is_end_connect = make_connect;
            }
            divider_info_ptr->task_id = ptr->task_id;

            return divider_info_ptr;
        }

        void DividerSplitCheck::AddDividerNodeInfo(
                std::shared_ptr<DividerGeomInfo> &divider_info_ptr,
                DividerNodeInfo *ptr, bool make_connect) {
            if (ptr->is_start) {
                if (divider_info_ptr->start_ptr == nullptr) {
                    divider_info_ptr->is_start_split = true;
                    divider_info_ptr->start_ptr = ptr->geom_ptr;
                    divider_info_ptr->is_start_connect = make_connect;
                }
            } else {
                if (divider_info_ptr->end_ptr == nullptr) {
                    divider_info_ptr->is_end_split = true;
                    divider_info_ptr->end_ptr = ptr->geom_ptr;
                    divider_info_ptr->is_end_connect = make_connect;
                }
            }
        }

        /**
 * 判读两个空间点之间的距离
 * @param point1 点1
 * @param point2 点2
 * @param buffer_size  水平容差
 * @param buffer_z 垂直容差
 * @return 容差内 true,容差外 false
 */
        bool DividerSplitCheck::PointIsConcurrent(
                const shared_ptr<geos::geom::Point> point1,
                const shared_ptr<geos::geom::Point> point2,
                double buffer_size, double buffer_z) {

            if (point1 == nullptr || point2 == nullptr) {
                return false;
            }

            const geos::geom::Coordinate *coord1 = point1->getCoordinate();
            const geos::geom::Coordinate *coord2 = point2->getCoordinate();

            double distance = GeometryUtil::getDistance(
                    coord1->x, coord1->y, coord2->x, coord2->y);
            if (distance < buffer_size) {
                double z_diff = fabs(coord1->z - coord2->z);
                if (z_diff < buffer_z) {
                    return true;
                }
            }

            return false;
        }

/**
 * 判读两个空间点的Z方向距离是否小于容差
 * @param point1 点1
 * @param point2 点2
 * @param buffer_z 垂直容差
 * @return 容差内 true,容差外 false
 */
        bool DividerSplitCheck::zIsInBuffer(
                const shared_ptr<geos::geom::Point> point1,
                const shared_ptr<geos::geom::Point> point2,
                double buffer_z) {

            if (point1 == nullptr || point2 == nullptr) {
                return false;
            }

            const geos::geom::Coordinate *coord1 = point1->getCoordinate();
            const geos::geom::Coordinate *coord2 = point2->getCoordinate();
            double z_diff = fabs(coord1->z - coord2->z);
            if (z_diff < buffer_z) {
                return true;
            }

            return false;
        }

        void DividerSplitCheck::FilterQueryPoints(
                DividerNodeInfo *test_point_ptr, vector<void *> &raw_query_objs,
                vector<DividerNodeInfo *> &query_objs,
                double buffer_dis, double buffer_z, bool task_check) {
            ///去除距离超过容差的点
            if (task_check) {
                bool dif_task = false;
                std::vector<DividerNodeInfo *> temp_list;

                for (int i = 0; i < raw_query_objs.size(); ++i) {
                    auto *ptr = (DividerNodeInfo *) raw_query_objs[i];
                    if (PointIsConcurrent(test_point_ptr->geom_ptr, ptr->geom_ptr, buffer_dis, buffer_z)) {

                        if (test_point_ptr->task_id != ptr->task_id)
                            dif_task = true;

                        temp_list.emplace_back(ptr);
                    }
                }
                if (dif_task) {
                    query_objs = temp_list;
                }
            } else {
                for (int i = 0; i < raw_query_objs.size(); ++i) {
                    auto *ptr = (DividerNodeInfo *) raw_query_objs[i];
                    if (PointIsConcurrent(test_point_ptr->geom_ptr, ptr->geom_ptr, buffer_dis, buffer_z)) {
                        query_objs.emplace_back(ptr);
                    }
                }
            }
        }

        void DividerSplitCheck::FilterQueryPoints(
                DividerNodeInfo *test_point_ptr, vector<void *> &raw_query_objs,
                vector<DividerNodeInfo *> &query_objs, double buffer_dis, double buffer_z) {
            ///去除距离超过容差的点
            bool dif_task = false;
            std::vector<DividerNodeInfo *> temp_list;

            for (int i = 0; i < raw_query_objs.size(); ++i) {
                auto *ptr = (DividerNodeInfo *) raw_query_objs[i];
                if (PointIsConcurrent(test_point_ptr->geom_ptr, ptr->geom_ptr, buffer_dis, buffer_z)) {
                    if (test_point_ptr->task_id != ptr->task_id)
                        dif_task = true;

                    temp_list.emplace_back(ptr);
                }
            }
            if (dif_task) {
                query_objs = temp_list;
            }
        }


        void DividerSplitCheck::WriteNoSplitAttributeErr(
                long task_id, long div_id, shared_ptr<DCDividerNode> &node_ptr) {
            if (Isvalid(task_id)) {
                stringstream errss;
                errss << "[DIV_ID:" << div_id << ", NODE_ID:" << node_ptr->id_ << "]";

                shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE);
                shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(node_ptr->coord_);
                error_item->checkId = CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE;
                error_item->checkName = CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE_DESC;
                error_item->detail_ = errss.str();
                error_item->sourceId = to_string(div_id);
                error_item->coord = coord;
                error_item->taskId_ = to_string(task_id);
                error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                errNodeInfo->dataType = DATA_TYPE_NODE;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_NODE;
                error_item->errNodeInfo.emplace_back(errNodeInfo);
                error_output()->saveError(error_item);
            }
        }

        void DividerSplitCheck::RemoveConnectedSplitPoint() {

            bool isOutNoSplitAttributeErr = CheckListConfig::getInstance().IsNeedCheck(
                    CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE);
            bool zOutBufferError = CheckListConfig::getInstance().IsNeedCheck(
                    CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER);
            ///从split待检测列表中移除匹配好的点(split点的一定范围缓冲区内有其他split点)
            auto it = split_node_info_map_.begin();
            std::unordered_map<long, std::shared_ptr<DividerGeomInfo>>::iterator it_divider;
            unordered_set<long> node_set;
            while (it != split_node_info_map_.end()) {

//        shared_ptr<DividerNodeInfo> current_dividerNode = it->second;
//        ///获取divder节点
//        long c_divider_id = current_dividerNode->divider_ptr->ID;
//        shared_ptr<DividerGeomInfo> c_divider_info_ptr = divider_info_map_[c_divider_id];


                shared_ptr<geos::geom::Point> test_point = it->second->geom_ptr;
                shared_ptr<geos::geom::Geometry> geom_buffer(test_point->buffer(node_search_buffer_len_));

                ///通过节点周边的缓冲区进行共点的查找
                vector<void *> raw_query_objs;
                vector<DividerNodeInfo *> query_objs;
                divider_node_quadtree_->query(geom_buffer->getEnvelopeInternal(), raw_query_objs);

                ///获取一定范围内的点,且此组点必须含有不同taskid,不判断Z方向容差
                FilterQueryPoints(it->second.get(), raw_query_objs, query_objs, node_search_buffer_len_, 1000);

                size_t connect_num = query_objs.size();
                if (connect_num > 1) {
                    ///将相关结果插入容器,以待后续从待检查列表删除
                    for (size_t i = 0; i < connect_num; ++i) {
                        DividerNodeInfo *ptr = query_objs[i];

                        ///判断Z方向容差
                        if ((it->second.get()->task_id != ptr->task_id) && Isvalid(it->second.get()->task_id)) {
                            bool zInBuffer = zIsInBuffer(it->second.get()->geom_ptr, ptr->geom_ptr,
                                                         split_node_find_buffer_z);
                            if ((!zInBuffer) && zOutBufferError) {
                                string divider_id = it->second.get()->divider_ptr->id_;
                                string node_id = it->second.get()->node_ptr->id_;
                                stringstream errss;
                                errss << "[DIV_ID:" << divider_id
                                      << ",NODE_ID:" << node_id << "与任务:"
                                      << std::to_string(ptr->task_id) << ",DIV_ID:"
                                      << ptr->divider_ptr->id_
                                      << ",NODE_ID:" << ptr->node_ptr->id_
                                      << "接边处Z坐标值相差太大]";

                                shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER);
                                shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(
                                        it->second.get()->node_ptr->coord_);
                                error_item->checkId = CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER;
                                error_item->checkName = CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER_DESC;
                                error_item->detail_ = errss.str();
                                error_item->sourceId = divider_id;
                                error_item->coord = coord;
                                error_item->taskId_ = to_string(it->second.get()->task_id);
                                error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                                errNodeInfo->dataType = DATA_TYPE_NODE;
                                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_NODE;
                                error_item->errNodeInfo.emplace_back(errNodeInfo);
                                error_output()->saveError(error_item);

                            }
                            ///接边DIVIDER的通行方向错误判断
                            CheckConnectLaneDA(it->second.get(), ptr);
                        }


                        node_set.insert(stol(ptr->node_ptr->id_));
                        ptr->conn_nodes.insert(it->second.get());
                        it->second->conn_nodes.insert(ptr);

                        ///获取该节点关联的道路,对其联通属性赋值,用于排除整体任务边界的split
                        it_divider = divider_info_map_.find(stol(ptr->divider_ptr->id_));
                        if (it_divider == divider_info_map_.end()) {
                            std::shared_ptr<DividerGeomInfo> divider_info_ptr =
                                    CreateDividerGeomInfo(ptr, true);
                            divider_info_map_.insert(std::make_pair(
                                    stol(divider_info_ptr->divider_ptr->id_), divider_info_ptr));
                        } else {
                            std::shared_ptr<DividerGeomInfo> &divider_info_ptr = it_divider->second;
                            AddDividerNodeInfo(divider_info_ptr, ptr, true);
                        }



                        ///范围内含有divider端点不含split属性,报出
                        //TODO 去除可能的内部点
                        if ((!ptr->is_split) && isOutNoSplitAttributeErr) {
                            WriteNoSplitAttributeErr(ptr->task_id, stol(ptr->divider_ptr->id_), ptr->node_ptr);
                        }
                    }
                } else {
                    ///获取该节点关联的道路,对其联通属性赋值,用于排除整体任务边界的split
                    it_divider = divider_info_map_.find(stol(it->second->divider_ptr->id_));
                    if (it_divider == divider_info_map_.end()) {
                        std::shared_ptr<DividerGeomInfo> divider_info_ptr =
                                CreateDividerGeomInfo(it->second.get(), false);
                        divider_info_map_.insert(std::make_pair(
                                stol(divider_info_ptr->divider_ptr->id_), divider_info_ptr));
                    } else {
                        std::shared_ptr<DividerGeomInfo> &divider_info_ptr = it_divider->second;
                        AddDividerNodeInfo(divider_info_ptr, it->second.get(), false);
                    }
                }
                ++it;
            }
            int split_node_size = split_node_info_map_.size();
            if (isOutNoSplitAttributeErr) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE, split_node_size);
            }
            if (zOutBufferError) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DIRECTION_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DIRECTION_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_R_LINE_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_R_LINE_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_TOLLFLAG_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_TOLLFLAG_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_COLOR_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_COLOR_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_TYPE_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_TYPE_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_OVERLAY_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_OVERLAY_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_MATERIAL_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_MATERIAL_ERR, split_node_size);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_WIDTH_ERR)) {
                error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_DA_WIDTH_ERR, split_node_size);
            }
            ///清除其他已确定的点
            for (auto it_node_id : node_set) {
                it = split_node_info_map_.find(it_node_id);
                if (it != split_node_info_map_.end()) {
                    split_node_info_map_.erase(it);
                }
            }
        }

        void DividerSplitCheck::CheckIntersectSplitDivider() {
            if (!CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_INTERSECT)) {
                return;
            }

            std::vector<long> check_split_ids;
            for (auto it : split_node_info_map_) {
                check_split_ids.emplace_back(it.first);

                std::shared_ptr<DividerGeomInfo> divider_info_ptr =
                        CreateDividerGeomInfo(it.second.get(), false);
                divider_info_map_.insert(std::make_pair(stol(
                        divider_info_ptr->divider_ptr->id_), divider_info_ptr));
            }

            for (auto it : divider_info_map_) {
                if (it.second->divider_geom_ptr == nullptr)
                    it.second->divider_geom_ptr =
                            GeosObjUtil::CreateLineString(it.second->divider_ptr->nodes_);
            }


            int size = check_split_ids.size();
            for (int i = 0; i < size; i++) {
                long current_id = check_split_ids[i];
                shared_ptr<DividerNodeInfo> current_dividerNode = split_node_info_map_[current_id];
                long task_id = current_dividerNode->task_id;

                if (!Isvalid(task_id)) {
                    ///不是主任务
                    continue;
                }
                ///获取divder节点
                long c_divider_id = stol(current_dividerNode->divider_ptr->id_);
                shared_ptr<DividerGeomInfo> c_divider_info_ptr = divider_info_map_[c_divider_id];
                shared_ptr<geos::geom::LineString> c_divider_geom = c_divider_info_ptr->divider_geom_ptr;

                for (int j = 0; j < size; j++) {
                    long next_id = check_split_ids[j];
                    shared_ptr<DividerNodeInfo> next_dividerNode = split_node_info_map_[next_id];
                    long sub_task_id = next_dividerNode->task_id;
                    if (Isvalid(sub_task_id)) {
                        ///是主任务
                        continue;
                    }
                    bool res = PointIsConcurrent(current_dividerNode->geom_ptr, next_dividerNode->geom_ptr,
                                                 intersect_search_buffer_len, intersect_search_buffer_z);
                    if (!res) {
                        continue;
                    }
                    ///获取divder节点
                    long n_divider_id = stol(next_dividerNode->divider_ptr->id_);
                    shared_ptr<DividerGeomInfo> &n_divider_info_ptr = divider_info_map_[n_divider_id];
                    shared_ptr<geos::geom::LineString> n_divider_geom = n_divider_info_ptr->divider_geom_ptr;
                    vector<shared_ptr<IntersectInfo>> intersect_infos;
                    GeosObjRelationUtil::LineStringIntersect(c_divider_geom, n_divider_geom,
                                                             intersect_split_divider_limit,
                                                             intersect_split_divider_limit_z, intersect_infos);
                    if (intersect_infos.size() == 1) {
                        shared_ptr<IntersectInfo> intersect = intersect_infos.front();
                        double c_dis = GeometryUtil::getDistance(current_dividerNode->geom_ptr->getX(),
                                                                 current_dividerNode->geom_ptr->getY(),
                                                                 intersect->coord_x_, intersect->coord_y_);
                        double n_dis = GeometryUtil::getDistance(next_dividerNode->geom_ptr->getX(),
                                                                 next_dividerNode->geom_ptr->getY(),
                                                                 intersect->coord_x_, intersect->coord_y_);
                        if (c_dis > intersect_point_to_split_point_len || n_dis > intersect_point_to_split_point_len) {
                            stringstream errss;
                            errss << "[DIV_ID:" << c_divider_id
                                  << ",NODE_ID:" << current_dividerNode->node_ptr->id_ << "与任务:"
                                  << std::to_string(current_dividerNode->task_id) << ",DIV_ID:"
                                  << n_divider_id
                                  << ",NODE_ID:" << next_dividerNode->node_ptr->id_
                                  << "相交容差超出范围]";
                            char zone[4] = {0};
                            double lat, lng, x, y;
                            kd::automap::Coordinates::ll2utm(current_dividerNode->node_ptr->coord_->x_, current_dividerNode->node_ptr->coord_->y_, x, y, zone);
                            kd::automap::Coordinates::utm2ll(intersect->coord_y_, intersect->coord_x_, zone, lat, lng);

                            shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(CHECK_DIV_SPLIT_INTERSECT);
                            shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                            coord->x_ = lng;
                            coord->y_ = lat;
                            coord->z_ = intersect->coord_z_;

                            error_item->checkId = CHECK_DIV_SPLIT_INTERSECT;
                            error_item->checkName = CHECK_DIV_SPLIT_INTERSECT_DESC;
                            error_item->detail_ = errss.str();
                            error_item->sourceId = to_string(c_divider_id);
                            error_item->coord = coord;
                            error_item->taskId_ = to_string(current_dividerNode->task_id);
                            error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                            errNodeInfo->dataType = DATA_TYPE_NODE;
                            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_NODE;
                            error_item->errNodeInfo.emplace_back(errNodeInfo);
                            error_output()->saveError(error_item);
                        }

                    } else if (intersect_infos.size() > 1) {
                        LOG(WARNING) << "CheckIntersectSplitDivider: intersect point...";
                    }

                }

            }
            error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_INTERSECT, size * size);
        }

        void DividerSplitCheck::CheckOverlapSplitDivider() {
            ///是否需要检查
            if (!CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_OVERLAP)) {
                return;
            }

            ///将split点和已匹配的无split点关联的divider生成geom要素
            for (auto it : divider_info_map_) {
                if (it.second->divider_geom_ptr == nullptr)
                    it.second->divider_geom_ptr = GeosObjUtil::CreateLineString(it.second->divider_ptr->nodes_);
            }

            ///检查重叠divider
            int min_index;
            double len;
            int check_total = 0;
            std::set<long> node_set;
            auto it = split_node_info_map_.begin();
            while (it != split_node_info_map_.end()) {
                long divider_id = stol(it->second->divider_ptr->id_);
                long task_id = it->second->task_id;

                ///创建待检测线的缓冲
                shared_ptr<DividerGeomInfo> &divider_info_ptr = divider_info_map_[divider_id];
                shared_ptr<geos::geom::LineString> divider_geom = divider_info_ptr->divider_geom_ptr;
                shared_ptr<geos::geom::Geometry> geom_buffer(divider_geom->buffer(overlap_divider_search_buffer_));

                bool matched = false;
                for (auto test_info_it : divider_info_map_) {
                    ///跳过当前要素(当前task要素)
                    shared_ptr<DividerGeomInfo> test_divider_info_ptr = test_info_it.second;
                    if (test_divider_info_ptr->task_id == it->second->task_id)
                        continue;

                    if (stol(test_divider_info_ptr->divider_ptr->id_ )== divider_id)
                        continue;
                    check_total++;
                    vector<shared_ptr<OverlapInfo>> overlap_infos;
                    GeosObjRelationUtil::LineStringOverlap(divider_geom, geom_buffer,
                                                           test_divider_info_ptr->divider_geom_ptr,
                                                           node_search_buffer_z_, overlap_infos);
                    size_t intersect_num = overlap_infos.size();
                    if (intersect_num > 0) {
                        for (size_t i = 0; i < intersect_num; ++i) {
                            shared_ptr<OverlapInfo> &overlap_info_ptr = overlap_infos[i];

                            ///当前divider要与待检查divider的起始或结束位置重合
                            bool at_start;
                            CoordinateSequence *sq = nullptr;
                            if (overlap_info_ptr->start_node_index_ == 0 &&
                                fabs(overlap_info_ptr->start_node_dist_) < 0.000001) {
                                at_start = true;
                                sq = test_divider_info_ptr->divider_geom_ptr->getCoordinates();
                            } else if (overlap_info_ptr->end_node_index_ ==
                                       test_divider_info_ptr->divider_geom_ptr->getNumPoints() - 2) {
                                ///确保末尾重合
                                sq = test_divider_info_ptr->divider_geom_ptr->getCoordinates();
                                const geos::geom::Coordinate &pt1 = sq->getAt(
                                        (size_t) overlap_info_ptr->end_node_index_);
                                const geos::geom::Coordinate &pt2 = sq->getAt(
                                        (size_t) overlap_info_ptr->end_node_index_ + 1);
                                double l = GeometryUtil::getDistance(pt1.x, pt1.y, pt2.x, pt2.y);
                                if (fabs(l - overlap_info_ptr->end_node_dist_) < 0.000001)
                                    at_start = false;
                                else
                                    continue;
                            } else
                                continue;

                            ///确保两线距离在容差之内
                            double dis_len = GeosObjRelationUtil::pt2LineDist(sq, it->second->geom_ptr->getCoordinate(),
                                                                              min_index);
                            if (dis_len <= overlap_divider_search_buffer_ &&
                                (min_index >= overlap_info_ptr->start_node_index_ &&
                                 min_index <= overlap_info_ptr->end_node_index_)) {
                                ///暂不检查方向,由于有高程限制,三维上不相交则不应该有交点
                                ///检查长度是否大于阀值

                                shared_ptr<DCDividerNode> test_node = nullptr;
                                ///获取重叠长度
                                bool is_split;
                                if (at_start) {
                                    len = 0;
                                    for (size_t j = 0; j < min_index; ++j) {
                                        const geos::geom::Coordinate &pt1 = sq->getAt(j);
                                        const geos::geom::Coordinate &pt2 = sq->getAt(j + 1);
                                        len += GeometryUtil::getDistance(pt1.x, pt1.y, pt2.x, pt2.y);
                                    }
                                    const geos::geom::Coordinate &pt2 = sq->getAt((size_t) min_index);
                                    double l = GeometryUtil::getDistance(pt2.x, pt2.y, it->second->geom_ptr->getX(),
                                                                         it->second->geom_ptr->getY());
                                    len += sqrt(l * l - dis_len * dis_len);
                                    is_split = test_divider_info_ptr->is_start_split;
                                    test_divider_info_ptr->is_start_connect = true;
                                    test_node = std::static_pointer_cast<DCDividerNode>(
                                            test_divider_info_ptr->divider_ptr->nodes_.front());
                                } else {
                                    len = 0;
                                    for (size_t j = (size_t) min_index; j < sq->getSize() - 1; ++j) {
                                        const geos::geom::Coordinate &pt1 = sq->getAt((size_t) j);
                                        const geos::geom::Coordinate &pt2 = sq->getAt((size_t) j + 1);
                                        len += GeometryUtil::getDistance(pt1.x, pt1.y, pt2.x, pt2.y);
                                    }
                                    const geos::geom::Coordinate &pt2 = sq->getAt((size_t) min_index);
                                    double l = GeometryUtil::getDistance(pt2.x, pt2.y, it->second->geom_ptr->getX(),
                                                                         it->second->geom_ptr->getY());
                                    len -= sqrt(l * l - dis_len * dis_len);
                                    is_split = test_divider_info_ptr->is_end_split;
                                    test_divider_info_ptr->is_end_connect = true;
                                    test_node = std::static_pointer_cast<DCDividerNode>(
                                            test_divider_info_ptr->divider_ptr->nodes_.back());
                                }

                                if (it->second->is_start)
                                    divider_info_ptr->is_start_connect = true;
                                else
                                    divider_info_ptr->is_end_connect = true;

                                ////当找到重叠的divider不含split属性???
                                if (!is_split) {
                                    WriteNoSplitAttributeErr(test_divider_info_ptr->task_id,
                                                             stol(test_divider_info_ptr->divider_ptr->id_), test_node);
                                }

                                if (len > overlap_divider_err_len_ && Isvalid(task_id)) {
                                    auto &node = it->second->node_ptr;
                                    stringstream errss;
                                    errss << "[DIV_ID:" << divider_id
                                          << ",NODE_ID:" << node->id_ << "与任务:"
                                          << std::to_string(task_id) << ",DIV_ID:"
                                          << test_divider_info_ptr->divider_ptr->id_
                                          << ",NODE_ID:" << test_node->id_
                                          << "重叠超过阀值]";

                                    shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(CHECK_DIV_SPLIT_OVERLAP);
                                    shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(node->coord_);
                                    error_item->checkId = CHECK_DIV_SPLIT_OVERLAP;
                                    error_item->checkName = CHECK_DIV_SPLIT_OVERLAP_DESC;
                                    error_item->detail_ = errss.str();
                                    error_item->sourceId = to_string(divider_id);
                                    error_item->coord = coord;
                                    error_item->taskId_ = to_string(task_id);
                                    error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                                    shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                                    errNodeInfo->dataType = DATA_TYPE_WAY;
                                    errNodeInfo->dataId= node->id_;
                                    errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                                    error_item->errNodeInfo.emplace_back(errNodeInfo);
                                    error_output()->saveError(error_item);

                                }

                                node_set.insert(stol(test_node->id_));
                                matched = true;
                            }
                        }
                    }
                }

                if (matched) {
                    node_set.insert(it->first);
                }

                ++it;
            }
            error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_OVERLAP, check_total);
            ///清除其他已检测的点
            for (auto it_node_id:node_set) {
                it = split_node_info_map_.find(it_node_id);
                if (it != split_node_info_map_.end()) {
                    split_node_info_map_.erase(it);
                }
            }
        }

//        void DividerSplitCheck::CreateTraceShapeHandle(SHPHandle &shp_ptr, DBFHandle &dbf_ptr) {
//            DataCheckIO *io = output_device_.get();
//            DataCheckShapeIO *shape_io = dynamic_cast<DataCheckShapeIO *>(io);
//            if (shape_io == nullptr)
//                return;
//
//            std::string path = shape_io->GetPath();
//            std::string::size_type index = path.find_last_of('/');
//            if (index == path.npos)
//                return;
//
//            path = path.substr(0, index);
//            path += "/RAW_CHECK_TRACE_temp";
//            std::string shape = path + ".shp";
//            std::string dbf = path + ".dbf";
//            std::string shx = path + ".shx";
//
//            if (access(shape.c_str(), F_OK) == 0) {
//                if (unlink(shape.c_str()) == -1) {
//                    return;
//                }
//            }
//            if (access(dbf.c_str(), F_OK) == 0) {
//                if (unlink(dbf.c_str()) == -1) {
//                    return;
//                }
//            }
//            if (access(shx.c_str(), F_OK) == 0) {
//                if (unlink(shx.c_str()) == -1) {
//                    return;
//                }
//            }
//
//            ProductShpUtil::CreateArcZShp(shape, dbf, &shp_ptr, &dbf_ptr);
//            DBFAddField(dbf_ptr, "ID", FTInteger, 10, 0);
//        }

//        void DividerSplitCheck::WriteTraceGeom(SHPHandle shp_ptr, DBFHandle dbf_ptr,
//                                               shared_ptr<geos::geom::Polygon> &trace_poly,
//                                               shared_ptr<KDSDividerNode> &node_ptr) {
//            char zone[8] = {0};
//            geos::geom::CoordinateSequence *coords = trace_poly->getCoordinates();
//            size_t ss = coords->getSize();
//            double *x = new double[ss];
//            double *y = new double[ss];
//            double *z = new double[ss];
//            kd::automap::Coordinates::utmZone(node_ptr->y, node_ptr->x, zone);
//            for (size_t i = 0; i < ss; ++i) {
//                x[i] = coords->getX(i);
//                y[i] = coords->getY(i);
//                kd::automap::Coordinates::utm2ll(y[i], x[i], zone, y[i], x[i]);
//                z[i] = 0;
//            }
//            int count = ProductShpUtil::WriteArcZ(shp_ptr, ss, x, y, z);
//            DBFWriteIntegerAttribute(dbf_ptr, count, 0, count);
//            delete[] x;
//            delete[] y;
//            delete[] z;
//        }

        void DividerSplitCheck::TraceDivider() {
            SHPHandle shp_ptr = nullptr;
            DBFHandle dbf_ptr = nullptr;
//            if (output_details_)
//                CreateTraceShapeHandle(shp_ptr, dbf_ptr);

            std::set<long> node_set;
            auto it = split_node_info_map_.begin();
            while (it != split_node_info_map_.end()) {
                shared_ptr<DividerNodeInfo> &info = it->second;

                shared_ptr<DividerGeomInfo> &divider_info = divider_info_map_[stol(info->divider_ptr->id_)];
                geos::geom::Coordinate start, end;

                GeosObjRelationUtil::GetLineStringExtendPoint(divider_info->divider_geom_ptr, split_trace_distance_,
                                                              info->is_start, start, end);

                shared_ptr<geos::geom::Polygon> trace_poly =
                        GeosObjUtil::CreateTriGeometry(start.x, start.y, end.x, end.y, start.z,
                                                       split_trace_angle_, split_trace_distance_);

//                if (output_details_)
//                    WriteTraceGeom(shp_ptr, dbf_ptr, trace_poly, info->node_ptr);

                vector<long> link_node_ids;
                vector<void *> raw_query_objs;
                vector<DividerNodeInfo *> query_objs;
                divider_node_quadtree_->query(trace_poly->getEnvelopeInternal(), raw_query_objs);
                for (int i = 0; i < raw_query_objs.size(); ++i) {
                    DividerNodeInfo *inter_node = static_cast<DividerNodeInfo *>(raw_query_objs[i]);
                    ///跳过当前task对象
                    if (inter_node->task_id == info->task_id)
                        continue;

                    ///判断点是否在多边形内
                    if (GeosObjRelationUtil::GeometryIntersectPoint(trace_poly, start.z, inter_node->geom_ptr,
                                                                    node_search_buffer_z_)) {
                        query_objs.push_back(inter_node);
                    }
                }

                if (!query_objs.empty()) {
                    if (info->is_start)
                        divider_info->is_start_connect = true;
                    else
                        divider_info->is_end_connect = true;

                    for (auto ptr : query_objs) {
                        auto it_div = divider_info_map_.find(stol(ptr->divider_ptr->id_));
                        if (it_div != divider_info_map_.end()) {
                            if (ptr->is_start)
                                it_div->second->is_start_connect = true;
                            else
                                it_div->second->is_end_connect = true;
                        } else {
                            std::shared_ptr<DividerGeomInfo> div_ptr = CreateDividerGeomInfo(ptr, true);
                            divider_info_map_.insert(std::make_pair(stol(ptr->divider_ptr->id_), div_ptr));
                        }

                        //TODO 处理可能存在的搜到多余点

                        if (!ptr->is_split)
                            WriteNoSplitAttributeErr(ptr->task_id, stol(ptr->divider_ptr->id_), ptr->node_ptr);
                        else {
                            node_set.insert(stol(ptr->node_ptr->id_));
                        }

                        info->conn_nodes.insert(ptr);
                        ptr->conn_nodes.insert(info.get());
                    }
                    node_set.insert(stol(info->node_ptr->id_));
                }
                ++it;
            }

//            if (output_details_)
//                ProductShpUtil::ReleaseShp(&shp_ptr, &dbf_ptr);

            ///清除其他已确定的点
            for (auto it_node_id : node_set) {
                it = split_node_info_map_.find(it_node_id);
                if (it != split_node_info_map_.end()) {
                    split_node_info_map_.erase(it);
                }
            }
        }

        void DividerSplitCheck::CheckEmergencyStandbyLaneSplitDivider() {
            if (!CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV)) {
                return;
            }
            ///TYPE ==36（应急车道） 无需接边
            std::unordered_set<long> check_split_ids;
            for (auto it : split_node_info_map_) {
                check_split_ids.insert(it.first);
            }

            for (const auto &it : check_split_ids) {
                const auto &divider_ptr = split_node_info_map_[it]->divider_ptr;
                auto das = divider_ptr->atts_;
                if (das.size() == 0) {
                    LOG(ERROR) << "not find da at divider " << divider_ptr->id_;
                    continue;
                }

                shared_ptr<DCDividerAttribute> div_da;
                if (split_node_info_map_[it]->is_start) {
                    div_da =das.front();
                } else {
                    div_da = das.back();
                }
                long type = div_da->type_;
                if (type == 36) {
                    split_node_info_map_.erase(it);
                }
            }

            error_output()->addCheckItemInfo(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV, check_split_ids.size());

        }

        void DividerSplitCheck::ExportErrSplitNode() {
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV)) {
                for (const auto &it : split_node_info_map_) {
                    const auto &node = it.second->node_ptr;
                    long task_id = it.second->task_id;

                    if (Isvalid(task_id)) {
                        shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV);
                        stringstream errss;
                        errss << "[DIV_ID:" << it.second->divider_ptr->id_ << ",NODE_ID:"
                              << node->id_ << "未找到相应split属性]";
                        shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(node->coord_);
                        error_item->checkId = CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV;
                        error_item->checkName = CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV_DESC;
                        error_item->detail_ = errss.str();
                        error_item->sourceId = it.second->divider_ptr->id_;
                        error_item->coord = coord;
                        error_item->taskId_ = to_string(task_id);
                        error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                        shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                        errNodeInfo->dataType = DATA_TYPE_NODE;
                        errNodeInfo->dataId = node->id_;
                        errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_NODE;
                        error_item->errNodeInfo.emplace_back(errNodeInfo);
                        error_output()->saveError(error_item);
                    }
                }
            }
        }

        void DividerSplitCheck::GetRelatedDividered(DividerNodeInfo *test_point_ptr,
                                                    std::unordered_set<long> &id_list,
                                                    std::list<DividerNodeInfo *> &related_dividers) {

            shared_ptr<geos::geom::Geometry> geom_buffer(test_point_ptr->geom_ptr->buffer(group_search_buffer_len_));
            ///通过节点周边的缓冲区进行共点的查找
            vector<void *> raw_query_objs;
            vector<DividerNodeInfo *> query_objs;
            divider_node_quadtree_->query(geom_buffer->getEnvelopeInternal(), raw_query_objs);
            FilterQueryPoints(test_point_ptr, raw_query_objs, query_objs, group_search_buffer_len_,
                              node_search_buffer_z_, false);

            size_t num = query_objs.size();
            for (int i = 0; i < num; ++i) {
                DividerNodeInfo *ptr = query_objs[i];
                auto it = id_list.find(stol(ptr->node_ptr->id_));
                if (it == id_list.end()) {
                    id_list.insert(stol(ptr->node_ptr->id_));
                    related_dividers.emplace_back(ptr);

                    GetRelatedDividered(ptr, id_list, related_dividers);
                }
//        else///剔除可能的内部线(相邻divider共点,split除外)
//        {
//            id_list.erase(it);
//            auto it_ptr = related_dividers.begin();
//            while (it_ptr != related_dividers.end()){
//                if(ptr->ID == (*it_ptr)->ID){
//                    related_dividers.erase(it_ptr);
//                    break;
//                }
//                ++it_ptr;
//            }
//        }
            }
        }

        void DividerSplitCheck::RemoveLineWithOneNodeInGroup(std::vector<DividerNodeInfo *> &div_list,
                                                             std::unordered_map<long, int> &node_count_map, double x,
                                                             double y) {
            std::vector<DividerNodeInfo *> other_list;
            long addr;
            for (auto ptr: div_list) {
                addr = (long) (ptr->node_ptr.get());
                if (node_count_map[addr] == 1)
                    other_list.emplace_back(ptr);
            }

            div_list = other_list;
        }

        void
        DividerSplitCheck::RemoveTwoNodeALineInGroup(std::vector<DividerNodeInfo *> &div_list, double x, double y) {
            std::unordered_map<long, int> div_node_map;
            std::unordered_map<long, int>::iterator it_node;
            for (int i = 0; i < div_list.size(); ++i) {
                DividerNodeInfo *cur_node = div_list[i];
                it_node = div_node_map.find(stol(cur_node->divider_ptr->id_));
                if (it_node != div_node_map.end()) {
                    bool last_is_split = div_list[it_node->second]->is_split;

                    if (last_is_split) {
                        ///divider 两端都是split点,都保留
                        if (cur_node->is_split)
                            continue;
                        else {
                            ///divider另一端为split,当前为非split,保留之前
                            div_list[i] = nullptr;
                            continue;
                        }
                    }

                    ///divider另一端非split,当前为split,保留split
                    if (!last_is_split && cur_node->is_split) {
                        div_list[it_node->second] = cur_node;
                        div_list[i] = nullptr;
                        continue;
                    }

                    ///处理divider两端都不是split点,保留离中心最近的点
                    double len1 = GeometryUtil::distance(x, y, cur_node->node_ptr->coord_->x_, cur_node->node_ptr->coord_->y_);
                    DividerNodeInfo *test_node = div_list[it_node->second];
                    double len2 = GeometryUtil::distance(x, y, test_node->node_ptr->coord_->x_, test_node->node_ptr->coord_->y_);
                    if (len1 < len2) {
                        div_list[it_node->second] = cur_node;
                    }
                    div_list[i] = nullptr;
                } else {
                    div_node_map.insert(std::make_pair(stol(cur_node->divider_ptr->id_), i));
                }
            }
        }

        void DividerSplitCheck::GroupRelatedDividers(std::vector<DividerNodeInfo *> &div_list, double x, double y) {

            ///key 点内存地址,被引用了几次,目的:去除中间点
            std::unordered_map<long, int> ptr_map;
            std::unordered_map<long, int>::iterator it;
            long addr;
            for (auto ptr: div_list) {
                if (!ptr->is_split)
                    continue;
                addr = (long) (ptr->node_ptr.get());
                it = ptr_map.find(addr);
                if (it != ptr_map.end()) {
                    int num = it->second;
                    it->second = num + 1;
                } else {
                    ptr_map.insert(std::make_pair(addr, 1));
                }
            }

            if (ptr_map.size() != div_list.size()) {
                RemoveLineWithOneNodeInGroup(div_list, ptr_map, x, y);

                RemoveTwoNodeALineInGroup(div_list, x, y);
            } else {
                RemoveTwoNodeALineInGroup(div_list, x, y);
            }
        }

        void DividerSplitCheck::GroupRelatedDividers(std::list<DividerNodeInfo *> &related_dividers,
                                                     std::list<shared_ptr<GroupedDividers>> &grouped_dividers) {
            //TODO 处理复杂情况


            //获取中心点,用于排除过短
            double x = 0.0, y = 0.0;
            for (auto it : related_dividers) {
                x += it->node_ptr->coord_->x_;
                y += it->node_ptr->coord_->y_;
            }
            int size = (int) related_dividers.size();
            x = x / size;
            y = y / size;

            ///key is node id
//    std::unordered_map<long,DividerNodeInfo*> div_node_map;
//    std::unordered_map<long,DividerNodeInfo*>::iterator it_node;

            ///key is task id
            std::unordered_map<long, shared_ptr<std::vector<DividerNodeInfo *>>> sorted_divider_list;
            std::unordered_map<long, shared_ptr<std::vector<DividerNodeInfo *>>>::iterator it;
            for (auto ptr : related_dividers) {
                it = sorted_divider_list.find(ptr->task_id);
                if (it != sorted_divider_list.end()) {
                    it->second->emplace_back(ptr);
                } else {
                    shared_ptr<std::vector<DividerNodeInfo *>> divider_list_ptr = nullptr;
                    divider_list_ptr = std::make_shared<std::vector<DividerNodeInfo *>>();
                    sorted_divider_list.insert(std::make_pair(ptr->task_id, divider_list_ptr));
                    divider_list_ptr->emplace_back(ptr);
                }
            }


            it = sorted_divider_list.begin();
            while (it != sorted_divider_list.end()) {

                std::vector<DividerNodeInfo *> &d_info_list = *(it->second);
                ///去除多余的点,只保留最可能是边界处的点(之前的点有不含split属性的点,避免有的边界divider不含split属性漏报)
                GroupRelatedDividers(d_info_list, x, y);

                ///将边界点按进入或退出分组
                shared_ptr<GroupedDividers> start_group = nullptr;
                bool start_split = false;
                shared_ptr<GroupedDividers> end_group = nullptr;
                bool end_split = false;

                for (auto ptr : d_info_list) {
                    if (ptr == nullptr)
                        continue;

                    if (ptr->is_start) {
                        if (start_group == nullptr) {
                            start_group = std::make_shared<GroupedDividers>();
                            start_group->is_start = true;
                            start_group->task_id = ptr->task_id;
                        }
                        start_group->node_list.emplace_back(ptr);
                        if (ptr->is_split)
                            start_group->has_split = true;
                    } else {
                        if (end_group == nullptr) {
                            end_group = std::make_shared<GroupedDividers>();
                            end_group->is_start = false;
                            end_group->task_id = ptr->task_id;
                        }
                        end_group->node_list.emplace_back(ptr);
                        if (ptr->is_split)
                            end_group->has_split = true;
                    }
                }

                if (start_group != nullptr) {
                    grouped_dividers.push_back(start_group);
                }

                if (end_group != nullptr) {
                    grouped_dividers.push_back(end_group);
                }

                ++it;
            }
        }

        bool DividerSplitCheck::GetRelativeDividerGroup(shared_ptr<GroupedDividers> &group,
                                                        std::list<shared_ptr<GroupedDividers>> &groups,
                                                        std::vector<shared_ptr<GroupedDividers>> &results,
                                                        std::vector<bool> &unique_flags) {

            ///处理边界任务1对多的情况,并兼容task id无序
            results.clear();
            std::unordered_set<long> tasks;
            auto it = groups.begin();
            for (DividerNodeInfo *ptr:group->node_list) {
                if (!ptr)
                    continue;

                if (!ptr->conn_nodes.empty()) {
                    for (DividerNodeInfo *ptr_c: ptr->conn_nodes) {
                        if (ptr_c->task_id != group->task_id) {
                            tasks.insert(ptr_c->task_id);
                        }
                    }
                }
            }
            if (tasks.empty())
                return false;

            std::unordered_set<long>::iterator it_task;
            shared_ptr<GroupedDividers> result_ptr = nullptr;
            bool find = false;
            while (it != groups.end()) {
                if ((*it)->is_start != group->is_start) {
                    it_task = tasks.find((*it)->task_id);
                    if (it_task != tasks.end()) {
                        for (auto ptr : (*it)->node_list) {
                            if (!ptr)
                                continue;
                            find = false;
                            for (auto node_ptr:ptr->conn_nodes) {
                                if (node_ptr->task_id == group->task_id) {
                                    results.emplace_back(*it);
                                    find = true;
                                    break;
                                }
                            }
                            if (find)
                                break;
                        }
                        ///判断目标任务对应一个源任务,
                        if (results.size() != unique_flags.size()) {
                            bool is_one_task = true;
                            for (auto ptr : (*it)->node_list) {
                                if (!ptr)
                                    continue;

                                for (auto node_ptr:ptr->conn_nodes) {
                                    if (node_ptr->task_id != group->task_id) {
                                        is_one_task = false;
                                        break;
                                    }
                                }
                                if (!is_one_task)
                                    break;
                            }

                            unique_flags.push_back(is_one_task);
                        }
                    }
                }

                ++it;
            }
            return !results.empty();
        }

        void DividerSplitCheck::CheckSplitConnectInfo(std::vector<DividerNodeInfo *> &t_related_dividers, int &lane_num,
                                                      int &conn_num) {
            ///检查相关的道路当前部分是否都无对应split
            lane_num = 0;
            conn_num = 0;

            shared_ptr<DividerGeomInfo> cur_divider_ptr = nullptr;
            DividerNodeInfo *cur_divider_node_ptr = nullptr;

            for (int j = 0; j < t_related_dividers.size(); ++j) {
                cur_divider_node_ptr = t_related_dividers[j];
                if (cur_divider_node_ptr != nullptr) {
                    ++lane_num;
                    auto it = divider_info_map_.find(stol(cur_divider_node_ptr->divider_ptr->id_));
                    if (it != divider_info_map_.end()) {
                        cur_divider_ptr = it->second;

                        if (cur_divider_node_ptr->is_start) {
                            if (cur_divider_ptr->is_start_connect)
                                ++conn_num;
                        } else {
                            if (cur_divider_ptr->is_end_connect)
                                ++conn_num;
                        }
                    }
                }
            }
        }

        void DividerSplitCheck::ExcludeTasksBorderSplit() {

            /** 判断方法:旁边有divider都未追踪到或未被追踪到,则可能到边界 **/
            ///查找方向相同的一组
            std::unordered_set<long> check_split_ids;
            for (auto it : split_node_info_map_) {
                check_split_ids.insert(it.first);
            }

            while (!check_split_ids.empty()) {
                auto it = check_split_ids.begin();
                ///查找当前点关联的一组点,这组点任意两点间剧小于容差
                std::unordered_set<long> id_list;
                std::list<DividerNodeInfo *> related_dividers;
                std::shared_ptr<DividerNodeInfo> &divider_info = split_node_info_map_[*it];
                id_list.insert(stol(divider_info->node_ptr->id_));
                related_dividers.emplace_back(divider_info.get());

                ///获取的divider包括不含split点的divider,防止有的divider没有split点漏报
                GetRelatedDividered(divider_info.get(), id_list, related_dividers);

                ///将divider按方向分组
                std::list<shared_ptr<GroupedDividers>> grouped_related_dividers;
                GroupRelatedDividers(related_dividers, grouped_related_dividers);

                ///兼容乱序task_id,避免误报
                for (shared_ptr<GroupedDividers> group_ptr: grouped_related_dividers) {
                    std::vector<shared_ptr<GroupedDividers>> test_results;
                    std::vector<bool> unique_flags;
                    bool is_ok = true;
                    std::vector<shared_ptr<GroupedDividers>> erase_groups;
                    if (GetRelativeDividerGroup(group_ptr, grouped_related_dividers, test_results, unique_flags)) {
                        int first_lane_num, first_conn, second_lane_num, second_conn;
                        CheckSplitConnectInfo(group_ptr->node_list, first_lane_num, first_conn);
                        if (first_lane_num != first_conn) {
                            if (test_results.size() == 1) {
                                CheckSplitConnectInfo(test_results[0]->node_list, second_lane_num, second_conn);
                                is_ok = second_conn == second_lane_num;
                            } else {
                                for (shared_ptr<GroupedDividers> &test_group_ptr : test_results) {
                                    CheckSplitConnectInfo(test_group_ptr->node_list, second_lane_num, second_conn);
                                    if (second_conn != second_lane_num) {
                                        is_ok = false;
                                        break;
                                    }
                                }
                            }
                        }

                        erase_groups.emplace_back(group_ptr);
                        for (int i = 0; i < test_results.size(); ++i) {
                            if (unique_flags[i])
                                erase_groups.emplace_back(test_results[i]);
                        }
                    } else
                        erase_groups.emplace_back(group_ptr);

                    for (auto erase_group_ptr : erase_groups) {
                        for (auto result_node_ptr: erase_group_ptr->node_list) {
                            if (!result_node_ptr)
                                continue;

                            if (is_ok && result_node_ptr->is_split)
                                split_node_info_map_.erase(stol(result_node_ptr->node_ptr->id_));

                            check_split_ids.erase(stol(result_node_ptr->node_ptr->id_));
                        }
                    }
                }
            }
        }

        bool DividerSplitCheck::Isvalid(long task_id) {
            if (0 != task_id_ && task_id != task_id_) {
                return false;
            }
            return true;
        }

        void DividerSplitCheck::CheckConnectLaneDA(DividerNodeInfo *split_node1, DividerNodeInfo *split_node2) {

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DIRECTION_ERR)) {
                checkDividerInfo(CHECK_DIV_SPLIT_DIRECTION_ERR, CHECK_DIV_SPLIT_DIRECTION_ERR_DESC,
                                 KDSDivider::DIRECTION, split_node1->divider_ptr->direction_,
                                 split_node2->divider_ptr->direction_, split_node1, split_node2);
            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_R_LINE_ERR)) {
                checkDividerInfo(CHECK_DIV_SPLIT_R_LINE_ERR, CHECK_DIV_SPLIT_R_LINE_ERR_DESC, KDSDivider::R_LINE,
                                 split_node1->divider_ptr->rLine_, split_node2->divider_ptr->rLine_, split_node1,
                                 split_node2);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_TOLLFLAG_ERR)) {
                checkDividerInfo(CHECK_DIV_SPLIT_TOLLFLAG_ERR, CHECK_DIV_SPLIT_TOLLFLAG_ERR_DESC, KDSDivider::TOLLFLAG,
                                 split_node1->divider_ptr->tollFlag_, split_node2->divider_ptr->tollFlag_, split_node1,
                                 split_node2);
            }

            std::vector<shared_ptr<DCDividerAttribute>> das1 = split_node1->divider_ptr->atts_;
            std::vector<shared_ptr<DCDividerAttribute>> das2 = split_node2->divider_ptr->atts_;

            if (das1.size() == 0) {
                LOG(ERROR) << "not find da at divider " << split_node1->divider_ptr->id_;
                return;
            }
            if (das2.size() == 0) {
                LOG(ERROR) << "not find da at divider " << split_node2->divider_ptr->id_;
                return;
            }

            shared_ptr<DCDividerAttribute> div_da1, div_da2;
            if (split_node1->is_start) {
                div_da1 = das1.front();
            } else {
                div_da1 = das1.back();
            }
            if (split_node2->is_start) {
                div_da2 = das2.front();
            } else {
                div_da2 = das2.back();
            }


            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR, CHECK_DIV_SPLIT_DA_VIRTUAL_ERR_DESC,
                        KDSDividerAttribute::VIRTUAL, div_da1->virtual_, div_da2->virtual_, split_node1, split_node2,
                        div_da1, div_da2);
            }
            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_COLOR_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_COLOR_ERR, CHECK_DIV_SPLIT_DA_COLOR_ERR_DESC, KDSDividerAttribute::COLOR,
                        div_da1->virtual_, div_da2->virtual_, split_node1, split_node2, div_da1, div_da2);
            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_TYPE_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_TYPE_ERR, CHECK_DIV_SPLIT_DA_TYPE_ERR_DESC, KDSDividerAttribute::TYPE,
                        div_da1->virtual_, div_da2->virtual_, split_node1, split_node2, div_da1, div_da2);

            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR, CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR_DESC,
                        KDSDividerAttribute::DRIVE_RULE, div_da1->virtual_, div_da2->virtual_, split_node1, split_node2,
                        div_da1, div_da2);
            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_OVERLAY_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_OVERLAY_ERR, CHECK_DIV_SPLIT_DA_OVERLAY_ERR_DESC,
                        KDSDividerAttribute::OVERLAY, div_da1->overlay_, div_da2->overlay_, split_node1, split_node2,
                        div_da1, div_da2);

            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_MATERIAL_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_MATERIAL_ERR, CHECK_DIV_SPLIT_DA_MATERIAL_ERR_DESC,
                        KDSDividerAttribute::MATERIAL, div_da1->material_, div_da2->material_, split_node1, split_node2,
                        div_da1, div_da2);
            }

            if (CheckListConfig::getInstance().IsNeedCheck(CHECK_DIV_SPLIT_DA_WIDTH_ERR)) {
                checkDA(CHECK_DIV_SPLIT_DA_WIDTH_ERR, CHECK_DIV_SPLIT_DA_WIDTH_ERR_DESC, KDSDividerAttribute::WIDTH,
                        div_da1->width_, div_da2->width_, split_node1, split_node2, div_da1, div_da2);
            }
        }

        void DividerSplitCheck::checkDividerInfo(const string check_id, const string check_desc, const string &da_key,
                                                 long da_value1, long da_value2, DividerNodeInfo *split_node1,
                                                 DividerNodeInfo *split_node2) {
            long divider_id = stol(split_node1->divider_ptr->id_);
            long node_id = stol(split_node1->node_ptr->id_);
//            long da_value1 = split_node1->divider_ptr->getPropertyLong(da_key);
//            long da_value2 = split_node2->divider_ptr->getPropertyLong(da_key);
            bool checkRes;
            if (da_key == KDSDivider::DIRECTION) {
                if (da_value1 == 2) {
                    if (da_value2 == 3) {
                        checkRes = false;
                    } else {
                        checkRes = true;
                    }
                } else if (da_value1 == 3) {
                    if (da_value2 == 2) {
                        checkRes = false;
                    } else {
                        checkRes = true;
                    }
                } else {
                    checkRes = true;
                }
            } else {
                checkRes = da_value1 == da_value2;
            }

            if (!checkRes) {
                stringstream errss;
                errss << "[DIV_ID:" << divider_id
                      << ",NODE_ID:" << node_id << "与任务:"
                      << std::to_string(split_node2->task_id) << ",DIV_ID:"
                      << split_node2->divider_ptr->id_
                      << ",NODE_ID:" << split_node2->node_ptr->id_;
                if (da_key == KDSDivider::DIRECTION) {
                    errss << "接边后的通行方向错误]";
                } else if (da_key == KDSDivider::TOLLFLAG) {
                    errss << "接边DIVIDER的收费站分隔线不同]";
                } else if (da_key == KDSDivider::R_LINE) {
                    errss << "接边DIVIDER的是否参考线不同]";
                }

                shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(check_id);
                shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(split_node1->node_ptr->coord_);
                error_item->checkId = check_id;
                error_item->checkName = check_desc;
                error_item->detail_ = errss.str();
                error_item->sourceId = to_string(divider_id);
                error_item->coord = coord;
                error_item->taskId_ = to_string(split_node1->task_id);
                error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error_item->errNodeInfo.emplace_back(errNodeInfo);
                error_output()->saveError(error_item);

            }
        }

        void
        DividerSplitCheck::checkDA(const string check_id, const string check_desc, const string &da_key, long da_value1,
                                   long da_value2, DividerNodeInfo *split_node1, DividerNodeInfo *split_node2,
                                   const shared_ptr<DCDividerAttribute> &div_da1,
                                   const shared_ptr<DCDividerAttribute> &div_da2) {
            long divider_id = stol(split_node1->divider_ptr->id_);
            long node_id = stol(split_node1->node_ptr->id_);

//            long da_value1 = div_da1->getPropertyLong(da_key);
//            long da_value2 = div_da2->getPropertyLong(da_key);

            if (!(da_value1 == da_value2)) {
                stringstream errss;
                errss << "[DIV_ID:" << divider_id
                      << ",NODE_ID:" << node_id << "与任务:"
                      << std::to_string(split_node2->task_id) << ",DIV_ID:"
                      << split_node2->divider_ptr->id_
                      << ",NODE_ID:" << split_node2->node_ptr->id_;
                if (da_key == KDSDividerAttribute::WIDTH) {
                    errss << "接边DA的分隔线宽度不同]";
                } else if (da_key == KDSDividerAttribute::MATERIAL) {
                    errss << "接边DA的分隔线材质不同]";
                } else if (da_key == KDSDividerAttribute::OVERLAY) {
                    errss << "接边DA的覆盖类型不同]";
                } else if (da_key == KDSDividerAttribute::VIRTUAL) {
                    errss << "接边DA的虚拟分隔线类型不同]";
                } else if (da_key == KDSDividerAttribute::COLOR) {
                    errss << "接边DA的分隔线颜色不同]";
                } else if (da_key == KDSDividerAttribute::TYPE) {
                    errss << "接边DA的分割线类型不同]";
                } else if (da_key == KDSDividerAttribute::DRIVE_RULE) {
                    errss << "接边DA的通行类型不同]";
                }

                shared_ptr<SplitCheckError> error_item = make_shared<SplitCheckError>(check_id);
                shared_ptr<DCCoord> coord = std::static_pointer_cast<DCCoord>(split_node1->node_ptr->coord_);
                error_item->checkId = check_id;
                error_item->checkName = check_desc;
                error_item->detail_ = errss.str();
                error_item->sourceId = div_da1->id_;
                error_item->coord = coord;
                error_item->taskId_ = to_string(split_node1->task_id);
                error_item->dataKey_ = DATA_TYPE_LANE+error_item->taskId_+DATA_TYPE_LAST_NUM;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
                errNodeInfo->dataType = DATA_TYPE_RELATION;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_ATTRIBUTE;
                error_item->errNodeInfo.emplace_back(errNodeInfo);
                error_output()->saveError(error_item);
            }
        }
    }

}