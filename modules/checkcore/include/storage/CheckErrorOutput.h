//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
#define AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H

#include <DataCheckConfig.h>
#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {
        struct ErrorOutPut{
            ErrorOutPut() {
                update_region_id = DataCheckConfig::getInstance().getProperty(DataCheckConfig::UPDATE_REGION);
            }
            string update_region_id;
            string checkModel_;// 检测模型
            string checkDesc_;//检查模型描述信息
            string detail;//错误详细信息
            string level;//错误等级，warn error
        };

        class CheckErrorOutput{

        public:
            explicit CheckErrorOutput(CppSQLite3::Database *pdb);

            ~CheckErrorOutput() = default;

            /**
             * 输出自定义文本信息
             * @param info 文本信息
             */
            void writeInfo(string info, bool bLongString = true);

            /**
             * 输出检查信息到sql中
             */
            int saveError();

            void saveError(shared_ptr<DCError> error);

            int countError();

            void saveTotalError();

            /**
             * 获取错误级别
             * @param check_model
             * @return
             */
            string get_error_level(string check_model);

        protected:
            CppSQLite3::Database *m_pdb;
            map<string, vector<ErrorOutPut>> check_model_2_output_maps_;
            set<string> error_check_levels_;
        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
