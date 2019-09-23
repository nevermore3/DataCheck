//
// Created by gaoyanhong on 2019/7/23.
//

#ifndef AUTOHDMAP_COMPILE_TASKINFO_H
#define AUTOHDMAP_COMPILE_TASKINFO_H

#include <string>
#include <vector>
#include <memory>
using namespace std;

#include "api/CommonModel.h"
using namespace kd::api;

#include "Poco/JSON/Parser.h"


class TaskInfo{
public:
    string task_frame_id_;

    vector<string> task_keys_;

    string task_pos_;

    vector<Coord2d> range_coords_;

    vector<string> tracks_;
};

class AdjustTaskInfo{
public:
    string task_id_;

    string input_path_;

    string output_path_;

    string logs_path_;

    vector<shared_ptr<TaskInfo>> input_infos_;

    vector<shared_ptr<TaskInfo>> output_infos_;
    //输出参数
    map<string,string> param_results;
};

class AdjustTaskInfoLoader{
public:
    bool Load(string config_file, AdjustTaskInfo & task_info);

private:
    bool ParseParams(Poco::JSON::Object::Ptr params_obj, AdjustTaskInfo & task_info);

    bool PareseParms(const map<string,string> & param_results, vector<shared_ptr<TaskInfo>> & infos, string mark, string parse_key);
};

#endif //AUTOHDMAP_COMPILE_TASKINFO_H
