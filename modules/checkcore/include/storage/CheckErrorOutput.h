//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
#define AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        class CheckErrorOutput{

        public:
            CheckErrorOutput() : outputFile_(false) {

            }

            ~CheckErrorOutput(){
                if (ss_.is_open()){
                    ss_.close();
                }
            }
            /**
             * 保存车道线检查相关的业务检查错误
             * @param error 错误信息
             */
            void saveError(shared_ptr<DCDividerCheckError> error);

            /**
             * 保存各字段属性值检查错误信息
             * @param error 错误信息
             */
            void saveError(shared_ptr<DCAttCheckError> error);


            /**
             * 保存车道检查相关的业务检查错误
             * @param error 错误信息
             */
            void saveError(shared_ptr<DCLaneCheckError> error);


            /**
             * 输出自定义文本信息
             * @param info 文本信息
             */
            void writeInfo(string info);


            /**
             * 设置输出结果文件
             * @param strFile
             */
            void setFile(string strFile){
                if (ss_.is_open()){
                    ss_.close();
                }
                ss_.open(strFile.c_str(), ios::app);
            }


            /**
             * 设置是否输出到文件还是控制台
             * @param bOutputFile
             */
            void setOutputFile(bool bOutputFile = false){
                outputFile_ = bOutputFile;
            }

        protected:
            bool outputFile_;
            ofstream ss_;
        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
