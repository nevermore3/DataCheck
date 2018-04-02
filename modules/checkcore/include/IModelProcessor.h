//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_IMODELPROCESSOR_H
#define AUTOHDMAP_DATACHECK_IMODELPROCESSOR_H

#include "CommonInclude.h"
#include "data/MapDataModel.h"
#include "storage/CheckErrorOutput.h"

namespace kd {
    namespace dc {

        class IModelProcessor{
        public:
            virtual ~IModelProcessor() = default;

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() = 0;

            /**
             * 进行任务处理
             * @param ModelDataManager 模型数据
             * @param modelDefine 模型定义
             * @param errorOutput 数据异常输出
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine, shared_ptr<CheckErrorOutput> errorOutput) = 0;

        };

    }
}

#endif //AUTOHDMAP_DATACHECK_IMODELPROCESSOR_H
