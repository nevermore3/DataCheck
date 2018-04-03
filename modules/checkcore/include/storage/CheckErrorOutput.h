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

        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
