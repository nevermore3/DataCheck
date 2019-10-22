#ifndef AUTOHDMAP_DATACHECK_JSONDATALOADER_H
#define AUTOHDMAP_DATACHECK_JSONDATALOADER_H
#include <api/KDSService.h>
#include <api/TaskBoundService.h>
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
            /**
             * 是否接边检查
             * @param splitCheck
             */
            void setSplitCheck(bool splitCheck);
            /**
             * 主框ID
             * @param mainFrameId
             */
            void setMainFrameId(string mainFrameId);
            /**
             * 主任务ID
             * @param mainTaskId
             */
            void setMainTaskId(string mainTaskId);
            /**
             * 设置框信息
             * @param task_bounds
             */
            void setTaskBound(map<string, shared_ptr<kd::api::TaskBound>> & task_bounds);
        private:
            /**
             * 是否接边
             * @param splitCheck true 接边，false 不是接边
             * @param resource_manager
             * @return
             */
            bool LoadJsonForConn(const map<string, shared_ptr<kd::api::TaskBound>> & task_bounds,shared_ptr<ResourceManager> resource_manager);
            bool LoadJsonData(shared_ptr<ResourceManager> resource_manager);
            /**
             * 从文件名称解析taskid,bound_id,data_key,兼容两种文件规范，如：ground_203187756_1.json和manual_990000_new_686574-lane_203187756_1.json
             * @param file_path 文件路径，包括文件名称
             * @param task_id 任务号
             * @param task_bound_id 框ID
             * @param data_key 数据ID
             */
            void GetFileTaskId(const string& file_path, string& task_id,
                               string& task_bound_id , string& data_key);

        private:
            const string id_ = "json_data_loader";
            string json_data_path_;
            string model_file_;
            string config_path_;
            shared_ptr<MapDataManager> map_data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;
            bool splitCheck_ = false;
            string mainFrameId_;
            string mainTaskId_;
            map<string, shared_ptr<kd::api::TaskBound>> task_bounds_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_JSONDATALOADER_H
