//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/DividerShapeDefectCheck.h"

#include "DataCheckConfig.h"

#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
namespace dc {

string DividerShapeDefectCheck::getId() {
    return id;
}

bool DividerShapeDefectCheck::execute(shared_ptr<MapDataManager> data_manager,
                                      shared_ptr<CheckErrorOutput> error_output) {
    if (data_manager == nullptr)
        return false;
    set_data_manager(data_manager);
    set_error_output(error_output);

    if (CheckItemValid(CHECK_ITEM_KXS_ORG_005)) {
        Check_kxs_01_005();
    }

    if (CheckItemValid(CHECK_ITEM_KXS_ORG_006)) {
        Check_kxs_01_006();
    }

    return true;
}

void DividerShapeDefectCheck::getComparePair(
        shared_ptr<DCDivider> div, int begin, int end, bool nodeDirection,
        vector<pair<int, int>> &pairs) {

    if (nodeDirection) {
        //正向查找
        for (int i = begin; i <= end; i++) {
            if (div->nodes_[i]->dashType_ == DN_DASH_TYPE_DOT_END) {

                for (int j = i + 1; j <= end; j++) {
                    if (div->nodes_[j]->dashType_ == DN_DASH_TYPE_DOT_START) {

                        pairs.emplace_back(pair<int, int>(i, j));
                        i = j; //check ???
                        break;
                    }
                }
            }
        }
    } else {
        //反向查找
        for (int i = begin; i >= end; i--) {
            if (div->nodes_[i]->dashType_ == DN_DASH_TYPE_DOT_END) {

                for (int j = i - 1; j >= end; j--) {
                    if (div->nodes_[j]->dashType_ == DN_DASH_TYPE_DOT_START) {

                        pairs.emplace_back(pair<int, int>(i, j));
                        i = j; //check ???
                        break;
                    }
                }
            }
        }
    }
}


void DividerShapeDefectCheck::checkShapeDefect(
        string checkModel, double distLimit, shared_ptr<DCDivider> div,
        int beginNodexIdx, int endNodeIdx, bool nodeDirection,int &subTotal) {
    vector<pair<int, int>> nodeSegs;
    getComparePair(div, beginNodexIdx, endNodeIdx, nodeDirection, nodeSegs);
    subTotal = nodeSegs.size();
    for (pair<int, int> oneseg : nodeSegs) {
        shared_ptr<DCDividerNode> node1 = div->nodes_[oneseg.first];
        shared_ptr<DCDividerNode> node2 = div->nodes_[oneseg.second];

        double distance = KDGeoUtil::distanceLL(
                node1->coord_->lng_, node1->coord_->lat_,
                node2->coord_->lng_, node2->coord_->lat_);

        if (distance > distLimit) {
            shared_ptr<DCDividerCheckError> error =
                    DCDividerCheckError::createByNode(checkModel, div, node1);
            error->checkName = "检查车道线节点间距判定是否中间有虚线丢失";
            stringstream ss;
            ss << "node_id:" << node1->id_ << "与node_id:" << node2->id_
               << "距离" << distance << "米";
            error->errorDesc_ = ss.str();
            error->checkLevel_ = LEVEL_WARNING;
            error->coord = node2->coord_;

            error_output()->saveError(error);
        }
    }
}

void DividerShapeDefectCheck::Check_kxs_01_005() {
    check_kxs(DataCheckConfig::DOTLINE_DEFECT_LEN,
              CHECK_ITEM_KXS_ORG_005, DA_TYPE_WHITE_DOTTED);
}

void DividerShapeDefectCheck::Check_kxs_01_006() {
    check_kxs(DataCheckConfig::BUSELINE_DEFECT_LEN,
              CHECK_ITEM_KXS_ORG_006, DA_TYPE_BUS_LANE);
}

void DividerShapeDefectCheck::check_kxs(
        const std::string& limit_key,
        const std::string& check_model,
        EnumDividerAttributeType div_attr_type) {
    double dist_dimit =
            DataCheckConfig::getInstance().getPropertyD(limit_key);
    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = check_model;
    int total=0;
    for (const auto& recordit : data_manager()->dividers_) {
        shared_ptr<DCDivider> div = recordit.second;
        if (!div->valid_)
            continue;

        //判断属性变化点的控制方向
        bool direction = true; //默认是正向
        if (div->nodes_[0]->id_ == div->toNodeId_) {
            direction = false;
        }

        //检查每个DA控制的段
        int attSize = div->atts_.size();
        for (int i = 0; i < attSize; i++) {
            auto& div_att = div->atts_.at(i);
            int begin_index = div->getAttNodeIndex(div_att->dividerNode_);
            int node_index = 0;
            if (i == attSize - 1) {
                if (direction)
                    node_index = div->nodes_.size() - 1;
                else
                    node_index = 0;
            } else {
                auto& div_att_end = div->atts_.at(i + 1);
                node_index = div->getAttNodeIndex(div_att_end->dividerNode_);
            }
            int subTotal=0;
            if (div_att->type_ == div_attr_type) {
                checkShapeDefect(check_model, dist_dimit, div,
                                 begin_index, node_index, direction,subTotal);
            }
            total +=subTotal;
        }
    }
    checkItemInfo->totalNum = total;
    error_output()->addCheckItemInfo(checkItemInfo);
}

}
}
