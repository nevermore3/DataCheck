//
// Created by ubuntu on 9/19/19.
//
#include "businesscheck/SCHCheck.h"
#include "util/CommonUtil.h"
#include <util/KDGeoUtil.hpp>
#include <util/GeosObjUtil.h>
#include <util/GeometryUtil.h>
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
         * 每个属性点的坡度值不能超过0.15 可以配置
         */
        void SCHCheck::SlopeValueCheck(shared_ptr<CheckErrorOutput> &errorOutput){
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::SCH_SLOPE_ERROR);
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_034;
            size_t  total = 0;

            for (const auto &obj : map_obj_schs_) {
                total += obj.second.size();
                for (const auto &node : obj.second) {
                    if (fabs(node->slope_) > threshold) {
                        auto error = DCSCHInfoError::createByKXS_01_034(file_name_, obj.first, node->index_, node->slope_,
                                                                        threshold, node->coord_);
                        errorOutput->saveError(error);
                    }
                }
                checkItemInfo->totalNum = total;
                errorOutput->addCheckItemInfo(checkItemInfo);
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        /*
         * 对车道和车道线上某一个SCH点，根据同一条线上前后SCH的空间关系计算出航向角，与SCH上记录的航向角对比
         * 误差不能超过15度 (可配置)
         */
        void SCHCheck::HeadingValueCheck(shared_ptr<CheckErrorOutput> &errorOutput) {
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::SCH_HEADING_ERROR);

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_033;
            size_t total = 0;
            //航向角的计算：
            //两个属性点组成的线段和开始属性点正北方向组成的线段之间的角度
            double angle = 0;
            for (const auto &obj : map_obj_schs_) {
                total += obj.second.size();
                vector<shared_ptr<DCSCHInfo>> nodes = obj.second;
                if (nodes.size() < 2) {
                    continue;
                }
                for (size_t i = 1; i < nodes.size(); i++) {
                    /*
                     * 计算 i-1 和 i 点组成的线段 和
                     * i-1 和 i-1正北方向组成线段
                     * 之间的夹角
                     */
                    angle = GeometryUtil::getAngleDiff(nodes[i-1]->coord_->x_, nodes[i-1]->coord_->y_,
                                                       nodes[i-1]->coord_->x_, nodes[i-1]->coord_->y_ + 1,
                                                       nodes[i-1]->coord_->x_, nodes[i-1]->coord_->y_,
                                                       nodes[i]->coord_->x_,   nodes[i]->coord_->y_);
                    if (fabs(angle - nodes[i-1]->heading_) > threshold) {
                        auto error = DCSCHInfoError::createByKXS_01_033(obj.first, i-1, angle, threshold, file_name_);
                        errorOutput->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        void SCHCheck::CurvatureValueCheck(shared_ptr<CheckErrorOutput> &errorOutput) {
            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::SCH_CURVATURE_ERROR);
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

        void SCHCheck::SCHNodeRelevantObjectSlope(long objID,
                                                  vector<shared_ptr<DCSCHInfo>> &nodes,
                                                  vector<shared_ptr<DCCoord>> &coords,
                                                  shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_035;
            size_t total = nodes.size();

            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::NODE_RELEVANT_OBJ_SLOPE_ERROR);
            size_t  i = 1;
            size_t  num = coords.size();

            for (const auto &node : nodes) {
                if (i < num - 1) {
                    double distance1 = GeosObjUtil::get_length_of_node(node->coord_, coords[i - 1]);
                    double distance2 = GeosObjUtil::get_length_of_node(node->coord_, coords[i + 1]);
                    if (distance1 > distance2) {
                        i++;
                    }
                }
                auto coordA = coords[i];
                auto coordB = coords[i - 1];
                double distanceAB = GeosObjUtil::get_length_of_node(coordA, coordB);
                double avgSlope = (coordA->z_ - coordB->z_) / distanceAB;
                double diffSlope = fabs(avgSlope - node->slope_);
                if (diffSlope > threshold) {
                    auto error = DCSCHInfoError::createByKXS_01_035(file_name_, objID, stol(node->id_),
                                                                    node->slope_, avgSlope, threshold);
                    errorOutput->saveError(error);
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }


        void SCHCheck::SCHNodeRelevantObjectSlope(long objID,
                                                  vector<shared_ptr<DCSCHInfo>> &nodes,
                                                  vector<shared_ptr<DCDividerNode>> &coords,
                                                  shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_035;
            size_t total = nodes.size();

            double threshold = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::NODE_RELEVANT_OBJ_SLOPE_ERROR);
            size_t  i = 1;
            size_t  num = coords.size();

            for (const auto &node : nodes) {
                if (i < num - 1) {
                    double distance1 = GeosObjUtil::get_length_of_node(node->coord_, coords[i - 1]->coord_);
                    double distance2 = GeosObjUtil::get_length_of_node(node->coord_, coords[i + 1]->coord_);
                    if (distance1 > distance2) {
                        i++;
                    }
                }
                auto coordA = coords[i]->coord_;
                auto coordB = coords[i - 1]->coord_;
                double distanceAB = GeosObjUtil::get_length_of_node(coordA, coordB);
                double avgSlope = (coordA->z_ - coordB->z_) / distanceAB;
                double diffSlope = fabs(avgSlope - node->slope_);
                if (diffSlope > threshold) {
                    auto error = DCSCHInfoError::createByKXS_01_035(file_name_, objID, stol(node->id_),
                                                                    node->slope_, avgSlope, threshold);
                    errorOutput->saveError(error);
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }
    }
}