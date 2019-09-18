//
// Created by gaoyanhong on 2018/4/5.
//

#include <util/CommonUtil.h>
#include <shp/ShpData.hpp>
#include <util/StringUtil.h>
#include "businesscheck/LaneTopoCheck.h"

namespace kd {
    namespace dc {

        string LaneTopoCheck::getId() {
            return id;
        }

        bool LaneTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                    shared_ptr<CheckErrorOutput> errorOutput) {
            check_JH_C_22(mapDataManager, errorOutput);
            check_topo_lanegroup_foreignkey(mapDataManager, errorOutput);
            return true;
        }

        //存在没有有进入车道和退出车道的车道
        void LaneTopoCheck::check_JH_C_22(shared_ptr<MapDataManager> mapDataManager,
                                          shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &f_node2_divider_maps = mapDataManager->fnode_id2_dividers_maps_;
            const auto &t_node2_divider_maps = mapDataManager->tnode_id2_dividers_maps_;
            int total =0;

            //构造查询map
            multimap<string, string> fromLaneConns;
            multimap<string, string> toLaneConns;

            for (auto laneConnit : mapDataManager->laneConnectivitys_) {

                shared_ptr<DCLaneConnectivity> laneConn = laneConnit.second;
                string fromLane = to_string(laneConn->fLaneId_);
                string toLane = to_string(laneConn->tLaneId_);

                fromLaneConns.insert(make_pair(fromLane, toLane));
                toLaneConns.insert(make_pair(toLane, fromLane));
            }

            //遍历所有的车道
            //修改错误描述,以及对孤立车道的检查逻辑,增加了对应急车道的判断以及前后DIVIDER连接关系判断,减少了一些冗余的错误输出
//            for(auto laneit : mapDataManager->lanes_){
            for (const auto &lg : mapDataManager->laneGroups_) {
                vector<shared_ptr<DCLane>> tag_emergency_lanes;
                for (const auto &lane : lg.second->lanes_) {

                    //查找以本车道为fromLane和toLane关联的记录数
                    int fromLaneCount = fromLaneConns.count(lane->id_);
                    int toLaneCount = toLaneConns.count(lane->id_);

                    if (fromLaneCount == 0 && toLaneCount == 0) {
                        tag_emergency_lanes.emplace_back(lane);
                    } else if (fromLaneCount == 0 && toLaneCount > 1) {
                        auto right_divider = lane->rightDivider_;
                        if (right_divider) {
                            auto div_back_node = right_divider->nodes_.back();
                            if (div_back_node) {
                                auto f_iter = f_node2_divider_maps.find(div_back_node->id_);
                                if (f_iter != f_node2_divider_maps.end()) {
                                    bool check = false;
                                    for (const auto &conn_div : f_iter->second) {
                                        if (conn_div->dividerNo_ == 0) {
                                            continue;
                                        }
                                        auto conn_lgs = CommonUtil::get_lane_groups_by_divider(mapDataManager,
                                                                                               conn_div->id_);
                                        if (!conn_lgs.empty()) {
                                            check = true;
                                            break;
                                        }
                                    }
                                    if (CheckItemValid(CHECK_ITEM_KXS_LANE_013) && check) {
                                        shared_ptr<DCLaneCheckError> error =
                                                DCLaneCheckError::createByNode(CHECK_ITEM_KXS_LANE_013, lane, nullptr);
                                        error->errorDesc_ = "lane_id:";
                                        error->errorDesc_ += lane->id_;
                                        error->errorDesc_ += "没有退出车道连接";
                                        error->checkName = "没有退出车道,divider拓扑存在,检查lane_connectivity是否正确";
                                        errorOutput->saveError(error);
                                    }
                                }
                            }
                        }

                    } else if (fromLaneCount > 1 && toLaneCount == 0) {
                        auto right_divider = lane->rightDivider_;
                        if (right_divider) {
                            auto div_front_node = right_divider->nodes_.front();
                            if (div_front_node) {
                                auto t_iter = t_node2_divider_maps.find(div_front_node->id_);
                                if (t_iter != t_node2_divider_maps.end()) {
                                    bool check = false;
                                    for (const auto &conn_div : t_iter->second) {
                                        auto conn_lgs = CommonUtil::get_lane_groups_by_divider(mapDataManager,
                                                                                               conn_div->id_);
                                        if (!conn_lgs.empty()) {
                                            check = true;
                                            break;
                                        }
                                    }
                                    if (CheckItemValid(CHECK_ITEM_KXS_LANE_014) && check) {
                                        shared_ptr<DCLaneCheckError> error =
                                                DCLaneCheckError::createByNode(CHECK_ITEM_KXS_LANE_014, lane, nullptr);
                                        error->errorDesc_ = "lane_id:";
                                        error->errorDesc_ += lane->id_;
                                        error->errorDesc_ += "没有进入车道连接";
                                        error->checkName = "没有进入车道,divider拓扑存在,检查lane_connectivity是否正确";
                                        errorOutput->saveError(error);
                                    }
                                }
                            }
                        }
                    }
                }

                if (!tag_emergency_lanes.empty()) {
                    if (tag_emergency_lanes.size() < lg.second->lanes_.size()) {
                        for (auto lane : tag_emergency_lanes) {
                            bool is_emergency = false;

                            for (const auto &attr : lane->atts_) {
                                if (attr->laneType_ == 9) {
                                    is_emergency = true;
                                    break;
                                }
                            }

                            if (CheckItemValid(CHECK_ITEM_KXS_LANE_012) && (!is_emergency)) {
                                shared_ptr<DCLaneCheckError> error =
                                        DCLaneCheckError::createByNode(CHECK_ITEM_KXS_LANE_012, lane, nullptr);
                                error->errorDesc_ = "lane_id:";
                                error->errorDesc_ += lane->id_;
                                error->errorDesc_ += "孤立车道,没有进入车道和退出车道";
                                error->checkName = "孤立车道,没有进入和退出车道连接,并且没有标注应急车道属性";
                                errorOutput->saveError(error);
                            }
                        }
                    }
                }
            }

            if(CheckItemValid(CHECK_ITEM_KXS_LANE_012)){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LANE_012,total);
            }
            if(CheckItemValid(CHECK_ITEM_KXS_LANE_014)){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LANE_014,total);
            }
            if(CheckItemValid(CHECK_ITEM_KXS_LANE_013)){
                errorOutput->addCheckItemInfo(CHECK_ITEM_KXS_LANE_013,total);
            }
        }

        void LaneTopoCheck::check_topo_lanegroup_foreignkey(shared_ptr<MapDataManager> mapDataManager,
                                                            shared_ptr<CheckErrorOutput> errorOutput) {
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string topoLaneGroupFile = basePath + "/HD_TOPO_LANEGROUP";
            ShpData shpFile(topoLaneGroupFile);
            if (!shpFile.isInit()) {
                LOG(ERROR) << "Open shpFile :" << topoLaneGroupFile << " Fail";
                return;
            }
            string modelName = "HD_TOPO_LANEGROUP";
            string foreignTable = "HD_LANE_GROUP";
            string keyName = "ID";
            size_t recordNums = shpFile.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpFile.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_POLYGONZ)
                    continue;

                //读取属性信息
                string strFromLG = shpFile.readStringField(i, "FROM_LG");
                string strToLG = shpFile.readStringField(i, "TO_LG");
                vector<string>fromLGArray;
                vector<string>toLGArray;
                StringUtil::Token(strFromLG.c_str(), ",", fromLGArray);
                StringUtil::Token(strToLG.c_str(), ",", toLGArray);
                for (const auto &fromLG : fromLGArray) {
                    if (fromLG == "0") {
                        continue;
                    }
                    if (mapDataManager->laneGroups_.find(fromLG) == mapDataManager->laneGroups_.end()) {
                        string foreignKeyName = "From_LG";
                        string value = fromLG;
                        auto error = DCForeignKeyCheckError::createByKXS_01_027(modelName,
                                                                                foreignKeyName,
                                                                                value,
                                                                                foreignTable,
                                                                                keyName);
                        errorOutput->saveError(error);
                    }
                }
                for (const auto &toLG : toLGArray) {
                    if (toLG == "0") {
                        continue;
                    }
                    if (mapDataManager->laneGroups_.find(toLG) == mapDataManager->laneGroups_.end()) {
                        string foreignKeyName = "TO_LG";
                        string value = toLG;
                        auto error = DCForeignKeyCheckError::createByKXS_01_027(modelName,
                                                                                foreignKeyName,
                                                                                value,
                                                                                foreignTable,
                                                                                keyName);
                        errorOutput->saveError(error);
                    }
                }

            }
        }
    }
}

