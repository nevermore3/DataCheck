//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H
#define AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H

#include "CommonInclude.h"
#include "data/DataManager.h"
#include "storage/CheckErrorOutput.h"
#include "util/check_list_config.h"
#include <ResourceManager.h>

namespace kd {
    namespace dc {

        class IMapProcessor{
        public:
            virtual ~IMapProcessor() = default;

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() = 0;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<MapDataManager> data_manager,
                                 shared_ptr<CheckErrorOutput> error_output) = 0;

            void set_data_manager(shared_ptr<MapDataManager> data_manager) {
                data_manager_ = data_manager;
            }
            shared_ptr<MapDataManager> data_manager() {
                return data_manager_;
            }

            void set_error_output(shared_ptr<CheckErrorOutput> error_output) {
                error_output_ = error_output;
            }
            shared_ptr<CheckErrorOutput> error_output() {
                return error_output_;
            }

            bool CheckItemValid(const std::string& check_item) {
                return CheckListConfig::getInstance().IsNeedCheck(check_item);
            }
        public:
            static shared_ptr<ResourceManager> resource_manager_;
            static string kxf_base_path_;
        private:
            shared_ptr<MapDataManager> data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H
