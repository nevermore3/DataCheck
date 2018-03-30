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

            void saveError(shared_ptr<DCDividerCheckError> error);

        };
    }
}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
