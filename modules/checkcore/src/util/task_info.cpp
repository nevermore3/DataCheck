
#include "util/task_info.h"

#include <math.h>

#include "Poco/JSON/Parser.h"
#include <Poco/StringTokenizer.h>
using namespace Poco;
using namespace Poco::JSON;

const char ktaskId[] = "taskId";
const char kInput[] = "input";
const char kOutput[] = "output";
const char krootPath[] = "rootPath";
const char klogsPath[] = "logsPath";
const char kparams[] = "params";
const char checkItemConfig[] = "checkItemConfig";


bool AdjustTaskInfoLoader::Load(string config_file, AdjustTaskInfo & task_info){
    bool parse_suc = true;
    try {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var jsonResult = parser.parse(config_file);

        Poco::JSON::Object::Ptr root_obj;
        if (jsonResult.type() == typeid(Poco::JSON::Object::Ptr)) {
            root_obj = jsonResult.extract<Poco::JSON::Object::Ptr>();
        } else {
            return false;
        }

        if(root_obj->has(ktaskId)){
            task_info.task_id_ = root_obj->getValue<string>(ktaskId);
        }

        if(root_obj->has(klogsPath)){
            task_info.logs_path_ = root_obj->getValue<string>(klogsPath);
        }

        if(root_obj->has(kInput)){
            Poco::JSON::Object::Ptr inputObj = root_obj->getObject(kInput);
            if(inputObj->has(krootPath)){
                task_info.input_path_ = inputObj->getValue<string>(krootPath);
            }else{
                return false;
            }

            if(inputObj->has(kparams)){
                if(!ParseParams(inputObj, task_info)){
                    return false;
                }
            }
        }

        if(root_obj->has(kOutput)){
            Poco::JSON::Object::Ptr outputObj = root_obj->getObject(kOutput);
            if(outputObj->has(krootPath)){
                task_info.output_path_ = outputObj->getValue<string>(krootPath);
            }else{
                return false;
            }
        }

    } catch (Exception &e) {
        std::cout << e.what() << std::endl;
        parse_suc = false;
    }

    return parse_suc;
}

const char kpastTaskFrameIds[] = "pastTaskFrameIds";
const char knewTaskFrameIds[] = "newTaskFrameIds";

bool AdjustTaskInfoLoader::ParseParams(Poco::JSON::Object::Ptr params_obj, AdjustTaskInfo & task_info){
    //解析全部值
//    map<string,string> param_results;
    Poco::JSON::Array::Ptr params_array = params_obj->getArray(kparams);
    for (auto itr = params_array->begin(); itr != params_array->end(); ++itr) {
        Dynamic::Var paramValue = *itr;
        Object::Ptr param_obj = paramValue.extract<Poco::JSON::Object::Ptr>();

        string keyv = param_obj->getValue<string>("k");
        string valuev = param_obj->getValue<string>("v");
        task_info.param_results.insert(make_pair(keyv, valuev));
    }

//    //获取输入任务信息
//    if(!PareseParms(param_results, task_info.input_infos_, "_old", kpastTaskFrameIds)){
//        return false;
//    }
//
//    //获取输出任务信息
//    if(!PareseParms(param_results, task_info.output_infos_, "_new", kpastTaskFrameIds)){
//        return false;
//    }

    return true;
}
void replace_str(std::string& str, const std::string& before, const std::string& after)
{
    for (std::string::size_type pos(0); pos != std::string::npos; pos += after.length())
    {
        pos = str.find(before, pos);
        if (pos != std::string::npos)
            str.replace(pos, before.length(), after);
        else
            break;
    }
}
bool AdjustTaskInfoLoader::PareseParms(const map<string, string> &param_results,
                                       vector<shared_ptr<TaskInfo>> &infos, string mark, string parse_key) {
    auto in_task_it = param_results.find(parse_key);
    if(in_task_it != param_results.end()){
        string frame_ids = in_task_it->second;
        Poco::StringTokenizer st(frame_ids, ",");
        for(int i = 0 ; i < st.count() ; i ++){
            string frame_id = st[i];

            shared_ptr<TaskInfo> taskInfo = make_shared<TaskInfo>();
            taskInfo->task_frame_id_ = frame_id;

            //parse key
            string key_key = frame_id + mark + "_key";
            auto key_it = param_results.find(key_key);
            if(key_it != param_results.end()){
                string frame_keys = key_it->second;
                Poco::StringTokenizer keyst(frame_keys, ",");
                for(int j = 0 ; j < keyst.count(); j ++){
                    taskInfo->task_keys_.emplace_back(keyst[j]);
                }
            }else{
                return false;
            }

            //parse pos
            string pos_key = frame_id + "_pos";
            auto pos_it = param_results.find(pos_key);
            if(pos_it != param_results.end()){
                taskInfo->task_pos_ = pos_it->second;
            }

            //parse range
            string range_key = frame_id + mark + "_range";
            auto range_it = param_results.find(range_key);
            if(range_it != param_results.end()){
                string range = range_it->second;
                replace_str(range,", ",",");
                replace_str(range,"POLYGON ((","POLYGON((");
                string range_value = range.substr(9, range.length()-11);
                Poco::StringTokenizer coordst(range_value, ",");
                for(int j = 0 ; j < coordst.count() ; j ++){
                    Poco::StringTokenizer xyst(coordst[j], " ");
                    if(xyst.count() != 2){
                        return false;
                    }

                    Coord2d coord2d;
                    coord2d.x = atof(xyst[0].c_str());
                    coord2d.y = atof(xyst[1].c_str());
                    taskInfo->range_coords_.emplace_back(coord2d);
                }
            }

            //parse tracks
            string track_key = frame_id + mark + "_track";
            auto track_it = param_results.find(track_key);
            if(track_it != param_results.end()){
                string track_keys = track_it->second;
                Poco::StringTokenizer trackst(track_keys, ",");
                for(int j = 0 ; j < trackst.count(); j ++){
                    taskInfo->tracks_.emplace_back(trackst[j]);
                }
            }

            infos.emplace_back(taskInfo);
        }

    }else{
        return false;
    }

    return true;
}