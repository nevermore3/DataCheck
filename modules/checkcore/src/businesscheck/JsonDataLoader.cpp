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
            if(splitCheck_){
                ret = LoadJsonForConn(task_bounds_,resource_manager);
            }else{
                ret = LoadJsonData(resource_manager);
            }
            if (ret) {
                shared_ptr<JsonDataInput> json_data_input = make_shared<JsonDataInput>(map_data_manager_, error_output_,
                                                                                      "", model_path, resource_manager);
                json_data_input->LoadModel();
                json_data_input->LoadData();
                json_data_input->CheckModelField(error_output_);
                json_data_input->CheckModelRelation();
            }
            return ret;
        }

        void JsonDataLoader::setSplitCheck(bool splitCheck){
            splitCheck_ = splitCheck;
        }
        void JsonDataLoader::setMainFrameId(string mainFrameId){
            mainFrameId_ = mainFrameId;
        }
        void JsonDataLoader::setMainTaskId(string mainTaskId){
            mainTaskId_ = mainTaskId;
        }
        void JsonDataLoader::setTaskBound(map<string, shared_ptr<kd::api::TaskBound>> & task_bounds){
            task_bounds_ = task_bounds;
        }
        bool JsonDataLoader::LoadJsonForConn(const map<string, shared_ptr<kd::api::TaskBound>> & task_bounds,shared_ptr<ResourceManager> resource_manager){
            vector<string> file_list;
            std::string suffix = ".json";

            auto& config_ins = DataCheckConfig::getInstance();
//            string input_path = json_data_path_;
            FileUtil::getFileNames(json_data_path_, file_list, suffix);

            set<long> filterNodes, filterWays, filterRels;
            if (file_list.empty()) {
                LOG(ERROR) << "inputFileName is empty";
                return false;
            }

            bool load_file_status = true;
            size_t file_count = file_list.size();
            const std::string config_file = "config.json";
            const std::string checklist = "checklist.json";
            const std::string split_str = "-";
            for (int i = 0; i < file_count; i++) {

                OSMDataParser parser(resource_manager);
                parser.isFilter_ = false;

                string inputJsonData;
                const string &file_path = file_list[i];
                if (file_path.find(config_file) != std::string::npos) {
                    continue;
                } else if (file_path.find(checklist) != std::string::npos) {
                    continue;
                } else if (file_path.find(split_str) == std::string::npos){
                    continue;
                }

                //load file
                LOG(INFO) << "load file " << (i + 1) << "/" << file_count << " " << file_path;
                if(!FileUtil::LoadFile(file_path, inputJsonData)){
                    LOG(ERROR) << "inputJsonData is empty";
                    return false;
                }

                    //optimize data load
                    string task_id, task_bound_id,data_key;
                    GetFileTaskId(file_path, task_id, task_bound_id,data_key);
                    if(mainFrameId_ == task_bound_id){
                        task_id = mainTaskId_;
                    }
                    parser.taskId_ = task_id;
                    int parse_ret = 0;
                    if (task_bound_id.length() == 0) {
                        parse_ret = parser.ParseKdsOSMJSONDataFilterByGason(
                                inputJsonData, "", filterNodes, filterWays, filterRels);
                    } else {
                        //先加载到临时资源对象中，然后进行过滤，最后将过滤后的数据拷贝到最终的数据集合中
                        auto one_resource_manager = make_shared<ResourceManager>();
                        OSMDataParser oneparser(one_resource_manager);
                        //
                        auto taskboundit = task_bounds.find(task_bound_id);
                        if (taskboundit == task_bounds.end()) {
                            LOG(ERROR) << "find task_bound " << task_bound_id
                                       << " info error.";
                            load_file_status = false;
                            break;
                        }

//                TaskInfoManager::GetInstance().AppendItem(task_id, task_bound_id);
                        DataCheckConfig::getInstance().addProperty(task_id+"_bound_id",task_bound_id);
                        shared_ptr<kd::api::TaskBound> taskBound = taskboundit->second;
                        set<long> filterNodesTemp, filterWaysTemp, filterRelsTemp;
                        multimap<long, set<long>> error_div_ids;
                        parse_ret = oneparser.ParseKdsOSMJSONDataFilterByGason(
                                inputJsonData, task_id, taskBound->coords_,
                                filterNodesTemp, filterWaysTemp, filterRelsTemp,
                                error_div_ids);

                        if (!error_div_ids.empty()) {
                            LOG(ERROR) << "error_div_ids is empty.";
                        }
                        if (parse_ret == 0) {
                            one_resource_manager->CopyAndProcessSameId(resource_manager);
                        }
                    }

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
        bool JsonDataLoader::LoadJsonData(shared_ptr<ResourceManager> resource_manager) {
            //加载源数据
            vector<string> file_list;
            std::string suffix = ".json";
            FileUtil::getFileNames(json_data_path_, file_list, suffix);

            if (file_list.empty()) {
                LOG(ERROR) << json_data_path_<<" inputFileName is empty";
                return false;
            }

            set<long> filterNodes, filterWays, filterRels;
            bool load_file_status = true;
            int file_count = file_list.size();
            const std::string config_file = "config.json";
            const std::string split_str = "-";
            bool parse = DataCheckConfig::getInstance().getPropertyBool(DataCheckConfig::CHECK_ALL_FILE);
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
                //全库检查过滤
                if(!parse && file_path.find(split_str) != std::string::npos){
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