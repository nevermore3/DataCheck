//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/MapDataInput.h"

//thirdparty
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include <storage/MapDataInput.h>


namespace kd {
    namespace dc {


        bool MapDataInput::loadDivider(string basePath, shared_ptr<MapDataManager> mapDataManager,
                                       shared_ptr<CheckErrorOutput> errorOutput) {
            auto &dividers = mapDataManager->dividers_;

            //由于divider引用dividernode,因此先读取dividernode

            //读取节点信息
            map<DCCoord, string> divNodeCoords;
            map<string, shared_ptr<DCDividerNode>> commonNodeInfos; //所有共用的节点

            string dividerNodeFile = basePath + "/HD_DIVIDER_NODE";
            ShpData shpNodeData(dividerNodeFile);
            if (shpNodeData.isInit()) {
                int record_nums = shpNodeData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpNodeData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                        continue;

                    //读取属性信息
                    long id = shpNodeData.readIntField(i, "ID");
                    long dividerId = shpNodeData.readIntField(i, "DIVIDER_ID");
                    long spIdx = shpNodeData.readIntField(i, "SPIDX");
                    long dashType = shpNodeData.readIntField(i, "DASHTYPE");

                    if(dividerId == -1 && spIdx == -1){
                        shared_ptr<DCDividerNode> divNode = make_shared<DCDividerNode>();
                        //确定属性信息
                        divNode->id_ = to_string(id);
                        divNode->dashType_ = dashType;

                        //读取坐标信息
                        int nVertices = shpObject->nVertices;
                        if(nVertices == 1){
                            divNode->coord_.lng_ = shpObject->padfX[0];
                            divNode->coord_.lat_ = shpObject->padfY[0];
                            divNode->coord_.z_   = shpObject->padfZ[0];
                            commonNodeInfos.insert(make_pair(divNode->id_, divNode));
                            divNodeCoords.insert(make_pair(divNode->coord_, divNode->id_));
                        }
                    }
                    else{
                        auto divit = dividers.find(to_string(dividerId));
                        if(divit == dividers.end()){
                            stringstream ss;
                            ss << "[Error] divider node not find ref divider.[dividerId:"
                               << dividerId << "][nodeIdx:" << spIdx << "]";
                            errorOutput->writeInfo(ss.str());
                            continue;
                        }

                        shared_ptr<DCDivider> div = divit->second;
                        if(spIdx < 0 || spIdx >= div->nodes_.size()){
                            stringstream ss;
                            ss << "[Error] divider node idx invalid. [dividerId:"
                               << dividerId << "][nodeIdx:" << spIdx << "]";
                            errorOutput->writeInfo(ss.str());
                            div->valid_ = false;
                            continue;
                        }

                        //关联属性
                        div->nodes_[spIdx]->id_ = to_string(id);
                        div->nodes_[spIdx]->dashType_ = dashType;
                    }
                }
            }else{
                stringstream ss;
                ss << "[Error] open divider node file error. fileName " << dividerNodeFile;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            //读取车道线基本信息
            string dividerFile = basePath + "/HD_DIVIDER";
            ShpData shpData(dividerFile);
            if (shpData.isInit()) {
                int record_nums = shpData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;

                    //读取基本属性
                    shared_ptr<DCDivider> divider = make_shared<DCDivider>();
                    divider->id_ = to_string(shpData.readIntField(i, "ID"));
                    divider->dividerNo_ = shpData.readIntField(i, "DIVIDER_NO");
                    divider->direction_ = shpData.readIntField(i, "DIRECTION");
                    divider->rLine_ = shpData.readIntField(i, "R_LINE");
                    divider->tollFlag_ = shpData.readIntField(i, "TOLLFLAG");
                    divider->fromNodeId_ = shpData.readStringField(i, "FDNODE");
                    divider->toNodeId_ = shpData.readStringField(i, "TDNODE");

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for( int i = 0 ; i < nVertices ; i ++ ){
                        DCCoord coord;
                        coord.lng_ = shpObject->padfX[i];
                        coord.lat_ = shpObject->padfY[i];
                        coord.z_   = shpObject->padfZ[i];
                        if (divNodeCoords.find(coord) != divNodeCoords.end()){
                            divider->nodes_.emplace_back(commonNodeInfos[divNodeCoords[coord]]);
                        } else {
                            shared_ptr<DCDividerNode> divNode = make_shared<DCDividerNode>();
                            divNode->coord_ = coord;
                            divider->nodes_.emplace_back(divNode);
                        }
                    }
                    if (divider->fromNodeId_ != divider->nodes_.front()->id_ &&
                        divider->fromNodeId_ != divider->nodes_.back()->id_) {
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByNode("KXS-01-018", divider, nullptr);
                        error->checkDesc_ = "DIVIDER的FDNODE与TDNODE应该是实际的首尾点";
                        stringstream ss;
                        ss << "divider:" << divider->id_ << ",from node_id:" << divider->fromNodeId_ << "标记错误";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }

                    if (divider->toNodeId_ != divider->nodes_.front()->id_ &&
                        divider->toNodeId_ != divider->nodes_.back()->id_) {
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByNode("KXS-01-018", divider, nullptr);
                        error->checkDesc_ = "DIVIDER的FDNODE与TDNODE应该是实际的首尾点";
                        stringstream ss;
                        ss << "divider:" << divider->id_ << ",to node_id:" << divider->fromNodeId_ << "标记错误";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }
                    dividers.insert(make_pair(divider->id_, divider));
                }
            }else{
                stringstream ss;
                ss << "[Error] open divider file error. fileName " << dividerFile;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            //补充divider首末点的node信息
            for( auto divit : dividers ){
                shared_ptr<DCDivider> div = divit.second;
                if(!div->valid_)
                    continue;

                if(!setDividerNode(div, div->fromNodeId_, commonNodeInfos)){
                    stringstream ss;
                    ss << "[Error] div" << div->id_ << " not find fromNode " << div->fromNodeId_ << " info.";
                    errorOutput->writeInfo(ss.str());
                    div->valid_ = false;
                    continue;
                }

                // 构建fnode与divider关系
                mapDataManager->insert_fnode_id2_dividers(div->fromNodeId_, div);
                mapDataManager->insert_node_id2_dividers(div->fromNodeId_, div);

                if(!setDividerNode(div, div->toNodeId_, commonNodeInfos)){
                    stringstream ss;
                    ss << "[Error] div" << div->id_ << " not find toNode " << div->toNodeId_ << " info.";
                    errorOutput->writeInfo(ss.str());
                    div->valid_ = false;
                    continue;
                }

                // 构建tnode与divider关系
                mapDataManager->insert_tnode_id2_dividers(div->toNodeId_, div);
                mapDataManager->insert_node_id2_dividers(div->toNodeId_, div);
            }

            //读取车道线属性信息
            string dividerAttFile = basePath + "/HD_DIVIDER_ATTRIBUTE";
            DbfData attDbfData(dividerAttFile);
            if (attDbfData.isInit()) {
                int record_nums = attDbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    shared_ptr<DCDividerAttribute> divAtt = make_shared<DCDividerAttribute>();
                    divAtt->id_ = to_string(attDbfData.readIntField(i, "ID"));
                    divAtt->virtual_ = attDbfData.readIntField(i, "VIRTUAL");
                    divAtt->color_ = attDbfData.readIntField(i, "COLOR");
                    divAtt->type_ = attDbfData.readIntField(i, "TYPE");
                    divAtt->driveRule_ = attDbfData.readIntField(i, "DRIVE_RULE");
                    divAtt->material_ = attDbfData.readIntField(i, "MATERIAL");
                    divAtt->width_ = attDbfData.readDoubleField(i, "WIDTH");

                    long dividerId = attDbfData.readIntField(i, "DIVIDER_ID");
                    long spIdx = attDbfData.readIntField(i, "SPIDX");

                    auto divit = dividers.find(to_string(dividerId));
                    if(divit == dividers.end()){
                        stringstream ss;
                        ss << "[Error] divider att not find ref divider.[dividerId:"
                              << dividerId << "][nodeIdx:" << spIdx << "]";
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }

                    shared_ptr<DCDivider> div = divit->second;
                    if(spIdx < 0 || spIdx >= div->nodes_.size()){
                        stringstream ss;
                        ss << "[Error] divider att ref node idx invalid. [dividerId:"
                              << dividerId << "][nodeIdx:" << spIdx << "]";
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }

                    //赋予节点关联: spidx是相对于FNode->TNode而言,而非矢量化
                    if (div->nodes_[0]->id_ == div->fromNodeId_){
                        divAtt->dividerNode_ = div->nodes_[spIdx];
                    } else {
                        divAtt->dividerNode_ = div->nodes_[spIdx];
                        stringstream ss;
                        ss << "[Error] divider from node is not the first node. [dividerId:"
                           << dividerId << "]";
                        errorOutput->writeInfo(ss.str());
                    }

                    div->atts_.emplace_back(divAtt);
                }
            }else{
                stringstream ss;
                ss << "[Error] open divider att file error. fileName " << dividerAttFile;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            return true;
        }

        bool MapDataInput::setDividerNode(shared_ptr<DCDivider> div, string nodeId,
                                          map<string, shared_ptr<DCDividerNode>> & commonNodeInfos){

            auto nodeit = commonNodeInfos.find(nodeId);
            if(nodeit != commonNodeInfos.end()){
                auto nodeObj = nodeit->second;

                //判断是否与第一个节点坐标相同
                shared_ptr<DCDividerNode> firstNode = div->nodes_[0];
                if(firstNode->coord_.lng_ == nodeObj->coord_.lng_ &&
                        firstNode->coord_.lat_ == nodeObj->coord_.lat_ &&
                        firstNode->coord_.z_ == nodeObj->coord_.z_){
                    firstNode->id_ = nodeObj->id_;
                    firstNode->dashType_ = nodeObj->dashType_;
                    return true;
                }

                //判断是否与最后一个节点坐标相同
                shared_ptr<DCDividerNode> lastNode = div->nodes_[div->nodes_.size()-1];
                if(lastNode->coord_.lng_ == nodeObj->coord_.lng_ &&
                        lastNode->coord_.lat_ == nodeObj->coord_.lat_ &&
                        lastNode->coord_.z_ == nodeObj->coord_.z_){
                    lastNode->id_ = nodeObj->id_;
                    lastNode->dashType_ = nodeObj->dashType_;
                    return true;
                }
            }

            return false;
        }

        bool MapDataInput::loadLane(string basePath, const map<string, shared_ptr<DCDivider>> &dividers,
                                    map<string, shared_ptr<DCLane>> & lanes, shared_ptr<CheckErrorOutput> errorOutput){
            //读取车道数据
            string laneFile = basePath + "/HD_LANE";
            ShpData shpData(laneFile);
            if (shpData.isInit()) {
                int record_nums = shpData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;

                    //读取基本属性
                    shared_ptr<DCLane> dcLane = make_shared<DCLane>();
                    dcLane->id_ = to_string(shpData.readIntField(i, "ID"));
                    //int iRoadId = shpData.readIntField(i, "ROAD_ID");
                    int iLeftDivider = shpData.readIntField(i, "DIVIDER_L");
                    int iRightDivider = shpData.readIntField(i, "DIVIDER_R");
                    dcLane->laneNo_ = shpData.readIntField(i, "LANE_NO");

                    //获取道路对象
                    //dcLane->road_ = nullptr;//iRoadId

                    //获取车道左分割线
                    auto itLeftDivider = dividers.find(to_string(iLeftDivider));
                    if (itLeftDivider != dividers.end()){
                        shared_ptr<DCDivider> leftDiv =itLeftDivider->second;
                        if (nullptr != leftDiv) {
                            dcLane->leftDivider_ = leftDiv;
                        }else{
                            stringstream ss;
                            ss << "[Error] lane's left divider is null.[laneId:" << dcLane->id_
                               << "][dividerId:" << iLeftDivider << "]";
                            errorOutput->writeInfo(ss.str());
                        }
                    }else{
                        stringstream ss;
                        ss << "[Error] lane not find ref left divider.[laneId:" << dcLane->id_
                           << "][dividerId:" << iLeftDivider << "]";
                        errorOutput->writeInfo(ss.str());
                    }

                    //获取车道右分割线
                    auto itRightDivider = dividers.find(to_string(iRightDivider));
                    if (itRightDivider != dividers.end()){
                        shared_ptr<DCDivider> rightDiv =itRightDivider->second;
                        if (nullptr != rightDiv) {
                            dcLane->rightDivider_ = rightDiv;
                        }else{
                            stringstream ss;
                            ss << "[Error] lane's right divider is null.[laneId:" << dcLane->id_
                               << "][dividerId:" << iRightDivider << "]";
                            errorOutput->writeInfo(ss.str());
                        }
                    }else{
                        stringstream ss;
                        ss << "[Error] lane not find ref right divider.[laneId:" << dcLane->id_
                           << "][dividerId:" << iRightDivider << "]";
                        errorOutput->writeInfo(ss.str());
                    }

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for( int i = 0 ; i < nVertices ; i ++ ){
                        shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                        coord->lng_ = shpObject->padfX[i];
                        coord->lat_ = shpObject->padfY[i];
                        coord->z_ = shpObject->padfZ[i];
                        dcLane->coords_.emplace_back(coord);
                    }
                    lanes.insert(make_pair(dcLane->id_, dcLane));
                }
            }else{
                stringstream ss;
                ss << "[Error] open lane file error. fileName " << laneFile;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            //读取车道属性信息
            string laneAttFile = basePath + "/HD_LANE_ATTRIBUTE";
            DbfData attDbfData(laneAttFile);
            if (attDbfData.isInit()) {
                int record_nums = attDbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    shared_ptr<DCLaneAttribute> laneAtt = make_shared<DCLaneAttribute>();
                    laneAtt->id_ = to_string(attDbfData.readIntField(i, "ID"));
                    int laneId = attDbfData.readIntField(i, "LANE_ID");
                    int spIdx = attDbfData.readIntField(i, "SPIDX");
                    laneAtt->laneType_ = attDbfData.readIntField(i, "LANETYPE");
                    laneAtt->subType_ = attDbfData.readIntField(i, "SUBTYPE");
                    laneAtt->direction_ = attDbfData.readIntField(i, "DIRECTION");
                    laneAtt->width_ = attDbfData.readDoubleField(i, "WIDTH");
                    laneAtt->maxSpeed_ = attDbfData.readIntField(i, "MAX_SPEED");
                    laneAtt->minSpeed_ = attDbfData.readIntField(i, "MIN_SPEED");
                    laneAtt->smType_ = attDbfData.readIntField(i, "SMTYPE");
                    laneAtt->status_ = attDbfData.readIntField(i, "STATUS");

                    auto itLane = lanes.find(to_string(laneId));
                    if(itLane == lanes.end()){
                        stringstream ss;
                        ss << "[Error] lane att not find lane.[lane attrId:"
                              << laneAtt->id_ << "],[laneId:" << laneId << "]";
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }

                    shared_ptr<DCLane> lane = itLane->second;
                    if (lane == nullptr){
                        stringstream ss;
                        ss << "[Error] lane is null when get lane attr.[lane attrId:"
                              << laneAtt->id_ << "],[laneId:" << laneId << "]";
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }

                    //赋予Divider节点对象
                    shared_ptr<DCDivider> rDivider = lane->rightDivider_;
                    if (spIdx < 0 || spIdx >= rDivider->nodes_.size()){
                        stringstream ss;
                        ss << "[Error] divider att ref node idx invalid. [dividerId:"
                              << rDivider->id_ << "][nodeIdx:" << spIdx << "]";
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }else{
                        laneAtt->dividerNode_ = rDivider->nodes_[spIdx];
                    }

                    lane->atts_.emplace_back(laneAtt);
                }
            }else{
                stringstream ss;
                ss << "[Error] open lane att file error. fileName " << laneAttFile;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            return true;
        }


        bool MapDataInput::loadLaneGroup(string basePath, shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){
            const auto &lanes = mapDataManager->lanes_;
            const auto &roads = mapDataManager->roads_;
            auto &laneGroups = mapDataManager->laneGroups_;
            //读取车道线属性信息
            string lgFileName = basePath + "/HD_LANE_GROUP";
            DbfData lgDbfData(lgFileName);
            if (lgDbfData.isInit()) {
                int record_nums = lgDbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    shared_ptr<DCLaneGroup> laneGroup = make_shared<DCLaneGroup>();
                    laneGroup->id_ = to_string(lgDbfData.readIntField(i, "ID"));
                    string road_id = lgDbfData.readStringField(i, "ROAD_ID");
                    laneGroup->direction_ = lgDbfData.readIntField(i, "DIRECTION");
                    laneGroup->is_virtual_ = lgDbfData.readIntField(i, "IS_VIR");

                    auto road_iter = roads.find(road_id);
                    if (road_iter != roads.end()) {
                        laneGroup->road_ = road_iter->second;
                    } else {
                        LOG(ERROR) << "find road failed! road : " << road_id << " lane group : " << laneGroup->id_;
                    }

                    laneGroups.insert(make_pair(laneGroup->id_, laneGroup));
                }
            } else {
                stringstream ss;
                ss << "[Error] open lane group file error. fileName " << lgFileName;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            //建立车道分组和车道之间的关系
            string rlgFileName = basePath + "/HD_R_LANE_GROUP";
            DbfData rlgDbfData(rlgFileName);
            if (rlgDbfData.isInit()) {
                int record_nums = rlgDbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    shared_ptr<DCLaneGroup> divAtt = make_shared<DCLaneGroup>();
                    divAtt->id_ = to_string(rlgDbfData.readIntField(i, "ID"));
                    string lgId = to_string(rlgDbfData.readIntField(i, "LG_ID"));
                    string laneId = to_string(rlgDbfData.readIntField(i, "LANE_ID"));

                    auto lgit = laneGroups.find(lgId);
                    if(lgit == laneGroups.end()){
                        LOG(ERROR) << "HD_R_LANE_GROUP has not exist lanegroup " << lgId;
                        continue;
                    }

                    auto laneit = lanes.find(laneId);
                    if(laneit == lanes.end()){
                        LOG(ERROR) << "HD_R_LANE_GROUP not find lane " << laneId;
                        continue;
                    }

                    auto ptr_lane = laneit->second;
                    // 最内侧lane
                    if (ptr_lane->laneNo_ == 1) {
                        mapDataManager->insert_divider2_lane_groups(ptr_lane->leftDivider_->id_, lgId);
                        mapDataManager->insert_divider2_lane_groups(ptr_lane->rightDivider_->id_, lgId);
                    } else {
                        mapDataManager->insert_divider2_lane_groups(ptr_lane->rightDivider_->id_, lgId);
                    }

                    lgit->second->lanes_.emplace_back(laneit->second);
                }
            }else{
                stringstream ss;
                ss << "[Error] open lane group ralation file error. fileName " << lgFileName;
                errorOutput->writeInfo(ss.str());
                return false;
            }

            return true;
        }

        bool MapDataInput::loadLaneConnectivity(string basePath,
                                                map<string, shared_ptr<DCLaneConnectivity>> & laneConnectivitys,
                                                shared_ptr<CheckErrorOutput> errorOutput){
            //读取拓扑信息
            string dbfFileName = basePath + "/HD_LANE_CONNECTIVITY";
            DbfData dbfData(dbfFileName);
            if (dbfData.isInit()) {
                int record_nums = dbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    //读取基本属性
                    shared_ptr<DCLaneConnectivity> laneConn = make_shared<DCLaneConnectivity>();
                    laneConn->id_ = to_string(dbfData.readIntField(i, "ID"));
                    laneConn->nodeType_ = dbfData.readIntField(i, "NODE_TYPE");
                    laneConn->nodeId_ = dbfData.readIntField(i, "NODE_ID");
                    laneConn->fLaneId_ = dbfData.readIntField(i, "FLANE_ID");
                    laneConn->tLaneId_ = dbfData.readIntField(i, "TLANE_ID");

                    laneConnectivitys.insert(make_pair(laneConn->id_, laneConn));
                }
            }else{
                stringstream ss;
                ss << "[Error] open lane connectivity file error. fileName " << dbfFileName;
                errorOutput->writeInfo(ss.str());
                return false;
            }
            return true;
        }


        bool MapDataInput::loadObjectLine(string basePath, map<string, shared_ptr<DCObjectPL>> & objectPLs,
                                          shared_ptr<CheckErrorOutput> errorOutput){
            //读取线对象信息
            string objLineFileName = basePath + "/HD_OBJECT_PL";
            ShpData objLineData(objLineFileName);
            if (objLineData.isInit()) {
                int record_nums = objLineData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = objLineData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;

                    //读取基本属性
                    shared_ptr<DCObjectPL> objPL = make_shared<DCObjectPL>();
                    objPL->id_ = to_string(objLineData.readIntField(i, "ID"));
                    objPL->type_ = objLineData.readIntField(i, "DIVIDER_NO");
                    objPL->subType_ = objLineData.readIntField(i, "DIRECTION");
                    objPL->material_ = objLineData.readIntField(i, "R_LINE");
                    objPL->color_ = objLineData.readIntField(i, "TOLLFLAG");

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for( int idx = 0 ; idx < nVertices ; idx ++ ){
                        shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                        coord->lng_ = shpObject->padfX[idx];
                        coord->lat_ = shpObject->padfY[idx];
                        coord->z_   = shpObject->padfZ[idx];
                        objPL->coords_.emplace_back(coord);
                    }
                    objectPLs.insert(make_pair(objPL->id_, objPL));
                }
            }else{
                stringstream ss;
                ss << "[Error] open object line file error. fileName " << objLineFileName;
                errorOutput->writeInfo(ss.str());
                return false;
            }
            return true;
        }

        bool MapDataInput::loadLaneGroupLogicInfo(string basePath, shared_ptr<MapDataManager> mapDataManager) {
            bool bRet = true;
            // 读取车道组与道路的关联索引点
            string lg_road_index_file = basePath + "/LG_ROADNODE_INDEX";
            DbfData lg_road_index_data(lg_road_index_file);
            if (lg_road_index_data.isInit()) {
                mapDataManager->is_auto_road = true;
                auto& road2LaneGroup2NodeIdxs = mapDataManager->road2LaneGroup2NodeIdxs_;

                int record_nums = lg_road_index_data.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    string lane_group_id = lg_road_index_data.readStringField(i, "LG_ID");
                    string road_id = lg_road_index_data.readStringField(i, "ROADID");
                    int f_index = lg_road_index_data.readIntField(i, "F_INDEX");
                    int t_index = lg_road_index_data.readIntField(i, "T_INDEX");

                    auto iter = road2LaneGroup2NodeIdxs.find(road_id);
                    if (iter != road2LaneGroup2NodeIdxs.end()) {
                        iter->second.insert(make_pair(lane_group_id, make_pair(f_index, t_index)));
                    } else {
                        unordered_map<string, std::pair<long,long>> lanegroup2_node_index_map;
                        lanegroup2_node_index_map[lane_group_id] = std::make_pair(f_index, t_index);
                        road2LaneGroup2NodeIdxs.insert(make_pair(road_id, lanegroup2_node_index_map));
                    }

                    // 插入lane group 与roads关联
                    mapDataManager->insert_lane_group2_roads(lane_group_id, road_id);
                }
            } else {
                LOG(ERROR) << "open LG_ROADNODE_INDEX failed! ";
                bRet = false;
            }
            return bRet;
        }

        bool MapDataInput::loadRoad(string basePath, shared_ptr<MapDataManager> mapDataManager) {
            bool bRet = true;
            // 读取车道组与道路的关联索引点
            string lg_road__file = basePath + "/ROAD";
            ShpData lg_road_data(lg_road__file);
            if (lg_road_data.isInit()) {
                auto& roads = mapDataManager->roads_;

                int record_nums = lg_road_data.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shp_object = lg_road_data.readShpObject(i);

                    shared_ptr<DCRoad> ptr_road = make_shared<DCRoad>();
                    ptr_road->id_ = lg_road_data.readStringField(i, "ID");
                    ptr_road->direction_ = lg_road_data.readIntField(i, "DIRECTION");
                    ptr_road->sLanes_ = lg_road_data.readIntField(i, "S_LANES");
                    ptr_road->f_node_id = to_string(lg_road_data.readIntField(i, "SNODE_ID"));
                    ptr_road->t_node_id = to_string(lg_road_data.readIntField(i, "ENODE_ID"));
                    ptr_road->fNode_ = nullptr;
                    ptr_road->tNode_ = nullptr;

                    //读取空间信息
                    int nVertices = shp_object->nVertices;
                    for (int idx = 0; idx < nVertices; idx++) {
                        shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                        coord->lng_ = shp_object->padfX[idx];
                        coord->lat_ = shp_object->padfY[idx];
                        coord->z_ = shp_object->padfZ[idx];
                        ptr_road->nodes_.emplace_back(coord);
                    }

                    roads.insert(make_pair(ptr_road->id_, ptr_road));
                    SHPDestroyObject(shp_object);
                }
            } else {
                LOG(ERROR) << "open LG_ROADNODE_INDEX failed! ";
                bRet = false;
            }
            return bRet;
        }

    }
}