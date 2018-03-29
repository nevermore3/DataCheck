//
// Created by gaoyanhong on 2018/3/29.
//

#include "process/ModelDataLoader.h"


#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"

namespace kd {
    namespace dc {

        ModelDataLoader::ModelDataLoader(string modelFile, string dataFile, string fileType) {
            modelFile_ = modelFile;
            dataFile_ = dataFile;
            fileType_ = fileType;
        }

        string ModelDataLoader::getId() {
            return id;
        }

        bool ModelDataLoader::execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine) {

            CheckTaskInput taskInput;
            if (taskInput.loadTaskModel(modelFile_, modelDefine)) {

                ModelDataInput dataInput;
                if (fileType_ == "point") {
                    return dataInput.loadPointFile(dataFile_, modelDefine->vecFieldDefines, modelData);
                } else if (fileType_ == "arc") {
                    return dataInput.loadArcFile(dataFile_, modelDefine->vecFieldDefines, modelData);
                } else if (fileType_ == "dbf") {
                    return dataInput.loadDBFFile(dataFile_, modelDefine->vecFieldDefines, modelData);
                } else {
                    cout << "[Error] data file type error " << fileType_ << endl;
                    return false;
                }

            }

            return false;
        }

    }
}

