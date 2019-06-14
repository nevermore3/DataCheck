//
// Created by zhangxingang on 19-6-13.
//

#ifndef AUTOHDMAP_DATACHECK_JSONDATAINPUT_H
#define AUTOHDMAP_DATACHECK_JSONDATAINPUT_H

#include <ResourceManager.h>
#include "DataInput.h"

namespace kd {
    namespace dc {
        class JsonDataInput  : public DataInput {
        public:
            JsonDataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                          const shared_ptr<CheckErrorOutput> &error_output_, const string &base_path_,
                          shared_ptr<ResourceManager> resource_manager);

            bool LoadData() override;

        private:
            bool LoadDivider();

            bool LoadLane();

            bool LoadLaneGroup();

            bool LoadLaneConnectivity();

            bool LoadRoad();

            bool LoadLaneGroupLogicInfo();

        private:
            shared_ptr<ResourceManager> resource_manager_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_JSONDATAINPUT_H
