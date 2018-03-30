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
                


            }

        }


        bool DividerTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            check_JH_C_6(mapDataManager, errorOutput);

            return true;
        }
    }
}

