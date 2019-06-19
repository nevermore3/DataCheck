//
// Created by zhangxingang on 19-6-18.
//

#ifndef AUTOHDMAP_DATACHECK_MODELINPUT_H
#define AUTOHDMAP_DATACHECK_MODELINPUT_H
#include "data/DataManager.h"
#include "CheckErrorOutput.h"

namespace kd {
    namespace dc {
        class ModelInput {
        public:
            ModelInput(const string &model_path_);

            virtual bool LoadModel();

            virtual bool CheckModelField(const shared_ptr<CheckErrorOutput> &error_output);

        protected:
            shared_ptr<DCModelDefine> GetModelDefine(string model_name);
            shared_ptr<DCModalData> GetModelData(string model_name);

        private:
            void checkDoubleValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                    const shared_ptr<DCModalData> &modelData,
                                    const string &fieldName, const shared_ptr<CheckErrorOutput> &error_output);

            void checkLongValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                  const shared_ptr<DCModalData> &modelData,
                                  const string &fieldName, const shared_ptr<CheckErrorOutput> &error_output);

            void checkStringValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                    const shared_ptr<DCModalData> &modelData,
                                    const string &fieldName, const shared_ptr<CheckErrorOutput> &error_output);

        protected:
            map<string, shared_ptr<DCModelDefine>> model_name2_define_maps_;
            shared_ptr<ModelDataManager> model_data_manager_;
            string model_path_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_MODELINPUT_H
