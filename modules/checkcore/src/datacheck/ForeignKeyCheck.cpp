//
// Created by ubuntu on 2019/9/2.
//

#include "datacheck/ForeignKeyCheck.h"
#include <util/FileUtil.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"

#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include <cppsqlite3/Database.h>
using namespace kd::api;


namespace kd {
    namespace dc {

        ForeignKeyCheck::ForeignKeyCheck() {
            pDataBase = new CppSQLite3::Database();
        }

        ForeignKeyCheck::~ForeignKeyCheck() {
            if (pDataBase != nullptr) {
                pDataBase->close();
                delete pDataBase;
                pDataBase = nullptr;
            }
        }

        string ForeignKeyCheck::getId() {
            return id_;
        }

        void ForeignKeyCheck::CheckForeignKeyExist(shared_ptr<ModelDataManager> modelDataManager,
                                                   shared_ptr<CheckErrorOutput> errorOutput) {

            map<string, shared_ptr<DCModelDefine>>modelDefines = modelDataManager->modelDefines_;
            for (const auto &model : modelDefines) {
                string modelName = model.first;
                shared_ptr<DCModelDefine>modelDefine = model.second;
                vector<shared_ptr<DCRelationDefine>> relations = modelDefine->vecRelation;

                //没有外键 或者数据库中该表不存在
                if (relations.empty() || !pDataBase->tableExists(modelName)) {
                    continue;
                }

                for (const auto &relation : relations) {
                    string foreignKeyName = relation->rule;
                    if (!pDataBase->columnExists(modelName, foreignKeyName)) {
                         auto error = DCForeignKeyCheckError::createByKXS_01_026(modelName, foreignKeyName);
                         errorOutput->saveError(error);
                    }
                }
            }
        }

        void ForeignKeyCheck::CheckForeignKeyIntegrity(shared_ptr<ModelDataManager> modelDataManager,
                                                       shared_ptr<CheckErrorOutput> errorOutput) {
            const size_t MAXCMDLEN = 256;
            char sqlCmd[MAXCMDLEN] = {0};
            map<string, shared_ptr<DCModelDefine>>modelDefines = modelDataManager->modelDefines_;
            for (const auto &model : modelDefines) {
                string modelName = model.first;
                shared_ptr<DCModelDefine> modelDefine = model.second;
                vector<shared_ptr<DCRelationDefine>> relations = modelDefine->vecRelation;

                if (relations.empty()) {
                    continue;
                }
                for (const auto &relation : relations) {
                    string relationTableName = relation->member;
                    string relationFieldName = relation->name;
                    string foreignKeyName = relation->rule;
                    try {
                        memset(sqlCmd, 0, MAXCMDLEN);
                        snprintf(sqlCmd, MAXCMDLEN - 1,"select %s from %s where %s not in (select %s from %s)",
                                 foreignKeyName.c_str(),
                                 modelName.c_str(),
                                 foreignKeyName.c_str(),
                                 relationFieldName.c_str(),
                                 relationTableName.c_str());

                        CppSQLite3::Query query = pDataBase->execQuery(sqlCmd);
                        while (!query.eof()) {

                            int value = query.getInt64Field(foreignKeyName);
                            string strValue = to_string(value);
                            auto error = DCForeignKeyCheckError::createByKXS_01_027(modelName,
                                                                                    foreignKeyName,
                                                                                    strValue,
                                                                                    relationTableName,
                                                                                    relationFieldName);
                            errorOutput->saveError(error);

                            query.nextRow();
                        }
                        query.finalize();
                    } catch (CppSQLite3::Exception &e) {
                        LOG(ERROR) << "Error:" << e.errorMessage().c_str();
                    }
                }
            }

        }

        bool ForeignKeyCheck::execute(shared_ptr<ModelDataManager> modelDataManager,
                                      shared_ptr<CheckErrorOutput> errorOutput) {

            try {
                if (pDataBase != nullptr) {
                    string dbFile = DataCheckConfig::getInstance().getProperty(DataCheckConfig::DB_INPUT_FILE);
                    pDataBase->open(dbFile);
                } else {
                    LOG(ERROR) << "Open DataBase Error";
                    return false;
                }
                //检查外键是否存在
                CheckForeignKeyExist(modelDataManager, errorOutput);
                //外键数据完备性检查
                CheckForeignKeyIntegrity(modelDataManager, errorOutput);

            } catch (CppSQLite3::Exception &e) {
                LOG(ERROR) << e.errorMessage().c_str();
                return false;
            }
            return true;
        }
    }
}

