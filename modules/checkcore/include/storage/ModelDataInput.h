//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_MODELDATAINPUT_H
#define AUTOHDMAP_DATACHECK_MODELDATAINPUT_H

#include "data/MapDataModel.h"


namespace kd {
    namespace dc {

        class ModelDataInput{

        public:

            bool loadPointFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData);

            bool loadArcFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData);

            bool loadDBFFile(const string & fileName, const vector<shared_ptr<DCFieldDefine>> & vecFieldDefines, shared_ptr<DCModalData> modelData);
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_MODELDATAINPUT_H
