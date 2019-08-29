//
// Created by ubuntu on 2019/8/22.
//

#include "data/ResourceDataManager.h"

#include <businesscheck/JsonDataLoader.h>
#include <util/FileUtil.h>
#include <storage/JsonDataInput.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"

namespace kd {
    namespace dc {
        string ResourceDataManager::getId() {
            return id_;
        }

        bool ResourceDataManager::execute(shared_ptr<MapDataManager> data_manager,
                                          shared_ptr<CheckErrorOutput> error_output) {
            LoadData();
        }

        bool ResourceDataManager::LoadData() {
            //加载源数据
            string jsonDataPath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::JSON_DATA_INPUT);
            vector<string>fileList;
            string suffix = ".json";

            FileUtil::getFileNames(jsonDataPath, fileList, suffix);

            if (fileList.empty()) {
                LOG(ERROR) << jsonDataPath<<" inputFileName is empty";
                return false;
            }

            set<long> filterNodes, filterWays, filterRels;
            bool loadStatus = true;
            size_t fileCount = fileList.size();
            const string configFile = "config.json";

            for (size_t i = 0; i < fileCount; i++) {
                string taskid = "";
                string bound_id = "";
                string data_key = "";

                OSMDataParser parser(resource_manager_);

                string inputJsonData;
                const string &filePath = fileList[i];
                if (filePath.find(configFile) != std::string::npos) {
                    continue;
                }
                LOG(INFO) << "load file " << (i + 1) << "/" << fileCount << " " << filePath;

                GetFileTaskId(filePath, taskid, bound_id, data_key);
                DataCheckConfig::getInstance().addProperty(taskid, bound_id);

                if(!FileUtil::LoadFile(filePath, inputJsonData)){
                    LOG(ERROR) << "inputJsonData is empty";
                    return false;
                }

                if (inputJsonData.length() < 10) {
                    continue;
                }

                DataCheckConfig::getInstance().addProperty(data_key, inputJsonData);
                int ret = parser.ParseKdsOSMJSONDataFilterByGason(inputJsonData, taskid, filterNodes,
                                                                  filterWays, filterRels);

                if (ret != 0) {
                    LOG(ERROR) << "Parse data error, retvalue is " << ret;
                    loadStatus = false;
                    break;
                }
            }

            filterNodes.clear();
            filterWays.clear();
            filterRels.clear();

            //数据过滤
            resource_manager_->filterKdsData();

            return loadStatus;
        }

        void ResourceDataManager::GetFileTaskId(const string &filePath, string &taskId, string &taskBoundId,
                                                string &dataKey) {

            string filePathNoExt = filePath.substr(0, filePath.find(".json"));

            string fileName = filePathNoExt.substr(filePathNoExt.find_last_of("/") + 1);
            if (fileName.find("-") != string::npos) {
                taskBoundId = fileName.substr(0, fileName.find("-"));
                fileName = fileName.substr(fileName.find("-") + 1);
            }
            vector<string> strArray;
            StringUtil::Token(fileName.c_str(), "_",  strArray);
            int size  = strArray.size();
            if (size >= 3) {
                taskId = strArray[size - 2];
            } else {
                LOG(ERROR) <<"input data error ,filePath is "<<filePath;
            }
            if (size == 3) {
                dataKey = fileName;
            } else if (size>3) {
                dataKey = strArray[size - 3] + "_" + strArray[size - 2] + "_" + strArray[size - 1];
            }
        }

    }
}