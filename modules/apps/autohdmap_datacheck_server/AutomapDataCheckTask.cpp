/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   AutomapDataCheckTask.cpp
 * Author: weihainan
 *
 * Created on 2018年3月20日, 上午11:54
 */

#include <string>
#include <iostream>
#include <fstream>
#include <thread>

//third party
#include <Poco/StringTokenizer.h>
#include <Poco/Stopwatch.h>
#include <Poco/Logger.h>
#include <Poco/File.h>
#include <Poco/JSON/Parser.h>

#include "AutomapDataCheckTask.h"

//module
#include "ServerConfig.h"
#include "data/DataManager.h"
#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"
#include "ProcessManager.h"
#include "core/task/VerboseTaskStatusLisenter.hpp"
#include "core/task/TaskProcessStatus.hpp"
#include "TaskStatus.h"

#include "DataCheckConfig.h"
#include "ModelProcessManager.h"
#include "process/ModelDataLoader.h"
#include "process/ModelFieldCheck.h"
#include "process/ModelBussCheck.h"
#include "process/ModelRelationCheck.h"

#include "MapProcessManager.h"
#include "businesscheck/MapDataLoader.h"
#include "businesscheck/DividerAttribCheck.h"
#include "businesscheck/DividerShapeNormCheck.h"
#include "businesscheck/DividerShapeDefectCheck.h"
#include "businesscheck/DividerTopoCheck.h"
#include "businesscheck/LaneAttribCheck.h"
#include "businesscheck/LaneShapeNormCheck.h"
#include "businesscheck/LaneTopoCheck.h"

using namespace kd::dc;
using namespace kd::automap;

using Poco::Timestamp;

void loadTaskInfo(string fileName, string & taskName, string & baseUrl,
                  vector<string> & taskIds, vector<string> & batchs, vector<string> & trackIds){
    ifstream in(fileName);
    if (in.is_open()) {
        string tempvalue;

        if(!in.eof()) {
            getline(in, tempvalue);

            taskName = tempvalue;

            getline(in, tempvalue);
            baseUrl = tempvalue;

            getline(in, tempvalue);
            int taskCount = stoi(tempvalue);
            for( int i = 0 ; i < taskCount ; i ++ ){
                getline(in, tempvalue);

                Poco::StringTokenizer st(tempvalue, ",");
                if(st.count() == 3){
                    taskIds.emplace_back(st[0]);
                    batchs.emplace_back(st[1]);
                    trackIds.emplace_back(st[2]);
                }
            }
        }
    }
}

int dataCheck(string basePath, string taskFileName){

    //输出错误文件
    string outputFile = basePath+"/check_result.csv";
    shared_ptr<CheckErrorOutput> errorOutput = make_shared<CheckErrorOutput>(outputFile);

    //交换格式基本属性检查
    {
        shared_ptr<ModelProcessManager> modelProcessManager = make_shared<ModelProcessManager>("modelCheck");

        //加载数据
        shared_ptr<ModelDataLoader> modelLoader = make_shared<ModelDataLoader>(basePath, taskFileName);
        modelProcessManager->registerProcessor(modelLoader);

        //属性字段检查
        shared_ptr<ModelFieldCheck> modelFiledCheck = make_shared<ModelFieldCheck>();
        modelProcessManager->registerProcessor(modelFiledCheck);

        //属性业务检查
        shared_ptr<ModelBussCheck> modelBussCheck = make_shared<ModelBussCheck>();
        modelProcessManager->registerProcessor(modelBussCheck);

        //属性关系检查
        shared_ptr<ModelRelationCheck> modelRelationCheck = make_shared<ModelRelationCheck>();
        modelProcessManager->registerProcessor(modelRelationCheck);

        //执行已注册检查项
        shared_ptr<ModelDataManager> modelDataManager = make_shared<ModelDataManager>();
        modelProcessManager->execute(modelDataManager, errorOutput);
    }

    //交换格式逻辑检查
    {
        DataCheckConfig::getInstance().load("config.properties");
        shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("mapCheck");

        //加载数据
        shared_ptr<MapDataLoader> loader = make_shared<MapDataLoader>(basePath);
        mapProcessManager->registerProcessor(loader);

        //车道线属性检查
        shared_ptr<DividerAttribCheck> divAttCheck = make_shared<DividerAttribCheck>();
        mapProcessManager->registerProcessor(divAttCheck);

        //车道线几何形态检查
        shared_ptr<DividerShapeNormCheck> divShpNormCheck = make_shared<DividerShapeNormCheck>();
        mapProcessManager->registerProcessor(divShpNormCheck);

        //车道线形状缺陷检查
        shared_ptr<DividerShapeDefectCheck> divShpDefCheck = make_shared<DividerShapeDefectCheck>();
        mapProcessManager->registerProcessor(divShpDefCheck);

        //车道线拓扑检查
        shared_ptr<DividerTopoCheck> divTopoCheck = make_shared<DividerTopoCheck>();
        mapProcessManager->registerProcessor(divTopoCheck);

        //车道属性检查
        shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
        mapProcessManager->registerProcessor(laneAttCheck);

        //车道几何形状检查
        shared_ptr<LaneShapeNormCheck> laneShpCheck = make_shared<LaneShapeNormCheck>();
        mapProcessManager->registerProcessor(laneShpCheck);

        //车道拓扑检查
        shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();
        mapProcessManager->registerProcessor(laneTopoCheck);

        //执行已注册检查项
        shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
        mapProcessManager->execute(mapDataManager, errorOutput);
    }

    return 1;
}



AutomapDataCheckTask::AutomapDataCheckTask() = default;

AutomapDataCheckTask::AutomapDataCheckTask(const AutomapDataCheckTask &orig) {
}

AutomapDataCheckTask::~AutomapDataCheckTask() = default;

void AutomapDataCheckTask::percent(const char *seqId, ICallback *callback) {
    //同一个进度持续时间最大阈值，大于该值，则报错退出。单位为秒
    const long TIME_THRESHOLD = 60 * 60;
    //向工作流发进度信息的时间间隔
    const int TIME_CALLBACK_THRESHOLD = 15;
    //最后进度为100%时，给工作流回传的重试次数
    const int RETRY_TIMES = 3;
    //
    TaskProcessStatus &status = TaskProcessStatus::getInstance();
    TaskStatus &taskStatus = TaskStatus::getInstance();
    int process = 0;
    string taskInfo;
    //记录某个时刻的进度
    int lastProcess = 0;
    //记录上次保存的时间点
    Timestamp lastTime;
    //记录相同进度的情况下，上次保存的时间点
    Timestamp lastTimeForSameProcess;

    while (process != 100) {
        Timestamp now;
        long timePassed = (now.utcTime() - lastTime.utcTime()) / 10000000;
        long timePassedForSameProcess = (now.utcTime() - lastTimeForSameProcess.utcTime()) / 10000000;
        cout << "time passed : " << timePassed << endl;
        if (taskStatus.getStatus(seqId) == 3) {
            taskInfo = status.getTaskInfo(seqId);
            callback->setProcess(taskInfo.c_str(), seqId, process);
            taskStatus.stopTask(seqId, taskInfo.c_str());
            taskStatus.delTask(seqId);
            return;
        }
        //
        process = status.getProcess(seqId);
        if (process != lastProcess) {
            lastTimeForSameProcess = now;
        }
        //判断同一个进度的持续时间是否已经超过了TIME_THRESHOLD, 如果超过则报错返回，退出该任务
        if (process == lastProcess && timePassedForSameProcess > TIME_THRESHOLD) {
            stringstream ss;
            ss << "进度" << process << "%" << "已经持续超过了" << TIME_THRESHOLD << "秒" << endl;
            taskInfo = ss.str();
            taskStatus.setStatus(seqId, 3);
            callback->setProcess(taskInfo.c_str(), seqId, process);
            taskStatus.stopTask(seqId, taskInfo.c_str());
            taskStatus.delTask(seqId);
            return;
        } else {
            lastProcess = process;
        }

        stringstream ss;
        ss << "Task:" << taskInfo << ", Seq:" << seqId << ", process: " << process << "%" << endl;
        Poco::Logger::get("FileLogger").information(ss.str());

        if (timePassed >= TIME_CALLBACK_THRESHOLD) {
            if (process != 0) {
                callback->setProcess(taskInfo.c_str(), seqId, process);
            }
            lastTime = now;
        }

        this_thread::sleep_for(chrono::seconds(1));
    }

    taskInfo = status.getTaskInfo(seqId);
    for (int i = 0; i < RETRY_TIMES; ++i) {
        callback->setProcess(taskInfo.c_str(), seqId, process);
    }
    taskStatus.delTask(seqId);
}

void AutomapDataCheckTask::start(const char *taskInfo, const char *seqId, ICallback *callback) {
    cout << "I start a task: taskInfo:" << taskInfo << ", seqId:" << seqId << endl;
    //
    TaskProcessStatus &status = TaskProcessStatus::getInstance();
    ServerConfig &config = ServerConfig::getInstance();
    TaskStatus &taskStatus = TaskStatus::getInstance();
    string errorInfo;
    string inputDir;
    //
    try {
        if (strlen(taskInfo) > 0) {
            Poco::Stopwatch sw;
            Poco::JSON::Parser parser;
            sw.start();
            Poco::Dynamic::Var result = parser.parse(taskInfo);
            sw.stop();

            Poco::JSON::Object::Ptr obj;
            if (result.type() == typeid(Poco::JSON::Object::Ptr))
                obj = result.extract<Poco::JSON::Object::Ptr>();

            if (obj->has("inputDir")) {
                inputDir = obj->getValue<string>("inputDir");
            }

        }
    } catch (Poco::Exception &e) {
        setStausToWorkFlow(3, seqId, e.what());
        throw e;
    }

    if (inputDir.empty()) {
        setStausToWorkFlow(3, seqId, "inputDir is empty");
        return;
    }
    //
    dataCheck(inputDir, "task.json");
    setFinishStatus(seqId);
    taskStatus.delTask(seqId);
    //
    cout << "I finish a task: taskInfo:" << taskInfo << ", seqId:" << seqId << endl;
}

void AutomapDataCheckTask::stop(const char *taskInfo, const char *seqId) {
    cout << "I stop a task: taskInfo:" << taskInfo << ", seqId:" << seqId << endl;
}

void AutomapDataCheckTask::rollBack(const char *taskInfo, const char *seqId, string &code, string &message) {
}

string AutomapDataCheckTask::jsonArrayToString(Poco::JSON::Array::Ptr input) {
    string result;
    auto iter = input->begin();
    size_t num = input->size();
    int i = 0;
    for (; iter != input->end(); iter++) {
        string str = iter->toString();
        if (i == num - 1) {
            result += str;
        } else {
            result += str + ",";
        }
        ++i;
    }
    return result;
}

string AutomapDataCheckTask::getUrlFromTaskInfo(const char *taskInfo, const string &urlType) {
    string urls;
    //
    Poco::Stopwatch sw;
    Poco::JSON::Parser parser;
    sw.start();
    Poco::Dynamic::Var result = parser.parse(taskInfo);
    sw.stop();
    //
    Poco::JSON::Object::Ptr obj;
    if (result.type() == typeid(Poco::JSON::Object::Ptr))
        obj = result.extract<Poco::JSON::Object::Ptr>();
    //
    if (!obj->has(urlType)) {
        return "";
        //
    } else {
        auto pUrls = obj->getArray(urlType);
        auto iter = pUrls->begin();
        size_t num = pUrls->size();
        int i = 0;
        for (; iter != pUrls->end(); iter++) {
            string str = iter->toString();
            if (i == num - 1) {
                urls += str;
            } else {
                urls += str + ",";
            }
            ++i;
        }
    }
    return urls;
}

void AutomapDataCheckTask::setStausToWorkFlow(int statusCode, const string &seq, const string &info) {
    TaskProcessStatus &status = TaskProcessStatus::getInstance();
    ServerConfig &config = ServerConfig::getInstance();
    TaskStatus &taskStatus = TaskStatus::getInstance();
    //
    Poco::Logger::get("FileLogger").error(info);
    status.setTotalFrame(seq, 0);
    taskStatus.setStatus(seq.c_str(), statusCode);
    status.setTaskInfo(seq, info);
}

void AutomapDataCheckTask::setFinishStatus(const string &seq) {
    TaskProcessStatus &status = TaskProcessStatus::getInstance();
    Poco::JSON::Object obj;
    int total, success, fail;
    int rate = 100;

    status.getFinishInfo(seq, total, success, fail);
    obj.set("total", total);
    obj.set("success", success);
    obj.set("fail", fail);

    std::stringstream str;
    obj.stringify(str);
    status.setTaskInfo(seq, str.str());
    status.setProcess(seq, rate);
}