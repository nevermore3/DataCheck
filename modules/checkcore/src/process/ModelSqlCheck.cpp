//
// Created by zhangxingang on 19-1-14.
//

#include <process/ModelSqlCheck.h>
#include <DataCheckConfig.h>

#include "process/ModelSqlCheck.h"

namespace kd {
    namespace dc {
        string ModelSqlCheck::getId() {
            return id;
        }

        bool ModelSqlCheck::execute(shared_ptr<CheckErrorOutput> errorOutput) {
            // check data
            if (LoadConfig() && !m_checkitems.empty()) {
                for (auto cItem : m_checkitems) {
                    for (auto it : cItem.second) {
                        LOG(INFO) << "checking " << it.Name << " ......";
                        DataCheckItem_t &dcItem = it;

                        if (e_DataCheckKind::DCK_STATISTICS == dcItem.Kind && !m_bCompare)
                            continue;

                        std::map<int, std::string> mFieldNameIndex; //error fields info
                        std::vector<std::vector<std::string>> mCheckResults; //errorlist;
                        std::vector<string> details;
                        if (GetCheckResults(dcItem.ID, dcItem.Sql, mFieldNameIndex, mCheckResults, details)) {
                            for (const auto& detail : details) {
                                shared_ptr<DCSqlCheckError> sql_check_error = make_shared<DCSqlCheckError>(dcItem.ID);
                                sql_check_error->checkDesc_ = dcItem.Info;
                                sql_check_error->detail = detail;
                                errorOutput->saveError(sql_check_error);
                            }
                        }
                    }
                }
            } else {
                LOG(WARNING) << "Loading the config of datacheckitem is failed...";
                return false;
            }

            return true;
        }

        void ModelSqlCheck::SetDB(CppSQLite3::Database *pdb) {
            m_pdb = pdb;
        }

        bool GetReplaceFields(std::string strsql, std::vector<std::string> &vecReplaceField) {
            std::string::size_type pos = 0;
            std::string::size_type posl = strsql.find("{", pos);
            std::string::size_type posr = strsql.find("}", pos);
            while (posl != std::string::npos) {
                posr = strsql.find("}", posl + 1);
                if (posr != std::string::npos) {
                    std::string strField = strsql.substr(posl + 1, posr - posl - 1);
                    if (std::find(vecReplaceField.begin(), vecReplaceField.end(), strField) == vecReplaceField.end())
                        vecReplaceField.push_back(strField);
                }
                posl = strsql.find("{", posr + 1);
            }
            return true;
        }

        std::string GetReplaceSql(std::string strsql, std::vector<std::string> &vecReplaceField,
                                  std::map<std::string, std::string> &mFieldValues) {
            for (std::vector<std::string>::iterator it = vecReplaceField.begin(); it != vecReplaceField.end(); ++it) {
                std::string strField = *it;
                if (mFieldValues.find(StringUtil::Lower(strField)) != mFieldValues.end()) {
                    std::string strReplace = "{" + strField + "}";
                    StringUtil::Replace(strsql, strReplace, mFieldValues[StringUtil::Lower(strField)]);
                }
            }
            return strsql;
        }


        bool ModelSqlCheck::GetCheckResults(const std::string &id, const std::string &strSql,
                                            std::map<int, std::string> &mFieldNameIndex,
                                            std::vector<std::vector<std::string>> &mCheckResults,
                                            vector<string> &details) {

            bool bRet = false;

            if (m_pdb == nullptr || strSql.empty()) {
                LOG(ERROR) << "get check result failed! ";
                return bRet;
            }

            std::vector<std::string> vecSqls;
            StringUtil::Token_IgnoreQuotes(strSql.c_str(), ";", vecSqls);
            auto it = vecSqls.begin();
            while (it != vecSqls.end()) {
                if (it->empty()) {
                    it = vecSqls.erase(it);
                } else {
                    ++it;
                }
            }

            std::vector<std::map<std::string, std::string>> vecFieldValueRowsPrev;
            for (int idxSql = 0; idxSql < vecSqls.size(); ++idxSql) {
                std::string strsql = vecSqls[idxSql];

                std::vector<std::string> vecReplaceField;
                GetReplaceFields(strsql, vecReplaceField);

                std::vector<std::map<std::string, std::string>> vecFieldValueRows;
                int nCount = (vecReplaceField.size() == 0) ? 1 : vecFieldValueRowsPrev.size();

                for (int idxRow = 0; idxRow < nCount; ++idxRow) {
                    std::string strSqlActual = (vecReplaceField.size() == 0) ? strsql
                                                                             : GetReplaceSql(strsql, vecReplaceField,
                                                                                             vecFieldValueRowsPrev[idxRow]);

                    StringUtil::Replace(strSqlActual, "\n", " ");
                    StringUtil::Replace(strSqlActual, "\r", " ");
                    strSqlActual = StringUtil::TrimFRSpace(strSqlActual);
                    try {
                        if (0 == StringUtil::Lower(strSqlActual).find("select")) {
                            std::map<std::string, std::string> mFieldValues;
                            CppSQLite3::Query qry = m_pdb->execQuery(strSqlActual);
                            while (!qry.eof()) {
                                string detail;
                                std::vector<std::string> valFields;
                                int numFields = qry.numFields();
                                for (int idx = 0; idx < numFields; idx++) {
                                    std::string strFieldName = qry.fieldName(idx);
                                    const char *pval = qry.fieldValue(qry.fieldName(idx));
                                    std::string strFieldValue = (pval == NULL) ? "" : std::string(pval);
                                    valFields.push_back(strFieldValue);
                                    mFieldValues[StringUtil::Lower(strFieldName)] = strFieldValue;

                                    mFieldNameIndex[idx] = strFieldName;
                                    if (idxSql == vecSqls.size() - 1) {
                                        detail += strFieldName + ":" + strFieldValue + " ";
                                    }
                                }
                                vecFieldValueRows.push_back(mFieldValues);

                                if (idxSql == vecSqls.size() - 1) {
                                    mCheckResults.push_back(valFields);
                                    details.emplace_back(detail);
                                    bRet = true;
                                } else {
                                    mFieldNameIndex.clear();
                                }
                                qry.nextRow();
                            }
                            qry.finalize();
                        } else {
                            m_pdb->execDML(strSqlActual);
                        }
                    }
                    catch (CppSQLite3::Exception e) {
                        LOG(ERROR) << "GetCheckResults failed, error:" << e.errorMessage().c_str() << " sql:"
                                   << strSqlActual.c_str();
                    }
                    catch (std::exception ex) {
                        LOG(ERROR) << "GetCheckResults failed, error:" << ex.what() << " sql:" << strSqlActual.c_str();
                    }
                    catch (...) {
                        LOG(ERROR) << "GetCheckResults failed, sql:" << strSqlActual.c_str();
                    }
                }

                vecFieldValueRowsPrev.clear();
                vecFieldValueRowsPrev = vecFieldValueRows;
            }

            return bRet;
        }

        bool ModelSqlCheck::LoadConfig() {
            std::string strCFGFile;
            if (access(strCFGFile.c_str(), 0) != 0) {
                strCFGFile = GetConfigFile();
                if (access(strCFGFile.c_str(), 0) != 0)
                    return false;
            }

            TiXmlDocument *pXmlDoc = new TiXmlDocument(strCFGFile.c_str());
            if (!pXmlDoc->LoadFile())//TIXML_ENCODING_UTF8
                return false;

            TiXmlElement *pRootElement = pXmlDoc->RootElement(); // <datachecks>
            if (NULL == pRootElement)
                return false;

            TiXmlElement *dataCheckElement = pRootElement->FirstChildElement("datacheck");
            while (NULL != dataCheckElement) {
                std::string strDataCheckType = dataCheckElement->Attribute("TYPE");
                std::string strDataCheckName = dataCheckElement->Attribute("NAME");
                int dataCheckType = atoi(strDataCheckType.c_str());

                TiXmlElement *dataCheckItemElement = dataCheckElement->FirstChildElement("datacheckitem");
                while (NULL != dataCheckItemElement) {
                    const char *szId = dataCheckItemElement->Attribute("ID");
                    const char *szName = dataCheckItemElement->Attribute("NAME");
                    const char *szLevel = dataCheckItemElement->Attribute("LEVEL");
                    const char *szKind = dataCheckItemElement->Attribute("KIND");
                    const char *szInfo = dataCheckItemElement->Attribute("INFO");
                    const char *szSQL = dataCheckItemElement->Attribute("SQL");
                    dataCheckItemElement = dataCheckItemElement->NextSiblingElement();

                    std::string strId = (NULL == szId) ? std::string("-1") : szId;
                    std::string strName = (NULL == szName) ? std::string("") : szName;
                    std::string strLevel = (NULL == szLevel) ? std::string("0") : szLevel;
                    std::string strKind = (NULL == szKind) ? std::string("0") : szKind;
                    std::string strInfo = (NULL == szInfo) ? std::string("") : szInfo;
                    std::string strSQL = (NULL == szSQL) ? std::string("") : szSQL;
                    m_checkitems[dataCheckType].push_back(
                            DataCheckItem_t(strName, strSQL, strInfo, strId, atoi(strLevel.c_str()),
                                            atoi(strKind.c_str())));
                }

                dataCheckElement = dataCheckElement->NextSiblingElement();
            }

            delete pXmlDoc;
            pXmlDoc = NULL;
            return true;
        }

        std::string ModelSqlCheck::GetConfigFile() {
            return DataCheckConfig::getInstance().getProperty(DataCheckConfig::CONFIG_FILE_PATH) +
                   DataCheckConfig::getInstance().getProperty(DataCheckConfig::SQL_TASK_FILE);
        }
    }
}