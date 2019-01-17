//
// Created by zhangxingang on 19-1-14.
//

#ifndef AUTOHDMAP_DATACHECK_MODELSQLCHECK_H
#define AUTOHDMAP_DATACHECK_MODELSQLCHECK_H

#include "IProcessor.h"
#include "data/DataManager.h"
#include "storage/CheckErrorOutput.h"
#include "util/StringUtil.h"

namespace kd {
    namespace dc {

#define SHEET_NAME_CHECKRESULT    "CheckResult"
#define SHEET_NAME_CONTENTS        "Contents"

        enum e_DataCheckKind {
            DCK_NONE = 0,
            DCK_ATTRIBUTE = 1,
            DCK_GEOMETRY = 2,
            DCK_TOPO = 3,
            DCK_LOGIC = 4,
            DCK_STATISTICS = 5,
            DCK_COMPARE = 6,
        };

        enum eCellFont {
            CF_DEFAULT = -1,
            CF_CELL = 0,
            CF_HEAD = 1,
            CF_FIELDS = 2,
            CF_LINK = 3,
        };

        enum eCellStyle {
            CS_DEFAULT = -1,
            CS_CELL = 0,
            CS_HEAD_LEVEL1 = 1,
            CS_HEAD_LEVEL2 = 2,
        };

        struct DataCheckItem_t {
            int ID;
            std::string Name;
            int Level;
            std::string Sql;
            std::string Info;
            e_DataCheckKind Kind;

            DataCheckItem_t() {
                ID = -1;
                Name = "";
                Level = 0;
                Sql = "";
                Info = "";
                Kind = e_DataCheckKind::DCK_NONE;
            }

            DataCheckItem_t(std::string name, std::string sql, std::string info = "", int id = 0, int level = 0,
                            int kind = 0) {
                ID = id;
                Name = name;
                Level = level;
                Sql = sql;
                Info = info;
                Kind = (e_DataCheckKind) kind;
            }
        };

        class ModelSqlCheck : public IProcessor {
        public:
            ModelSqlCheck(CppSQLite3::Database *p_db) {
                m_pdb = p_db;
            }

            ~ModelSqlCheck() {
            }

        public:
            string getId() override;

            bool execute(shared_ptr<CheckErrorOutput> errorOutput) override;

        public:
            void SetDB(CppSQLite3::Database *pdb);

            int AddCheckItem(DataCheckItem_t &item) {
                m_checkitems[item.Level].push_back(item);
                return m_checkitems.size();
            };

            bool GetCheckResults(int id, const std::string &strSql, std::map<int, std::string> &mFieldNameIndex,
                                 std::vector<std::vector<std::string>> &mCheckResults, vector<string> &details);

            bool LoadConfig();

            void SetCompare(bool bCompare = true) { m_bCompare = bCompare; };

        protected:

            std::string GetConfigFile();


        private:

            bool m_bCompare;

            CppSQLite3::Database *m_pdb;

            std::map<int, std::vector<DataCheckItem_t>> m_checkitems;//key:Type, value:Items

        private:
            const string id = "model_sql_check";
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_MODELSQLCHECK_H
