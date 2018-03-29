//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/MapDataLoader.h"

#include "storage/MapDataInput.h"

namespace kd {
    namespace dc {

        MapDataLoader::MapDataLoader(string basePath) {
            basePath_ = basePath;
        }

        string MapDataLoader::getId() {
            return id;
        }

        bool MapDataLoader::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            if( mapDataManager == nullptr)
                return false;

            MapDataInput mapDataInput;
            if(mapDataInput.loadDivider(basePath_, mapDataManager->dividers_)){

                for(auto recordit : mapDataManager->dividers_){
                    string divid = recordit.first;
                    shared_ptr<DCDivider> div = recordit.second;

                    //属性重排
                    div->sortAtts();

                    //构造空间几何属性
                    div->buildGeometryInfo();
                }
                return true;
            }

            return false;
        }
    }
}
