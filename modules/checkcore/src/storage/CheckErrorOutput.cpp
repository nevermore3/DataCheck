//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/CheckErrorOutput.h"


namespace kd {
    namespace dc {

        void CheckErrorOutput::saveError(shared_ptr<DCDividerCheckError> error) {

            cout << error->toString() << endl;
        }
    }
}

