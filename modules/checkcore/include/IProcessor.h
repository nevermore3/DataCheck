#ifndef AUTOHDMAP_ROADFUSION_IPROCESSOR_H
#define AUTOHDMAP_ROADFUSION_IPROCESSOR_H

#include "CommonInclude.h"
#include "storage/CheckErrorOutput.h"

namespace kd {
   namespace dc {

        class IProcessor{
        public:
            virtual ~IProcessor() = default;

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() = 0;

            /**
             * 进行任务处理
             * @param dataManager 操作数据
             * @param listener 状态监听器
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<CheckErrorOutput> errorOutput) = 0;

        };

    }
}

#endif //AUTOHDMAP_ROADFUSION_IPROCESSOR_H
