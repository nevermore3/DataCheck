//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/DividerAttribCheck.h"

#include "DataCheckConfig.h"

#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {

        DividerAttribCheck::DividerAttribCheck() {
            yellowTypes.insert(pair<long, long>(DA_TYPE_YELLOW_DOTTED, DA_TYPE_YELLOW_DOTTED));
            yellowTypes.insert(pair<long, long>(DA_TYPE_YELLOW_SOLID, DA_TYPE_YELLOW_SOLID));
            yellowTypes.insert(pair<long, long>(DA_TYPE_DUAL_YELLOW_DOTTED, DA_TYPE_DUAL_YELLOW_DOTTED));
            yellowTypes.insert(pair<long, long>(DA_TYPE_DUAL_YELLOW_SOLID, DA_TYPE_DUAL_YELLOW_SOLID));
            yellowTypes.insert(pair<long, long>(DA_TYPE_YELLOW_LSOLID_RDOT, DA_TYPE_YELLOW_LSOLID_RDOT));
            yellowTypes.insert(pair<long, long>(DA_TYPE_YELLOW_RSOLID_LDOT, DA_TYPE_YELLOW_RSOLID_LDOT));
            yellowTypes.insert(pair<long, long>(DA_TYPE_BUS_LANE, DA_TYPE_BUS_LANE));

            whiteTypes.insert(pair<long, long>(DA_TYPE_WHITE_SOLID, DA_TYPE_WHITE_SOLID));
            whiteTypes.insert(pair<long, long>(DA_TYPE_WHITE_DOTTED, DA_TYPE_WHITE_DOTTED));
            whiteTypes.insert(pair<long, long>(DA_TYPE_WHITE_LSOLID_RDOT, DA_TYPE_WHITE_LSOLID_RDOT));
            whiteTypes.insert(pair<long, long>(DA_TYPE_WHITE_RSOLID_LDOT, DA_TYPE_WHITE_RSOLID_LDOT));
            whiteTypes.insert(pair<long, long>(DA_TYPE_BUS_LANE, DA_TYPE_BUS_LANE));

            // 颜色为1时，分割线类型为1、2、3、4、5、6、7、11、14、16、18；
            map<long,long> whiteMaps;
            whiteMaps[DA_TYPE_ROAD_EDGE] = DA_TYPE_ROAD_EDGE;//1：车行道边缘线
            whiteMaps[DA_TYPE_WHITE_DOTTED] = DA_TYPE_WHITE_DOTTED;//2：白虚线
            whiteMaps[DA_TYPE_WHITE_SOLID] = DA_TYPE_WHITE_SOLID;//3：白实线
            whiteMaps[DA_TYPE_BUS_LANE] = DA_TYPE_BUS_LANE;//4：公交专用车道线
            whiteMaps[DA_TYPE_INOUT_LANE] = DA_TYPE_INOUT_LANE;//5：道路出入口标线
            whiteMaps[DA_TYPE_PENDING_AREA_LANE] = DA_TYPE_PENDING_AREA_LANE;//6：左弯待转区线
            whiteMaps[DA_TYPE_VARIABLE_DIRECTION] = DA_TYPE_VARIABLE_DIRECTION;//7：可变导向车道线
            whiteMaps[DA_TYPE_DECELERATION] = DA_TYPE_DECELERATION;//11：纵向减速标线
            whiteMaps[DA_TYPE_WHITE_LSOLID_RDOT] = DA_TYPE_WHITE_LSOLID_RDOT;//14：白左实右虚线
            whiteMaps[DA_TYPE_WHITE_RSOLID_LDOT] = DA_TYPE_WHITE_RSOLID_LDOT;//16：白右实左虚线
            whiteMaps[DA_TYPE_HOV_LANE] = DA_TYPE_HOV_LANE;//18：HOV专用车道线

            // 颜色为2时，分隔线类型为4、8、9、10、12、13、15、17、11；
            map<long,long> yellowMaps;
            yellowMaps[DA_TYPE_BUS_LANE] = DA_TYPE_BUS_LANE;//4：公交专用车道线
            yellowMaps[DA_TYPE_TIDAL_LANE] = DA_TYPE_TIDAL_LANE;//8：潮汐车道线
            yellowMaps[DA_TYPE_YELLOW_DOTTED] = DA_TYPE_YELLOW_DOTTED;//9：黄虚线
            yellowMaps[DA_TYPE_YELLOW_SOLID] = DA_TYPE_YELLOW_SOLID;//10：黄实线
            yellowMaps[DA_TYPE_DECELERATION] = DA_TYPE_DECELERATION;//11：纵向减速标线
            yellowMaps[DA_TYPE_DUAL_YELLOW_DOTTED] = DA_TYPE_DUAL_YELLOW_DOTTED;//12：双黄虚线
            yellowMaps[DA_TYPE_DUAL_YELLOW_SOLID] = DA_TYPE_DUAL_YELLOW_SOLID;//13：双黄实线
            yellowMaps[DA_TYPE_YELLOW_RSOLID_LDOT] = DA_TYPE_YELLOW_RSOLID_LDOT;//17：黄右实左虚线

            colorDividerTypeMaps.insert(pair<long,map<long,long>>(1, whiteMaps));
            colorDividerTypeMaps.insert(pair<long,map<long,long>>(2, yellowMaps));

            // 通行类型为1，分隔线类型为1、3、4、7、10、11、13、18；
            map<long,long> solidMaps;
            solidMaps[DA_TYPE_ROAD_EDGE] = DA_TYPE_ROAD_EDGE;//1：车行道边缘线
            solidMaps[DA_TYPE_WHITE_SOLID] = DA_TYPE_WHITE_SOLID;//3：白实线
            solidMaps[DA_TYPE_BUS_LANE] = DA_TYPE_BUS_LANE;//4：公交专用车道线
            solidMaps[DA_TYPE_VARIABLE_DIRECTION] = DA_TYPE_VARIABLE_DIRECTION;//7：可变导向车道线
            solidMaps[DA_TYPE_YELLOW_SOLID] = DA_TYPE_YELLOW_SOLID;//10：黄实线
            solidMaps[DA_TYPE_DECELERATION] = DA_TYPE_DECELERATION;//11：纵向减速标线
            solidMaps[DA_TYPE_DUAL_YELLOW_SOLID] = DA_TYPE_DUAL_YELLOW_SOLID;//13：双黄实线
            solidMaps[DA_TYPE_HOV_LANE] = DA_TYPE_HOV_LANE;//18：HOV专用车道线

            // 通行类型为2，分隔线类型为2、4、5、6、8、9、12、18；
            map<long,long> dotMaps;
            dotMaps[DA_TYPE_WHITE_DOTTED] = DA_TYPE_WHITE_DOTTED;//2：白虚线
            dotMaps[DA_TYPE_BUS_LANE] = DA_TYPE_BUS_LANE;//4：公交专用车道线
            dotMaps[DA_TYPE_INOUT_LANE] = DA_TYPE_INOUT_LANE;//5：道路出入口标线
            dotMaps[DA_TYPE_PENDING_AREA_LANE] = DA_TYPE_PENDING_AREA_LANE;//6：左弯待转区线
            dotMaps[DA_TYPE_TIDAL_LANE] = DA_TYPE_TIDAL_LANE;//8：潮汐车道线
            dotMaps[DA_TYPE_YELLOW_DOTTED] = DA_TYPE_YELLOW_DOTTED;//9：黄虚线
            dotMaps[DA_TYPE_DUAL_YELLOW_DOTTED] = DA_TYPE_DUAL_YELLOW_DOTTED;//12：双黄虚线
            dotMaps[DA_TYPE_HOV_LANE] = DA_TYPE_HOV_LANE;//18：HOV专用车道线

            // 通行类型为3，分隔线类型为14、15；
            map<long,long> lsrdMaps;
            lsrdMaps[DA_TYPE_WHITE_LSOLID_RDOT] = DA_TYPE_WHITE_LSOLID_RDOT;//14：白左实右虚线
            lsrdMaps[DA_TYPE_YELLOW_LSOLID_RDOT] = DA_TYPE_YELLOW_LSOLID_RDOT;//15：黄左实右虚线

            // 通行类型为4，分隔线类型为16、17；
            map<long,long> rsldMaps;
            rsldMaps[DA_TYPE_WHITE_RSOLID_LDOT] = DA_TYPE_WHITE_RSOLID_LDOT;//16：白右实左虚线
            rsldMaps[DA_TYPE_YELLOW_RSOLID_LDOT] = DA_TYPE_YELLOW_RSOLID_LDOT;//17：黄右实左虚线

            driveRuleDividerTypeMaps.insert(pair<long,map<long,long>>(1, solidMaps));
            driveRuleDividerTypeMaps.insert(pair<long,map<long,long>>(2, dotMaps));
            driveRuleDividerTypeMaps.insert(pair<long,map<long,long>>(3, lsrdMaps));
            driveRuleDividerTypeMaps.insert(pair<long,map<long,long>>(4, rsldMaps));

        }

        string DividerAttribCheck::getId() {
            return id;
        }

        //黄（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，
        //白（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，都可以变公交专用车道
        void DividerAttribCheck::check_JH_C_3(shared_ptr<MapDataManager> mapDataManager,
                                              shared_ptr<CheckErrorOutput> errorOutput) {

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                if (div->atts_.size() <= 1)
                    continue;

                for (int i = 0; i < div->atts_.size(); i++) {
                    long refType = div->atts_[i]->type_;

                    auto whiteit = whiteTypes.find(refType);
                    if (whiteit != whiteTypes.end()) {
                        //校验白色车道线兼容性
                        for (int j = i + 1; j < div->atts_.size(); j++) {
                            long checkType = div->atts_[j]->type_;
                            if (whiteTypes.find(checkType) == whiteTypes.end()) {
                                //找到非同类属性数据
                                shared_ptr<DCDividerCheckError> error =
                                        DCDividerCheckError::createByAtt("JH_C_3", div, div->atts_[j]);
                                stringstream ss;
                                ss << "[first att index " << (j - 1) << "][type: " << div->atts_[j - 1]->type_;
                                ss << "][secondd att index " << j << "][type: " << div->atts_[j]->type_ << "]";
                                error->errorDesc_ = ss.str();
                                errorOutput->saveError(error);
                            }
                        }
                    } else {

                        auto yellowit = yellowTypes.find(refType);
                        if (yellowit != yellowTypes.end()) {
                            //校验黄色车道线兼容性
                            for (int j = i + 1; j < div->atts_.size(); j++) {
                                long checkType = div->atts_[j]->type_;
                                if (yellowTypes.find(checkType) == yellowTypes.end()) {
                                    //找到非同类属性数据
                                    shared_ptr<DCDividerCheckError> error =
                                            DCDividerCheckError::createByAtt("JH_C_3", div, div->atts_[j]);
                                    stringstream ss;
                                    ss << "[first att index " << (j - 1) << "][type: " << div->atts_[j - 1]->type_;
                                    ss << "][secondd att index " << j << "][type: " << div->atts_[j]->type_ << "]";
                                    error->errorDesc_ = ss.str();
                                    errorOutput->saveError(error);
                                }
                            }
                        }
                    }
                };

            };
        }


        //字段匹配关系：
        // 虚拟分隔线（1、2、3）时，分隔线类型为0，颜色类型为0，通行类型为0；
        // 颜色为2时，分隔线类型为4、8、9、10、12、13、15、17、11；
        // 颜色为1时，分割线类型为1、2、3、4、5、6、7、11、14、16、18；
        // 通行类型为1，分隔线类型为1、3、4、7、10、11、13、18；
        // 通行类型为2，分隔线类型为2、4、5、6、8、9、12、18；
        // 通行类型为3，分隔线类型为14、15；
        // 通行类型为4，分隔线类型为16、17；
        void DividerAttribCheck::check_JH_C_10(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;
                if(div->atts_.size() == 0)
                    continue;

                for( auto att : div->atts_){

                    //虚拟分隔线（1、2、3）时，分隔线类型为0，颜色类型为0，通行类型为0；
                    if(att->virtual_ == 1 || att->virtual_ == 2 || att->virtual_ == 3){
                        if(att->type_ != 0 || att->color_ != 0 || att->driveRule_ != 0){
                            //车道线没有属性变化点
                            shared_ptr<DCDividerCheckError> error =
                                    DCDividerCheckError::createByAtt("JH_C_11", div, att);
                            stringstream ss;
                            ss << "virtual divider att error. [type:" << att->type_ << "][color:";
                            ss << att->color_ << "][dirveRule:" << att->driveRule_ << "]";
                            error->errorDesc_ = ss.str();

                            errorOutput->saveError(error);
                        }
                    }

                    // 颜色为1时，分割线类型为1、2、3、4、5、6、7、11、14、16、18；
                    // 颜色为2时，分隔线类型为4、8、9、10、12、13、15、17、11；
                    auto colormapit = colorDividerTypeMaps.find(att->color_);
                    if(colormapit != colorDividerTypeMaps.end()){
                        auto dividerTypes = colormapit->second;
                        auto typeit = dividerTypes.find(att->type_);
                        if(typeit == dividerTypes.end()){
                            //颜色和车道线线型不匹配
                            shared_ptr<DCDividerCheckError> error =
                                    DCDividerCheckError::createByAtt("JH_C_11", div, att);
                            stringstream ss;
                            ss << "divider color & type not match. [color:" << att->color_ << "][type:";
                            ss << att->type_ << "]";
                            error->errorDesc_ = ss.str();

                            errorOutput->saveError(error);
                        }
                    }

                    // 通行类型为1，分隔线类型为1、3、4、7、10、11、13、18；
                    // 通行类型为2，分隔线类型为2、4、5、6、8、9、12、18；
                    // 通行类型为3，分隔线类型为14、15；
                    // 通行类型为4，分隔线类型为16、17；
                    map<long,map<long,long>> driveRuleDividerTypeMaps;
                    auto driveruleit = driveRuleDividerTypeMaps.find(att->driveRule_);
                    if(driveruleit != driveRuleDividerTypeMaps.end()){
                        auto dividerTypes = driveruleit->second;
                        auto typeit = dividerTypes.find(att->type_);
                        if(typeit == dividerTypes.end()){
                            //通行类型和车道线线型不匹配
                            shared_ptr<DCDividerCheckError> error =
                                    DCDividerCheckError::createByAtt("JH_C_11", div, att);
                            stringstream ss;
                            ss << "divider driverule & type not match. [driveRule:" << att->driveRule_ << "][type:";
                            ss << att->type_ << "]";
                            error->errorDesc_ = ss.str();

                            errorOutput->saveError(error);

                        }
                    }
                }
            }
        }


        //起点没有有DA
        void DividerAttribCheck::check_JH_C_11(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                int daCount = div->atts_.size();
                //没有DA的情况
                if ( daCount == 0){
                    //车道线没有属性变化点
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByAtt("JH_C_11", div, nullptr);
                    error->errorDesc_ = "divider no da";

                    errorOutput->saveError(error);
                    continue;
                }

                //判断属性变化点的控制方向
                bool nodeDirection = true; //默认是正向
                if(div->nodes_[0]->id_ == div->toNodeId_){
                    nodeDirection = false;
                }

                //起点没有DA的情况
                {
                    string startAttNodeId;
                    if(nodeDirection)
                        startAttNodeId = div->atts_[0]->dividerNode_->id_; //正向第一个
                    else
                        startAttNodeId = div->atts_[daCount-1]->dividerNode_->id_; //反向第一个

                    if(startAttNodeId != div->fromNodeId_){
                        //车道线起点没有属性变化点
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByAtt("JH_C_11", div, nullptr);
                        error->errorDesc_ = "divider no da on start point";

                        errorOutput->saveError(error);
                        continue;
                    }
                }

                //判断起点和终点都有属性变化点的情形
                if(daCount > 1)
                {
                    string startAttNodeId = div->atts_[0]->dividerNode_->id_;
                    string endAttNodeId = div->atts_[daCount-1]->dividerNode_->id_;

                    if( (startAttNodeId == div->fromNodeId_ && endAttNodeId == div->toNodeId_) ||
                        (startAttNodeId == div->toNodeId_ && endAttNodeId == div->fromNodeId_) ){
                        //车道线起点和终点都有属性变化点
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByAtt("JH_C_11", div, nullptr);
                        error->errorDesc_ = "divider has da on start and end.";

                        errorOutput->saveError(error);
                        continue;
                    }
                }
            }
        }

        //同一个divider上相邻两个DA属性完全一样
        void DividerAttribCheck::check_JH_C_12(shared_ptr<MapDataManager> mapDataManager,
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

        //同一个divider上相邻两个DA距离<1米
        void DividerAttribCheck::check_JH_C_13(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){

            double daSpaceLen = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DA_SPACE_LEN);

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                if(div->atts_.size() <= 1){
                    continue;
                }


                //判断属性变化点的控制方向
                bool nodeDirection = true; //默认是正向
                if(div->nodes_[0]->id_ == div->toNodeId_){
                    nodeDirection = false;
                }

                for( int i = 1 ; i < div->atts_.size() ; i ++ ){

                    shared_ptr<DCDividerAttribute> da1 = div->atts_[i-1];
                    shared_ptr<DCDividerAttribute> da2 = div->atts_[i];

                    if(da1 == nullptr || da1->dividerNode_ == nullptr || da2 == nullptr || da2->dividerNode_ == nullptr){
                        continue;
                    }

                    int beginIndex = div->getAttNodeIndex(da1->dividerNode_);
                    int endIndex = div->getAttNodeIndex(da2->dividerNode_);

                    //1）只计算了平面距离；2）由于存在立交桥的情形，不直接计算两个节点间的直线距离，而是计算所有线段的距离和
                    double distance = calLength(div, beginIndex, endIndex, nodeDirection);

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

        double DividerAttribCheck::calLength(shared_ptr<DCDivider> div, int begin, int end, bool direction){

            double distTotal = 0.0;
            if(direction){
                for( int i = begin ; i < end ; i ++ ){
                    auto node1 = div->nodes_[i];
                    auto node2 = div->nodes_[i+1];
                    double distTemp = KDGeoUtil::distanceLL(node1->coord_.lng_, node1->coord_.lat_,
                                                            node2->coord_.lng_, node2->coord_.lat_);
                    distTotal += distTemp;
                }
            }else{
                for( int i = begin ; i > end ; i -- ){
                    auto node1 = div->nodes_[i];
                    auto node2 = div->nodes_[i-1];
                    double distTemp = KDGeoUtil::distanceLL(node1->coord_.lng_, node1->coord_.lat_,
                                                            node2->coord_.lng_, node2->coord_.lat_);
                    distTotal += distTemp;
                }
            }

            return distTotal;
        }


        bool DividerAttribCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            if (mapDataManager == nullptr)
                return false;

            check_JH_C_3(mapDataManager, errorOutput);

            check_JH_C_10(mapDataManager, errorOutput);

            check_JH_C_11(mapDataManager, errorOutput);

            check_JH_C_12(mapDataManager, errorOutput);

            check_JH_C_13(mapDataManager, errorOutput);

            return true;
        }

    }
}