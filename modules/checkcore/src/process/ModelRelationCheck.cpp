//
// Created by gaoyanhong on 2018/3/29.
//

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
                        long nodetype = (modelrec->longDatas.find("NODE_TYPE"))->second;
                        auto tablereliter = model->mapRelation.find(nodetype);
                        if (tablereliter != model->mapRelation.end())
                        {
                            bool ischeck = false;
                            string relmodelname = tablereliter->second.begin()->first;
                            string field = tablereliter->second.begin()->second;
                            long fieldvalue = (modelrec->longDatas.find(field))->second;
                            shared_ptr<DCModalData> relmodeldata = modelDataManager->getModelData(relmodelname);
                            if(relmodeldata == nullptr){
                                stringstream ss;
                                ss << "[Error] Relation Check model name :" << relmodelname <<  "  is not exist";
                                errorOutput->writeInfo(ss.str());
                                break;
                            }
                            for (int k = 0; k < relmodeldata->records.size(); k++)
                            {
                                shared_ptr<DCModelRecord> relmodelrec = relmodeldata->records[k];
                                auto reliter = relmodelrec->longDatas.find("ID");
                                long id =reliter->second;
                                if (fieldvalue == reliter->second)
                                {
                                    ischeck = true;
                                    break;
                                }
                            }
                            if (!ischeck)
                            {
                                stringstream ss;
                                ss << "[Error] Relation Check model is :" << modelname
                                   <<"field is :"<< field << " th " << j
                                   << " records is not exist";
                                errorOutput->writeInfo(ss.str());
                            }

                        }
                    }
                }
            }
            return true;
        }

    }
}
