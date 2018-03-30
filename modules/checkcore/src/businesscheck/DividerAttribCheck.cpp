//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/DividerAttribCheck.h"

#include "DataCheckConfig.h"

#include "util/KDGeoUtil.hpp"
using namespace kd::automap;

namespace kd {
    namespace dc {

        DividerAttribCheck::DividerAttribCheck(){
            yellowTypes.insert(pair<long,long>(DA_TYPE_YELLOW_DOTTED, DA_TYPE_YELLOW_DOTTED));
            yellowTypes.insert(pair<long,long>(DA_TYPE_YELLOW_SOLID,DA_TYPE_YELLOW_SOLID));
            yellowTypes.insert(pair<long,long>(DA_TYPE_DUAL_YELLOW_DOTTED,DA_TYPE_DUAL_YELLOW_DOTTED));
            yellowTypes.insert(pair<long,long>(DA_TYPE_DUAL_YELLOW_SOLID,DA_TYPE_DUAL_YELLOW_SOLID));
            yellowTypes.insert(pair<long,long>(DA_TYPE_YELLOW_LSOLID_RDOT,DA_TYPE_YELLOW_LSOLID_RDOT));
            yellowTypes.insert(pair<long,long>(DA_TYPE_YELLOW_RSOLID_LDOT,DA_TYPE_YELLOW_RSOLID_LDOT));
            yellowTypes.insert(pair<long,long>(DA_TYPE_BUS_LANE,DA_TYPE_BUS_LANE));

            whiteTypes.insert(pair<long,long>(DA_TYPE_WHITE_SOLID,DA_TYPE_WHITE_SOLID));
            whiteTypes.insert(pair<long,long>(DA_TYPE_WHITE_DOTTED,DA_TYPE_WHITE_DOTTED));
            whiteTypes.insert(pair<long,long>(DA_TYPE_WHITE_LSOLID_RDOT,DA_TYPE_WHITE_LSOLID_RDOT));
            whiteTypes.insert(pair<long,long>(DA_TYPE_WHITE_RSOLID_LDOT,DA_TYPE_WHITE_RSOLID_LDOT));
            whiteTypes.insert(pair<long,long>(DA_TYPE_BUS_LANE,DA_TYPE_BUS_LANE));
        }

        string DividerAttribCheck::getId() {
            return id;
        }


        //int checkAttType( )

        //黄（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，
        //白（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，都可以变公交专用车道
        void DividerAttribCheck::attCheck_JH_C_3(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            for( auto recordit : mapDataManager->dividers_ ){
                shared_ptr<DCDivider> div = recordit.second;
                if(!div->valid_)
                    continue;

                if(div->atts_.size() <= 1)
                    continue;

                for(int i = 0 ; i < div->atts_.size() ; i ++) {
                    long refType = div->atts_[i]->type_;

                    auto yelloit = yellowTypes.find(refType);
                    if(yelloit != yellowTypes.end()){
                        //校验黄色车道线兼容性
                        for( int j = i+1 ; j < div->atts_.size() ; j ++ ){
                            long checkType = div->atts_[j]->type_;
                            if(yellowTypes.find(refType) == yellowTypes.end()){
                                //找到非同类属性数据
//                                shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>("JH_C_3");
//                                error->dividerId_ = div->id_;
//                                error->nodeId_ = div->atts_[j]->dividerNode_->id_;
//                                error->attId_ = div->atts_[j]->id_;
//                                error->lng_ = node1->coord_.lng_;
//                                error->lat_ = node1->coord_.lat_;
//                                error->z_ = node1->coord_.z_;
//                                error->errorDesc_ = to_string(distance);
                            }
                        }
                    }else{

                        auto whiteit = whiteTypes.find(refType);
                        if(whiteit != whiteTypes.end()){
                            //校验白色车道线兼容性
                            for( int j = i + 1 ; j < div->atts_.size() ; j ++){
                                long checkType = div->atts_[j]->type_;
                                if(whiteTypes.find(refType) == whiteTypes.end()){
                                    //找到非同类属性数据
                                }
                            }
                        }
                    }
                };

            };
        }


        bool DividerAttribCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            if(mapDataManager == nullptr)
                return false;


//            for( auto recordit : mapDataManager->dividers_){
//
//                shared_ptr<DCDivider> div = recordit.second;
//                if(!div->valid_)
//                    continue;
//
//                //检查每个DA控制的段
//                int attSize = div->atts_.size();
//                for( int i = 0 ;i < attSize ; i ++){
//                    shared_ptr<DCDividerAttribute> divAtt = div->atts_[i];
//                    int attIndex = i;
//                    int nodeBeginIndex = div->getAttNodeIndex(divAtt->dividerNode_);
//                    int nodeEndIndex;
//                    if(i == attSize-1){
//                        nodeEndIndex = div->nodes_.size() - 1;
//                    }else{
//                        shared_ptr<DCDividerAttribute> divAttEnd = div->atts_[i+1];
//                        nodeEndIndex = div->getAttNodeIndex(divAttEnd->dividerNode_);
//                    }
//
//                    checkShapeDefect(div, divAtt, nodeBeginIndex, nodeEndIndex, errorOutput);
//                }
//            }

            return true;
        }

    }
}
