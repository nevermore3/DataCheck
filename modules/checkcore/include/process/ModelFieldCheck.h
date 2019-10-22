

#ifndef AUTOHDMAP_DATACHECK_MODELATTCHECK_H
#define AUTOHDMAP_DATACHECK_MODELATTCHECK_H

#include "IModelProcessor.h"

namespace kd {
    namespace dc {

        class ModelFieldCheck : public IModelProcessor {
        public:


        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override ;

            /**
             * 进行任务处理
             * @param modelData 模型数据
             * @param modelDefine 模型定义
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) override ;


        private:
            void checkDoubleValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef, const shared_ptr<DCModalData> &modelData,
                                    const string &fieldName, const shared_ptr<CheckErrorOutput> &errorOutput);

            void checkLongValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef, const shared_ptr<DCModalData> &modelData,
                                  const string &fieldName, const shared_ptr<CheckErrorOutput> &errorOutput);

            void checkStringValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef, const shared_ptr<DCModalData> &modelData,
                                    const string &fieldName, const shared_ptr<CheckErrorOutput> &errorOutput);


        private:
            const string id = "model_field_check";

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_MODELATTCHECK_H
