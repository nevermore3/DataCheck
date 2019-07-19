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

                GetFileTaskId(file_path,taskid,bound_id,data_key);
                DataCheckConfig::getInstance().addProperty(taskid,bound_id);

                if(!FileUtil::LoadFile(file_path, inputJsonData)){
                    LOG(ERROR) << "inputJsonData is empty";
                    return false;
                }

                if (inputJsonData.length() < 10) {
                    continue;
                }

                DataCheckConfig::getInstance().addProperty(data_key,inputJsonData);
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
        void JsonDataLoader::GetFileTaskId(const string& file_path, string& task_id,
                           string& task_bound_id , string& data_key) {

            string file_path_noext = file_path.substr(0, file_path.find(".json"));
            string file_name = file_path_noext.substr(file_path_noext.find_last_of("/")+1);
            if(file_name.find("-") != string::npos){
                task_bound_id = file_name.substr(0,file_name.find("-"));
                file_name = file_name.substr(file_name.find("-")+1);
            }
            vector<string> v_substr;
            StringUtil::Token(file_name.c_str(),"_",v_substr);
            int size  = v_substr.size();
            if(size >= 3){
                task_id = v_substr[size-2];
            }else{
                LOG(ERROR) <<"input data error ,file_path is "<<file_path;
            }
            if(size == 3){
                data_key = file_name;
            }else if(size>3){
                data_key = v_substr[size-3] + "_" + v_substr[size-2]+"_"+v_substr[size-1];
            }
        }


    }
}