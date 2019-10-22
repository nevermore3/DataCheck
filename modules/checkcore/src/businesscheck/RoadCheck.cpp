
#include <businesscheck/RoadCheck.h>
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>
#include <util/CommonCheck.h>
#include <shp/ShpData.hpp>
#include "util/product_shp_util.h"
#include <shp/ShpData.hpp>
#include <util/KDGeoUtil.hpp>
#include <util/geos_obj_relation_util.h>

using namespace kd::automap;
namespace kd {
    namespace dc {

        RoadCheck::RoadCheck(string fileName) : SCHCheck(fileName) {}

        string RoadCheck::getId() {
            return id;
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
            
            if (data_manager()->laneGroups_.find(to_string(lgID)) == data_manager()->laneGroups_.end()) {
                return;
            }
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::NODE_RELEVANT_OBJ_SLOPE_ERROR);
            auto laneGroup = data_manager()->laneGroups_[to_string(lgID)];
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
            // adasnode点 距离 divider小于1.7米(根据测试数据集配置的) 就认为关联此divider
            double distance = 1.7;

            shared_ptr<DCDivider>divider = dividers.front();
            char zone[8] = {0};
            auto nodeCoord = GeosObjUtil::create_coordinate(nodes[start]->coord_, zone);
            double PtA[2] = {nodeCoord->x,  nodeCoord->y};
            double PtB[2] = {0};
            double PtC[4] = {0};
            int index = 0;
            double minDistance = DBL_MAX;
            // 若adasNode点到divider的距离小于distance，则认为和此divider关联
            // 若adasNode到所有divid的距离都大于distance，则取距离最小的divider
            for (const auto & iter : dividers) {
                double dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(iter->line_->getCoordinatesRO()),
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

            for (size_t i = start; i <= end; i++) {
                nodeCoord = GeosObjUtil::create_coordinate(nodes[i]->coord_, zone);
                PtA[0] = nodeCoord->x;
                PtA[1] = nodeCoord->y;

                double dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(divider->line_->getCoordinatesRO()),
                                                    PtA, PtB, PtC, index);
                if (dis > distance) {
                    //重新选择divider
                    minDistance = DBL_MAX;
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
                // 距离都大于distance，重新选择的divider，需要重新计算index
                if (dis > distance) {
                    dis = KDGeoUtil::pt2LineDist(const_cast<CoordinateSequence*>(divider->line_->getCoordinatesRO()),
                                                 PtA, PtB, PtC, index);
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

            set_data_manager(mapDataManager);

            set_error_output(errorOutput);

            set_error_output(errorOutput);

            //adasNode曲率检查
            CurvatureValueCheck(errorOutput);

            //坡度值检查
            SlopeValueCheck(errorOutput);

            //航向角值检查
            HeadingValueCheck(errorOutput);

            // 属性点间距离检查
            CheckAdjacentNodeDistance(errorOutput);

            // 每一Road的形状点周围1.5米内必有一个关联该Road的AdasNode
            RoadRelevantAdasNode(errorOutput);

            // AdasNode点离关联Road的距离不超过0.1米
            AdasNodeVerticalDistance(errorOutput);

            // 检查road的起始终止坐标位置
            CheckStartEndNodeLocation(errorOutput);

            //定位目标与道路关联关系存在性检查
            if (CheckItemValid(CHECK_ITEM_KXS_ROAD_021)) {
                CheckRLORoad();
            }


            if (LoadLGLaneGroupIndex()) {
                AdasNodeRelevantDividerSlope(errorOutput);
            }


            if(CheckItemValid(CHECK_ITEM_KXS_ROAD_002)){
                check_road_divider_intersect(mapDataManager, errorOutput);
            }
            check_road_node_height(mapDataManager, errorOutput);
            check_road_node(mapDataManager, errorOutput);

            preCheckConn();

            checkCNodeConn();

            checkNodeConn();

            CheckProhibition();

            CheckNodesAndCNodeRelation(errorOutput);

            // 检查是否有通行孤立的道路
            CheckIsolatedRoad();

            // 检查道路等级通行
            CheckRoadGradesInterConnection();

            CheckRoadNode();

            return true;
        }


        /*
         * 每一Road的形状点周围1.5米内必有一个关联该Road的AdasNode
         */
        void RoadCheck::RoadRelevantAdasNode(shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_019;
            size_t total = 0;

            double threshold = 1.5;
            /*
             * 1、找到一条Road对象
             * 2、在map_obj_schs中找到对应 该Road对象的属性点集合
             * 3、遍历该Road对象的形点
             * 3、按照属性点中的obj_index索引，第一个遍历到的属性点就是离该形点最近的点
             */
            auto roads = data_manager()->roads_;
            for (const auto &road : roads) {
                long roadID = stol(road.first);
                if (map_obj_schs_.find(roadID) == map_obj_schs_.end()) {
                    continue;
                }
                auto adasNodes = map_obj_schs_[roadID];
                total += road.second->nodes_.size();
                // 关联同一个Road对象的属性点的索引
                size_t j = 0;
                for (size_t i = 0; i < road.second->nodes_.size(); i++) {
                    while (j < adasNodes.size()) {
                        if (adasNodes[j]->obj_index_ == i) {
                            // 找到第一个和Road形点关联的属性点
                            double distance = GeosObjUtil::get_length_of_node(road.second->nodes_[i], adasNodes[j]->coord_);
                            if (distance > threshold) {
                                auto error = DCRoadCheckError::createByKXS_04_019(roadID, i, road.second->nodes_[i], 1);
                                errorOutput->saveError(error);
                            }

                            // Road的起点和终点之处（buffer20cm）必有一个关联该Road的AdasNode
                            if (i == 0 || i == road.second->nodes_.size() - 1) {
                                if (distance > 0.2) {
                                    auto error = DCRoadCheckError::createByKXS_04_019(roadID, i, road.second->nodes_[i], 2);;
                                    errorOutput->saveError(error);
                                }
                            }

                            j++;
                            break;
                        }

                        // 该形点没有被属性点关联
                        if (j != 0 && (adasNodes[j-1]->obj_index_ < i && adasNodes[j]->obj_index_ > i)) {
                            double distance1 = GeosObjUtil::get_length_of_node(road.second->nodes_[i], adasNodes[j-1]->coord_);
                            double distance2 = GeosObjUtil::get_length_of_node(road.second->nodes_[i], adasNodes[j]->coord_);
                            if (distance1 > threshold && distance2 > threshold) {
                                auto error = DCRoadCheckError::createByKXS_04_019(roadID, i, road.second->nodes_[i], 1);
                                errorOutput->saveError(error);
                            }
                            break;
                        }

                        j++;
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        // AdasNode点离关联Road的距离不超过0.1米
        void RoadCheck::AdasNodeVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_020;
            size_t total = 0;

            for (const auto &adasNodes : map_obj_schs_) {
                long roadID = adasNodes.first;
                total += adasNodes.second.size();
                string strRoadID = to_string(roadID);
                if (data_manager()->roads_.find(strRoadID) == data_manager()->roads_.end()) {
                    continue;
                }
                auto road = data_manager()->roads_[strRoadID];
                for (const auto &node : adasNodes.second) {
                    shared_ptr<geos::geom::Point> point = GeosObjUtil::CreatePoint(node->coord_);
                    double distance = GeosObjUtil::GetVerticleDistance(road->line_, point);
                    if (distance > 0.1) {
                        auto error = DCRoadCheckError::createByKXS_04_020(stol(node->id_), node->coord_);
                        errorOutput->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }


        /*
         *  对于每一条road， 在roadnode中找到起fnode 和 tnode 和其形点比较坐标
         */
        void RoadCheck::CheckStartEndNodeLocation(shared_ptr<CheckErrorOutput> &errorOutput) {

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_018;

            //Read file:RoadNode
            data_manager()->initKxsNode(kRoadNode);
            map<long, shared_ptr<KxsData>> roadNodes = data_manager()->getKxfData(kRoadNode);

            //Read file:Road
            data_manager()->initPolyline(kRoad);
            map<long, shared_ptr<KxsData>> roads = data_manager()->getKxfData(kRoad);

            size_t total = roads.size();

            //auto roads = map_data_manager_->roads_;
            for (const auto &iter : roads) {
                shared_ptr<PolyLine> road = static_pointer_cast<PolyLine>(iter.second);
                vector<shared_ptr<DCCoord>> coords = road->coords_;
                long sNodeID = road->getPropertyLong(SNODE_ID);
                long eNodeID = road->getPropertyLong(ENODE_ID);

                // 在ROADNODE表中 找到fnode 和 tnode
                if (roadNodes.find(sNodeID) != roadNodes.end()) {
                    auto sNode = static_pointer_cast<KxfNode>(roadNodes[sNodeID]);
                    if (!(*(coords.front()) == *(sNode->coord_))) {
                        auto error = DCRoadCheckError::createByKXS_04_018(iter.first, sNodeID, sNode->coord_, 0);
                        errorOutput->saveError(error);
                    }
                }

                if (roadNodes.find(eNodeID) != roadNodes.end()) {
                    auto eNode = static_pointer_cast<KxfNode>(roadNodes[eNodeID]);
                    if (!(*(coords.back()) == *(eNode->coord_))) {
                        auto error = DCRoadCheckError::createByKXS_04_018(iter.first, eNodeID, eNode->coord_, 1);
                        errorOutput->saveError(error);
                    }
                }

            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);

            // clear memory
            data_manager()->clearData(kRoadNode);
        }

        /*
         * 定位目标与道路关联关系存在性检查
         * 一个 定位目标HD_POLYGON，地面定位线HD_POLYLINE，杆HD_POINT，交通灯TRAFFIC_LIGHT，交通牌TRAFFIC_SIGN
         * 检查其是否在表HD_R_LO_ROAD中被一个ROAD关联，若未被关联,报错。 若关联多次，报错（交通灯除外）
         */
        void RoadCheck::CheckRLORoad() {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_021;
            size_t total = 0;

            //Read File: HD_R_LO_ROAD
            data_manager()->initRelation(kRLoRoad);
            map<long, shared_ptr<KxsData>> rLoRoads = data_manager()->getKxfData(kRLoRoad);

            if (rLoRoads.empty()) {
                return;
            }

            //key: 类型ID, value: 同类型的目标ID
            map<long, vector<long>> mapType2IDs;

            for (const auto &iter : rLoRoads) {
                long type = iter.second->getPropertyLong(TYPE);
                long objID = iter.second->getPropertyLong(LO_ID);

                if (mapType2IDs.find(type) == mapType2IDs.end()) {
                    vector<long> array;
                    array.push_back(objID);
                    mapType2IDs.insert(make_pair(type, array));
                } else {
                    mapType2IDs[type].push_back(objID);
                }
            }

            map<string, long> name2Type {{kPolygon, 1},
                                         {kPolyline, 2},
                                         {kPoint, 3},
                                         {kTrafficSign, 4},
                                         {kTrafficLight, 5}};

            // key: tableName, value: loadFuntion
            map<string, function<void(string)>> name2Function {
                    {kPolygon, bind(&MapDataManager::initPolygon, data_manager(), std::placeholders::_1)},
                    {kPolyline, bind(&MapDataManager::initPolyline, data_manager(), std::placeholders::_1, true)},
                    {kPoint, bind(&MapDataManager::initKxsNode, data_manager(), std::placeholders::_1)},
                    {kTrafficSign, bind(&MapDataManager::initPolygon, data_manager(), std::placeholders::_1)},
                    {kTrafficLight, bind(&MapDataManager::initKxsNode, data_manager(), std::placeholders::_1)}
            };

            for (const auto &iter : name2Function) {
                string tableName = iter.first;
                auto loadFunction = iter.second;
                if (mapType2IDs.find(name2Type[tableName]) == mapType2IDs.end()) {
                    continue;
                }
                long type = name2Type[tableName];
                // Read File
                loadFunction(tableName);
                map<long, shared_ptr<KxsData>> objs = data_manager()->getKxfData(tableName);

                for (const auto &obj : objs) {
                    long id = obj.first;
                    auto array = mapType2IDs[type];
                    int num = count(array.begin(), array.end(), id);
                    //// 交通灯可以关联多个road
                    if (num == 1 || (num > 1 && tableName == kTrafficLight)) {
                        continue;
                    }

                    auto error = DCRoadCheckError::createByKXS_04_021(tableName, id, num);
                    error_output()->saveError(error);
                }
                // clear memory
                data_manager()->clearData(tableName);
            }
            error_output()->addCheckItemInfo(checkItemInfo);
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
        void RoadCheck::preCheckConn(){
            LoadTrafficRule();

            LoadRoadNode();

            LoadCNode();

            LoadCNodeConn();

            LoadNodeConn();

            BuildInfo();
            // 建立Node 和 road、之间的拓扑关系
            BuildNodeID2Road();
            ///建立禁止通行信息MAP
            BuildProhibitionMap();
        }
        void RoadCheck::checkNodeConn(){
            for(auto node_id_to_froad_:map_node_id_to_froad_){
                auto froad_v = node_id_to_froad_.second;
                auto troad_v = map_node_id_to_troad_.find(node_id_to_froad_.first);
                if(troad_v!=map_node_id_to_troad_.end()) {
                    for (auto froad:froad_v) {
                        for (auto troad:troad_v->second) {
                            string froad_id_troad_id=froad->id_+"_"+troad->id_+"_"+to_string(node_id_to_froad_.first);
                            auto nodeconn = map_ft_road_id_node_id_to_conn_id.find(froad_id_troad_id);
                            if(nodeconn == map_ft_road_id_node_id_to_conn_id.end()){
                                auto ptr_error = DCRoadCheckError::createByKXS_04_012(1,"0",froad->id_,troad->id_,froad->tNode_->coord_);
                                error_output()->saveError(ptr_error);
                            }else{
                                map_ft_road_id_node_id_to_conn_id.erase(froad_id_troad_id);
                            }
                        }
                    }
                }else{
                    LOG(INFO) << "not find to road for node,node id is "<<node_id_to_froad_.first;
                }
            }
            ///遍历多余的记录

            for(auto conn:map_ft_road_id_node_id_to_conn_id){
                auto node_conn = map_node_conn_.find(conn.second);
                auto ptr_error = DCRoadCheckError::createByKXS_04_012(2,node_conn->second->id_,to_string(node_conn->second->fRoad_id_),to_string(node_conn->second->tRoad_id_),node_conn->second->coord_);
                error_output()->saveError(ptr_error);
            }
        }
        void RoadCheck::checkCNodeConn(){
            for(auto cnode:map_cnodes_){
                ///cnode 关联的roadnode集合
                auto cnode_nodes = map_cnode_node.find(cnode.first);
                for(auto node_id:cnode_nodes->second){
                    auto road_topo_from = map_node_id_to_froad_.find(node_id);
                    if(road_topo_from!=map_node_id_to_froad_.end()){
                        ///road_node关联的所有road
                        auto road_from_v = road_topo_from->second;
                        for(auto road_it:road_from_v){
                            if(road_it->fow_ == 2){
                                ///跳过交叉点内道路
                                continue;
                            }
                            ///需要对比的数据
                            set<long> t_road_ids;
                            long from_road_id = stol(road_it->id_);
                            getTRoadByFRoad(cnode_nodes->first,from_road_id,t_road_ids);
                            set<long> checkedNodes;
                            auto road_to_v = map_node_id_to_troad_.find(node_id);
                            checkedNodes.insert(node_id);
                            if(road_to_v!=map_node_id_to_troad_.end()) {
                                for (auto road_t_it:road_to_v->second) {
                                    findAccessibleRoad(cnode.first, from_road_id, road_t_it, node_id, t_road_ids,
                                                       checkedNodes);
                                }
                            }else{
                                LOG(INFO) << "node id:" << node_id << " not find to road!";
                            }
                            ///未遍历到的多余记录
                            if(t_road_ids.size()!=0){
                                for(auto t_road_id:t_road_ids){
                                    if(road_it->direction_==1 && from_road_id == t_road_id){
                                        continue;
                                    }
                                    auto road_node = map_road_nodes_.find(node_id);
                                    auto ptr_error = DCRoadCheckError::createByKXS_04_009(2,road_it->id_,to_string(t_road_id),road_node->second->coord_);
                                    error_output()->saveError(ptr_error);
                                }
                            }
                        }
                    }
                }
            }

            error_output()->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_009,item_data_total);
        }
        void RoadCheck::findAccessibleRoad(long cnode_id,long from_road_id,shared_ptr<DCRoad> t_road,long t_road_start_node_id,set<long> &t_road_ids,set<long> &checkedNodes){
            if(t_road->fow_ == 2){
                long road_t_it_end_node_id = stol(t_road->f_node_id) == t_road_start_node_id ? stol(t_road->t_node_id) : stol(t_road->f_node_id);
                if(checkedNodes.find(road_t_it_end_node_id)!= checkedNodes.end()){
                    ///避免重复循环
                    return;
                }
                checkedNodes.insert(road_t_it_end_node_id);
                auto road_to_v = map_node_id_to_troad_.find(road_t_it_end_node_id);
                if(road_to_v!=map_node_id_to_troad_.end()){
                    for(auto road_t_it:road_to_v->second){
                        findAccessibleRoad(cnode_id,from_road_id,road_t_it,road_t_it_end_node_id,t_road_ids,checkedNodes);
                    }
                }else{
                    LOG(INFO) << "node id:" << road_t_it_end_node_id << " not find to road!";
                }

            } else {
                item_data_total++;
                auto t_road_id = t_road_ids.find(stol(t_road->id_));
                if(t_road_id == t_road_ids.end()){
                    auto ptr_error = DCRoadCheckError::createByKXS_04_009(1,to_string(from_road_id),t_road->id_,t_road->fNode_->coord_);
                    error_output()->saveError(ptr_error);
                }else{
                    auto froad_to_cnode = map_froad_to_cnode.find(from_road_id);
                    if(froad_to_cnode ==map_froad_to_cnode.end() || froad_to_cnode->second.find(cnode_id) == froad_to_cnode->second.end()){
                        auto ptr_error = DCRoadCheckError::createByKXS_04_009(3,to_string(from_road_id),t_road->id_,t_road->fNode_->coord_);
                        error_output()->saveError(ptr_error);
                    }
                    auto troad_to_cnode = map_troad_to_cnode.find(stol(t_road->id_));
                    if(troad_to_cnode ==map_troad_to_cnode.end() || troad_to_cnode->second.find(cnode_id) == troad_to_cnode->second.end()){
                        auto ptr_error = DCRoadCheckError::createByKXS_04_009(3,to_string(from_road_id),t_road->id_,t_road->fNode_->coord_);
                        error_output()->saveError(ptr_error);
                    }

                    t_road_ids.erase(stol(t_road->id_));
                }
            }
        }
        void RoadCheck::getTRoadByFRoad(long cnode_id,long from_road_id,set<long> &t_road_set){
            string key = to_string(from_road_id)+"_"+to_string(cnode_id);
            auto t_road_v = map_froad_troad.find(key);
            if(t_road_v!= map_froad_troad.end())
                for(auto road_id:t_road_v->second){
                    if(t_road_set.find(road_id) == t_road_set.end()){
                        t_road_set.insert(road_id);
                    }
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
                if (!shpObject )
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
                if (!shpObject || shpObject->nSHPType != SHPT_POINT)
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
                if(roadNode->cnode_id_>0){
                    auto cnode_nodes = map_cnode_node.find(roadNode->cnode_id_);
                    if(cnode_nodes!=map_cnode_node.end()){
                        cnode_nodes->second.emplace_back(stol(roadNode->id_));
                    } else{
                        vector<long> node_ids;
                        node_ids.emplace_back(stol(roadNode->id_));
                        map_cnode_node.insert(make_pair(roadNode->cnode_id_,node_ids));
                    }

                    map_node_cnode.insert(make_pair(stol(roadNode->id_),roadNode->cnode_id_));
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
                if (!shpObject || shpObject->nSHPType != SHPT_POINT)
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
                if (!shpObject || shpObject->nSHPType != SHPT_POINT)
                    continue;

                shared_ptr<DCCNodeConn> cNodeConn = make_shared<DCCNodeConn>();

                //读取属性信息
                cNodeConn->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                cNodeConn->fRoad_id_ = shpFile.readLongField(i, "EROAD_ID");
                cNodeConn->tRoad_id_ = shpFile.readLongField(i, "QROAD_ID");
                cNodeConn->cNode_id_ = shpFile.readLongField(i, "C_NODE_ID");

                string key = to_string(cNodeConn->fRoad_id_)+"_"+to_string(cNodeConn->cNode_id_);
                auto froad_troads = map_froad_troad.find(key);
                if(froad_troads!= map_froad_troad.end()){
                    froad_troads->second.emplace_back(cNodeConn->tRoad_id_);
                }else{
                    vector<long> troad_ids;
                    troad_ids.emplace_back(cNodeConn->tRoad_id_);
                    map_froad_troad.insert(make_pair(key,troad_ids));
                }

                auto troad_froads = map_cconn_troad_froad.find(cNodeConn->tRoad_id_);
                if(troad_froads!= map_cconn_troad_froad.end()){
                    troad_froads->second.emplace_back(cNodeConn->fRoad_id_);
                }else{
                    vector<long> froad_ids;
                    froad_ids.emplace_back(cNodeConn->fRoad_id_);
                    map_cconn_troad_froad.insert(make_pair(cNodeConn->tRoad_id_,froad_ids));
                }

                auto fraod_cnode = map_froad_to_cnode.find(cNodeConn->fRoad_id_);
                if(fraod_cnode == map_froad_to_cnode.end()){
                    set<long> cNode_ids;
                    cNode_ids.insert(cNodeConn->cNode_id_);
                    map_froad_to_cnode.insert(make_pair(cNodeConn->fRoad_id_,cNode_ids));
                }else{
                    auto cnode = fraod_cnode->second.find(cNodeConn->cNode_id_);
                    if(cnode == fraod_cnode->second.end()){
                        fraod_cnode->second.insert(cNodeConn->cNode_id_);
                    }
                }

                auto traod_cnode = map_troad_to_cnode.find(cNodeConn->tRoad_id_);
                if(traod_cnode == map_troad_to_cnode.end()){
                    set<long> cNode_ids;
                    cNode_ids.insert(cNodeConn->cNode_id_);
                    map_troad_to_cnode.insert(make_pair(cNodeConn->tRoad_id_,cNode_ids));
                }else{
                    auto cnode = traod_cnode->second.find(cNodeConn->cNode_id_);
                    if(cnode == traod_cnode->second.end()){
                        traod_cnode->second.insert(cNodeConn->cNode_id_);
                    }
                }

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
                if (!shpObject || shpObject->nSHPType != SHPT_POINT)
                    continue;

                shared_ptr<DCNodeConn> nodeConn = make_shared<DCNodeConn>();

                //读取属性信息
                nodeConn->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                long id = stol(nodeConn->id_);
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
                long troad_id = nodeConn->tRoad_id_;

                auto troad_froads = map_conn_troad_froad.find(nodeConn->tRoad_id_);
                if(troad_froads!= map_conn_troad_froad.end()){
                    troad_froads->second.emplace_back(nodeConn->fRoad_id_);
                }else{
                    vector<long> froad_ids;
                    froad_ids.emplace_back(nodeConn->fRoad_id_);
                    map_conn_troad_froad.insert(make_pair(nodeConn->tRoad_id_,froad_ids));
                }

                map_node_conn_.insert(make_pair(stol(nodeConn->id_), nodeConn));
                ///避免双向道路记录重复,增加node_id_
                string froad_id_to_troad_id = to_string(nodeConn->fRoad_id_)+"_"+to_string(nodeConn->tRoad_id_)+"_"+to_string(nodeConn->node_id_);
                if(map_ft_road_id_node_id_to_conn_id.find(froad_id_to_troad_id) != map_ft_road_id_node_id_to_conn_id.end()){
                    auto ptr_error = DCRoadCheckError::createByKXS_04_012(4,"0",to_string(nodeConn->fRoad_id_),to_string(nodeConn->tRoad_id_),nodeConn->coord_);
                    error_output()->saveError(ptr_error);
                }else{
                    map_ft_road_id_node_id_to_conn_id.insert(make_pair(froad_id_to_troad_id,stol(nodeConn->id_)));
                }

                SHPDestroyObject(shpObject);
            }
            error_output()->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_012,map_ft_road_id_node_id_to_conn_id.size());
            return true;
        }

        void RoadCheck::BuildInfo() {
            // 填充road的 fnode 和 tnode
            auto roads = data_manager()->roads_;
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





        void RoadCheck::BuildNodeID2Road() {
            auto roads = data_manager()->roads_;
            for (const auto &iter : roads) {
                long fNodeID = stol(iter.second->f_node_id);
                long tNodeID = stol(iter.second->t_node_id);

                if (iter.second->direction_ == 1 || iter.second->direction_ == 2) {
                    // 入度
                    if (map_node_id_to_froad_.find(tNodeID) == map_node_id_to_froad_.end()) {
                        vector<shared_ptr<DCRoad>> vRoad;
                        vRoad.emplace_back(iter.second);
                        map_node_id_to_froad_.insert(make_pair(tNodeID, vRoad));
                    } else {
                        map_node_id_to_froad_[tNodeID].emplace_back(iter.second);
                    }

                    // 出度
                    if (map_node_id_to_troad_.find(fNodeID) == map_node_id_to_troad_.end()) {
                        vector<shared_ptr<DCRoad>> vRoad;
                        vRoad.emplace_back(iter.second);
                        map_node_id_to_troad_.insert(make_pair(fNodeID, vRoad));
                    } else {
                        map_node_id_to_troad_[fNodeID].emplace_back(iter.second);
                    }

                    if (iter.second->direction_ == 1) {
                        // 入度
                        if (map_node_id_to_froad_.find(fNodeID) == map_node_id_to_froad_.end()) {
                            vector<shared_ptr<DCRoad>> vRoad;
                            vRoad.emplace_back(iter.second);
                            map_node_id_to_froad_.insert(make_pair(fNodeID, vRoad));
                        } else {
                            map_node_id_to_froad_[fNodeID].emplace_back(iter.second);
                        }

                        // 出度
                        if (map_node_id_to_troad_.find(tNodeID) == map_node_id_to_troad_.end()) {
                            vector<shared_ptr<DCRoad>> vRoad;
                            vRoad.emplace_back(iter.second);
                            map_node_id_to_troad_.insert(make_pair(tNodeID, vRoad));
                        } else {
                            map_node_id_to_troad_[tNodeID].emplace_back(iter.second);
                        }
                    }

                } else if (iter.second->direction_ == 3) {
                    // 逆向
                    // 入度
                    if (map_node_id_to_froad_.find(fNodeID) == map_node_id_to_froad_.end()) {
                        vector<shared_ptr<DCRoad>> vRoad;
                        vRoad.emplace_back(iter.second);
                        map_node_id_to_froad_.insert(make_pair(fNodeID, vRoad));
                    } else {
                        map_node_id_to_froad_[fNodeID].emplace_back(iter.second);
                    }

                    // 出度
                    if (map_node_id_to_troad_.find(tNodeID) == map_node_id_to_troad_.end()) {
                        vector<shared_ptr<DCRoad>> vRoad;
                        vRoad.emplace_back(iter.second);
                        map_node_id_to_troad_.insert(make_pair(tNodeID, vRoad));
                    } else {
                        map_node_id_to_troad_[tNodeID].emplace_back(iter.second);
                    }

                } else {
                    LOG(ERROR) << "Road's direction Error, road id is :"<< iter.first;
                }
            }
        }
        void RoadCheck::BuildProhibitionMap(){
            for(auto it:map_traffic_rule_){
                long node_type = it.second->node_type_;
                if( node_type== 1){
                    ///简单路口
                    auto conn = map_node_conn_.find(it.second->node_conn_id_);
                    if(conn!=map_node_conn_.end()){
                        long froad_id = conn->second->fRoad_id_;
                        long troad_id = conn->second->tRoad_id_;

                        auto troad_froads = map_prohibition_conn.find(troad_id);
                        if(troad_froads!= map_prohibition_conn.end()){
                            troad_froads->second.emplace_back(froad_id);
                        }else{
                            vector<long> froad_ids;
                            froad_ids.emplace_back(froad_id);
                            map_prohibition_conn.insert(make_pair(troad_id,froad_ids));
                        }

                    }
//                    else{
//                        ///禁止通行信息的简单路口联通关系不存在
//
//
//                        auto ptr_error = DCRoadCheckError::createByKXS_04_015(1,it.first,it.second->node_conn_id_);
//                        error_output()->saveError(ptr_error);
//                    }
                } else if( node_type == 2){
                    ///复杂路口
                    auto conn = map_cnode_conn_.find(it.second->node_conn_id_);
                    if(conn!=map_cnode_conn_.end()){
                        long froad_id = conn->second->fRoad_id_;
                        long troad_id = conn->second->tRoad_id_;

                        auto troad_froads = map_prohibition_cconn.find(troad_id);
                        if(troad_froads!= map_prohibition_cconn.end()){
                            troad_froads->second.emplace_back(froad_id);
                        }else{
                            vector<long> froad_ids;
                            froad_ids.emplace_back(froad_id);
                            map_prohibition_cconn.insert(make_pair(troad_id,froad_ids));
                        }
                    }
//                    else{
//                        ///禁止通行信息的复杂路口联通关系不存在
//                        auto ptr_error = DCRoadCheckError::createByKXS_04_015(2,it.first,it.second->node_conn_id_);
//                        error_output()->saveError(ptr_error);
//                    }

                }
            }
//            error_output()->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_015,map_traffic_rule_.size());

        }
        void RoadCheck::CheckNodesAndCNodeRelation(shared_ptr<CheckErrorOutput> &errorOutput) {

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_010;
            size_t  total = 0;
            auto roads = data_manager()->roads_;
            // road是否访问过，防止重复访问
            map<string, int> visit;

            for (const auto &iter : roads) {
                if (iter.second->fow_ != 2 || visit[iter.first] == 1) {
                    continue;
                }
                long cNodeID = iter.second->fNode_->cnode_id_;

                stack<shared_ptr<DCRoad>> s;
                s.push(iter.second);
                visit[iter.first] = 1;

                while (!s.empty()) {
                    total++;
                    auto road = s.top();
                    s.pop();
                    string tNodeID;
                    if (road->direction_ == 3) {
                        tNodeID = road->f_node_id;
                    } else {
                        tNodeID = road->t_node_id;
                    }
                    if (map_road_nodes_[stol(tNodeID)]->cnode_id_ != cNodeID) {
                        auto error = DCRoadCheckError::createByKXS_04_010(stol(iter.second->f_node_id),
                                                                          stol(tNodeID),
                                                                          cNodeID,
                                                                          map_road_nodes_[stol(tNodeID)]->cnode_id_);
                        errorOutput->saveError(error);
                    }

                    vector<shared_ptr<DCRoad>> vRoad = map_node_id_to_troad_[stol(tNodeID)];
                    for (auto &t : vRoad) {
                        if (t->fow_ != 2 || visit[t->id_] == 1) {
                            //已经访问过，或非复杂路口中道路
                            continue;
                        }
                        visit[t->id_] = 1;
                        s.push(t);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        void RoadCheck::CheckIsolatedRoad() {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_011;
            auto roads = data_manager()->roads_;
            size_t total = roads.size();

            for (const auto &iter : roads) {
                long fNodeID = stol(iter.second->f_node_id);
                long tNodeID = stol(iter.second->t_node_id);
                if (iter.second->direction_ == 2) {
                    auto from_road = map_node_id_to_froad_.find(tNodeID);
                    if(map_node_id_to_troad_.find(tNodeID) == map_node_id_to_troad_.end()&& from_road!=map_node_id_to_froad_.end() &&from_road->second.size()>1){
                        // 起点的入度和终点的出度同时为空，则为孤立的道路
                        auto error = DCRoadCheckError::createByKXS_04_011(stol(iter.first));
                        error_output()->saveError(error);
                    }

                    auto to_road = map_node_id_to_troad_.find(fNodeID);
                    if(map_node_id_to_froad_.find(fNodeID) == map_node_id_to_froad_.end()&& to_road!=map_node_id_to_troad_.end() &&to_road->second.size()>1){
                        // 起点的入度和终点的出度同时为空，则为孤立的道路
                        auto error = DCRoadCheckError::createByKXS_04_011(stol(iter.first));
                        error_output()->saveError(error);
                    }

                } else if (iter.second->direction_ == 3) {

                    auto from_road = map_node_id_to_froad_.find(fNodeID);
                    if(map_node_id_to_troad_.find(fNodeID) == map_node_id_to_troad_.end()&& from_road!=map_node_id_to_froad_.end() &&from_road->second.size()>1){
                        // 起点的入度和终点的出度同时为空，则为孤立的道路
                        auto error = DCRoadCheckError::createByKXS_04_011(stol(iter.first));
                        error_output()->saveError(error);
                    }

                    auto to_road = map_node_id_to_troad_.find(tNodeID);
                    if(map_node_id_to_froad_.find(tNodeID) == map_node_id_to_froad_.end()&& to_road!=map_node_id_to_troad_.end() &&to_road->second.size()>1){
                        // 起点的入度和终点的出度同时为空，则为孤立的道路
                        auto error = DCRoadCheckError::createByKXS_04_011(stol(iter.first));
                        error_output()->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void RoadCheck::CheckRoadGradesInterConnection() {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ROAD_013;
            auto roads = data_manager()->roads_;
            size_t total = roads.size();

            for (const auto &iter : roads) {
                long roadClass = iter.second->r_class_;
                long fNodeID = stol(iter.second->f_node_id);
                long tNodeID = stol(iter.second->t_node_id);
                // 入度
                vector<shared_ptr<DCRoad>> fromRoads;
                //出度
                vector<shared_ptr<DCRoad>> toRoads;
                if (iter.second->direction_ == 3) {
                    //逆向
                    fromRoads = map_node_id_to_froad_[tNodeID];
                    toRoads = map_node_id_to_troad_[fNodeID];
                } else {
                    fromRoads = map_node_id_to_froad_[fNodeID];
                    toRoads = map_node_id_to_troad_[tNodeID];
                }
                if (fromRoads.empty() && toRoads.empty()) {
                    continue;
                }
                // 若两端道路有一个等级比他高的，则认为联通
                int flag = 0;
                for (auto &road : fromRoads) {
                    if (road->r_class_ <=  roadClass) {
                        flag = 1;
                        break;
                    }
                }
                for (auto &road : toRoads) {
                    if (road->r_class_ <= roadClass) {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1) {
                    continue;
                }

                auto error = DCRoadCheckError::createByKXS_04_013(stol(iter.first), roadClass);
                error_output()->saveError(error);
            }
            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void RoadCheck::CheckProhibition(){
            int total=0;
            ///简单路口
            for(auto conn:map_prohibition_conn){
                long id = conn.first;
                auto froad_v = map_conn_troad_froad.find(conn.first);
                if(froad_v!=map_conn_troad_froad.end()){
                    if(conn.second.size()>=froad_v->second.size()){
                        auto error = DCRoadCheckError::createByKXS_04_014(1, conn.first);
                        error_output()->saveError(error);
                    }
                }
            }

            ///复杂路口
            for(auto conn:map_prohibition_cconn){
                long id = conn.first;
                auto froad_v = map_cconn_troad_froad.find(conn.first);
                if(froad_v!=map_cconn_troad_froad.end()){
                    if(conn.second.size()>=froad_v->second.size()){
                        auto error = DCRoadCheckError::createByKXS_04_014(2, conn.first);
                        error_output()->saveError(error);
                    }
                }
            }

            error_output()->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_014,map_prohibition_conn.size()+map_prohibition_cconn.size());

        }

        void RoadCheck::CheckRoadNode(){
            auto road = data_manager()->roads_;
            error_output()->addCheckItemInfo(CHECK_ITEM_KXS_ROAD_016,road.size());
            for(auto it:road){
                if(it.second->fow_ != 2){
                    continue;
                }
                if(map_node_cnode.find(stol(it.second->f_node_id)) == map_node_cnode.end() || map_node_cnode.find(stol(it.second->t_node_id)) == map_node_cnode.end()){
                    auto error = DCRoadCheckError::createByKXS_04_016(1,stol(it.first), it.second->fNode_->coord_);
                    error_output()->saveError(error);
                }else if(map_node_cnode.find(stol(it.second->f_node_id))->second != map_node_cnode.find(stol(it.second->t_node_id))->second){
                    auto error = DCRoadCheckError::createByKXS_04_016(2,stol(it.first), it.second->fNode_->coord_,map_node_cnode.find(stol(it.second->f_node_id))->second,map_node_cnode.find(stol(it.second->t_node_id))->second);
                    error_output()->saveError(error);
                }else{
                    int min_index;
                    CoordinateSequence *sq = nullptr;
                    sq = it.second->line_->getCoordinates();

                    shared_ptr<DCCoord> cnode_coord = map_cnodes_.find(map_node_cnode.find(stol(it.second->t_node_id))->second)->second->coord_;
                    shared_ptr<geos::geom::Point> point = GeosObjUtil::CreatePoint(cnode_coord);
                    double dis_len = GeosObjRelationUtil::pt2LineDist(sq, point->getCoordinate(), min_index);
                    if(dis_len>dis_cnode_2_road){
                        auto error = DCRoadCheckError::createByKXS_04_016(3,stol(it.first), cnode_coord,map_node_cnode.find(stol(it.second->f_node_id))->second);
                        error_output()->saveError(error);
                    }
                }
            }
        }

    }
}