//
// Created by ubuntu on 9/19/19.
//
#include "businesscheck/SCHCheck.h"
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>
#include <util/CommonCheck.h>
#include "shp/ShpData.hpp"
#include <algorithm>

namespace kd {
    namespace dc {

        SCHCheck::SCHCheck(string &fileName) : file_name_(fileName) {
            LoadData();
        }


        bool SCHCheck::LoadData() {
            vector<string> whiteList {"HD_DIVIDER_SCH", "HD_LANE_SCH", "ADAS_NODE"};
            if (find(whiteList.begin(), whiteList.end(), file_name_) == whiteList.end()) {
                LOG(ERROR) <<"Not HD_DIVIDER_SCH、 HD_LANE_SCH or ADAS_NODE";
                return false;
            }
            string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
            string filePath = basePath + "/" + file_name_;
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

                shared_ptr<DCSCHInfo> schInfo = make_shared<DCSCHInfo>();

                //读取属性信息
                schInfo->id_ = std::to_string(shpFile.readIntField(i, "ID"));
                schInfo->index_ = shpFile.readIntField(i, "A_NodeID");
                schInfo->curvature_ = shpFile.readDoubleField(i, "Curvature");
                schInfo->slope_ = shpFile.readDoubleField(i, "Slope");
                schInfo->heading_ = shpFile.readDoubleField(i, "Heading");

                size_t nVertices = shpObject->nVertices;
                if (nVertices == 1) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->x_ = shpObject->padfX[0];
                    coord->y_ = shpObject->padfY[0];
                    coord->z_ = shpObject->padfZ[0];
                    schInfo->coord_ = coord;
                }
                if (file_name_ == "ADAS_NODE") {
                    schInfo->obj_id_ = shpFile.readIntField(i, "ROAD_ID");
                    schInfo->obj_index_ = shpFile.readIntField(i, "R_NodeIdx");
                } else if (file_name_ == "HD_LANE_SCH") {
                    schInfo->obj_id_ = shpFile.readIntField(i, "LANE_ID");
                    schInfo->obj_index_ = shpFile.readIntField(i, "L_NodeIdx");
                } else if (file_name_ == "HD_DIVIDER_SCH") {
                    schInfo->obj_id_ = shpFile.readIntField(i, "Divider_ID");
                    schInfo->obj_index_ = shpFile.readIntField(i, "D_NodeIdx");
                }

                // key : 关联对象 ID, value : 关联同一个对象ID的 属性点
                if (map_obj_schs_.find(schInfo->obj_id_) == map_obj_schs_.end()) {
                    vector<shared_ptr<DCSCHInfo>> nodes;
                    nodes.push_back(schInfo);
                    map_obj_schs_.insert(make_pair(schInfo->obj_id_, nodes));
                } else {
                    map_obj_schs_[schInfo->obj_id_].push_back(schInfo);
                }
            }

            // 同一个对象ID的 属性点数组 按照 属性点索引index_ 排序
            for (auto &nodesArray : map_obj_schs_) {

                sort(nodesArray.second.begin(), nodesArray.second.end(),
                    [](const shared_ptr<DCSCHInfo> &a, const shared_ptr<DCSCHInfo> &b) {
                           return a->index_ < b->index_;
                    });

            }
        }

        /*
         * 相邻的属性点之间距离不能超过1.3m
         */
        void SCHCheck::CheckAdjacentNodeDistance(shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_032;
            size_t  total = 0;

            double threshold = 1.3;
            double distance = 0;

            for (const auto &obj : map_obj_schs_) {
                long objID = obj.first;
                vector<shared_ptr<DCSCHInfo>> nodes = obj.second;
                total += nodes.size();
                size_t  i = 1;
                while (i < nodes.size()) {
                    distance = GeosObjUtil::get_length_of_node(nodes[i-1]->coord_, nodes[i]->coord_);
                    if (distance > threshold) {
                        auto error = DCSCHInfoError::createByKXS_01_032(objID, i, i-1, distance, threshold, file_name_);
                        errorOutput->saveError(error);
                    }
                    i++;
                }
            }

            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        /*
         * 每个属性点坡度 和前后两个属性点的坡度的均值比较 差值不能超过0.1
         */
        void SCHCheck::SlopeCheck() {
//            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::AVG_SLOPE_ERROR);
//            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
//            checkItemInfo->checkId = CHECK_ITEM_KXS_NORM_002;
//            size_t  total = 0;
//

        }

        void SCHCheck::CurvatureValueCheck(shared_ptr<CheckErrorOutput> &errorOutput) {
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_CURVATURE);
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_031;
            size_t  total = 0;

            for (const auto &obj : map_obj_schs_) {
                total += obj.second.size();
                for (const auto &node : obj.second) {
                    if (fabs(node->curvature_) > threshold) {
                        auto error = DCSCHInfoError::createByKXS_01_031(file_name_, to_string(obj.first), node->curvature_,
                                                                        threshold, node->coord_);

                        errorOutput->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

    }
}