/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AutoMapTask.h
 * Author: zhangbenxing
 *
 * Created on 2017年10月20日, 上午11:54
 */

#ifndef AutomapDataCheckTask_H
#define AutomapDataCheckTask_H

#include <Poco/JSON/Array.h>
#include "ITask.h"
#include "ICallback.h"

class AutomapDataCheckTask : public ITask {
public:
    AutomapDataCheckTask();

    AutomapDataCheckTask(const AutomapDataCheckTask &orig);

    ~AutomapDataCheckTask() override;

    void start(const char *taskInfo, const char *seqId, ICallback *callback) override;

    void stop(const char *taskInfo, const char *seqId) override;

    void rollBack(const char *taskInfo, const char *seqId, string &code, string &message) override;

    void percent(const char *seqId, ICallback *callback);



private:
    /**
     * 解析json数组，拼接多服务地址
     * @param input
     * @return
     */
    string jsonArrayToString(Poco::JSON::Array::Ptr input);

    /**
     * 从工作流触发信息中提取指定类型的服务地址
     * @param taskInfo
     * @param urlType : krsUrls, krmsUrls, kdsDataUrls
     * @return
     */
    string getUrlFromTaskInfo(const char *taskInfo, const string &urlType);

    /**
     * 向工作流返回错误信息
     * @param statusCode
     * @param seq
     * @param info
     */
    void setStausToWorkFlow(int statusCode, const string &seq, const string& info);

    void setFinishStatus(const string &seq);
};

#endif /* AUTOMAPTASK_H */

