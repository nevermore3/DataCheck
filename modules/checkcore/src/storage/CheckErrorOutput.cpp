//
// Created by gaoyanhong on 2018/3/29.
//

#include <storage/CheckErrorOutput.h>
#include <util/TimerUtil.h>

namespace kd {
    namespace dc {

        CheckErrorOutput::CheckErrorOutput() {
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

        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
            LOG(ERROR) << info;
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

                auto check_model_iter = check_model_2_output_maps_.find(error->checkId);
                if (check_model_iter != check_model_2_output_maps_.end()) {
                    check_model_iter->second.emplace_back(error_output);
                } else {
                    vector<ErrorOutPut> error_output_vec;
                    error_output_vec.emplace_back(error_output);
                    check_model_2_output_maps_.insert(make_pair(error->checkId, error_output_vec));
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

