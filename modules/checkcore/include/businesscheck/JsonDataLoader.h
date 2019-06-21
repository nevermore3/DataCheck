//
// Created by zhangxingang on 19-6-13.
//

#ifndef AUTOHDMAP_DATACHECK_JSONDATALOADER_H
#define AUTOHDMAP_DATACHECK_JSONDATALOADER_H

#include <ResourceManager.h>
#include "IMapProcessor.h"

namespace kd {
    namespace dc {
        class JsonDataLoader : public IMapProcessor {
        public:
            explicit JsonDataLoader();

            ~JsonDataLoader();

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            bool LoadJsonData(shared_ptr<ResourceManager> resource_manager);
            void GetFileTaskId(const string& file_path, int file_name_mode, string& task_id,
                               string& task_bound_id , string& data_key);
        private:
            const string id_ = "json_data_loader";
            string json_data_path_;
            string model_file_;
            string config_path_;
            shared_ptr<MapDataManager> map_data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_JSONDATALOADER_H
