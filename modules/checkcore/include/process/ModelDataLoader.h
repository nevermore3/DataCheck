//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MODELDATALOADER_H
#define AUTOHDMAP_DATACHECK_MODELDATALOADER_H

#include "IModelProcessor.h"

namespace kd {
    namespace dc {

        class ModelDataLoader : public IModelProcessor {
        public:
            ModelDataLoader(string modelFile, string dataFile, string fileType);

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
            virtual bool execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine) override ;

        private:
            const string id = "model_data_loader";

            string modelFile_;

            string dataFile_;

            string fileType_;

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_MODELDATALOADER_H
