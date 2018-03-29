//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_DCFIELDCHECKUTIL_H
#define AUTOHDMAP_DATACHECK_DCFIELDCHECKUTIL_H

#include "data/MapDataModel.h"

namespace kd {
    namespace dc {

        class DCFieldCheckUtil{
        public:

            static void checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

            static void checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

            static void checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName);

            static void checkDoubleValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName);

            static void checkLongValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName);

        };

    }
}

#endif //AUTOHDMAP_DATACHECK_DCFIELDCHECKUTIL_H
