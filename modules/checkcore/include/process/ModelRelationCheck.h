//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MODELRELATIONCHECK_H
#define AUTOHDMAP_DATACHECK_MODELRELATIONCHECK_H
#include "IModelProcessor.h"

#include "data/DataManager.h"
#include "storage/CheckErrorOutput.h"

namespace kd {
    namespace dc {

        class ModelRelationCheck : public IModelProcessor {
        public:


        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId();

            /**
             * 进行任务处理
             * @param modelData 模型数据
             * @param modelDefine 模型定义
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) override ;

        private:
            const string id = "model_relation_check";

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_MODELRELATIONCHECK_H
