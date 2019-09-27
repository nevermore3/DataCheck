//
// Created by zhangxingang on 19-1-25.
//

#include <businesscheck/RoadCheck.h>
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>
#include <util/CommonCheck.h>
#include "util/product_shp_util.h"
#include <shp/ShpData.hpp>
#include <util/KDGeoUtil.hpp>
using namespace kd::automap;
namespace kd {
    namespace dc {

        RoadCheck::RoadCheck(string fileName) : SCHCheck(fileName) {}

        string RoadCheck::getId() {
            return id;
        }

        void RoadCheck::SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager) {
            map_data_manager_ = mapDataManager;
        }

        bool RoadCheck::LoadLGLaneGroupIndex() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + kLgRoadNodeIndex;

            DbfData dbfFile(filePath);
            if (!dbfFile.isInit()) {
                LOG(ERROR) << "Open dbfFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = dbfFile.getRecords();
            long roadID = 0;
            long lgID = 0;
            long fIndex = 0;
            long tIndex = 0;
            for (size_t i = 0; i < recordNums; i++) {

                //读取属性信息
                lgID = dbfFile.readLongField(i, "LG_ID");
                roadID = dbfFile.readLongField(i, "ROAD_ID");
                fIndex = dbfFile.readLongField(i, "F_INDEX");
                tIndex = dbfFile.readLongField(i, "T_INDEX");
                if (map_road_lg_index_.find(roadID) == map_road_lg_index_.end()) {

                    map<long, vector<pair<long, long>>> mapFromIndexLG;
                    vector<pair<long,long>> tIndexLG;
                    tIndexLG.emplace_back(make_pair(tIndex, lgID));
                    mapFromIndexLG.insert(make_pair(fIndex, tIndexLG));
                    map_road_lg_index_.insert(make_pair(roadID, mapFromIndexLG));

                } else {

                    map<long, vector<pair<long, long>>> &mapFromIndexLG = map_road_lg_index_[roadID];
                    if (mapFromIndexLG.find(fIndex) == mapFromIndexLG.end()) {
                        vector<pair<long, long>> tIndexLG;
                        tIndexLG.emplace_back(make_pair(tIndex, lgID));
                        mapFromIndexLG.insert(make_pair(fIndex, tIndexLG));
                    } else {
                        mapFromIndexLG[fIndex].emplace_back(make_pair(tIndex, lgID));
                    }
                }
            }
            return true;
        }

        map<long, pair<long, long>> RoadCheck::GetLaneGroupsIndex(long roadID) {
            /*
             * 进入和退出虚拟路口的lanegroup 丢弃
             */
            map<long, pair<long, long>> fromToIndexLG;
            if (map_road_lg_index_.find(roadID) == map_road_lg_index_.end()) {
                return fromToIndexLG;
            }
            map<long, vector<pair<long, long>>> roadLaneGroup = map_road_lg_index_[roadID];
            auto iter = roadLaneGroup.begin();
            // 去除进入虚拟路口的laneGroup
            while (iter->second.size() != 1) {
                iter++;
            }

            long start = iter->first;
            while (iter != roadLaneGroup.end()) {
                // 去除退出虚拟路口的laneGroup
                if (iter->second.size() == 1 && iter->first == start) {
                    fromToIndexLG.insert(make_pair(iter->first, make_pair(iter->second.front().first, iter->second.front().second)));
                    start = iter->second.front().first;
                }
                iter++;
            }
            return fromToIndexLG;
        }



        void RoadCheck::DoNode2DividerSlope(long lgID, long fromIndex, long toIndex, vector<shared_ptr<DCSCHInfo>> &nodes,
                                            shared_ptr<CheckErrorOutput> &errorOutput) {

            if (map_data_manager_->laneGroups_.find(to_string(lgID)) == map_data_manager_->laneGroups_.end()) {
                return;
            }
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::NODE_RELEVANT_OBJ_SLOPE_ERROR);
            auto laneGroup = map_data_manager_->laneGroups_[to_string(lgID)];
            vector<shared_ptr<DCDivider>> dividers;
            for (const auto &lane : laneGroup->lanes_) {
                dividers.push_back(lane->leftDivider_);
                //dividers.push_back(lane->rightDivider_);
            }
            // adasnode 在该lanegroup中的起始位置
            long start = 0;
            // adasnode 在该lanegroup中的终止位置
            long end = 0;
            long tempIndex = 0;
            while (tempIndex < nodes.size()) {
                if (nodes[tempIndex]->obj_index_ == fromIndex) {
                    start = tempIndex;
                    break;
                }
                tempIndex++;
            }

            while (tempIndex < nodes.size()) {
                if (nodes[tempIndex]->obj_index_ == toIndex) {
                    end = tempIndex;
                    if (tempIndex < nodes.size() - 1 && nodes[tempIndex + 1]->obj_index_ != toIndex) {
                        break;
                    }
                }
                tempIndex++;
            }
            // adasnode点 距离 divider小于50厘米 就认为关联此divider
            double distance = 50;

            shared_ptr<DCDivider>divider = dividers.front();
            char zone[8] = {0};
            auto nodeCoord = GeosObjUtil::create_coordinate(nodes[start]->coord_, zone);
            double PtA[2] = {nodeCoord->x,  nodeCoord->y};
            double PtB[2] = {0};
            double PtC[4] = {0};
            int index = 0;
            for (const auto & iter : dividers) {
                double dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(iter->line_->getCoordinatesRO()),
                                                    PtA, PtB, PtC, index);
                if (dis < distance) {
                    divider = iter;
                    break;
                }
            }

            for (size_t i = start; i <= end; i++) {
                nodeCoord = GeosObjUtil::create_coordinate(nodes[i]->coord_, zone);
                PtA[0] = nodeCoord->x;
                PtA[1] = nodeCoord->y;

                double dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(divider->line_->getCoordinatesRO()),
                                                    PtA, PtB, PtC, index);
                if (dis > distance) {
                    //重新选择divider
                    double minDistance = DBL_MAX;
                    for (const auto &iter : dividers) {
                        dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(iter->line_->getCoordinatesRO()),
                                                     PtA, PtB, PtC, index);
                        if (dis < distance) {
                            divider = iter;
                            break;
                        }
                        if (dis < minDistance) {
                            minDistance = dis;
                            divider = iter;
                        }
                    }
                }
                shared_ptr<DCCoord>coordA = nullptr;
                shared_ptr<DCCoord>coordB = nullptr;
                // 比较adas node点和关联divider 最近两个点的坡度
                if (index >= divider->nodes_.size() - 1) {
                    coordA = divider->nodes_[index - 1]->coord_;
                    coordB = divider->nodes_[index]->coord_;
                } else {
                    coordA = divider->nodes_[index]->coord_;
                    coordB = divider->nodes_[index + 1]->coord_;
                }

                double distanceAB = GeosObjUtil::get_length_of_node(coordA, coordB);
                double avgSlope = (coordB->z_ - coordA->z_) / distanceAB;
                double diffSlope = fabs(avgSlope - nodes[i]->slope_);
                if (diffSlope > threshold) {
                    auto error = DCSCHInfoError::createByKXS_01_035("ADAS_NODE", stol(divider->id_), i,
                                                                    nodes[i]->slope_, avgSlope, threshold);
                    errorOutput->saveError(error);
                }

            }
        }

        /*
         * 每个AdasNode点的坡度和关联的Divider对象中距离最近的两个形点计算出的坡度对比
         */
        void RoadCheck::AdasNodeRelevantDividerSlope(shared_ptr<CheckErrorOutput> &errorOutput) {

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_035;
            size_t  total = 0;
            //形点索引
            for (const auto &adasNode : map_obj_schs_) {
                long roadID = adasNode.first;
                vector<shared_ptr<DCSCHInfo>> nodes = adasNode.second;
                total += nodes.size();
                // 得到和该roadID想关联的laneGroup集合
                map<long, pair<long, long>> mapLgIndex = GetLaneGroupsIndex(roadID);
                if (mapLgIndex.empty()) {
                    continue;
                }

                for (const auto &iter : mapLgIndex) {
                    long lgID = iter.second.second;
                    long fromIndex = iter.first;
                    long toIndex = iter.second.first;
                    DoNode2DividerSlope(lgID, fromIndex, toIndex, nodes, errorOutput);
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        bool RoadCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            SetMapDataManager(mapDataManager);

            //adasNode曲率检查
            CurvatureValueCheck(errorOutput);

            // 属性点间距离检查
            CheckAdjacentNodeDistance(errorOutput);

            if (LoadLGLaneGroupIndex()) {
                AdasNodeRelevantDividerSlope(errorOutput);
            }


            if(CheckItemValid(CHECK_ITEM_KXS_ROAD_002)){
                check_road_divider_intersect(mapDataManager, errorOutput);
            }
            check_road_node_height(mapDataManager, errorOutput);
            check_road_node(mapDataManager, errorOutput);
            return true;
        }

        void RoadCheck::check_road_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                                     shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &ptr_lane_groups = mapDataManager->laneGroups_;
            int total = 0;
            shared_ptr<DCError> ptr_error = nullptr;
            for (const auto &lg : ptr_lane_groups) {
                auto lane_group_id = lg.first;
                auto ptr_lane_group = lg.second;

                auto roads = CommonUtil::get_roads_by_lg(mapDataManager, lane_group_id);
                auto ptr_dividers = CommonUtil::get_dividers_by_lg(mapDataManager, lane_group_id);

                if (!ptr_dividers.empty()) {
                    auto left_ptr_divider = ptr_dividers.front();
                    auto right_ptr_divider = ptr_dividers.back();
                    for (const auto &road_id : roads) {
                        total++;
                        auto ptr_road = CommonUtil::get_road(mapDataManager, road_id);
                        if (ptr_road) {
                            if (ptr_lane_group->is_virtual_ == 1) {
                                continue;
                            }
                            if (ptr_road->direction_ == 2) {
                                // 最左最右DIVIDER检查交点
                                if (road_divider_intersect(mapDataManager, ptr_road, lane_group_id, left_ptr_divider) ||
                                    road_divider_intersect(mapDataManager, ptr_road, lane_group_id,
                                                           right_ptr_divider)) {
                                    ptr_error = DCRoadCheckError::createByKXS_04_002(ptr_road->id_, lane_group_id);
                                    ptr_error->taskId_ = right_ptr_divider->task_id_;
                                    ptr_error->coord = ptr_road->nodes_[0];
                                    shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(ptr_error->coord);
                                    errNodeInfo->dataType = DATA_TYPE_WAY;
                                    errNodeInfo->dataLayer = MODEL_NAME_ROAD;
                                    ptr_error->errNodeInfo.emplace_back(errNodeInfo);
                                    errorOutput->saveError(ptr_error);
                                }
                            } else if (ptr_road->direction_ == 1) {
                                // 只检查最右交点
                                if (road_divider_intersect(mapDataManager, ptr_road, lane_group_id,
                                                           right_ptr_divider)) {
                                    ptr_error = DCRoadCheckError::createByKXS_04_002(ptr_road->id_, lane_group_id);
                                    ptr_error->taskId_ = right_ptr_divider->task_id_;
                                    ptr_error->coord = ptr_road->nodes_[0];
                                    shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(ptr_error->coord);
                                    errNodeInfo->dataType = DATA_TYPE_WAY;
                                    errNodeInfo->dataLayer = MODEL_NAME_ROAD;
                                    ptr_error->errNodeInfo.emplace_back(errNodeInfo);
                                    errorOutput->saveError(ptr_error);

                                }
                            }
                        } else {
                            LOG(ERROR) << "get road failed! road id : " << road_id;
                        }
                    }
                }
            }

            errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_002,total);
        }


        bool RoadCheck::road_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCRoad> &ptr_road,
                                               const string &lane_group_id,
                                               const shared_ptr<DCDivider> &ptr_divider ) {
            bool ret = false;
            auto ptr_road_line_string = get_road_line_string(mapDataManager, ptr_road, lane_group_id);
            auto ptr_div_line_string = CommonUtil::get_divider_line_string(ptr_divider->nodes_);


            if (ptr_road_line_string && ptr_div_line_string) {
                CoordinateSequence *coor_seq = nullptr;
                ret = kd::automap::KDGeoUtil::isLineCross(ptr_road_line_string.get(),
                                                          ptr_div_line_string.get(), &coor_seq, 1);

                if (ret) {
                    auto f_node = ptr_div_line_string->getCoordinates()->front();
                    auto t_node = ptr_div_line_string->getCoordinates()->back();
                    bool is_same = true;
                    for (size_t i = 0; i < coor_seq->size(); i++) {
                        if (!GeosObjUtil::is_same_coord(coor_seq->getAt(i), f_node) &&
                            !GeosObjUtil::is_same_coord(coor_seq->getAt(i), t_node)) {
                            is_same = false;
                            break;
                        }
                    }
                    if (is_same) {
                        ret = false;
                    }

                }
            }
            return ret;
        }


        shared_ptr<geos::geom::LineString>
        RoadCheck::get_road_line_string(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCRoad> &ptr_road,
                                        const string &lane_group_id) {

            string road_id = ptr_road->id_;
            const auto &road2_lane_group_index = mapDataManager->road2LaneGroup2NodeIdxs_;
            auto road_node_index_iter = road2_lane_group_index.find(road_id);

            vector<shared_ptr<DCCoord>> road_nodes;
            if (road_node_index_iter != road2_lane_group_index.end()) {
                const unordered_map<string, std::pair<long, long>> &lane_group_index = road_node_index_iter->second;
                auto node_index_iter = lane_group_index.find(lane_group_id);
                if (node_index_iter != lane_group_index.end()) {
                    auto index_pair = node_index_iter->second;
                    if (index_pair.first >= 0 && index_pair.first < ptr_road->nodes_.size() &&
                        index_pair.second >= 0 && index_pair.second < ptr_road->nodes_.size()) {
                        if (index_pair.first < index_pair.second) {
                            for (long idx = index_pair.first; idx <= index_pair.second; idx++) {
                                road_nodes.emplace_back(ptr_road->nodes_.at(idx));
                            }
                        } else if (index_pair.first > index_pair.second) {
                            for (long idx = index_pair.second; idx <= index_pair.first; idx++) {
                                road_nodes.emplace_back(ptr_road->nodes_.at(idx));
                            }
                        }
                    }
                }
            }

            return CommonUtil::get_line_string(road_nodes);
        }

        void RoadCheck::check_road_node_height(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            double heightchange = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER);

            const auto &ptr_roads_map = mapDataManager->roads_;
            for (const auto &road : ptr_roads_map) {
                auto ptr_road = road.second;
                if (!ptr_road->valid_) {
                    continue;
                }
                size_t nodeCount = ptr_road->nodes_.size();
                if (nodeCount < 2)
                    continue;

                vector<NodeCheck> error_index_pair;
                bool check = false;

                for (int i = 1; i < nodeCount; i++) {

                    auto node1 = ptr_road->nodes_[i - 1];
                    auto node2 = ptr_road->nodes_[i];

                    //间距判断
                    double distance = kd::automap::KDGeoUtil::distanceLL(node1->x_, node1->y_,
                                                                         node2->x_, node2->y_);

                    //坡度判断
                    double slopLimit = distance * heightchange;
                    double realDeltaZ = node1->z_ - node2->z_;
                    if (fabs(realDeltaZ) > slopLimit) {
                        check = true;
                        NodeCheck node_check{};
                        node_check.pre_index = i - 1;
                        node_check.index = i;
                        node_check.diff_height = realDeltaZ;
                        node_check.distance = distance;
                        error_index_pair.emplace_back(node_check);
                    }
                }
                if (check) {
                    shared_ptr<DCRoadCheckError> error =
                            DCRoadCheckError::createByKXS_04_003(ptr_road->id_, error_index_pair);
                    errorOutput->saveError(error);
                }
            }
        }

        void RoadCheck::check_road_node(shared_ptr<MapDataManager> mapDataManager,
                                        shared_ptr<CheckErrorOutput> errorOutput) {
            int total = 0;
            for (auto road_iter : mapDataManager->roads_) {
                auto ptr_road = road_iter.second;
                if (ptr_road) {
                    total+=ptr_road->nodes_.size();
                    // 检查结点是否重复
                    check_road_node_repeat(errorOutput, ptr_road);

                    // 检查结点间角度是否过大出现拐点
                    if(CheckItemValid(CHECK_ITEM_KXS_ROAD_007)) {
                        check_road_node_angle(errorOutput, ptr_road);

                    }

                    // 结点间距检查
                    if(CheckItemValid(CHECK_ITEM_KXS_ROAD_008)) {
                        check_road_node_distance(errorOutput, ptr_road);
                    }
                }
            }
            if(CheckItemValid(CHECK_ITEM_KXS_ROAD_007)) {
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_007, total);
            }
            if(CheckItemValid(CHECK_ITEM_KXS_ROAD_008)) {
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_008, total);
            }
        }

        void RoadCheck::check_road_node_repeat(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road) {
            vector<shared_ptr<NodeError>> ptr_error_nodes;
            auto first_node = ptr_road->nodes_.front();
            shared_ptr<NodeError> ptr_e_node = make_shared<NodeError>();
            ptr_e_node->index = 0;
            ptr_e_node->ptr_coord = first_node;
            ptr_error_nodes.emplace_back(ptr_e_node);
            for (int i = 1; i < ptr_road->nodes_.size(); i++) {
                if (first_node->x_ == ptr_road->nodes_.at(i)->x_ &&
                    first_node->y_ == ptr_road->nodes_.at(i)->y_) {
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = ptr_road->nodes_.at(i);
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                } else {
                    if (ptr_error_nodes.size() > 1) {
                        auto ptr_error = DCRoadCheckError::createByKXS_04_006(ptr_road->id_, ptr_error_nodes);
                        errorOutput->saveError(ptr_error);
                    }
                    ptr_error_nodes.clear();
                    first_node = ptr_road->nodes_.at(i);
                    shared_ptr<NodeError> ptr_cur_e_node = make_shared<NodeError>();
                    ptr_cur_e_node->index = i;
                    ptr_cur_e_node->ptr_coord = first_node;
                    ptr_error_nodes.emplace_back(ptr_cur_e_node);
                }
            }

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCRoadCheckError::createByKXS_04_006(ptr_road->id_, ptr_error_nodes);
                errorOutput->saveError(ptr_error);
            }
            ptr_error_nodes.clear();
        }

        void RoadCheck::check_road_node_angle(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road) {
            double road_node_angle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::ROAD_NODE_ANGLE);
            auto ptr_error_nodes = CommonCheck::AngleCheck(ptr_road->nodes_, road_node_angle);
            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCRoadCheckError::createByKXS_04_007(ptr_road->id_, ptr_error_nodes);
                ptr_error->taskId_ = ptr_road->task_id_;
                errorOutput->saveError(ptr_error);
            }
        }

        void RoadCheck::check_road_node_distance(shared_ptr<CheckErrorOutput> errorOutput,
                                                 shared_ptr<DCRoad> ptr_road) {
            double road_node_dis = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::ROAD_NODE_DISTANCE);
            auto ptr_error_nodes = CommonCheck::DistanceCheck(ptr_road->nodes_, road_node_dis);

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCRoadCheckError::createByKXS_04_008(ptr_road->id_, ptr_error_nodes);
                ptr_error->coord = ptr_road->nodes_[0];
                ptr_error->taskId_ = ptr_road->task_id_;
                errorOutput->saveError(ptr_error);
            }
        }
    }
}