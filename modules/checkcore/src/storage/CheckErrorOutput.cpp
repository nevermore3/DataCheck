//
// Created by gaoyanhong on 2018/3/29.
//

#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>
#include <util/check_list_config.h>

namespace kd {
    namespace dc {

        CheckErrorOutput::CheckErrorOutput(int check_state) {
            check_state_ = check_state;
            p_db_out_ = new CppSQLite3::Database();

            initErrorLevel();
        }


        CheckErrorOutput::~CheckErrorOutput() {
            if (p_db_out_) {
                p_db_out_->close();
                delete p_db_out_;
                p_db_out_ = nullptr;
            }

            check_model_2_output_maps_.clear();
            check_total_.clear();
            error_check_levels_.clear();
        }


        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
            LOG(ERROR) << info;
        }
        int CheckErrorOutput::saveErrorReport(string err_file_name){
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;
            try {

                for (const auto check_item : check_total_) {
                    ReportLogTotal checkItem;
                    string errCode = check_item.first;
                    shared_ptr<CheckItemInfo> checkItemInfo = check_item.second;
                    vector<ErrorOutPut> errs_v = check_model_2_output_maps_[errCode];
                    string errMessage = CheckListConfig::getInstance().GetCheckItemDesc(checkItemInfo->checkId);
                    int size = errs_v.size();
                    int failNum = 0;
                    if(size == 0){
                        checkItemInfo->successNum = checkItemInfo->totalNum;
                        checkItemInfo->failNum = 0;

                    }else{
                       for(const auto errCase : errs_v){
                           failNum++;
                           ReportLogItem reportLogItem;
                           reportLogItem.task_id = errCase.taskId;
                           if(errMessage.length()>0){
                               reportLogItem.err_message = errMessage;
                           }else{
                               reportLogItem.err_message = errCase.checkName;
                           }
                           reportLogItem.err_desc = errCase.errDesc;
                           reportLogItem.node = errCase.coord;
                           reportLogItem.errNodeInfo = errCase.errNodeInfo;
                           reportLogItem.sourceId = errCase.sourceId;
                           ReportJsonLog::GetInstance().AppendErrorCase(reportLogItem);
                       }
                        checkItemInfo->failNum = failNum;
                        checkItemInfo->successNum = checkItemInfo->totalNum - failNum;
                    }
                    checkItem.totalNum = checkItemInfo->totalNum;
                    checkItem.successNum = checkItemInfo->successNum;
                    checkItem.failNum = checkItemInfo->failNum;
                    checkItem.checkItemCode = checkItemInfo->checkId;
                    ReportJsonLog::GetInstance().AppendCheckItemTotal(checkItem);
                }
                string errJsonPath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::OUTPUT_PATH)+err_file_name;
                ReportJsonLog::GetInstance().WriteToFile(errJsonPath);
            } catch (std::exception &e) {
                LOG(ERROR) << e.what();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }
        int CheckErrorOutput::saveJsonError(){
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;
            try {

                for (const auto &check_item : check_model_2_output_maps_) {
                    for (const auto& item : check_item.second) {
                        string err_type = "E2";
                        if (item.level == LEVEL_ERROR) {
                            err_type = "E1";
                            ret = 1;
                        }
                        JsonLog::GetInstance().AppendCheckError(item.checkId,item.checkName,item.errDesc,item.taskId,err_type,item.dataKey,item.boundId,item.flag, item.coord);
                    }
                }
                string errJsonPath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::ERR_JSON_PATH);
                JsonLog::GetInstance().WriteToFile(errJsonPath);
            } catch (std::exception &e) {
                LOG(ERROR) << e.what();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }


        void CheckErrorOutput::saveError(shared_ptr<DCError> error) {
            if (error) {
                if (check_state_ == DataCheckConfig::ALL_AUTO_CHECK) {
                    ErrorOutPut error_output;

                    error_output.checkId = error->checkId;
                    error_output.checkName = error->checkName;
                    error_output.level = get_error_level(error->checkId);
                    error_output.errDesc = error->toString();
                    error_output.taskId = error->taskId_;
                    error_output.boundId = DataCheckConfig::getInstance().getProperty(error->taskId_);
                    error_output.dataKey = error->dataKey_;
                    error_output.flag = error->flag;
                    error_output.coord = error->coord;
                    error_output.errNodeInfo = error->errNodeInfo;
                    error_output.sourceId = error->sourceId;
                    auto check_model_iter = check_model_2_output_maps_.find(error->checkId);
                    if (check_model_iter != check_model_2_output_maps_.end()) {
                        check_model_iter->second.emplace_back(error_output);
                    } else {
                        vector<ErrorOutPut> error_output_vec;
                        error_output_vec.emplace_back(error_output);
                        check_model_2_output_maps_.insert(make_pair(error->checkId, error_output_vec));
                    }
                } else if (check_state_ == DataCheckConfig::ALL_AUTO_CHECK) {
                    ErrorOutPut error_output;

                    error_output.checkId = error->checkId;
                    error_output.checkName = error->checkName;
                    error_output.level = get_error_level(error->checkId);
                    error_output.errDesc = error->toString();
                    error_output.coord = error->coord;
                    auto check_model_iter = check_model_2_output_maps_.find(error->checkId);
                    if (check_model_iter != check_model_2_output_maps_.end()) {
                        check_model_iter->second.emplace_back(error_output);
                    } else {
                        vector<ErrorOutPut> error_output_vec;
                        error_output_vec.emplace_back(error_output);
                        check_model_2_output_maps_.insert(make_pair(error->checkId, error_output_vec));
                    }
                }

            } else {
                LOG(ERROR) << "saveError error is null!";
            }
        }

        void CheckErrorOutput::addCheckItemInfo(shared_ptr<CheckItemInfo> &checkItemInfo){
            if(checkItemInfo){
                auto check_id_iter = check_total_.find(checkItemInfo->checkId);
                if (check_id_iter != check_total_.end()) {
                    check_id_iter->second->totalNum +=checkItemInfo->totalNum;
                }else{
//                    LOG(ERROR) << "totalNum:" << checkItemInfo->totalNum;
                    check_total_[checkItemInfo->checkId] = checkItemInfo;
                }
            } else {
                LOG(ERROR) << "saveError error is null!";
            }

        }

        void CheckErrorOutput::addCheckItemInfo(string checkId,int total){
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->totalNum = total;
            checkItemInfo->checkId = checkId;
            addCheckItemInfo(checkItemInfo);
        }

        string CheckErrorOutput::get_error_level(string check_model) {
            string ret = LEVEL_WARNING;

            if (error_check_levels_.find(check_model) != error_check_levels_.end()) {
                ret = LEVEL_ERROR;
            }

            return ret;
        }

        void CheckErrorOutput::initErrorLevel() {
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_002);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_003);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_004);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_008);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_015);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_020);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_021);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_022);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_023);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_024);
            error_check_levels_.insert(CHECK_ITEM_KXS_ORG_025);

            error_check_levels_.insert(CHECK_ITEM_KXS_LG_002);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_004);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_005);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_006);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_011);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_012);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_014);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_015);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_016);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_019);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_020);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_021);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_022);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_023);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_024);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_025);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_026);
            error_check_levels_.insert(CHECK_ITEM_KXS_LG_027);


            error_check_levels_.insert(CHECK_ITEM_KXS_ROAD_001);
            error_check_levels_.insert(CHECK_ITEM_KXS_ROAD_002);
            error_check_levels_.insert(CHECK_ITEM_KXS_ROAD_004);
            error_check_levels_.insert(CHECK_ITEM_KXS_ROAD_005);
            error_check_levels_.insert(CHECK_ITEM_KXS_ROAD_007);

            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_001);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_003);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_006);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_008);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_012);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_013);
            error_check_levels_.insert(CHECK_ITEM_KXS_LANE_014);

            error_check_levels_.insert(CHECK_ITEM_KXS_LM_001);
            error_check_levels_.insert(CHECK_ITEM_KXS_LM_002);
        }

        int CheckErrorOutput::saveErrorToDb(const string &ouput_file) {
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;

            try {
                if (p_db_out_ != nullptr) {
                    p_db_out_->open(ouput_file);
                    string create_table_sql = "CREATE TABLE IF NOT EXISTS dataCheckResultTable(   \
                                        sequenceId INTEGER PRIMARY KEY AUTOINCREMENT,\
                                        productGroupId INTEGER,\
                                        productId INTEGER,\
                                        updateRegionId INTEGER,\
                                        updateAreaId INTEGER,\
                                        buildingBlockId INTEGER,\
                                        name TEXT,\
                                        level TEXT,\
                                        testId TEXT NOT NULL,\
                                        testName TEXT NOT NULL,\
                                        details TEXT NOT NULL)";
                    p_db_out_->execDML(create_table_sql);

                    string insert_sql = "INSERT INTO dataCheckResultTable VALUES(?,?,?,?,?,?,?,?,?,?,?);";
                    auto statement = p_db_out_->compileStatement(insert_sql);
                    p_db_out_->execDML("BEGIN;");

                    for (const auto &check_item : check_model_2_output_maps_) {
                        for (const auto& item : check_item.second) {
                            if (item.level == LEVEL_ERROR) {
                                ret = 1;
                            }
                            int count = 1;
                            statement.bindNull(count++);
                            statement.bindNull(count++);
                            statement.bindNull(count++);
                            statement.bindString(count++, item.update_region_id);
                            statement.bindNull(count++);
                            statement.bindNull(count++);
                            statement.bindNull(count++);
                            statement.bindString(count++, item.level);
                            statement.bindString(count++, item.checkId);
                            statement.bindString(count++, item.checkName);
                            statement.bindString(count++, item.errDesc);
                            statement.execDML();
                            statement.reset();
                        }
                    }
                    statement.finalize();
                    p_db_out_->execDML("COMMIT;");

                    create_table_sql = "CREATE TABLE IF NOT EXISTS dataCheckCountTable(   \
                                        sequenceId INTEGER PRIMARY KEY AUTOINCREMENT,\
                                        level TEXT,\
                                        testId TEXT NOT NULL,\
                                        testName TEXT NOT NULL,\
                                        count INTEGER NOT NULL)";
                    p_db_out_->execDML(create_table_sql);

                    insert_sql = "INSERT INTO dataCheckCountTable (level,testId,testName,count) \
                                    select level,testId,testName,count(*) from \
                                    dataCheckResultTable group by level , testId, testName;";
                    p_db_out_->execDML(insert_sql);
                } else {
                    ret = 1;
                }
            } catch (CppSQLite3::Exception &e) {
                LOG(ERROR) << e.errorMessage().c_str();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }

    }
}

