//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/MapDataInput.h"

//thirdparty
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

namespace kd {
    namespace dc {


        bool MapDataInput::loadDivider(string basePath, map<string, shared_ptr<DCDivider>> &dividers, shared_ptr<CheckErrorOutput> errorOutput) {

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
                        shared_ptr<DCDividerNode> divNode = make_shared<DCDividerNode>();
                        divNode->coord_.lng_ = shpObject->padfX[i];
                        divNode->coord_.lat_ = shpObject->padfY[i];
                        divNode->coord_.z_   = shpObject->padfZ[i];
                        divider->nodes_.emplace_back(divNode);
                    }
                    dividers.insert(make_pair(divider->id_, divider));
                }
            }else{
                cout << "[Error] open divider file error. fileName " << dividerFile << endl;
                return false;
            }

            //读取节点信息
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
                        }
                    }
                    else{
                        auto divit = dividers.find(to_string(dividerId));
                        if(divit == dividers.end()){
                            cout << "[Error] divider node not find ref divider.[dividerId:" << dividerId << "][nodeIdx:" << spIdx << "]" << endl;
                            continue;
                        }

                        shared_ptr<DCDivider> div = divit->second;
                        if(spIdx < 0 || spIdx >= div->nodes_.size()){
                            cout << "[Error] divider node idx invalid. [dividerId:" << dividerId << "][nodeIdx:" << spIdx << "]" << endl;
                            div->valid_ = false;
                            continue;
                        }

                        //关联属性
                        div->nodes_[spIdx]->id_ = to_string(id);
                        div->nodes_[spIdx]->dashType_ = dashType;
                    }
                }
            }else{
                cout << "[Error] open divider node file error. fileName " << dividerNodeFile << endl;
                return false;
            }

            //补充divider首末点的node信息
            for( auto divit : dividers ){
                shared_ptr<DCDivider> div = divit.second;
                if(!div->valid_)
                    continue;

                if(!setDividerNode(div, div->fromNodeId_, commonNodeInfos)){
                    cout << "[Error] div" << div->id_ << " not find fromNode " << div->fromNodeId_ << " info." << endl;
                    div->valid_ = false;
                    continue;
                }

                if(!setDividerNode(div, div->toNodeId_, commonNodeInfos)){
                    cout << "[Error] div" << div->id_ << " not find toNode " << div->toNodeId_ << " info." << endl;
                    div->valid_ = false;
                    continue;
                }
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
                    divAtt->driveRule_ = attDbfData.readIntField(i, "DRIVERULE");
                    divAtt->material_ = attDbfData.readIntField(i, "MATERIAL");
                    divAtt->width_ = attDbfData.readDoubleField(i, "WIDTH");

                    long dividerId = attDbfData.readIntField(i, "DIVIDER_ID");
                    long spIdx = attDbfData.readIntField(i, "SPIDX");

                    auto divit = dividers.find(to_string(dividerId));
                    if(divit == dividers.end()){
                        cout << "[Error] divider att not find ref divider.[dividerId:" << dividerId << "][nodeIdx:" << spIdx << "]" << endl;
                        continue;
                    }

                    shared_ptr<DCDivider> div = divit->second;
                    if(spIdx < 0 || spIdx >= div->nodes_.size()){
                        cout << "[Error] divider att ref node idx invalid. [dividerId:" << dividerId << "][nodeIdx:" << spIdx << "]" << endl;
                        continue;
                    }

                    //赋予节点关联
                    divAtt->dividerNode_ = div->nodes_[spIdx];

                    div->atts_.emplace_back(divAtt);
                }
            }else{
                cout << "[Error] open divider att file error. fileName " << dividerAttFile << endl;
                return false;
            }

            return true;
        }

        bool MapDataInput::setDividerNode(shared_ptr<DCDivider> div, string nodeId, map<string, shared_ptr<DCDividerNode>> & commonNodeInfos){

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

        bool MapDataInput::loadLane(string basePath, const map<string, shared_ptr<DCDivider>> &dividers, map<string, shared_ptr<DCLane>> & lanes, shared_ptr<CheckErrorOutput> errorOutput){
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
                            cout << "[Error] lane's left divider is null.[laneId:" << dcLane->id_ << "][dividerId:" << iLeftDivider << "]" << endl;
                        }
                    }else{
                        cout << "[Error] lane not find ref left divider.[laneId:" << dcLane->id_ << "][dividerId:" << iLeftDivider << "]" << endl;
                    }

                    //获取车道右分割线
                    auto itRightDivider = dividers.find(to_string(iRightDivider));
                    if (itRightDivider != dividers.end()){
                        shared_ptr<DCDivider> rightDiv =itRightDivider->second;
                        if (nullptr != rightDiv) {
                            dcLane->rightDivider_ = rightDiv;
                        }else{
                            cout << "[Error] lane's right divider is null.[laneId:" << dcLane->id_ << "][dividerId:" << iRightDivider << "]" << endl;
                        }
                    }else{
                        cout << "[Error] lane not find ref right divider.[laneId:" << dcLane->id_ << "][dividerId:" << iRightDivider << "]" << endl;
                    }

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for( int i = 0 ; i < nVertices ; i ++ ){
                        shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                        coord->lat_ = shpObject->padfX[i];
                        coord->lat_ = shpObject->padfY[i];
                        coord->z_ = shpObject->padfZ[i];
                        dcLane->coords_.emplace_back(coord);
                    }
                    lanes.insert(make_pair(dcLane->id_, dcLane));
                }
            }else{
                cout << "[Error] open lane file error. fileName " << laneFile << endl;
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
                        cout << "[Error] lane att not find lane.[laneId:" << laneId << "]" << endl;
                        continue;
                    }

                    shared_ptr<DCLane> lane = itLane->second;
                    if (lane == nullptr){
                        cout << "[Error] lane is null when get lane attr.[laneId:" << laneId << "]" << endl;
                        continue;
                    }

                    //赋予Divider节点对象
                    shared_ptr<DCDivider> rDivider = lane->rightDivider_;
                    if (spIdx < 0 || spIdx >= rDivider->nodes_.size()){
                        cout << "[Error] divider att ref node idx invalid. [dividerId:" << 0 << "][nodeIdx:" << spIdx << "]" << endl;
                        continue;
                    }else{
                        laneAtt->dividerNode_ = rDivider->nodes_[spIdx];
                    }

                    lane->atts_.emplace_back(laneAtt);
                }
            }else{
                cout << "[Error] open lane att file error. fileName " << laneAttFile << endl;
                return false;
            }

            return true;
        }


        bool MapDataInput::loadLaneGroup(string basePath, const map<string, shared_ptr<DCLane>> & lanes, map<string,
                shared_ptr<DCLaneGroup>> & laneGroups, shared_ptr<CheckErrorOutput> errorOutput){

            //读取车道线属性信息
            string lgFileName = basePath + "/HD_LANE_GROUP";
            DbfData lgDbfData(lgFileName);
            if (lgDbfData.isInit()) {
                int record_nums = lgDbfData.getRecords();
                for (int i = 0; i < record_nums; i++) {

                    shared_ptr<DCLaneGroup> laneGroup = make_shared<DCLaneGroup>();
                    laneGroup->id_ = to_string(lgDbfData.readIntField(i, "ID"));
                    string roadId = lgDbfData.readStringField(i, "ROAD_ID");

                    if(roadId == "-1"){
                        cout << "[Errro] lane group " << laneGroup->id_ << " relate road id is -1 " << endl;
                        laneGroup->valid_ = false;
                    }

                    laneGroups.insert(make_pair(laneGroup->id_, laneGroup));
                }
            }else{
                cout << "[Error] open lane group file error. fileName " << lgFileName << endl;
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
                    string lgId = to_string(rlgDbfData.readIntField(i, "LANE_GROUP"));
                    string laneId = to_string(rlgDbfData.readIntField(i, "LANE_ID"));

                    auto lgit = laneGroups.find(lgId);
                    if(lgit == laneGroups.end()){
                        cout << "[Error] HD_R_LANE_GROUP has not exist lanegroup " << lgId << endl;
                        continue;
                    }

                    auto laneit = lanes.find(laneId);
                    if(laneit == lanes.end()){
                        cout << "[Error] HD_R_LANE_GROUP not find lane " << laneId << endl;
                        continue;
                    }

                    lgit->second->lanes_.emplace_back(laneit->second);
                }
            }else{
                cout << "[Error] open lane group ralation file error. fileName " << lgFileName << endl;
                return false;
            }


            return true;
        }

    }
}