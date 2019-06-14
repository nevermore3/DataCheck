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

            shared_ptr<MapDataInput> map_data_input = make_shared<MapDataInput>(mapDataManager, errorOutput, basePath_);

            map_data_input->LoadData();

            return true;
        }
    }
}
