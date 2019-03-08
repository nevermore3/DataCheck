//
// Created by gaoyanhong on 2018/3/29.
//

#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>

namespace kd {
    namespace dc {

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
            if (check_model == "KXS-01-002" || check_model == "KXS-01-003" || check_model == "KXS-01-004" ||
                check_model == "KXS-01-008" || check_model == "KXS-01-015" || check_model == "KXS-01-020") {
                ret = LEVEL_ERROR;
            } else if (check_model == "KXS-03-002" || check_model == "KXS-03-004" || check_model == "KXS-03-005" ||
                       check_model == "KXS-03-006" || check_model == "KXS-03-011" || check_model == "KXS-03-012" ||
                       check_model == "KXS-03-014" || check_model == "KXS-03-015" ||
                       check_model == "KXS-03-016" || check_model == "KXS-03-019" || check_model == "KXS-03-020" ||
                       check_model == "KXS-03-021" || check_model == "KXS-03-022" || check_model == "KXS-03-023" ||
                       check_model == "KXS-03-024" || check_model == "KXS-03-025" || check_model == "KXS-03-026" ||
                       check_model == "KXS-03-027") {
                ret = LEVEL_ERROR;
            } else if (check_model == "KXS-04-001" || check_model == "KXS-04-002") {
                ret = LEVEL_ERROR;
            } else if (check_model == "KXS-05-001" || check_model == "KXS-05-003" || check_model == "KXS-05-006" ||
                       check_model == "KXS-05-008" || check_model == "KXS-05-012") {
                ret = LEVEL_ERROR;
            } else if (check_model == "KXS-06-001") {
                ret = LEVEL_ERROR;
            }


            return ret;
        }
    }
}

