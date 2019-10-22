
#include <process/ModelCheckFunc.h>
#include "process/ModelRelationCheck.h"

namespace kd {
    namespace dc {


        string ModelRelationCheck::getId() {
            return id;
        }


        bool ModelRelationCheck::execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            //check SPIDX
            map<string, shared_ptr<DCModalData>> modeldatas = modelDataManager->modelDatas_;
            auto modeldataiter = modeldatas.begin();
            for (; modeldataiter != modeldatas.end(); modeldataiter++)
            {
                for (int i = 0; i < modeldataiter->second->records.size(); ++i)
                {
                    shared_ptr<DCModelRecord> modelrecord = modeldataiter->second->records[i];
                    auto iter = modelrecord->longDatas.find("SPIDX");
                    if (iter != modelrecord->longDatas.end())
                    {
                        string fieldname = iter->first;
                        long value = iter->second;
                        if (value < 0)
                        {
                            stringstream ss;
                            ss << "[Error] Relation Check model is :" << modeldataiter->first << " th " << i
                               << " records SPIDX is error";
                            errorOutput->writeInfo(ss.str());
                        }
                    }
                }
            }

            //cheeck relation
            map<string, shared_ptr<DCModelDefine>>::iterator iter = modelDataManager->modelDefines_.begin();
            for (; iter != modelDataManager->modelDefines_.end(); iter++)
            {
                string modelname = iter->first;
                shared_ptr<DCModelDefine> model = iter->second;
                int relnum = model->vecRelation.size();
                if (relnum > 0)
                {
                    for (int i = 0; i < relnum; i++)
                    {
                        shared_ptr<DCRelationDefine> relation = model->vecRelation[i];
                        string member = relation->member;
                        string rule = relation->rule;
                        shared_ptr<DCFieldDefine> field = model->getFieldDefine(rule);
                        if(nullptr == field) {
                            stringstream ss;
                            ss << "[Error] Relation Check model [" << modelname << "], rule :" << rule <<  "  is not exist";
                            errorOutput->writeInfo(ss.str());
                            continue;
                        }

                        DCFieldType type = field->type;
                        shared_ptr<DCModalData> modeldata = modelDataManager->getModelData(modelname);
                        if (modeldata == nullptr){
                            stringstream ss;
                            ss << "[Error] Relation Check model name :" << modelname <<  "  is not exist";
                            errorOutput->writeInfo(ss.str());
                            break;
                        }

                        for (int j = 0; j < modeldata->records.size(); j++)
                        {
                            shared_ptr<DCModelRecord> modelrec = modeldata->records[j];
                            if (type == DC_FIELD_TYPE_LONG)
                            {
                                bool ischeck = false;
                                auto reliter = modelrec->longDatas.find(rule);
                                if(reliter == modelrec->longDatas.end()){
                                    stringstream ss;
                                    ss << "[Error] Relation Check model name :" << member <<  ", rule: " << rule << "  is not exist";
                                    errorOutput->writeInfo(ss.str());
                                    continue;
                                }
                                string fieldname = reliter->first;
                                long value = reliter->second;
                                shared_ptr<DCModalData> relmodeldata = modelDataManager->getModelData(member);
                                if(relmodeldata == nullptr){
                                    stringstream ss;
                                    ss << "[Error] Relation Check model name :" << member <<  "  is not exist";
                                    errorOutput->writeInfo(ss.str());
                                    break;
                                }
                                for (int k = 0; k < relmodeldata->records.size(); k++)
                                {
                                    shared_ptr<DCModelRecord> relmodelrec = relmodeldata->records[k];
                                    auto reliter = relmodelrec->longDatas.find("ID");
                                    if (value == reliter->second)
                                    {
                                        ischeck = true;
                                        break;
                                    }
                                }
                                if (!ischeck)
                                {
                                    stringstream ss;
                                    ss << "[Error] Relation Check model is :" << modelname
                                       << " field is :"<< field->name << " th " << j
                                       << " records is not exist";
                                    errorOutput->writeInfo(ss.str());
                                }
                            }
                            else
                            {
                                errorOutput->writeInfo("field type is error !");
                            }
                        }
                    }
                }

                int tablerelnum = model->mapRelation.size();
                if (tablerelnum > 0)
                {
                    shared_ptr<DCModalData> modeldata = modelDataManager->getModelData(modelname);
                    if (modeldata == nullptr){
                        stringstream ss;
                        ss << "[Error] Relation Check model name :" << modelname <<  "  is not exist";
                        errorOutput->writeInfo(ss.str());
                        break;
                    }

                    for (int j = 0; j < modeldata->records.size(); j++)
                    {
                        shared_ptr<DCModelRecord> modelrec = modeldata->records[j];

                        for (int idxRel = 0; idxRel < tablerelnum; ++idxRel) {
                            stringstream ss;
                            pair<vector<DCRelField>, vector<DCRelField>>& prSrcDst = model->mapRelation[idxRel];

                            bool bSrcValid = true;
                            vector<DCRelField>& srcRelFlds = prSrcDst.first;
                            for (auto itrl : srcRelFlds){
                                DCRelField& rf = itrl;
                                rf.Table = (rf.Table.length()==0)?modelname:rf.Table;

                                int idxRecord = j;
                                if (rf.Table != modelname){
                                    idxRecord = -1;//getRecordIndex
                                }

                                shared_ptr<DCModelDefine> pModelDefine = modelDataManager->getModelDefine(rf.Table);
                                shared_ptr<DCModalData> pModelData = modelDataManager->getModelData(rf.Table);
                                if (pModelDefine == nullptr && pModelData == nullptr)
                                    continue;

                                shared_ptr<DCFieldDefine> pFieldDefine = pModelDefine->getFieldDefine(rf.Field);
                                if (pFieldDefine == nullptr)
                                    continue;

                                ss << "srclimit: " << rf.Field << " in '" << rf.Value << "', fieldvalue=";

                                if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_LONG){
                                    auto val = pModelData->records[idxRecord]->longDatas[rf.Field];
                                    bSrcValid &= IsValid<long>(rf.Value, val);
                                    ss << val;
                                } else if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_DOUBLE){
                                    auto val = pModelData->records[idxRecord]->doubleDatas[rf.Field];
                                    bSrcValid &= IsValid<double>(rf.Value, val);
                                    ss << val;
                                } else if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_DOUBLE){
                                    auto val = pModelData->records[idxRecord]->textDatas[rf.Field];
                                    bSrcValid &= IsValid<string>(rf.Value, val);
                                    ss << val;
                                }
                            }

                            if (!bSrcValid)
                                continue;

                            bool bDstValid = true;
                            vector<DCRelField>& dstRelFlds = prSrcDst.second;
                            for (auto itrl : dstRelFlds){
                                DCRelField& rf = itrl;
                                rf.Table = (rf.Table.length()==0)?modelname:rf.Table;

                                int idxRecord = j;
                                if (rf.Table != modelname){
                                    idxRecord = -1;//getRecordIndex
                                }

                                shared_ptr<DCModelDefine> pModelDefine = modelDataManager->getModelDefine(rf.Table);
                                shared_ptr<DCModalData> pModelData = modelDataManager->getModelData(rf.Table);
                                if (pModelDefine == nullptr && pModelData == nullptr)
                                    continue;

                                shared_ptr<DCFieldDefine> pFieldDefine = pModelDefine->getFieldDefine(rf.Field);
                                if (pFieldDefine == nullptr)
                                    continue;

                                ss << "; dstlimit: " << rf.Field << " in '" << rf.Value << "', fieldvalue=";
                                if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_LONG){
                                    auto val = pModelData->records[idxRecord]->longDatas[rf.Field];
                                    bDstValid &= IsValid<long>(rf.Value, val);
                                    ss << val;
                                } else if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_DOUBLE){
                                    auto val = pModelData->records[idxRecord]->doubleDatas[rf.Field];
                                    bDstValid &= IsValid<double>(rf.Value, val);
                                    ss << val;
                                } else if (pFieldDefine->type == DCFieldType::DC_FIELD_TYPE_DOUBLE){
                                    auto val = pModelData->records[idxRecord]->textDatas[rf.Field];
                                    bDstValid &= IsValid<string>(rf.Value, val);
                                    ss << val;
                                }
                            }

                            if (!bDstValid){
                                stringstream sss;
                                sss << "[Error] model relation is error:" << ss.str();
                                errorOutput->writeInfo(sss.str());
                            }
                        }
                    }
                }
            }
            return true;
        }

    }
}
