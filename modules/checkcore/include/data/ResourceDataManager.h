
#ifndef AUTOHDMAP_DATACHECK_RESOURCEDATAMANAGER_H
#define AUTOHDMAP_DATACHECK_RESOURCEDATAMANAGER_H


#include "CommonInclude.h"
#include <ResourceManager.h>
#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        class ResourceDataManager : public IMapProcessor  {
        public:
            static shared_ptr<ResourceDataManager> GetInstance() {
                if (instance_ == nullptr) {
                    instance_ = shared_ptr<ResourceDataManager>(new ResourceDataManager());
                    //instance_ = make_shared<ResourceDataManager>();
                }
                return instance_;
            }

            string getId() override ;

            bool execute(shared_ptr<MapDataManager> data_manager, shared_ptr<CheckErrorOutput> error_output) override;

            // 加载json数据
            bool LoadData();

            ~ResourceDataManager() = default;

        public:
            shared_ptr<ResourceManager> resource_manager_;

            static shared_ptr<ResourceDataManager> instance_;
        private:
            ResourceDataManager() {
                resource_manager_ = make_shared<ResourceManager>();
            }

        private:
            const string id_ = "load json data";

            void GetFileTaskId(const string& filePath, string& taskId, string& taskBoundId , string& dataKey);
        };



    }
}

#endif //AUTOHDMAP_DATACHECK_RESOURCEDATAMANAGER_H
