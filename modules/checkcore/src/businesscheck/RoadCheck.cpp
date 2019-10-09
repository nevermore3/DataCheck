//
// Created by zhangxingang on 19-1-25.
//

#include <businesscheck/RoadCheck.h>
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>
#include <util/CommonCheck.h>
#include <shp/ShpData.hpp>


namespace kd {
    namespace dc {

        RoadCheck::RoadCheck(string fileName) : SCHCheck(fileName) {}

        string RoadCheck::getId() {
            return id;
        }

        void RoadCheck::SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager) {
            map_data_manager_ = mapDataManager;
        }

        bool RoadCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            SetMapDataManager(mapDataManager);

            //adasNode曲率检查
            CurvatureValueCheck(errorOutput);

            // 属性点间距离检查
            CheckAdjacentNodeDistance(errorOutput);


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

        bool RoadCheck::LoadTrafficRule() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + "TRAFFICRULE";
            ShpData shpFile(filePath);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCTrafficRule> trafficRule = make_shared<DCTrafficRule>();

                //读取属性信息
                trafficRule->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                trafficRule->node_type_ = shpFile.readLongField(i, "NODE_TYPE");
                trafficRule->node_conn_id_ = shpFile.readLongField(i, "NODECONN_I");
                trafficRule->type_ = shpFile.readLongField(i, "TYPE");
                trafficRule->vehicle_ = shpFile.readStringField(i, "VEHICLE");
                trafficRule->time_ = shpFile.readStringField(i, "TIME");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    trafficRule->coord_ = coord;
                }
                map_traffic_rule_.insert(make_pair(stol(trafficRule->id_), trafficRule));
                SHPDestroyObject(shpObject);
            }
            return true;
        }

        bool RoadCheck::LoadRoadNode() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + "ROAD_NODE";
            ShpData shpFile(filePath);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCRoadNode> roadNode = make_shared<DCRoadNode>();

                //读取属性信息
                roadNode->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                roadNode->cnode_id_ = shpFile.readLongField(i, "C_NODE_ID");


                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    roadNode->coord_ = coord;
                }
                map_road_nodes_.insert(make_pair(stol(roadNode->id_), roadNode));
                SHPDestroyObject(shpObject);
            }
            return true;
        }

        bool RoadCheck::LoadCNode() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + "C_NODE";
            ShpData shpFile(filePath);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCCNode> cNode = make_shared<DCCNode>();

                //读取属性信息
                cNode->id_ = std::to_string(shpFile.readIntField(i, "ID"));

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    cNode->coord_ = coord;
                }
                map_cnodes_.insert(make_pair(stol(cNode->id_), cNode));
                SHPDestroyObject(shpObject);
            }
            return true;
        }

        bool RoadCheck::LoadCNodeConn() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + "C_NODECONN";
            ShpData shpFile(filePath);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCCNodeConn> cNodeConn = make_shared<DCCNodeConn>();

                //读取属性信息
                cNodeConn->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                cNodeConn->fRoad_id_ = shpFile.readLongField(i, "EROAD_ID");
                cNodeConn->tRoad_id_ = shpFile.readLongField(i, "QROAD_ID");
                cNodeConn->cNode_id_ = shpFile.readLongField(i, "C_NODE_ID");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    cNodeConn->coord_ = coord;
                }
                map_cnode_conn_.insert(make_pair(stol(cNodeConn->id_), cNodeConn));
                SHPDestroyObject(shpObject);
            }
            return true;
        }

        bool RoadCheck::LoadNodeConn() {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + "NODECONN";
            ShpData shpFile(filePath);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCNodeConn> nodeConn = make_shared<DCNodeConn>();

                //读取属性信息
                nodeConn->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                nodeConn->fRoad_id_ = shpFile.readLongField(i, "EROAD_ID");
                nodeConn->tRoad_id_ = shpFile.readLongField(i, "QROAD_ID");
                nodeConn->node_id_ = shpFile.readLongField(i, "NODE_ID");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    nodeConn->coord_ = coord;
                }
                map_node_conn_.insert(make_pair(stol(nodeConn->id_), nodeConn));
                SHPDestroyObject(shpObject);
            }
            return true;
        }

        void RoadCheck::BuildInfo() {
            // 填充road的 fnode 和 tnode
            auto roads = map_data_manager_->roads_;
            for (auto &iter : roads) {
                long fNodeID = stol(iter.second->f_node_id);
                long tNodeID = stol(iter.second->t_node_id);
                if (map_road_nodes_.find(fNodeID) != map_road_nodes_.end()) {
                    iter.second->fNode_ = map_road_nodes_[fNodeID];
                }
                if (map_road_nodes_.find(tNodeID) != map_road_nodes_.end()) {
                    iter.second->tNode_ = map_road_nodes_[tNodeID];
                }
            }

            // 填充roadnode中的 cnode
            for (auto &iter : map_road_nodes_) {
                long cNodeID = iter.second->cnode_id_;
                if (map_cnodes_.find(cNodeID) != map_cnodes_.end()) {
                    iter.second->cNode_ = map_cnodes_[cNodeID];
                }
            }

            // 填充nodeconn中的froad 和troad 和roadnode
            for (auto &iter : map_node_conn_) {
                string fRoadID = to_string(iter.second->fRoad_id_);
                string tRoadID = to_string(iter.second->tRoad_id_);
                long nodeID = iter.second->node_id_;
                if (roads.find(fRoadID) != roads.end()) {
                    iter.second->fRoad_ = roads[fRoadID];
                }
                if (roads.find(tRoadID) != roads.end()) {
                    iter.second->tRoad_ = roads[tRoadID];
                }
                if (map_road_nodes_.find(nodeID) != map_road_nodes_.end()) {
                    iter.second->roadNode_ = map_road_nodes_[nodeID];
                }

            }

            // 填充cnodeconn中的 froad 和 troad 和 cnode
            for (auto &iter : map_cnode_conn_) {
                string fRoadID = to_string(iter.second->fRoad_id_);
                string tRoadID = to_string(iter.second->tRoad_id_);
                long cNodeID = iter.second->cNode_id_;
                if (roads.find(fRoadID) != roads.end()) {
                    iter.second->fRoad_ = roads[fRoadID];
                }
                if (roads.find(tRoadID) != roads.end()) {
                    iter.second->tRoad_ = roads[tRoadID];
                }
                if (map_cnodes_.find(cNodeID) != map_cnodes_.end()) {
                    iter.second->cNode_ = map_cnodes_[cNodeID];
                }
            }
            
        }

    }
}