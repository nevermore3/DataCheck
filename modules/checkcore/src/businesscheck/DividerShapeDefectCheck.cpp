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


        void DividerShapeDefectCheck::getComparePair(shared_ptr<DCDivider> div, int begin, int end, bool nodeDirection, vector<pair<int,int>> & pairs){

            if(nodeDirection){
                //正向查找
                for( int i = begin ; i <= end ; i ++ ){
                    if(div->nodes_[i]->dashType_ == DN_DASH_TYPE_DOT_END){

                        for( int j = i+1 ; j <= end ; j ++){
                            if(div->nodes_[j]->dashType_ == DN_DASH_TYPE_DOT_START){

                                pairs.emplace_back(pair<int,int>(i,j));
                                i = j; //check ???
                                break;
                            }
                        }
                    }
                }
            }else{
                //反向查找
                for( int i = begin ; i >= end ; i -- ){
                    if(div->nodes_[i]->dashType_ == DN_DASH_TYPE_DOT_END){

                        for( int j = i-1 ; j >= end ; j --){
                            if(div->nodes_[j]->dashType_ == DN_DASH_TYPE_DOT_START){

                                pairs.emplace_back(pair<int,int>(i,j));
                                i = j; //check ???
                                break;
                            }
                        }
                    }
                }
            }
        }


        void DividerShapeDefectCheck::checkShapeDefect( string checkModel, double distLimit, shared_ptr<DCDivider> div, shared_ptr<DCDividerAttribute> divAtt,
                               int beginNodexIdx, int endNodeIdx, bool nodeDirection, shared_ptr<CheckErrorOutput> errorOutput){
            vector<pair<int,int>> nodeSegs;
            getComparePair(div, beginNodexIdx, endNodeIdx, nodeDirection, nodeSegs);

            for( pair<int,int> oneseg : nodeSegs){
                shared_ptr<DCDividerNode> node1 = div->nodes_[oneseg.first];
                shared_ptr<DCDividerNode> node2 = div->nodes_[oneseg.second];

                double distance = KDGeoUtil::distanceLL( node1->coord_.lng_, node1->coord_.lat_, node2->coord_.lng_, node2->coord_.lat_);

                if(distance > distLimit){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode(checkModel, div, node1);

                    stringstream ss;
                    ss << "from node " << node1->id_ << " to node " << node2->id_ << " distance is " << distance << " meter.";
                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }
            }
        }



        void DividerShapeDefectCheck::checkShapeDefect(shared_ptr<DCDivider> div, shared_ptr<DCDividerAttribute> divAtt,
                              int beginNodexIdx, int endNodeIdx, bool nodeDirection, shared_ptr<CheckErrorOutput> errorOutput){

            if(divAtt->type_ == DA_TYPE_WHITE_DOTTED){
                //需求编号：JH_C_1
                double distLimit = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DOTLINE_DEFECT_LEN);
                string checkModel = "JH_C_1";
                checkShapeDefect(checkModel, distLimit, div, divAtt, beginNodexIdx, endNodeIdx, nodeDirection, errorOutput);

            }else if(divAtt->type_ == DA_TYPE_BUS_LANE){
                //需求编号：JH_C_2
                double distLimit = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::BUSELINE_DEFECT_LEN);

                string checkModel = "JH_C_2";
                checkShapeDefect(checkModel, distLimit, div, divAtt, beginNodexIdx, endNodeIdx, nodeDirection, errorOutput);
            }
        }


        void DividerShapeDefectCheck::check_JH_C_1_AND_JH_C_2(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){
            for( auto recordit : mapDataManager->dividers_){

                shared_ptr<DCDivider> div = recordit.second;
                if(!div->valid_)
                    continue;

                //判断属性变化点的控制方向
                bool nodeDirection = true; //默认是正向
                if(div->nodes_[0]->id_ == div->toNodeId_){
                    nodeDirection = false;
                }

                //检查每个DA控制的段
                int attSize = div->atts_.size();
                for( int i = 0 ;i < attSize ; i ++){
                    shared_ptr<DCDividerAttribute> divAtt = div->atts_[i];
                    int attIndex = i;
                    int nodeBeginIndex = div->getAttNodeIndex(divAtt->dividerNode_);
                    int nodeEndIndex;
                    if(i == attSize-1){
                        if(nodeDirection)
                            nodeEndIndex = div->nodes_.size() - 1;
                        else
                            nodeEndIndex = 0;
                    }else{
                        shared_ptr<DCDividerAttribute> divAttEnd = div->atts_[i+1];
                        nodeEndIndex = div->getAttNodeIndex(divAttEnd->dividerNode_);
                    }

                    checkShapeDefect(div, divAtt, nodeBeginIndex, nodeEndIndex, nodeDirection, errorOutput);
                }
            }
        }

        bool DividerShapeDefectCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            if(mapDataManager == nullptr)
                return false;

            errorOutput->writeInfo("<DividerShapeDefectCheck>\n" + make_shared<DCDividerCheckError>("")->getHeader());
            check_JH_C_1_AND_JH_C_2(mapDataManager, errorOutput);

            return true;
        }

    }
}
