//
// Created by ubuntu on 19-6-19.
//

#include "storage/JsonDataTypes.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
void JsonLog::SetGeometry(double x, double y, double z) {
    Poco::JSON::Array ja_coordinates;
    ja_coordinates.add(x);
    ja_coordinates.add(y);
    ja_coordinates.add(z);
    geometry_jobj_.set("coordinates", ja_coordinates);
    geometry_jobj_.set("type", JSONLOG_GEOMETRY_POINT);
}


void JsonLog::SetGeometry(shared_ptr<DCCoord> node) {
    Poco::JSON::Array ja_coordinates;
    if (node) {
        ja_coordinates.add(node->lng_);
        ja_coordinates.add(node->lat_);
        ja_coordinates.add(node->z_);
    } else {
        ja_coordinates.add(0);
        ja_coordinates.add(0);
        ja_coordinates.add(0);
    }
    geometry_jobj_.set("coordinates", ja_coordinates);
    geometry_jobj_.set("type", JSONLOG_GEOMETRY_POINT);
}

void JsonLog::SetProperties(LogProperty &log_property) {
    properties_jobj_.set("FLAG", log_property.flag);
    properties_jobj_.set("CHECK_ID", log_property.check_id);
    properties_jobj_.set("CHECK_NAME", log_property.check_name);
    properties_jobj_.set("CHECK_PROCESS", log_property.check_process);
    properties_jobj_.set("ERRTYPE", log_property.err_type);
    properties_jobj_.set("ERRDESC", log_property.err_desc);
    properties_jobj_.set("SCENE_ID", log_property.scene_id);
    properties_jobj_.set("STATE", log_property.state);
    properties_jobj_.set("COMPILE_VERSION", log_property.compile_version);
    properties_jobj_.set("BRANCH_NAME", log_property.branch_name);
    properties_jobj_.set("FRAME_ID", log_property.frame_id);
    properties_jobj_.set("DATA_KEY", log_property.data_key);
    properties_jobj_.set("CREATEBY", log_property.create_by);
    properties_jobj_.set("EDITBY", log_property.edit_by);
    properties_jobj_.set("CHECKBY", log_property.check_by);
    properties_jobj_.set("TASKID", log_property.task_id);
    properties_jobj_.set("PROJECTID", log_property.project_id);
}


bool JsonLog::RecursivelyCreateDir(const std::string &path, int mode) {
    if (access(path.c_str(), F_OK) == 0)
        return true;

    if (mkdir(path.c_str(), mode) == 0)
        return true;

    if (access(path.c_str(), F_OK) == 0)
        return false;

    // Try creating the parent.
    std::string::size_type slashpos = path.find_last_of("/");
    if (slashpos == std::string::npos) {
        // No parent given.
        return false;
    }

    return RecursivelyCreateDir(path.substr(0, slashpos), mode) &&
           mkdir(path.c_str(), mode) == 0;;
}

void JsonLog::AppendCheckError(
        const string& check_no,
        const string& check_no_desc,
        const string& error_desc,
        const string& task_id,
        const string& err_type,
        const string& data_key,
        const string& bound_id,
        const string& flag,
        shared_ptr<DCCoord> node) {
    LogProperty log_property;
    log_property.flag = flag;
    if (log_property.flag.empty()) {
        log_property.flag = "1";
    }
    log_property.check_id = check_no;
    log_property.check_name = check_no_desc;
    log_property.err_desc = error_desc;

    log_property.task_id = task_id;
    log_property.err_type = err_type;
    log_property.data_key =data_key;
//            GetKeyByTask(task_id, ORIGIN_TYPE_LANE);
    log_property.frame_id = bound_id;//TaskInfoManager::GetInstance().GetBoundIdByTask(task_id);

    AppendCheckError(log_property, node);
}

void JsonLog::AppendCheckError(LogProperty &log_propert, shared_ptr<DCCoord> node) {
    if(node != nullptr){
        SetGeometry(node);
    }
    SetProperties(log_propert);
    json_array_element_.set("geometry", geometry_jobj_);
    json_array_element_.set("type", "Feature");
    json_array_element_.set("properties", properties_jobj_);
    log_json_array_.add(json_array_element_);
}

void JsonLog::Stringify(ostream &out) {
    log_json_array_.stringify(out, 1);
}

JsonLog &JsonLog::GetInstance() {
    static JsonLog instance;
    return instance;
}

bool JsonLog::WriteToFile(string path) {
    if (path.empty())
        return false;

    std::lock_guard<std::mutex> locker(mutex_);

    std::string base_path = path;
    if (base_path[base_path.length() - 1] == '/')
        base_path = base_path.substr(0, base_path.length() - 1);

    std::string::size_type index = base_path.find_last_of('/');
    if (index == std::string::npos)
        return false;

    std::string dir = base_path.substr(0, index);

    if (access(dir.c_str(), F_OK) != 0) {
        if (!RecursivelyCreateDir(dir))
            return false;
    }

    stringstream ss;
    Stringify(ss);
    ofstream outfile;

    outfile.open(path, ios::out | ios::trunc);

    outfile << ss.str();

    outfile.close();

    return true;
}

std::string JsonLog::JsonLog::GetKeyByTask(const std::string& task_id,
                                           OriginTypeEnum type) {
    return GetEnumString(type) + "_" + task_id + "_1";
}

std::string JsonLog::GetEnumString(OriginTypeEnum type) {
    std::string ret = "unknown";
    switch (type) {
        case ORIGIN_TYPE_GROUND:
            ret = "ground";
            break;
        case ORIGIN_TYPE_LANE:
            ret = "lane";
            break;
        case ORIGIN_TYPE_POLE:
            ret = "pole";
            break;
        case ORIGIN_TYPE_SIGN:
            ret = "sign";
            break;
        default:
            break;
    }
    return ret;
}