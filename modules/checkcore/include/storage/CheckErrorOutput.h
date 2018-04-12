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
            CheckErrorOutput(string file){
                if (ss_.is_open()){
                    ss_.close();
                }
                setlocale(LC_ALL,"");
                if (file.length() > 0){
                    ss_.open(file.c_str(), ios::ate);
                }
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
            void writeInfo(string info, bool bLongString = true);

        protected:
            ofstream ss_;
        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
