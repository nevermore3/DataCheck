//
// Created by ubuntu on 19-7-2.
//

#include "storage/ErrorReportJsonOutput.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <util/KDSUtil.h>

bool ReportJsonLog::RecursivelyCreateDir(const std::string &path, int mode) {
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

void ReportJsonLog::AppendCheckItemTotal(ReportLogTotal &checkItem){
    Poco::JSON::Object oneCheck;
    oneCheck.set("batchId",checkItem.batchId);
    oneCheck.set("checkItemCode",checkItem.checkItemCode);
    oneCheck.set("failNum",checkItem.failNum);
    oneCheck.set("successNum",checkItem.successNum);
    oneCheck.set("totalNum",checkItem.totalNum);
    oneCheck.set("startAt",0);
    oneCheck.set("endAt",0);
    oneCheck.set("checkDataResults",check_item);
    log_json_array_.add(oneCheck);
    check_item.clear();
}
void ReportJsonLog::AppendErrorCase(ReportLogItem &errorCase){
    Poco::JSON::Object oneCase;
    oneCase.set("taskId",errorCase.task_id);
    oneCase.set("errorMsg",errorCase.err_desc);
    oneCase.set("status",errorCase.state);
    if(errorCase.node!= nullptr){
        oneCase.set("dataX",errorCase.node->lng_);
        oneCase.set("dataY",errorCase.node->lat_);
    }
    Poco::JSON::Array checkDataRefs;
    if(errorCase.errNodeInfo.size()>0){
        for(auto point:errorCase.errNodeInfo){
            Poco::JSON::Object json_point;
            json_point.set("dataId",point->dataId);
            json_point.set("dataLayer",point->dataLayer);
            json_point.set("dataType",point->dataType);
            json_point.set("dataY",point->lat_);
            json_point.set("dataX",point->lng_);
            checkDataRefs.add(json_point);
        }
        Object::Ptr entity = nullptr;
        KDSUtil::getResourceData("lane",errorCase.task_id,errorCase.errNodeInfo[0]->dataType,errorCase.sourceId,entity);
        if(entity){
            oneCase.set("entity",entity);
        }
    }
    oneCase.set("checkDataRefs",checkDataRefs);
    check_item.add(oneCase);
}

void ReportJsonLog::Stringify(ostream &out) {
    log_json_array_.stringify(out, 1);
}

ReportJsonLog &ReportJsonLog::GetInstance() {
    static ReportJsonLog instance;
    return instance;
}

bool ReportJsonLog::WriteToFile(string path) {
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


