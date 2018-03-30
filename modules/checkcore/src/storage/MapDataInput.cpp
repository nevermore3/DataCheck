//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/MapDataInput.h"

//thirdparty
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

namespace kd {
    namespace dc {


        bool MapDataInput::loadDivider(string basePath, map<string, shared_ptr<DCDivider>> &dividers) {

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
            }else{
                cout << "[Error] open divider node file error. fileName " << dividerNodeFile << endl;
                return false;
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

    }
}