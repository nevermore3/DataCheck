//
// Created by gaoyanhong on 2018/3/29.
//

#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>

namespace kd {
    namespace dc {

        CheckErrorOutput::CheckErrorOutput(CppSQLite3::Database *pdb) {
            m_pdb = pdb;
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

        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
            LOG(ERROR) << info;
        }

        int CheckErrorOutput::saveError() {
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;

            try {
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
                m_pdb->execDML(create_table_sql);

                string insert_sql = "INSERT INTO dataCheckResultTable VALUES(?,?,?,?,?,?,?,?,?,?,?);";
                auto statement = m_pdb->compileStatement(insert_sql);
                m_pdb->execDML("BEGIN;");

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
                        statement.bindString(count++, item.checkModel_);
                        statement.bindString(count++, item.checkDesc_);
                        statement.bindString(count++, item.detail);
                        statement.execDML();
                        statement.reset();
                    }
                }
                statement.finalize();
                m_pdb->execDML("COMMIT;");

            } catch (CppSQLite3::Exception &e) {
                LOG(ERROR) << e.errorMessage().c_str();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }

        int CheckErrorOutput::countError() {
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;

            try {
                string create_table_sql = "CREATE TABLE IF NOT EXISTS dataCheckCountTable(   \
                                        sequenceId INTEGER PRIMARY KEY AUTOINCREMENT,\
                                        level TEXT,\
                                        testId TEXT NOT NULL,\
                                        testName TEXT NOT NULL,\
                                        count INTEGER NOT NULL)";
                m_pdb->execDML(create_table_sql);

                string insert_sql = "INSERT INTO dataCheckCountTable (level,testId,testName,count) \
                                    select level,testId,testName,count(*) from \
                                    dataCheckResultTable group by level , testId, testName;";
                m_pdb->execDML(insert_sql);


            } catch (CppSQLite3::Exception &e) {
                LOG(ERROR) << e.errorMessage().c_str();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }

        void CheckErrorOutput::saveError(shared_ptr<DCError> error) {
            if (error) {
                ErrorOutPut error_output;
                error_output.checkModel_ = error->checkModel_;
                error_output.checkDesc_ = error->checkDesc_;
                error_output.level = get_error_level(error->checkModel_);
                error_output.detail = error->toString();
                auto check_model_iter = check_model_2_output_maps_.find(error->checkModel_);
                if (check_model_iter != check_model_2_output_maps_.end()) {
                    check_model_iter->second.emplace_back(error_output);
                } else {
                    vector<ErrorOutPut> error_output_vec;
                    error_output_vec.emplace_back(error_output);
                    check_model_2_output_maps_.insert(make_pair(error->checkModel_, error_output_vec));
                }
            } else {
                LOG(ERROR) << "saveError error is null!";
            }
        }

        string CheckErrorOutput::get_error_level(string check_model) {
            string ret = LEVEL_WARNING;

            if (error_check_levels_.find(check_model) != error_check_levels_.end()) {
                ret = LEVEL_ERROR;
            }

            return ret;
        }
    }
}

