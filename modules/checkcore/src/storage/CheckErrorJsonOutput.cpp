//
// Created by ubuntu on 19-6-19.
//
#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>


namespace kd {
    namespace dc {

        CheckErrorOutput::CheckErrorOutput() {
            jsonLog_ = JsonLog::GetInstance();
        }

        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
            LOG(ERROR) << info;
        }

        int CheckErrorOutput::saveError() {
            int ret = 0;
            LOG(INFO) << "task [save error] start. ";
            TimerUtil compilerTimer;

            try {
                string taskId = DataCheckConfig::getInstance().getTaskId();

                for (const auto &check_item : check_model_2_output_maps_) {
                    for (const auto& item : check_item.second) {
                        string err_type = LEVEL_WARNING;
                        if (item.level == LEVEL_ERROR) {
                            err_type = LEVEL_ERROR;
                            ret = 1;
                        }
                        jsonLog_->AppendCheckError(item.checkId,item.checkName,item.errDesc,taskId,err_type,1, nullptr);
                    }
                }

            } catch (Poco::Exception &e) {
                LOG(ERROR) << e.errorMessage().c_str();
                ret = 1;
            }

            LOG(INFO) << "task [save error] end successfully " << " costs : " << compilerTimer.elapsed_message();
            return ret;
        }

        int CheckErrorOutput::countError() {
            int ret = 0;

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

        void CheckErrorOutput::saveTotalError() {


        }
    }
}
