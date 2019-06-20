//
// Created by gaoyanhong on 2018/3/30.
//

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

        bool DividerShapeNormCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                            shared_ptr<CheckErrorOutput> errorOutput) {
            data_manager_ = mapDataManager;
            error_output_ = errorOutput;
            if (mapDataManager == nullptr)
                return false;

            for (auto dc_data : data_manager_->dividers_) {
                SmoothCheck(dc_data.second);
                DistanceCheck(dc_data.second);
                HeightCheck(dc_data.second);
            }

            return true;
        }

        void DividerShapeNormCheck::SmoothCheck(shared_ptr<DCDivider> dc_divider) {
            double divider_node_angle = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_EDGE_ANGLE);
            auto ptr_error_nodes = CommonCheck::AngleCheck(dc_divider->nodes_, divider_node_angle);

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCDividerCheckError::createByKXS_01_011(dc_divider->id_, ptr_error_nodes);
                error_output_->saveError(ptr_error);
            }

        }

        void DividerShapeNormCheck::DistanceCheck(shared_ptr<DCDivider> dc_divider) {
            double divider_node_len = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_NODE_SPACE_LEN);
            auto ptr_error_nodes = CommonCheck::DistanceCheck(dc_divider->nodes_, divider_node_len);

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCDividerCheckError::createByKXS_01_012(dc_divider->id_, ptr_error_nodes);
                error_output_->saveError(ptr_error);
            }
        }

        void DividerShapeNormCheck::HeightCheck(shared_ptr<DCDivider> dc_divider) {
            double divider_height = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER);
            auto ptr_error_nodes = CommonCheck::NodeHeightCheck(dc_divider->nodes_, divider_height);

            if (ptr_error_nodes.size() > 1) {
                auto ptr_error = DCDividerCheckError::createByKXS_01_013(dc_divider->id_, ptr_error_nodes);
                error_output_->saveError(ptr_error);
            }
        }

    }
}


