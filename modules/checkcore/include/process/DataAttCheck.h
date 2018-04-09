//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_DATAATTCHECK_H
#define AUTOHDMAP_DATACHECK_DATAATTCHECK_H

#include <data/MapDataModel.h>
#include "storage/CheckErrorOutput.h"
#include "IProcessor.h"

namespace kd {
    namespace dc {

        class DataAttCheck : public IProcessor {
        public:
            DataAttCheck(string basePath, string taskName);

            virtual ~DataAttCheck() = default;

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override;

            /**
             * 进行任务处理
             * @param dataManager 操作数据
             * @param listener 状态监听器
             * @return 操作是否成功
             */
            virtual bool execute() override;

            /**
             * 设置错误输出
             * @param errorOutput
             */
            void setCheckErrorOutput(const shared_ptr<CheckErrorOutput> errorOutput){
                errorOutput_ = errorOutput;
            };

        private:
            const string id = "data_att_check_process";

            //基础数据路径
            string basePath_;

            //任务名称
            string taskName_;

            //错误输出
            shared_ptr<CheckErrorOutput> errorOutput_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_DATAATTCHECK_H
