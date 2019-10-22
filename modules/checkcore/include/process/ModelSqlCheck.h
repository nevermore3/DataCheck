

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
            std::string ID;
            std::string Name;
            std::string Level;
            std::string Sql;
            std::string Info;
            e_DataCheckKind Kind;

            DataCheckItem_t() {
                ID = "";
                Name = "";
                Level = "";
                Sql = "";
                Info = "";
                Kind = e_DataCheckKind::DCK_NONE;
            }

            DataCheckItem_t(std::string name, std::string sql, std::string info = "", string id = "", string level = "",
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
            ModelSqlCheck() {
                m_pdb = new CppSQLite3::Database();
            }

            ~ModelSqlCheck() {
                if (m_pdb) {
                    m_pdb->close();
                    delete m_pdb;
                    m_pdb = nullptr;
                }
            }

        public:
            string getId() override;

            bool execute(shared_ptr<CheckErrorOutput> errorOutput) override;

        public:

            bool GetCheckResults(const std::string &id, const std::string &strSql, std::map<int, std::string> &mFieldNameIndex,
                                 std::vector<std::vector<std::string>> &mCheckResults, vector<string> &details);

            bool LoadConfig();

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
