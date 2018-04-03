//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MODELBUSSCHECK_H
#define AUTOHDMAP_DATACHECK_MODELBUSSCHECK_H


#include "IModelProcessor.h"

namespace kd {
    namespace dc {

        class ModelBussCheck : public IModelProcessor {
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
            virtual bool execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine, shared_ptr<CheckErrorOutput> errorOutput) override ;


        private:

            void checkFieldIdentify(shared_ptr<DCModalData> modelData, shared_ptr<DCFieldDefine> fieldDef);

            void checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

            void checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

            void checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

        private:
            const string id = "model_buss_check";
            multimap<long,string> multimapid;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_MODELBUSSCHECK_H
