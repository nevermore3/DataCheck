
#ifndef AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
#define AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H

#include <DataCheckConfig.h>
#include "data/ErrorDataModel.h"
#include "JsonDataTypes.h"
#include "ErrorReportJsonOutput.h"

#include "cppsqlite3/Database.h"
#include "cppsqlite3/Statement.h"
#include "cppsqlite3/Query.h"

namespace kd {
    namespace dc {
        struct ErrorOutPut{
            ErrorOutPut() {
                update_region_id = DataCheckConfig::getInstance().getProperty(DataCheckConfig::UPDATE_REGION);
            }
            string update_region_id;
            string checkId;// 检测模型
            string checkName;//检查模型描述信息
            string errDesc;//错误详细信息
            string level;//错误等级，warn error
            string taskId;//任务号
            string boundId;//任务框号
            string dataKey;//数据KEY
            string flag;
            string sourceId;//源数据ID
            shared_ptr<DCCoord> coord;
            vector<shared_ptr<ErrNodeInfo>> errNodeInfo;
            string model_name;//数据模型
            string projectId;//项目ID
            string frameId;//任务框号
        };
        struct CheckItemInfo{
            CheckItemInfo(){

            }
            // 检测模型
            string checkId;
            //已经检查的数据数量
            int totalNum;
            //失败数量
            int failNum;
            //成功数量
            int successNum;

        };

        class CheckErrorOutput{

        public:
            CheckErrorOutput(int check_state);
            ~CheckErrorOutput();

            /**
             * 输出自定义文本信息
             * @param info 文本信息
             */
            void writeInfo(string info, bool bLongString = true);
            /**
             * 输出错误报表
             * @return
             */
            int saveErrorReport(string err_file_name);

            /**
             * 输出检查信息到sql中
             */
            int saveErrorToDb(const string &ouput_file);

            int saveJsonError(string err_file_name);
            void saveError(shared_ptr<DCError> error);

            /**
             * 添加检查项错误信息
             */
            void addCheckItemInfo(shared_ptr<CheckItemInfo> &checkItemInfo);
            /**
             * 添加检查项错误信息
             */
            void addCheckItemInfo(string checkId,int total);
            /**
             * 获取错误级别
             * @param check_model
             * @return
             */
            string get_error_level(string check_model);

            void calErrorTotal();

        private:
            void initErrorLevel();
            void outputErrorInfo();
        protected:
            map<string, vector<ErrorOutPut>> check_model_2_output_maps_;
            map<string,shared_ptr<CheckItemInfo>> check_total_;//统计检查项信息
            set<string> error_check_levels_;

            int check_state_ = 0;

            CppSQLite3::Database *p_db_out_ = nullptr;
        };
    }

}



#endif //AUTOHDMAP_DATACHECK_CHECKERROROUTPUT_H
