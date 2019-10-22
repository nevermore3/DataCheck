
#ifndef AUTOHDMAP_DATACHECK_CONSISTENCYCHECKMODEL_H
#define AUTOHDMAP_DATACHECK_CONSISTENCYCHECKMODEL_H

#include "CommonInclude.h"
#include <ResourceManager.h>
namespace kd {
    namespace dc {

        class DataCheckModel{
        public:
            //数据路径
            string fileName;

            //数据类型
            string fileType;

            //
            string shpLengthKey;

            //模型配置文件
            string modelName;

            //
            string modelLengthKey;
        };


        //一致性检查 载入配置模型
        class ConsistencyCheckModel {
        public:
            static ConsistencyCheckModel *GetInstance() {
                static ConsistencyCheckModel instance_;
                return &instance_;
            }

            bool LoadModel();

            // 载入长度一致性检查模型
            bool LoadLengthCheckModel();
            // 载入数量一致性检查模型
            bool LoadCountCheckModel();
            // 载入形点一致性检查模型
            bool LoadShapeCheckModel();

            vector<shared_ptr<DataCheckModel>> length_check_model_;
            vector<shared_ptr<DataCheckModel>> count_check_model_;
            vector<shared_ptr<DataCheckModel>> shape_check_model_;

        protected:
            ConsistencyCheckModel() {
            }

        };

    }
}

#endif //AUTOHDMAP_DATACHECK_CONSISTENCYCHECKMODEL_H
