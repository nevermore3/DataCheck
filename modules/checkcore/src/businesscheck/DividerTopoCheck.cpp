//
// Created by gaoyanhong on 2018/3/30.
//

#include "businesscheck/DividerTopoCheck.h"

namespace kd {
    namespace dc {

        string DividerTopoCheck::getId() {
            return id;
        }


        void DividerTopoCheck::check_JH_C_6(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            //记录所有车道线共点信息， map的key为节点id
            map<string, shared_ptr<DCDividerTopoNode>> topoNodes_;
            for( auto nodeit : mapDataManager->topoNodes_){
                shared_ptr<DCDividerTopoNode> topoNode = nodeit.second;

                if(topoNode == nullptr)
                    continue;

                if(topoNode->endRelateNodes_.size() != 1 || topoNode->startRelateNodes_.size() != 1)
                    continue;

                //检查两个线的方向是否一致
                string divId1 = topoNode->endRelateNodes_.begin()->first;
                string divId2 = topoNode->startRelateNodes_.begin()->first;

                auto divit1 = mapDataManager->dividers_.find(divId1);
                auto divit2 = mapDataManager->dividers_.find(divId2);

                if(divit1 == mapDataManager->dividers_.end() || divit2 == mapDataManager->dividers_.end()){
                    cout << "[Error] not find divider " << divId1 << "," << divId2 << endl;
                    continue;
                }

                shared_ptr<DCDivider> div1 = divit1->second;
                shared_ptr<DCDivider> div2 = divit2->second;
                if( (div1->direction_ == 2 && div2->direction_ == 1) ||
                        (div1->direction_ == 1 && div2->direction_ == 2)){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode("JH_C_6", div1, div1->nodes_[0]);

                    stringstream ss;
                    ss << "divider direction conflict. [div1:" << div1->id_ << "],[direction:" << div1->direction_ ;
                    ss << "];[div2:" << div2->id_ << "],[direction:" << div2->direction_ << "]";

                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }
            }
        }


        bool DividerTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            check_JH_C_6(mapDataManager, errorOutput);

            return true;
        }
    }
}

