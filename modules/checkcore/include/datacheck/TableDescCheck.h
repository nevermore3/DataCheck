
#ifndef AUTOHDMAP_DATACHECK_TABLEDESCCHECK_H
#define AUTOHDMAP_DATACHECK_TABLEDESCCHECK_H

#include "IModelProcessor.h"

namespace kd {
    namespace dc {

        class TableDescCheck : public IModelProcessor {
        public:
            TableDescCheck();

            ~TableDescCheck() = default;
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override;

            /**
             * 进行任务处理
             * @param modelData 模型数据
             * @param modelDefine 模型定义
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;


        private:
            void CheckModelName(const shared_ptr<ModelDataManager> &modelDataManager,
                                shared_ptr<CheckErrorOutput> errorOutput);

            void CheckGeometricType(const shared_ptr<ModelDataManager> &modelDataManager,
                                    shared_ptr<CheckErrorOutput> errorOutput);

            void CheckRelationType(const shared_ptr<ModelDataManager> &modelDataManager,
                                   shared_ptr<CheckErrorOutput> errorOutput);

            vector<string >RemoveFileNamePrefix(vector<string> &allFiles);

            const string id_ = "table_desc_check";

            string base_path_;

        };
    }
}


#endif //AUTOHDMAP_DATACHECK_TABLEDESCCHECK_H
