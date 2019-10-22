
#include <util/dc_data_relation_util.h>
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

        bool
        MapDataLoader::execute(shared_ptr<MapDataManager> map_data_manager, shared_ptr<CheckErrorOutput> error_output) {
            if (map_data_manager == nullptr)
                return false;

            map_data_manager_ = map_data_manager;
            error_output_ = error_output;
            shared_ptr<MapDataInput> map_data_input = make_shared<MapDataInput>(map_data_manager_, error_output_,
                                                                                basePath_);

            map_data_input->LoadData();

            DCDataRelationUtil::BuildTopoLaneGroup(map_data_manager_, map_data_manager_->topo_lane_groups_);

            return true;
        }
    }
}
