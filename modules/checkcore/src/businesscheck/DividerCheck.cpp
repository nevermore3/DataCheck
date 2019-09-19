//
// Created by ubuntu on 2019/9/10.
//

#include "businesscheck/DividerCheck.h"
#include <util/FileUtil.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"
#include <util/distance.h>
#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

using namespace kd::api;

namespace kd {
    namespace dc {

        DividerCheck::DividerCheck() {
            base_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
        }

        DividerCheck::~DividerCheck() {
            map_divider_sch_.clear();
        }

        string DividerCheck::getId() {
            return id_;
        }

        /*
         * 1、每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 2、DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 3、相邻HD_DIVIDER_SCH点之间距离不超过1.3m
         */
        bool DividerCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                   shared_ptr<CheckErrorOutput> errorOutput) {
            if (!LoadDividerSCH()) {
                LOG(ERROR) << "Open HD_DIVIDER_SCH fail ";
                return false;
            }
            //每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
            DividerRelevantDividerSCH(errorOutput);
            //相邻HD_DIVIDER_SCH点之间距离不超过1.3m
            AdjacentDividerSCHNodeDistance(errorOutput);
            //HD_DIVIDER_SCH点离关联的DIVIDER的垂直距离不超过10cm
            DividerSCHVerticalDistance(errorOutput);

            return true;
        }

        bool DividerCheck::LoadDividerSCH() {
            string dividerFile = base_path_ + "/HD_DIVIDER_SCH";
            ShpData shpFile(dividerFile);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << dividerFile << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCDivideSCH> dividerSCH = make_shared<DCDivideSCH>();
                //读取属性信息
                dividerSCH->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                dividerSCH->dividier_id_ = shpFile.readIntField(i, "ROAD_ID");
                dividerSCH->dividier_node_inidex_ = shpFile.readIntField(i, "D_NodeIdx");
                dividerSCH->att_node_id_ = shpFile.readIntField(i, "A_NodeID");
                dividerSCH->curvature_ = shpFile.readDoubleField(i, "Curvature");
                dividerSCH->slope_ = shpFile.readDoubleField(i, "Slope");
                dividerSCH->heading_ = shpFile.readDoubleField(i, "Heading");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    dividerSCH->coord_ = coord;
                }

                if (map_divider_sch_.find(dividerSCH->dividier_id_) == map_divider_sch_.end()) {
                    map<long, shared_ptr<DCDivideSCH>> mapDividerSCH;
                    mapDividerSCH.insert(make_pair(dividerSCH->att_node_id_, dividerSCH));
                    map_divider_sch_.insert(make_pair(dividerSCH->dividier_id_, mapDividerSCH));
                } else {
                    map_divider_sch_[dividerSCH->dividier_id_].insert(make_pair(dividerSCH->att_node_id_, dividerSCH));
                }
            }
            return true;
        }


        /*
         * 1、每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 2、DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
         */
        void DividerCheck::DividerRelevantDividerSCH(shared_ptr<CheckErrorOutput> errorOutput) {

            // 保存divider形点 和 HD_DIVIDER_SCH点之间的关系
            // key : dividerID ,value: {key: dividerNodeindex , value: {divider_sch}}
            unordered_map<long, map<long, vector<shared_ptr<DCDivideSCH>>>>mapDividerNodeSCH;
            map<string, shared_ptr<DCDivider>>dividers = map_data_manager_->dividers_;
            for (const auto &dividerSCH : map_divider_sch_) {
                long dividerID = dividerSCH.first;

                map<long, vector<shared_ptr<DCDivideSCH>>>mapNodeIndex2SCH;
                for (const auto &node : dividerSCH.second) {
                    long dividerNodeIndex = node.second->dividier_node_inidex_;
                    if (mapNodeIndex2SCH.find(dividerNodeIndex) == mapNodeIndex2SCH.end()) {
                        vector<shared_ptr<DCDivideSCH>>dividerSCHArray;
                        dividerSCHArray.emplace_back(node.second);
                        mapNodeIndex2SCH.insert(make_pair(dividerNodeIndex, dividerSCHArray));
                    } else {
                        mapNodeIndex2SCH[dividerNodeIndex].emplace_back(node.second);
                    }
                }
                mapDividerNodeSCH.insert(make_pair(dividerID, mapNodeIndex2SCH));
            }

            double distanceThreshold = 1.5;
            for (const auto &divider : dividers) {
                long dividerID = stol(divider.first);
                if (mapDividerNodeSCH.find(dividerID) == mapDividerNodeSCH.end()) {
                    continue;
                }

                for (size_t i = 0; i < divider.second->nodes_.size(); i++) {
                    if (mapDividerNodeSCH[dividerID].find(i) == mapDividerNodeSCH[dividerID].end()) {
                        auto error = DCDividerCheckError::createByKXS_01_030(dividerID, i,
                                                                             divider.second->nodes_[i]->coord_, 1);
                        errorOutput->saveError(error);
                        continue;
                    }
                    vector<shared_ptr<DCDivideSCH>>dividerSCHArray = mapDividerNodeSCH[dividerID][i];
                    //求divider形点到 DIVIDER_SCH集合点中最近的一个点
                    double minDistance = DBL_MAX;
                    for (const auto &schNode : dividerSCHArray) {
                        double distance = GeosObjUtil::get_length_of_node(divider.second->nodes_[i]->coord_, schNode->coord_);
                        if (distance < minDistance) {
                            minDistance = distance;
                        }
                    }

                    if (minDistance > distanceThreshold) {
                        auto error = DCDividerCheckError::createByKXS_01_030(dividerID, i,
                                                                             divider.second->nodes_[i]->coord_, 1);
                        errorOutput->saveError(error);
                    }

                    //DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
                    if (i == 0 || i == divider.second->nodes_.size() - 1) {
                        if (minDistance > 0.2) {
                            auto error = DCDividerCheckError::createByKXS_01_030(dividerID, i,
                                                                                 divider.second->nodes_[i]->coord_, 2);
                            errorOutput->saveError(error);
                        }
                    }
                }

            }
        }

        /*
         * 相邻HD_DIVIDER_SCH点之间距离不能超过1.3m
         */
        void DividerCheck::AdjacentDividerSCHNodeDistance(shared_ptr<CheckErrorOutput> errorOutput) {

            double distanceThreshold = 1.3;
            double distance = 0;
            for (const auto &dividerSCH : map_divider_sch_) {
                long dividerID = dividerSCH.first;
                map<long, shared_ptr<DCDivideSCH>> dividerNodes = dividerSCH.second;
                auto currIter = dividerNodes.begin();
                auto preIter = currIter;
                currIter++;
                while (currIter != dividerNodes.end()) {

                    distance = GeosObjUtil::get_length_of_node(preIter->second->coord_, currIter->second->coord_);
                    if (distance > distanceThreshold) {
                        auto error = DCDividerCheckError::createByKXS_01_028(dividerID, preIter->first, currIter->first,
                                                                             distance, distanceThreshold);
                        errorOutput->saveError(error);
                    }
                    preIter = currIter;
                    currIter++;
                }
            }
        }

        void DividerCheck::DividerSCHVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput) {

            for (const auto &dividerSCH : map_divider_sch_) {
                long dividerID = dividerSCH.first;
                string strDividerID = to_string(dividerID);
                if (map_data_manager_->dividers_.find(strDividerID) == map_data_manager_->dividers_.end()) {
                    continue;
                }
                auto divider = map_data_manager_->dividers_[strDividerID];
                for (const auto &node : dividerSCH.second) {
                    shared_ptr<geos::geom::Point> point = GeosObjUtil::CreatePoint(node.second->coord_);
                    double distance = GeosObjUtil::GetVerticleDistance(divider->line_, point);
                    if (distance > 0.1) {
                        auto error = DCDividerCheckError::createByKXS_01_029(node.first, node.second->coord_);
                        errorOutput->saveError(error);
                    }
                }
            }
        }

    }
}
