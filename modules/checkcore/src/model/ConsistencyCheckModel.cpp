//
// Created by ubuntu on 2019/8/22.
//

#include <model/ConsistencyCheckModel.h>
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include "DataCheckConfig.h"
#include "parsers/OSMDataParser.hpp"

using namespace kd::api;
namespace kd {
    namespace dc {

        bool ConsistencyCheckModel::LoadModel() {
            LoadLengthCheckModel();
            LoadCountCheckModel();
        }

        bool ConsistencyCheckModel::LoadCountCheckModel() {
            string modelFile = DataCheckConfig::getInstance().getProperty(DataCheckConfig::COUNT_CHECK_FILE);
            Poco::File inFile(modelFile);
            if (!inFile.exists()) {
                LOG(ERROR) << modelFile << " is not Exists!";
                return false;
            }
            try {
                std::filebuf inFileBuffer;
                if (!inFileBuffer.open(modelFile, std::ios::in)) {
                    LOG(ERROR) << "Fail to open modelFile  : " << modelFile;
                    return false;
                }

                Poco::JSON::Parser parser;
                std::istream iss(&inFileBuffer);
                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootObj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr)) {
                    rootObj = result.extract<Poco::JSON::Object::Ptr>();
                }

                //获得基本信息
                string taskName = rootObj->getValue<string>("taskName");
                string dataPath = rootObj->getValue<string>("dataPath");
                //获得任务信息
                if (!rootObj->has("tasks")) {
                    LOG(ERROR) << "No Field tasks Exist";
                    return false;
                }

                Poco::JSON::Array::Ptr taskArray = rootObj->getArray("tasks");
                size_t totalCount = taskArray->size();
                for (size_t i = 0 ; i < totalCount ; i++) {
                    Poco::Dynamic::Var value = taskArray->get(i);
                    Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                    string fileName = nodeObj->getValue<string>("fileName");
                    string fileType = nodeObj->getValue<string>("fileType");
                    string modelName = nodeObj->getValue<string>("modelName");

                    shared_ptr<DataCheckModel> model = make_shared<DataCheckModel>();
                    model->fileName = dataPath + fileName;
                    model->fileType = fileType;
                    model->modelName = modelName;

                    count_check_model_.push_back(model);
                }
            }catch (Exception &e) {
                LOG(ERROR) << e.what();
                return false;
            }
            return true;
        }

        bool ConsistencyCheckModel::LoadLengthCheckModel() {
            string modelFile = DataCheckConfig::getInstance().getProperty(DataCheckConfig::LENGTH_CHECK_FILE);
            Poco::File inFile(modelFile);
            if (!inFile.exists()) {
                LOG(ERROR) << modelFile << " is not Exists!";
                return false;
            }

            try {
                std::filebuf inFileBuffer;
                if (!inFileBuffer.open(modelFile, std::ios::in)) {
                    LOG(ERROR) << "Fail to open modelFile  : " << modelFile;
                    return false;
                }

                Poco::JSON::Parser parser;
                std::istream iss(&inFileBuffer);
                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootObj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr)) {
                    rootObj = result.extract<Poco::JSON::Object::Ptr>();
                }

                //获得基本信息
                string taskName = rootObj->getValue<string>("taskName");
                string dataPath = rootObj->getValue<string>("dataPath");
                //获得任务信息
                if (!rootObj->has("tasks")) {
                    LOG(ERROR) << "No Field tasks Exist";
                    return false;
                }

                Poco::JSON::Array::Ptr taskArray = rootObj->getArray("tasks");
                size_t totalCount = taskArray->size();
                for (size_t i = 0 ; i < totalCount ; i++) {
                    Poco::Dynamic::Var value = taskArray->get(i);
                    Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                    string fileName = nodeObj->getValue<string>("fileName");
                    string fileType = nodeObj->getValue<string>("fileType");
                    string modelName = nodeObj->getValue<string>("modelName");

                    shared_ptr<DataCheckModel> model = make_shared<DataCheckModel>();
                    model->fileName = dataPath + fileName;
                    model->fileType = fileType;
                    model->shpLengthKey = nodeObj->getValue<string>("shpLength");
                    model->modelName = modelName;
                    model->modelLengthKey = nodeObj->getValue<string>("modelLength");
                    length_check_model_.push_back(model);
                }
            }catch (Exception &e) {
                LOG(ERROR) << e.what();
                return false;
            }
            return true;
        }

    }
}