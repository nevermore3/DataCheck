
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

        DividerCheck::DividerCheck(string fileName) : SCHCheck(fileName) {}

        DividerCheck::~DividerCheck() {}

        string DividerCheck::getId() {
            return id_;
        }

        void DividerCheck::SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager) {
            map_data_manager_ = mapDataManager;
        }
        /*
         * 1、每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 2、DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 3、相邻HD_DIVIDER_SCH点之间距离不超过1.3m
         */
        bool DividerCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                   shared_ptr<CheckErrorOutput> errorOutput) {

            SetMapDataManager(mapDataManager);
            //曲率值检查
            CurvatureValueCheck(errorOutput);

            //坡度值检查
            SlopeValueCheck(errorOutput);

            //航向角检查
            HeadingValueCheck(errorOutput);

            //相邻HD_DIVIDER_SCH点之间距离不超过1.3m
            CheckAdjacentNodeDistance(errorOutput);

            //每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
            DividerRelevantDividerSCH(errorOutput);

            //HD_DIVIDER_SCH点离关联的DIVIDER的垂直距离不超过10cm
            DividerSCHVerticalDistance(errorOutput);

            DividerSCHRelevantDividerSlope(errorOutput);
            return true;
        }



        /*
         * 1、每一DIVIDER的形状点周围1.5米内必有一个关联该DIVIDER的HD_DIVIDER_SCH
         * 2、DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
         */
        void DividerCheck::DividerRelevantDividerSCH(shared_ptr<CheckErrorOutput> &errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_030;
            size_t  total = 0;

            double threshold = 1.5;
            /*
             * 1、找到一条divider对象
             * 2、在map_obj_schs中找到对应 该divider对象的属性点集合
             * 3、遍历该divider对象的形点
             * 3、按照属性点中的obj_index索引，第一个遍历到的属性点就是离该形点最近的点
             */
            auto dividers = map_data_manager_->dividers_;
            for (const auto &divider : dividers) {
                long dividerID = stol(divider.first);
                if (map_obj_schs_.find(dividerID) == map_obj_schs_.end()) {
                    continue;
                }
                auto schNodes = map_obj_schs_[dividerID];
                total += divider.second->nodes_.size();
                // 关联同一个divider对象的属性点的索引
                size_t j = 0;
                for (size_t i = 0; i < divider.second->nodes_.size(); i++) {
                    while (j < schNodes.size()) {
                        if (schNodes[j]->obj_index_ == i) {
                            // 找到第一个和divider形点关联的属性点
                            double distance = GeosObjUtil::get_length_of_node(divider.second->nodes_[i]->coord_, schNodes[j]->coord_);
                            if (distance > threshold) {
                                auto error = DCDividerCheckError::createByKXS_01_030(dividerID, i,
                                                                                     divider.second->nodes_[i]->coord_, 1);
                                errorOutput->saveError(error);
                            }

                            // DIVIDER的起点和终点之处（buffer20cm）必有一个关联该DIVIDER的HD_DIVIDER_SCH
                            if (i == 0 || i == divider.second->nodes_.size() - 1) {
                                if (distance > 0.2) {
                                    auto error = DCDividerCheckError::createByKXS_01_030(dividerID, i,
                                                                                         divider.second->nodes_[i]->coord_, 2);
                                    errorOutput->saveError(error);
                                }
                            }

                            j++;
                            break;
                        }
                        j++;
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }


        void DividerCheck::DividerSCHVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput) {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_029;
            size_t total = 0;

            for (const auto &dividerSCH : map_obj_schs_) {
                total += dividerSCH.second.size();
                long dividerID = dividerSCH.first;
                string strDividerID = to_string(dividerID);
                if (map_data_manager_->dividers_.find(strDividerID) == map_data_manager_->dividers_.end()) {
                    continue;
                }
                auto divider = map_data_manager_->dividers_[strDividerID];
                for (const auto &node : dividerSCH.second) {
                    shared_ptr<geos::geom::Point> point = GeosObjUtil::CreatePoint(node->coord_);
                    double distance = GeosObjUtil::GetVerticleDistance(divider->line_, point);
                    if (distance > 0.1) {
                        auto error = DCDividerCheckError::createByKXS_01_029(node->index_, node->coord_);
                        errorOutput->saveError(error);
                    }
                }
            }
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        void DividerCheck::DividerSCHRelevantDividerSlope(shared_ptr<CheckErrorOutput> &errorOutput) {

            for (const auto &dividerSCH : map_obj_schs_) {
                long dividerID = dividerSCH.first;
                string strDividerID = to_string(dividerID);
                vector<shared_ptr<DCSCHInfo>> schNodes = dividerSCH.second;

                //get relevant divider
                if (map_data_manager_->dividers_.find(strDividerID) == map_data_manager_->dividers_.end()) {
                    continue;
                }
                auto divider = map_data_manager_->dividers_[strDividerID];
                SCHNodeRelevantObjectSlope(dividerID, schNodes, divider->nodes_, errorOutput);
            }

        }

    }
}
