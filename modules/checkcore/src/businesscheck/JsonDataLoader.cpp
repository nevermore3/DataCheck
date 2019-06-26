//
// Created by zhangxingang on 19-6-13.
//

#include <businesscheck/JsonDataLoader.h>
#include <util/FileUtil.h>
#include <storage/JsonDataInput.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
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
                json_data_input->CheckModelRelation();
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
                string taskid = "";
                string bound_id = "";
                string data_key = "";

                OSMDataParser parser(resource_manager);


                string inputJsonData;
                const string &file_path = file_list[i];
                if (file_path.find(config_file) != std::string::npos) {
                    continue;
                }

                //load file
                LOG(INFO) << "load file " << (i + 1) << "/" << file_count << " " << file_path;

                GetFileTaskId(file_path,2,taskid,bound_id,data_key);
                DataCheckConfig::getInstance().addProperty(taskid,bound_id);

                if(!FileUtil::LoadFile(file_path, inputJsonData)){
                    LOG(ERROR) << "inputJsonData is empty";
                    return false;
                }

                if (inputJsonData.length() < 10) {
                    continue;
                }


                int parse_ret = parser.ParseKdsOSMJSONDataFilterByGason(
                        inputJsonData, taskid, filterNodes, filterWays, filterRels);

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

        //通过文件名获取任务id和框id
        void JsonDataLoader::GetFileTaskId(const string& file_path, int file_name_mode, string& task_id,
                           string& task_bound_id , string& data_key) {
            if (1 != file_name_mode && 2 != file_name_mode) {
                LOG(ERROR) << "invalid fileNameMode value : " << file_name_mode;
                return;
            }

            string file_path_noext = file_path.substr(0, file_path.find(".json"));
            string file_name = file_path_noext.substr(file_path_noext.find_last_of("/")+1);

            if (1 == file_name_mode) {
                // taskFrameId_taskId.json
                int pos = file_name.find("_");
                if (file_name.find("_") != string::npos) {
                    task_id = file_name.substr(file_name.find_last_of("_") + 1);
                    task_bound_id = file_name.substr(0, file_name.find_last_of("_"));
                } else {
                    task_id = file_name;
                    task_bound_id = "";
                }
            } else {
                // taskFrameId-objectType_taskId_seqNumber.json
                int pos = file_name.find("-");
                if (pos != string::npos) {
                    task_bound_id = file_name.substr(0, pos);
                    data_key = file_name.substr(pos + 1,file_name.length());
                    std::string sub_str = file_name.substr(pos + 1);

                    int f_pos = sub_str.find("_");
                    int l_pos = sub_str.find_last_of("_");
                    if (f_pos != string::npos && l_pos != string::npos) {
                        task_id = sub_str.substr(f_pos + 1, l_pos - f_pos - 1);
                    }
                } else {
                    task_id = file_name;
                    task_bound_id = "";
                }
            }
        }


    }
}