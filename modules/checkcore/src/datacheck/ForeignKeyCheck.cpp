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
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_026;
            size_t total = 0;
            map<string, shared_ptr<DCModelDefine>>modelDefines = modelDataManager->modelDefines_;
            for (const auto &model : modelDefines) {
                total++;
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
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }

        bool ForeignKeyCheck::CheckForeignKey(shared_ptr<ModelDataManager> modelDataManager,
                                              shared_ptr<CheckErrorOutput> errorOutput, string modelName) {
            const size_t MAXCMDLEN = 256;
            char sqlCmd[MAXCMDLEN] = {0};
            if (modelName == "HD_TOPO_LANEGROUP") {
                //在lanetopocheck 里进行检查
                return true;
            }
            if (modelName == "HD_R_LO_ROAD" || modelName == "HD_R_LO_LANE") {
                // HD_R_LO_ROAD 和 HD_R_LO_LANE中相关联的表单独考虑
                map<string, long> tables {{"HD_POLYGON", 1},
                                          {"HD_TRAFFIC_LIGHT", 5}};
                for (const auto &table : tables) {
                    memset(sqlCmd, 0, MAXCMDLEN);
                    snprintf(sqlCmd, MAXCMDLEN - 1, "select a.LO_ID from %s a, %s b "\
                                                                    "where a.TYPE = %ld and a.LO_ID not in "\
                                                                    "(select ID from %s)",
                                                                    modelName.c_str(),
                                                                    table.first.c_str(),
                                                                    table.second,
                                                                    table.first.c_str());
                    LOG(INFO)<<"SQL："<<sqlCmd;
                    CppSQLite3::Query query = pDataBase->execQuery(sqlCmd);
                    string foreignKeyName = "LO_ID";
                    string keyName = "ID";
                    string foreignTable = table.first;
                    while (!query.eof()) {
                        int value = query.getInt64Field(foreignKeyName);
                        string strValue = to_string(value);
                        auto error = DCForeignKeyCheckError::createByKXS_01_027(modelName,
                                                                                foreignKeyName,
                                                                                strValue,
                                                                                foreignTable,
                                                                                keyName);
                        errorOutput->saveError(error);
                        query.nextRow();
                    }
                    query.finalize();
                }
                // 还有一项检查，所以 不返回true
            }
            return false;
        }

        void ForeignKeyCheck::CheckForeignKeyIntegrity(shared_ptr<ModelDataManager> modelDataManager,
                                                       shared_ptr<CheckErrorOutput> errorOutput) {
            map<string, string>whiteList {{"HD_LANE_CONNECTIVITY", "NODE_ID"},
                                          {"ROAD_NODE", "C_NODE_ID"}};
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_ORG_027;
            size_t total = 0;

            const size_t MAXCMDLEN = 256;
            char sqlCmd[MAXCMDLEN] = {0};
            map<string, shared_ptr<DCModelDefine>>modelDefines = modelDataManager->modelDefines_;
            for (const auto &model : modelDefines) {
                total++;
                string modelName = model.first;
                shared_ptr<DCModelDefine> modelDefine = model.second;
                vector<shared_ptr<DCRelationDefine>> relations = modelDefine->vecRelation;

                if (relations.empty()) {
                    continue;
                }
                if (CheckForeignKey(modelDataManager, errorOutput, modelName)) {
                    continue;
                }
                for (const auto &relation : relations) {
                    string relationTableName = relation->member;
                    string relationFieldName = relation->name;
                    string foreignKeyName = relation->rule;
                    if (whiteList.find(modelName) != whiteList.end() && whiteList[modelName] == foreignKeyName) {
                        continue;
                    }
                    try {
                        memset(sqlCmd, 0, MAXCMDLEN);
                        snprintf(sqlCmd, MAXCMDLEN - 1,"select %s from %s where %s not in (select %s from %s);",
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
            checkItemInfo->totalNum = total;
            errorOutput->addCheckItemInfo(checkItemInfo);
        }
        void ForeignKeyCheck::PreCheck(){
            string sqlCmd = "create index HD_R_LO_ROAD_TYPE_INDEX on HD_R_LO_ROAD(TYPE);";
            LOG(INFO)<<"SQL："<<sqlCmd;
            pDataBase->execQuery(sqlCmd);
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
                PreCheck();
                LOG(INFO) << "PreCheck finished!";
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

