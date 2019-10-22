
#include <util/CommonCheck.h>
#include <businesscheck/DividerShapeNormCheck.h>

#include "businesscheck/DividerShapeNormCheck.h"

//core
#include "geom/geo_util.h"

using namespace geo;

//module
#include "DataCheckConfig.h"
#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
namespace dc {


string DividerShapeNormCheck::getId() {
    return id;
}

bool DividerShapeNormCheck::execute(shared_ptr<MapDataManager> data_manager,
                                    shared_ptr<CheckErrorOutput> error_output) {
    if (data_manager == nullptr)
        return false;

    set_data_manager(data_manager);
    set_error_output(error_output);

    if (CheckItemValid(CHECK_ITEM_KXS_ORG_011)) {
        Check_kxs_01_011();
    }

    if (CheckItemValid(CHECK_ITEM_KXS_ORG_012)) {
        Check_kxs_01_012();
    }

    if (CheckItemValid(CHECK_ITEM_KXS_ORG_013)) {
        Check_kxs_01_013();
    }

    return true;
}

void DividerShapeNormCheck::Check_kxs_01_011() {
    double divider_node_angle =
            DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_EDGE_ANGLE);
    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_011;
    int total = 0;
    for (auto& it : data_manager()->dividers_) {
        shared_ptr<DCDivider> div = it.second;
        auto ptr_error_nodes = CommonCheck::AngleCheck(
                div->nodes_, divider_node_angle);
        total += div->nodes_.size();
        if (ptr_error_nodes.size() > 1) {
            auto ptr_error = DCDividerCheckError::createByKXS_01_011(
                    div->id_, ptr_error_nodes);

            ptr_error->taskId_ = div->task_id_;
            ptr_error->flag = div->flag_;
            ptr_error->dataKey_ =
                    DATA_TYPE_LANE + div->task_id_+ DATA_TYPE_LAST_NUM;
            error_output()->saveError(ptr_error);
        }
    }
    checkItemInfo->totalNum = total;
    error_output()->addCheckItemInfo(checkItemInfo);
}

void DividerShapeNormCheck::Check_kxs_01_012() {
    double divider_node_len =
            DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_NODE_SPACE_LEN);
    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_012;
    int total = 0;
    for (auto& it : data_manager()->dividers_) {
        shared_ptr<DCDivider> div = it.second;
        auto ptr_error_nodes = CommonCheck::DistanceCheck(
                div->nodes_, divider_node_len);
        total += div->nodes_.size();
        if (ptr_error_nodes.size() > 1) {
            auto ptr_error = DCDividerCheckError::createByKXS_01_012(
                    div->id_, ptr_error_nodes);
            ptr_error->taskId_ = div->task_id_;
            ptr_error->flag = div->flag_;
            ptr_error->dataKey_ =
                    DATA_TYPE_LANE + div->task_id_ + DATA_TYPE_LAST_NUM;
            error_output()->saveError(ptr_error);
        }
    }
    checkItemInfo->totalNum = total;
    error_output()->addCheckItemInfo(checkItemInfo);
}

void DividerShapeNormCheck::Check_kxs_01_013() {
    double divider_height = DataCheckConfig::getInstance().getPropertyD(
            DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER);
    shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
    checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_013;
    int total = 0;
    for (auto& it : data_manager()->dividers_) {
        shared_ptr<DCDivider> div = it.second;
        auto ptr_error_nodes = CommonCheck::NodeHeightCheck(
                div->nodes_, divider_height);
        total += div->nodes_.size();
        if (ptr_error_nodes.size() > 1) {
            auto ptr_error = DCDividerCheckError::createByKXS_01_013(
                    div->id_, ptr_error_nodes);
            ptr_error->taskId_ = div->task_id_;
            ptr_error->flag = div->flag_;
            ptr_error->dataKey_ =
                    DATA_TYPE_LANE + div->task_id_ + DATA_TYPE_LAST_NUM;
            error_output()->saveError(ptr_error);
        }
    }
    checkItemInfo->totalNum = total;
    error_output()->addCheckItemInfo(checkItemInfo);
}

}
}