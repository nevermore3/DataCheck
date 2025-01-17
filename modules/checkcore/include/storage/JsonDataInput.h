
#ifndef AUTOHDMAP_DATACHECK_JSONDATAINPUT_H
#define AUTOHDMAP_DATACHECK_JSONDATAINPUT_H

#include <ResourceManager.h>
#include "DataInput.h"
#include "ModelInput.h"

namespace kd {
    namespace dc {
        class JsonDataInput  : public DataInput, public ModelInput {
        public:
            JsonDataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                          const shared_ptr<CheckErrorOutput> &error_output_,
                          const string &base_path_, const string &model_path_,
                          shared_ptr<ResourceManager> resource_manager);

            bool LoadData() override;

            void LoadModelField(string model_name, shared_ptr<KDSData> kds_data);

            void LoadModelRelation(string model_name, shared_ptr<KDSRelation> kds_relation);

            void CheckModelRelation();

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
