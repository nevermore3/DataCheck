//
// Created by ubuntu on 2019/8/27.
//
#include "datacheck/TableDescCheck.h"
#include <util/FileUtil.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"
#include "IMapProcessor.h"
#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
using namespace kd::api;

namespace kd {
    namespace dc {
        TableDescCheck::TableDescCheck() {
            base_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
        }
        string TableDescCheck::getId() {
            return id_;
        }

        void TableDescCheck::CheckModelName(const shared_ptr<ModelDataManager> &modelDataManager,
                                            shared_ptr<CheckErrorOutput> errorOutput) {
            map<string, shared_ptr<DCTask>> tasks = modelDataManager->tasks_;
            for (const auto &model : modelDataManager->modelDatas_) {
                if (tasks.find(model.first) == tasks.end()) {

                    stringstream ss;
                    ss << " 要素" << model.first << " 在规格中不存在";
                    shared_ptr<DCError> pError = DCTableDescError::createByKXS_10_001(ss.str());
                    errorOutput->saveError(pError);
                }
            }

        }


        void TableDescCheck::CheckGeometricType(const shared_ptr<ModelDataManager> &modelDataManager,
                                                shared_ptr<CheckErrorOutput> errorOutput) {
            map<string, int> typeMap {{"point", 11}, {"arc", 13}, {"ploygon", 15}};

            for (const auto &task : modelDataManager->tasks_) {
                string fileType = task.second->fileType;
                string fileName = task.second->fileName;
                if (fileType != "dbf") {

                    string shpFile = base_path_ + "/" + fileName + ".shp";
                    SHPHandle pSHP = SHPOpen(shpFile.c_str(), "rb");
                    if (pSHP != nullptr) {
                        if (typeMap[fileType] != pSHP->nShapeType) {
                            stringstream ss;
                            ss << " 要素 " << fileName << " 的几何类型与规格中的不一致";
                            shared_ptr<DCError> pError = DCTableDescError::createByKXS_10_001(ss.str());
                            errorOutput->saveError(pError);
                        }
                    } else {
                        continue;
                    }
                }
            }

        }

        void TableDescCheck::CheckRelationType(const shared_ptr<ModelDataManager> &modelDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            for (const auto &task : modelDataManager->tasks_) {
                string fileName = task.second->fileName;
                string fileType = task.second->fileType;
                if (fileType == "dbf") {
                    //只有dbf文件存在，没有shp文件存在为关系类型
                    string shpFile = base_path_ + "/" + fileName + ".shp";
                    string dbfFile = base_path_ + "/" + fileName + ".dbf";
                    if (access(dbfFile.c_str(), F_OK) == 0 && access(shpFile.c_str(), F_OK) != 0) {
                        continue;
                    }
                    stringstream ss;
                    ss << " 要素 " << fileName << " 的关系类型与规格中的不一致";
                    shared_ptr<DCError> pError = DCTableDescError::createByKXS_10_001(ss.str());
                    errorOutput->saveError(pError);
                }
            }
        }

        bool TableDescCheck::execute(shared_ptr<ModelDataManager> modelDataManager,
                                     shared_ptr<CheckErrorOutput> errorOutput) {
            //检查表名是否一致
            CheckModelName(modelDataManager, errorOutput);

            //检查几何类型是否一致
            CheckGeometricType(modelDataManager, errorOutput);

            //检查关系类型是否一致
            CheckRelationType(modelDataManager, errorOutput);
            return true;
        }

    }
}
