//
// Created by gaoyanhong on 2018/3/29.
//

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        string DCDividerCheckError::toString() {
            stringstream ss;
            ss << "DCDividerCheckError: " << dividerId_ << "," << nodeId_ << endl;
            return  ss.str();
        }

    }
}

