//
// Created by zhangxingang on 19-6-13.
//

#include <businesscheck/JsonDataLoader.h>
#include <util/FileUtil.h>
#include <storage/JsonDataInput.h>
#include <storage/CheckTaskInput.h>
#include "parsers/OSMDataParser.hpp"


using namespace kd::api;

namespace kd {
    namespace dc {
        JsonDataLoader::JsonDataLoader(){
            json_data_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::JSON_DATA_INPUT);
            model_file_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::RESOURCE_FILE);
            config_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::CONFIG_FILE_PATH);
        }

        JsonDataLoader::~JsonDataLoader() {

        }

        string JsonDataLoader::getId() {
            return id_;
        }

        bool JsonDataLoader::execute(shared_ptr<MapDataManager> mapDataManager,
                                     shared_ptr<CheckErrorOutput> errorOutput) {
            bool ret = true;
            map_data_manager_ = mapDataManager;
            error_output_ = errorOutput;
            string model_path = config_path_ + model_file_;
            shared_ptr<ResourceManager> resource_manager = make_shared<ResourceManager>();
            if (LoadJsonData(resource_manager)) {
                shared_ptr<JsonDataInput> json_data_input = make_shared<JsonDataInput>(map_data_manager_, error_output_,
                                                                                       "", model_path, resource_manager);
                json_data_input->LoadModel();
                json_data_input->LoadData();
                json_data_input->CheckModelField(error_output_);
            }
            return ret;
        }

        bool JsonDataLoader::LoadJsonData(shared_ptr<ResourceManager> resource_manager) {
            //加载源数据
            vector<string> file_list;
            std::string suffix = ".json";
            FileUtil::getFileNames(json_data_path_, file_list, suffix);

            if (file_list.empty()) {
                LOG(ERROR) << "inputFileName is empty";
                return false;
            }

            set<long> filterNodes, filterWays, filterRels;
            bool load_file_status = true;
            int file_count = file_list.size();
            const std::string config_file = "config.json";
            for (int i = 0; i < file_count; i++) {

                OSMDataParser parser(resource_manager);


                string inputJsonData;
                const string &file_path = file_list[i];
                if (file_path.find(config_file) != std::string::npos) {
                    continue;
                }

                //load file
                LOG(INFO) << "load file " << (i + 1) << "/" << file_count << " " << file_path;
                if(!FileUtil::LoadFile(file_path, inputJsonData)){
                    LOG(ERROR) << "inputJsonData is empty";
                    return false;
                }

                if (inputJsonData.length() < 10) {
                    continue;
                }

                int parse_ret = parser.ParseKdsOSMJSONDataFilterByGason(
                        inputJsonData, "", filterNodes, filterWays, filterRels);

                if(parse_ret != 0){
                    LOG(ERROR) << "Parse data error, retvalue is " << parse_ret;
                    load_file_status = false;
                    break;
                }
            }

            filterNodes.clear();
            filterWays.clear();
            filterRels.clear();

            //数据过滤
            resource_manager->filterKdsData();

            return load_file_status;
        }


    }
}