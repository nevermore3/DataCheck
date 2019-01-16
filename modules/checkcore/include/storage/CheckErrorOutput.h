//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
#define AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {
        struct ErrorOutPut{
            string checkModel_;// 检测模型
            string checkDesc_;//检查模型描述信息
            string detail;//错误详细信息
        };

        class CheckErrorOutput{

        public:
            CheckErrorOutput(CppSQLite3::Database *pdb){
                m_pdb = pdb;
            }

            ~CheckErrorOutput(){
            }
//            /**
//             * 保存车道线检查相关的业务检查错误
//             * @param error 错误信息
//             */
//            void saveError(shared_ptr<DCDividerCheckError> error);
//
//            /**
//             * 保存各字段属性值检查错误信息
//             * @param error 错误信息
//             */
//            void saveError(shared_ptr<DCAttCheckError> error);
//
//
//            /**
//             * 保存车道检查相关的业务检查错误
//             * @param error 错误信息
//             */
//            void saveError(shared_ptr<DCLaneCheckError> error);


            /**
             * 输出自定义文本信息
             * @param info 文本信息
             */
            void writeInfo(string info, bool bLongString = true);

            /**
             * 输出检查信息到sql中
             */
            void saveError();

            void saveError(shared_ptr<DCError> error);

        protected:
            CppSQLite3::Database *m_pdb;
            map<string, vector<ErrorOutPut>> check_model_2_output_maps_;
        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
