//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_MODELDATAINPUT_H
#define AUTOHDMAP_DATACHECK_MODELDATAINPUT_H

#include "data/MapDataModel.h"

#include "storage/CheckErrorOutput.h"

namespace kd {
    namespace dc {

        class ModelDataInput{

        public:

            bool loadPointFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput);

            bool loadArcFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput);

            bool loadDBFFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput);
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_MODELDATAINPUT_H
