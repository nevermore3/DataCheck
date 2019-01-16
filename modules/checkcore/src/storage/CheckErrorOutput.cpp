//
// Created by gaoyanhong on 2018/3/29.
//

#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>

namespace kd {
    namespace dc {

//        void CheckErrorOutput::saveError(shared_ptr<DCDividerCheckError> error) {
////            if (ss_.is_open()){
////                ss_<< error->toString() << "\n";
////            }else{
////                LOG(ERROR) << error->toString();
////            }
//        }
//
//        void CheckErrorOutput::saveError(shared_ptr<DCAttCheckError> error){
////            if (ss_.is_open()){
////                ss_<< error->toString() << "\n";
////            }else{
////                LOG(ERROR) << error->toString();
////            }
//        }
//
//        void CheckErrorOutput::saveError(shared_ptr<DCLaneCheckError> error){
////            if (ss_.is_open()){
////                ss_<< error->toString() << "\n";
////            }else{
////                LOG(ERROR) << error->toString();
////            }
//        }

        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
//            if (ss_.is_open()){
//                if (bLongString)
//                    ss_<< "\"" << info.c_str() << "\"\n";
//                else
//                    ss_<< info.c_str() << "\n";
//            }else{
//                LOG(ERROR) << info.c_str();
//            }
        }

        void CheckErrorOutput::saveError() {
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;

            try {
                string create_table_sql = "CREATE TABLE failedTestResultTable(   \
                                     testId TEXT,                   \
                                     testName TEXT,                 \
                                     details TEXT)";
                m_pdb->execDML(create_table_sql);

                string insert_sql = "INSERT INTO failedTestResultTable VALUES(?,?,?);";
                auto statement = m_pdb->compileStatement(insert_sql);
                m_pdb->execDML("BEGIN;");

                for (const auto &check_item : check_model_2_output_maps_) {
                    for (const auto& item : check_item.second) {
                        int count = 1;
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
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
        }

        void CheckErrorOutput::saveError(shared_ptr<DCError> error) {
            ErrorOutPut error_output;
            error_output.checkModel_ = error->checkModel_;
            error_output.checkDesc_ = error->checkDesc_;
            error_output.detail = error->toString();
            auto check_model_iter = check_model_2_output_maps_.find(error->checkModel_);
            if (check_model_iter != check_model_2_output_maps_.end()) {
                check_model_iter->second.emplace_back(error_output);
            } else {
                vector<ErrorOutPut> error_output_vec;
                error_output_vec.emplace_back(error_output);
                check_model_2_output_maps_.insert(make_pair(error->checkModel_, error_output_vec));
            }
        }
    }
}

