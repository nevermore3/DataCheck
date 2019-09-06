//
// Created by ubuntu on 2019/9/5.
//
#include "datacheck/SlopeCheck.h"
#include <util/FileUtil.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"

#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

using namespace kd::api;


namespace kd {
    namespace dc {

        string SlopeCheck::getId() {
            return id_;
        }

        SlopeCheck::SlopeCheck() {
            slope_threshold_ = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::AVG_SLOPE_ERROR);
            base_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
        }

        SlopeCheck::~SlopeCheck() {
            map_road_adas_node_.clear();
            map_node_conn_.clear();
            map_lane_sch_.clear();
            map_divider_sch_.clear();
        }

        bool SlopeCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput) {
            data_manager_ = mapDataManager;

            // 检查每个ADAS_NODE 与其前后两个node(跨road) 坡度的平均值比较
            CheckAdasNode(mapDataManager, errorOutput);

            // 检查每个DIVIDER_SCH 与其前后两个node(跨divider) 坡度的平均值比较
            CheckDividerSCH(mapDataManager, errorOutput);

            // 检查每个LANE_SCH 与其前后两个node(跨lane) 坡度的平均值比较
            CheckLaneSCH(mapDataManager, errorOutput);

            return true;
        }


        bool SlopeCheck::LoadNodeConn() {
            string nodeConnFile = base_path_ + "/NODECONN";

            DbfData dbfFile(nodeConnFile);
            if (!dbfFile.isInit()) {
                LOG(ERROR) << "Open dbfFile :" << nodeConnFile << " Fail";
                return false;
            }

            size_t recordNums = dbfFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {

                shared_ptr<NodeConn> nodeConn = make_shared<NodeConn>();
                nodeConn->id_ = std::to_string(dbfFile.readIntField(i, "ID"));
                nodeConn->e_road_id_ = dbfFile.readLongField(i, "EROAD_ID");
                nodeConn->node_id_ = dbfFile.readLongField(i, "NODE_ID");
                nodeConn->q_road_id_ = dbfFile.readLongField(i, "QROAD_ID");
                nodeConn->flag_ = dbfFile.readLongField(i, "FLAG");

                map_node_conn_.insert(make_pair(nodeConn->id_, nodeConn));
            }
            return true;
        }

        bool SlopeCheck::LoadAdasNode() {
            string adasNodeFile = base_path_ + "/ADAS_NODE";
            ShpData shpFile(adasNodeFile);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << adasNodeFile << " Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<AdasNode> adasNode = make_shared<AdasNode>();
                //读取属性信息
                adasNode->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                adasNode->road_id_ = shpFile.readIntField(i, "ROAD_ID");
                adasNode->road_node_idx_ = shpFile.readIntField(i, "R_NodeIdx");
                adasNode->adas_node_id_ = shpFile.readIntField(i, "A_NodeID");
                adasNode->curvature_ = shpFile.readDoubleField(i, "Curvature");
                adasNode->slope_ = shpFile.readDoubleField(i, "Slope");
                adasNode->heading_ = shpFile.readDoubleField(i, "Heading");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                    ptr_coord->x_ = shpObject->padfX[0];
                    ptr_coord->y_ = shpObject->padfY[0];
                    ptr_coord->z_ = shpObject->padfZ[0];
                    adasNode->coord_ = ptr_coord;
                }

                if (map_road_adas_node_.find(adasNode->road_id_) == map_road_adas_node_.end()) {
                    map<long, shared_ptr<AdasNode>> mapAdasNode;
                    mapAdasNode.insert(make_pair(adasNode->adas_node_id_, adasNode));
                    map_road_adas_node_.insert(make_pair(adasNode->road_id_, mapAdasNode));
                } else {
                    map_road_adas_node_[adasNode->road_id_].insert(make_pair(adasNode->adas_node_id_, adasNode));
                }

            }
            return true;
        }

        bool SlopeCheck::LoadDividerSCH() {
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

        bool SlopeCheck::LoadLaneSCH() {
            string laneFile = base_path_ + "/HD_LANE_SCH";
            ShpData shpFile(laneFile);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << laneFile << "Fail";
                return false;
            }

            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                    continue;

                shared_ptr<DCLaneCurvature> laneSCH = make_shared<DCLaneCurvature>();
                //读取属性信息
                laneSCH->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                laneSCH->lane_id_ = shpFile.readIntField(i, "LANE_ID");
                laneSCH->lane_node_index_ = shpFile.readIntField(i, "L_NodeIdx");
                laneSCH->att_node_id_ = shpFile.readIntField(i, "A_NodeID");
                laneSCH->curvature_ = shpFile.readDoubleField(i, "CURVATURE");
                laneSCH->slope_ = shpFile.readDoubleField(i, "SLOPE");
                laneSCH->heading_ = shpFile.readDoubleField(i, "HEADING");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    laneSCH->coord_ = coord;
                }

                if (map_lane_sch_.find(laneSCH->lane_id_) == map_lane_sch_.end()) {
                    map<long, shared_ptr<DCLaneCurvature>> mapLaneSCH;
                    mapLaneSCH.insert(make_pair(laneSCH->att_node_id_, laneSCH));
                    map_lane_sch_.insert(make_pair(laneSCH->lane_id_, mapLaneSCH));
                } else {
                    map_lane_sch_[laneSCH->lane_id_].insert(make_pair(laneSCH->att_node_id_, laneSCH));
                }
            }
            return true;
        }

        shared_ptr<AdasNode> SlopeCheck::GetPreRoadAdasNode(long roadID) {
            // 如果获取不到前一条road 返回 nullptr
            // 获取到前一条road, 获取最后一个adasnode
            long preRoadID = INT_MAX;
            for (const auto &nodeConn : map_node_conn_) {
                if (nodeConn.second->q_road_id_ == roadID) {
                    preRoadID = nodeConn.second->e_road_id_;
                    break;
                }

            }
            if (map_road_adas_node_.find(preRoadID) == map_road_adas_node_.end()) {
                return nullptr;
            }
            map<long, shared_ptr<AdasNode>> mapAdasNode = map_road_adas_node_[preRoadID];
            auto iter = mapAdasNode.rbegin();
            iter++;
            return iter->second;
        }

        shared_ptr<AdasNode> SlopeCheck::GetNextRoadAdasNode(long roadID) {
            // 如果获取不到下一条road 返回 nullptr
            // 获取到下一条road, 获取第一个adasnode
            long nextRoadID = INT_MAX;
            for (const auto &nodeConn : map_node_conn_) {
                if (nodeConn.second->e_road_id_ == roadID) {
                    nextRoadID = nodeConn.second->q_road_id_;
                    break;
                }
            }
            if (map_road_adas_node_.find(nextRoadID) == map_road_adas_node_.end()) {
                return nullptr;
            }
            map<long, shared_ptr<AdasNode>> mapAdasNode = map_road_adas_node_[nextRoadID];

            auto iter = mapAdasNode.begin();
            iter++;
            return iter->second;
        }

        void SlopeCheck::CheckAdasNode(shared_ptr<MapDataManager> mapDataManager,
                                       shared_ptr<CheckErrorOutput> errorOutput) {
            if (!LoadAdasNode()) {
                return;
            }
            LoadNodeConn();

            for (const auto &roadAdasNode : map_road_adas_node_) {
                long roadID = roadAdasNode.first;
                map<long, shared_ptr<AdasNode>> mapAdasNode = roadAdasNode.second;

                auto curIter = mapAdasNode.begin();
                shared_ptr<AdasNode> pre = GetPreRoadAdasNode(roadID);
                if (pre == nullptr) {
                    pre = curIter->second;
                    curIter++;
                }
                while (curIter != mapAdasNode.end()) {
                    auto nextIter = curIter;
                    nextIter++;
                    shared_ptr <AdasNode> next = nullptr;
                    if (nextIter == mapAdasNode.end()) {
                        next = GetNextRoadAdasNode(roadID);
                    } else {
                        next = nextIter->second;
                    }

                    if (next == nullptr) {
                        break;
                    }

                    double avgSlope = (pre->slope_ + next->slope_) / 2;
                    double curSlope = curIter->second->slope_;
                    if (fabs(avgSlope - curSlope) > slope_threshold_) {
                        stringstream ss;
                        ss << "当前的 Road ID 是" << roadID << ", Index 是" << curIter->second->adas_node_id_;
                        ss << ", 当前的坡度是" << curSlope << "前后两个node 平均坡度是 " << avgSlope;
                        ss << ", 误差大于 " << slope_threshold_;
                        auto error = DCAttributeCheckError::createByKXS_10_002(ss.str());
                        errorOutput->saveError(error);
                    }

                    pre = curIter->second;
                    curIter++;
                }
            }
        }

        shared_ptr<DCLaneCurvature> SlopeCheck::GetPreLaneAdasNode(long laneID) {
            map<string, shared_ptr<DCLaneConnectivity>> laneConnectivitys = data_manager_->laneConnectivitys_;
            long preLaneID = INT_MAX;
            for (const auto &laneConn : laneConnectivitys) {
                if (laneConn.second->tLaneId_ == laneID) {
                    preLaneID = laneConn.second->fLaneId_;
                    break;
                }
            }
            if (map_lane_sch_.find(preLaneID) == map_lane_sch_.end()) {
                return nullptr;
            }
            map<long, shared_ptr<DCLaneCurvature>> mapLaneConn = map_lane_sch_[preLaneID];
            auto iter = mapLaneConn.rbegin();
            iter++;
            return iter->second;
        }

        shared_ptr<DCLaneCurvature> SlopeCheck::GetNextLaneAdasNode(long laneID) {
            map<string, shared_ptr<DCLaneConnectivity>> laneConnectivitys = data_manager_->laneConnectivitys_;
            long nextLaneID = INT_MAX;
            for (const auto &laneConn : laneConnectivitys) {
                if (laneConn.second->fLaneId_ == laneID) {
                    nextLaneID = laneConn.second->tLaneId_;
                    break;
                }
            }
            if (map_lane_sch_.find(nextLaneID) == map_lane_sch_.end()) {
                return nullptr;
            }
            map<long, shared_ptr<DCLaneCurvature>> mapLaneConn = map_lane_sch_[nextLaneID];
            auto iter = mapLaneConn.begin();
            iter++;
            return iter->second;
        }

        void SlopeCheck::CheckLaneSCH(shared_ptr<MapDataManager> modelDataManager,
                                      shared_ptr<CheckErrorOutput> errorOutput) {
            if (!LoadLaneSCH()) {
                return;
            }
            for (const auto &laneSCH : map_lane_sch_) {
                long laneID = laneSCH.first;
                map<long, shared_ptr<DCLaneCurvature>> mapLaneSCH = laneSCH.second;

                auto curIter = mapLaneSCH.begin();
                shared_ptr<DCLaneCurvature> pre = GetPreLaneAdasNode(laneID);
                if (pre == nullptr) {
                    pre = curIter->second;
                    curIter++;
                }
                while (curIter != mapLaneSCH.end()) {
                    auto nextIter = curIter;
                    nextIter++;
                    shared_ptr <DCLaneCurvature> next = nullptr;
                    if (nextIter == mapLaneSCH.end()) {
                        next = GetNextLaneAdasNode(laneID);
                    } else {
                        next = nextIter->second;
                    }

                    if (next == nullptr) {
                        break;
                    }

                    double avgSlope = (pre->slope_ + next->slope_) / 2;
                    double curSlope = curIter->second->slope_;
                    if (fabs(avgSlope - curSlope) > slope_threshold_) {
                        stringstream ss;
                        ss << "当前的 lane ID 是" << laneID << ", Index 是" << curIter->second->att_node_id_;
                        ss << ", 当前的坡度是" << curSlope << "前后两个node 平均坡度是 " << avgSlope;
                        ss << ", 误差大于 " << slope_threshold_;
                        auto error = DCAttributeCheckError::createByKXS_10_002(ss.str());
                        errorOutput->saveError(error);
                    }

                    pre = curIter->second;
                    curIter++;
                }
            }
        }

        shared_ptr<DCDivideSCH> SlopeCheck::GetPreDivideAdasNode(long divideID) {
            string strDividerID = to_string(divideID);
            map<string, shared_ptr<DCDivider>> dividers = data_manager_->dividers_;
            if (dividers.find(strDividerID) == dividers.end()) {
                return nullptr;
            }
            string fromID = dividers[strDividerID]->fromNodeId_;
            if (data_manager_->tnode_id2_dividers_maps_.find(fromID) == data_manager_->tnode_id2_dividers_maps_.end()) {
                return nullptr;
            }
            shared_ptr<DCDivider> dividerObj = data_manager_->tnode_id2_dividers_maps_[fromID].front();
            if (map_divider_sch_.find(stol(dividerObj->id_)) == map_divider_sch_.end()) {
                return nullptr;
            }
            auto iter = map_divider_sch_[stol(dividerObj->id_)].rbegin();
            iter++;
            return iter->second;
        }

        shared_ptr<DCDivideSCH> SlopeCheck::GetNextDivideAdasNode(long divideID) {
            string strDividerID = to_string(divideID);
            map<string, shared_ptr<DCDivider>> dividers = data_manager_->dividers_;
            if (dividers.find(strDividerID) == dividers.end()) {
                return nullptr;
            }
            string toID = dividers[strDividerID]->toNodeId_;
            if (data_manager_->fnode_id2_dividers_maps_.find(toID) == data_manager_->fnode_id2_dividers_maps_.end()) {
                return nullptr;
            }
            shared_ptr<DCDivider> dividerObj = data_manager_->fnode_id2_dividers_maps_[toID].front();
            if (map_divider_sch_.find(stol(dividerObj->id_)) == map_divider_sch_.end()) {
                return nullptr;
            }
            auto iter = map_divider_sch_[stol(dividerObj->id_)].begin();
            iter++;
            return iter->second;
        }

        void SlopeCheck::CheckDividerSCH(shared_ptr<MapDataManager> modelDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            if (!LoadDividerSCH()) {
                return;
            }
            for (const auto &divideSCH : map_divider_sch_) {
                long divideID = divideSCH.first;
                map<long, shared_ptr<DCDivideSCH>> mapDivideSCH = divideSCH.second;

                auto curIter = mapDivideSCH.begin();
                shared_ptr<DCDivideSCH> pre = GetPreDivideAdasNode(divideID);
                if (pre == nullptr) {
                    pre = curIter->second;
                    curIter++;
                }
                while (curIter != mapDivideSCH.end()) {
                    auto nextIter = curIter;
                    nextIter++;
                    shared_ptr<DCDivideSCH> next = nullptr;
                    if (nextIter == mapDivideSCH.end()) {
                        next = GetNextDivideAdasNode(divideID);
                    } else {
                        next = nextIter->second;
                    }

                    if (next == nullptr) {
                        break;
                    }

                    double avgSlope = (pre->slope_ + next->slope_) / 2;
                    double curSlope = curIter->second->slope_;
                    if (fabs(avgSlope - curSlope) > slope_threshold_) {
                        stringstream ss;
                        ss << "当前的 Divide ID 是" << divideID << ", Index 是" << curIter->second->att_node_id_;
                        ss << ", 当前的坡度是" << curSlope << "前后两个node 平均坡度是 " << avgSlope;
                        ss << ", 误差大于 " << slope_threshold_;
                        auto error = DCAttributeCheckError::createByKXS_10_002(ss.str());
                        errorOutput->saveError(error);
                    }

                    pre = curIter->second;
                    curIter++;
                }
            }
        }
    }
}