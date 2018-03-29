//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/DividerShapeDefectCheck.h"

namespace kd {
    namespace dc {

        string DividerShapeDefectCheck::getId() {
            return id;
        }

        bool DividerShapeDefectCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            if(mapDataManager == nullptr)
                return false;

            for( auto recordit : mapDataManager->dividers_){

                shared_ptr<DCDivider> div = recordit.second;
                if(!div->valid_)
                    continue;

            }

            map<string, > dividers_;


            return true;
        }

    }
}
