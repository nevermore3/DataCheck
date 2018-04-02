//
// Created by gaoyanhong on 2018/4/1.
//


#include "businesscheck/LaneAttribCheck.h"

#include "DataCheckConfig.h"

#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {

        LaneAttribCheck::LaneAttribCheck() {

        }

        string LaneAttribCheck::getId() {
            return id;
        }

        //车道右侧车道线起点没有LA
        void LaneAttribCheck::check_JH_C_19(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                if (div->atts_.size() == 0){
                    //车道线没有属性变化点
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByAtt("JH_C_11", div, nullptr);
                    error->errorDesc_ = "divider no da";

                    errorOutput->saveError(error);
                    continue;
                }

                int nodeIndex = div->getAttNodeIndex( div->atts_[0]->dividerNode_ );
                if( nodeIndex != 0){
                    //车道线起点没有属性变化点
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByAtt("JH_C_11", div, nullptr);
                    error->errorDesc_ = "divider no da on start point";

                    errorOutput->saveError(error);
                    continue;
                }
            }
        }

        //同一Divider上相邻两个LA属性完全一样
        void LaneAttribCheck::check_JH_C_21(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){
            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                if(div->atts_.size() <= 1){
                    continue;
                }

                for( int i = 1 ; i < div->atts_.size() ; i ++ ){

                    shared_ptr<DCDividerAttribute> da1 = div->atts_[i-1];
                    shared_ptr<DCDividerAttribute> da2 = div->atts_[i];

                    if(da1 != nullptr && da1->typeSame(da2)){
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByAtt("JH_C_12", div, da1);

                        stringstream ss;
                        ss << "da [id:"<< da1->id_ << "] same as da [id:" << da2->id_ << "]";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }
                }
            }
        }

        //同一Divider上相邻两个LA距离<1米
        void LaneAttribCheck::check_JH_C_20(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){

            double daSpaceLen = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DA_SPACE_LEN);

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                if(div->atts_.size() <= 1){
                    continue;
                }

                for( int i = 1 ; i < div->atts_.size() ; i ++ ){

                    shared_ptr<DCDividerAttribute> da1 = div->atts_[i-1];
                    shared_ptr<DCDividerAttribute> da2 = div->atts_[i];

                    if(da1 == nullptr || da1->dividerNode_ == nullptr || da2 == nullptr || da2->dividerNode_ == nullptr){
                        continue;
                    }

                    auto node1 = da1->dividerNode_;
                    auto node2 = da2->dividerNode_;
                    double distance = KDGeoUtil::distanceLL(node1->coord_.lng_, node1->coord_.lat_, node2->coord_.lng_, node2->coord_.lat_);

                    if(distance < daSpaceLen){
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByAtt("JH_C_13", div, da1);

                        stringstream ss;
                        ss << "length from da [id:"<< da1->id_ << "] to da [id:" << da2->id_ << "] is " << distance << " meter";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }
                }
            }
        }


        bool LaneAttribCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            if (mapDataManager == nullptr)
                return false;

            check_JH_C_19(mapDataManager, errorOutput);

            check_JH_C_20(mapDataManager, errorOutput);

            check_JH_C_21(mapDataManager, errorOutput);

            return true;
        }

    }
}

