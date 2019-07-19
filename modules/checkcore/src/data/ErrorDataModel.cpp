//
// Created by gaoyanhong on 2018/3/29.
//

#include <data/ErrorDataModel.h>
#include "data/DataManager.h"
#include "DataCheckConfig.h"

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCkError
        /////////////////////////////////////////////////////////////////////////////////////////

        DCError::DCError(string checkModel) {
            checkId = checkModel;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCAttCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCAttCheckError::DCAttCheckError(string checkModel) : DCError(checkModel) {

        }

        DCAttCheckError::DCAttCheckError(string checkModel, string modelName, string fieldName, string recordId) :
                DCError(checkModel) {
            modelName_ = modelName;
            fieldName_ = fieldName;
            recordId_ = recordId;
        }

        string DCAttCheckError::toString() {
            stringstream ss;
            ss << "\"" << checkId << ":" << checkName << "\"," << modelName_ << "," << fieldName_ << ","
               << recordId_ << "," << errorDesc_;
            return ss.str();
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCRelationCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCRelationCheckError::DCRelationCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCRelationCheckError::toString() {
            return detail_;
        }

        shared_ptr<DCRelationCheckError>
        DCRelationCheckError::createByKXS_01_25(string model_name, string field,
                                                string relation_name) {
            shared_ptr<DCRelationCheckError> error = make_shared<DCRelationCheckError>(CHECK_ITEM_KXS_ORG_025);
            error->checkName = "字段关系检查";
            stringstream ss;
            ss << model_name << "字段" << field << "在" << relation_name << "不存在";
            error->detail_ = ss.str();

            return error;
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCDividerCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCDividerCheckError::DCDividerCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCDividerCheckError::toString() {
            stringstream ss;
            ss << errorDesc_;
            return ss.str();
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByAtt(string checkModel, shared_ptr<DCDivider> div,
                                         shared_ptr<DCDividerAttribute> att) {

            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            if (div == nullptr)
                return error;
            error->dividerId_ = div->id_;
            error->sourceId = div->id_;
            if (att != nullptr) {
                error->nodeId_ = att->dividerNode_->id_;
                error->attId_ = att->id_;
                error->coord = att->dividerNode_->coord_;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(att->dividerNode_->coord_);
                errNodeInfo->dataId = att->id_;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                errNodeInfo -> dataType = DATA_TYPE_WAY;
                error->errNodeInfo.emplace_back(errNodeInfo);
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->coord = make_shared<DCCoord>();
                error->coord->lng_=0;
                error->coord->lat_=0;
                error->coord->z_=0;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                errNodeInfo -> dataType = DATA_TYPE_WAY;
                error->errNodeInfo.emplace_back(errNodeInfo);
            }
            error->taskId_ = div->task_id_;
            error->flag = div->flag_;
            error->dataKey_ = DATA_TYPE_LANE+div->task_id_+DATA_TYPE_LAST_NUM;
            return error;
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByNode(string checkModel, shared_ptr<DCDivider> div,
                                          shared_ptr<DCDividerNode> node) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            if (div == nullptr)
                return error;

            error->dividerId_ = div->id_;
            error->sourceId = div -> id_;
            if (node != nullptr) {
                error->nodeId_ = node->id_;
                error->attId_ = "";
                error->coord = node->coord_;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(node->coord_);
                errNodeInfo->dataId =  node->id_;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->coord = make_shared<DCCoord>();
                error->coord->lng_=0;
                error->coord->lat_=0;
                error->coord->z_=0;
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
                errNodeInfo->dataId =  "";
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
            }
            error->taskId_ = div->task_id_;
            error->flag = div->flag_;
            error->dataKey_ = DATA_TYPE_LANE+div->task_id_+DATA_TYPE_LAST_NUM;
            return error;
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByNode(string checkModel, string nodeId, double lng, double lat, double z) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            error->dividerId_ = "";
            error->nodeId_ = nodeId;
            error->attId_ = "";
            error->coord = make_shared<DCCoord>();
            error->coord->lng_ = lng;
            error->coord->lat_ = lat;
            error->coord->z_ = z;

            return error;
        }

        shared_ptr<DCDividerCheckError> DCDividerCheckError::createByKXS_01_011(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(CHECK_ITEM_KXS_ORG_011);
            error->checkName = "车道线不平滑夹角";
//            error->coord = make_shared<DCCoord>();
            error->coord = ptr_error_nodes[0]->ptr_current_coord;
            error->detail_ += "divider:";
            error->detail_ += divider_id;
            error->sourceId = divider_id;
            for (const auto &error_node : ptr_error_nodes) {
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
                errNodeInfo->lng_ = error_node->ptr_current_coord->lng_;
                errNodeInfo->lat_ = error_node->ptr_current_coord->lat_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->lng_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->lat_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->z_);
                error->detail_ += ")";
                error->detail_ += "前后点角度：";
                error->detail_ += to_string(error_node->angle);
            }

            return error;
        }

        shared_ptr<DCDividerCheckError> DCDividerCheckError::createByKXS_01_012(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(CHECK_ITEM_KXS_ORG_012);
            error->checkName = "存在长度小于0.2米的弧段";
            error->detail_ += "divider_id:";
            error->detail_ += divider_id;
            error->coord = ptr_error_nodes[0]->ptr_current_coord;
            error->sourceId = divider_id;
            for (const auto &error_node : ptr_error_nodes) {
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
                errNodeInfo->lng_ = error_node->ptr_current_coord->lng_;
                errNodeInfo->lat_ = error_node->ptr_current_coord->lat_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->previous);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_previous_coord->lng_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->lat_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->z_);
                error->detail_ += ")";
                error->detail_ += "与索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->lng_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->lat_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->z_);
                error->detail_ += ")";
                error->detail_ += "距离：";
                error->detail_ += to_string(error_node->distance);
            }

            return error;
        }

        shared_ptr<DCDividerCheckError> DCDividerCheckError::createByKXS_01_013(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(CHECK_ITEM_KXS_ORG_013);
            error->checkName = "车道线高程突变";
            error->detail_ += "divider_id:";
            error->detail_ += divider_id;
            error->coord = ptr_error_nodes[0]->ptr_current_coord;
            error->sourceId = divider_id;
            for (const auto &error_node : ptr_error_nodes) {
                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
                errNodeInfo->lng_ = error_node->ptr_current_coord->lng_;
                errNodeInfo->lat_ = error_node->ptr_current_coord->lat_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->previous);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_previous_coord->lng_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->lat_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->z_);
                error->detail_ += ")";
                error->detail_ += "与索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->lng_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->lat_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->z_);
                error->detail_ += ")";
                error->detail_ += ",高度：";
                error->detail_ += to_string(error_node->height);
                error->detail_ += ",距离：";
                error->detail_ += to_string(error_node->distance);
            }
            return error;
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneCheckError::DCLaneCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCLaneCheckError::toString() {
            stringstream ss;
            ss << errorDesc_;
            return ss.str();
        }

        shared_ptr<DCLaneCheckError>
        DCLaneCheckError::createByAtt(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCLaneAttribute> att) {

            shared_ptr<DCLaneCheckError> error = make_shared<DCLaneCheckError>(checkModel);
            if (lane == nullptr)
                return error;

            error->laneId_ = lane->id_;
            if (lane->leftDivider_)
                error->leftDividerId_ = lane->leftDivider_->id_;
            else
                error->leftDividerId_ = "";

            if (lane->rightDivider_)
                error->rightDividerId_ = lane->rightDivider_->id_;
            else
                error->rightDividerId_ = "";


            if (att != nullptr) {
                error->nodeId_ = att->dividerNode_->id_;
                error->attId_ = att->id_;
                error->coord = att->dividerNode_->coord_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->coord = make_shared<DCCoord>();
                error->coord->lng_=0;
                error->coord->lat_=0;
                error->coord->z_=0;
            }

            error->taskId_ = lane->task_id_;
            error->flag = lane->flag_;
            error->dataKey_ = DATA_TYPE_LANE+lane->task_id_+DATA_TYPE_LAST_NUM;

            return error;
        }

        shared_ptr<DCLaneCheckError>
        DCLaneCheckError::createByNode(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCDividerNode> node) {
            shared_ptr<DCLaneCheckError> error = make_shared<DCLaneCheckError>(checkModel);
            if (lane == nullptr)
                return error;

            error->laneId_ = lane->id_;
            if (lane->leftDivider_)
                error->leftDividerId_ = lane->leftDivider_->id_;
            else
                error->leftDividerId_ = "";

            if (lane->rightDivider_)
                error->rightDividerId_ = lane->rightDivider_->id_;
            else
                error->rightDividerId_ = "";

            if (node != nullptr) {
                error->nodeId_ = node->id_;
                error->attId_ = "";
                error->coord = node->coord_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->coord = make_shared<DCCoord>();
                error->coord->lng_=0;
                error->coord->lat_=0;
                error->coord->z_=0;
            }

            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCSqlCheckError::DCSqlCheckError(string checkModel) : DCError(checkModel) {

        }

        DCSqlCheckError::DCSqlCheckError(string checkModel, string modelName, string fieldName, string recordId) :
                DCError(checkModel) {
            modelName_ = modelName;
            fieldName_ = fieldName;
            recordId_ = recordId;
        }

        string DCSqlCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneGroupCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneGroupCheckError::DCLaneGroupCheckError(const string &checkModel) : DCError(checkModel) {

        }

        string DCLaneGroupCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long s_index, long e_index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_005);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的范围是" +
                            to_string(s_index) + "," + to_string(e_index) + "," +
                            "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_005);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的节点" +
                            to_string(index) + "," + "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_006(string road_id, string lg1,
                                                                                    long s_index1, long e_index1,
                                                                                    string lg2, long s_index2,
                                                                                    long e_index2, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_006);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "ValidityRange之间不重叠或交叉";
            error->detail = "roadid:" + road_id + "上的车道组关联关系有交叉." +
                            lg1 + ":" + to_string(s_index1) + "," + to_string(e_index1) + "," +
                            lg2 + ":" + to_string(s_index2) + "," + to_string(e_index2) + "," +
                            "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_004(string divider_id,
                                                                                    set<string> lane_groups) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_004);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "一条普通车道线存在于多个车道组中。两个车道组共用的双向车道线除外";
            error->detail += "divider_id:";
            error->detail += divider_id;
            error->detail += "存在于分组";
            error->sourceId = divider_id;
            for (const auto &lg : lane_groups) {
                error->detail += lg;
                error->detail += " ";
            }
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
            errNodeInfo->lng_ = 0;
            errNodeInfo->lat_ = 0;
            errNodeInfo->z_ = 0;
            errNodeInfo->dataId = "";
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_002(string lane_group_id) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_002);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "分组中车道编号从零开始，从内向外递增。编号连续，不缺失，不重复。";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "车道编号异常";

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_001(string lane_group_id, const vector<string> &dividers) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_001);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "同一个车道组内，单根车道线的长度同组内车道线平均长度不应该偏差超过";
            error->checkName += to_string((int) (DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_LENGTH_RATIO) * 100));
            error->checkName += "%以上。";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "的divider长度异常：";
            for (const auto &div : dividers) {
                error->detail += div;
                error->detail += " ";

                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
                errNodeInfo->lng_ = 0;
                errNodeInfo->lat_ = 0;
                errNodeInfo->z_ = 0;
                errNodeInfo->dataId = div;
                errNodeInfo->dataType = DATA_TYPE_RELATION;
                errNodeInfo->dataLayer = MODEL_NAME_R_DIVIDER_DREF;
                error->errNodeInfo.emplace_back(errNodeInfo);

            }
            error->sourceId = lane_group_id;

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_027(string lane_group_id) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_027);
            error->checkName = "车道组没有打断，不应该存在既是入口又是出口的组";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "车道组没有打断";

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_003( shared_ptr<DCDivider> div) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_003);
            error->checkName = "车道线不存在于车道组中";
            error->detail += "divider_id:";
            error->detail += std::to_string(div->dividerNo_);
            error->detail += "是孤立的";
            error->taskId_ = div->task_id_;
            error->flag = div->flag_;
            error->dataKey_ = DATA_TYPE_LANE+div->task_id_+DATA_TYPE_LAST_NUM;
            error->coord = make_shared<DCCoord>();
            error->sourceId = div->id_;
            shared_ptr<ErrNodeInfo> errNodeInfo;
            if(div->nodes_.size()>0) {
                error->coord = div->nodes_[0]->coord_;
                errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
                errNodeInfo->dataId = div->nodes_[0]->id_;
            }else{
                error->coord = make_shared<DCCoord>();
                error->coord->lng_ = 0;
                error->coord->lat_ = 0;
                error->coord->z_ = 0;
                errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
                errNodeInfo->dataId="";
            }
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneGroupTopoCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneGroupTopoCheckError::DCLaneGroupTopoCheckError(const string &checkModel) : DCError(checkModel) {

        }

        string DCLaneGroupTopoCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_04_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>(
                    CHECK_ITEM_KXS_ROAD_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "若车道组之间连通，则其内部的道路必然连通。";
            error->detail += "lane group id:";
            error->detail += lg_id1;
            error->detail += ",lane group id:";
            error->detail += lg_id2;
            error->detail += "道路不连通";

            return error;
        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_05_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>(
                    CHECK_ITEM_KXS_LANE_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "如果车道组之间是“封闭”的，则车道组之间的车道不会存在孤立的车道。";
            error->detail += "lane group id:";
            error->detail += lg_id1;
            error->detail += ",lane group id:";
            error->detail += lg_id2;
            error->detail += "车道中心线不连通";

            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCRoadCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCRoadCheckError::DCRoadCheckError(const string &checkModel) : DCError(checkModel) {

        }

        string DCRoadCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_002(const string &road_id,
                                                                          const string &lane_group_id) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_002);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "道路与车道组的两个车道边缘线无交叉点。双向道路除外。";
            error->detail += "road_id:";
            error->detail += road_id;
            error->detail += ",lane group id:";
            error->detail += lane_group_id;
            error->detail += "边缘线有交叉";

            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_003(const string &road_id,
                                                                          vector<NodeCheck> &error_index_pair) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_003);
            error->checkName = "道路高程突变>±10厘米/米。";
            error->detail += "road_id:";
            error->detail += road_id;
            error->detail += ",结点索引:";
            for (auto index_pair : error_index_pair) {
                error->detail += to_string(index_pair.pre_index);
                error->detail += ",";
                error->detail += to_string(index_pair.index);
                error->detail += ",高度差：";
                error->detail += to_string(index_pair.diff_height);
                error->detail += ",距离：";
                error->detail += to_string(index_pair.distance);
                error->detail += " ";
            }

            return error;
        }

        shared_ptr<DCRoadCheckError>
        DCRoadCheckError::createByKXS_04_006(const string &road_id,
                                             const vector<shared_ptr<NodeError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_006);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "道路前后结点重复,前后点xy坐标相同（z值可能相同）";
            error->detail += "road_id:";
            error->detail += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail += ",索引点";
                error->detail += to_string(error_node->index);
                error->detail += ",坐标(";
                error->detail += to_string(error_node->ptr_coord->lng_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_coord->lat_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_coord->z_);
                error->detail += ")";
            }
            error->detail += "重复";

            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_007(const string &road_id,
                                                                          const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_007);
            error->checkName = "道路结点出现拐点，或者角度过大";
            error->detail += "road_id:";
            error->detail += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail += ",索引点";
                error->detail += to_string(error_node->current);
                error->detail += ",坐标(";
                error->detail += to_string(error_node->ptr_current_coord->lng_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_current_coord->lat_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_current_coord->z_);
                error->detail += ")";
                error->detail += "前后点角度：";
                error->detail += to_string(error_node->angle);
            }

            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_008(const string &road_id,
                                                                          const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_008);
            error->checkName = "道路形态点间距过近";
            error->detail += "road_id:";
            error->detail += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail += ",索引点";
                error->detail += to_string(error_node->previous);
                error->detail += ",坐标(";
                error->detail += to_string(error_node->ptr_previous_coord->lng_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_previous_coord->lat_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_previous_coord->z_);
                error->detail += ")";
                error->detail += "与索引点";
                error->detail += to_string(error_node->current);
                error->detail += ",坐标(";
                error->detail += to_string(error_node->ptr_current_coord->lng_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_current_coord->lat_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_current_coord->z_);
                error->detail += ")";
                error->detail += "距离：";
                error->detail += to_string(error_node->distance);
            }
            error->detail += "距离过近";

            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneError::DCLaneError(const string &checkModel) : DCError(checkModel) {

        }

        string DCLaneError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_002(const string &lane_id,
                                                                const string &divider_id) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_002);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "车道中心线与车道线在5米内不能存在两个交点（只检查组内车道线有共点的做检查）。";
            error->detail += "lane_id:";
            error->detail += lane_id;
            error->detail += ",divider_id:";
            error->detail += divider_id;
            error->detail += "在";
            error->detail += to_string(
                    DataCheckConfig::getInstance().getPropertyI(DataCheckConfig::LANE_INTERSECT_LENGTH));
            error->detail += "米内存在两个或存在多于2个交点";

            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_003(const string &lane_id,
                                                                const string &divider_id) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_003);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "车道中心线与本组的车道边缘线无交叉点。";
            error->detail += "lane_id:";
            error->detail += lane_id;
            error->detail += ",divider_id:";
            error->detail += divider_id;
            error->detail += "边缘线有交点";

            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_015(const string &lane_id1, const string &lane_id2) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_015);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "检查同组内中心线与中心线是否存在交叉问题（组内有共点的中心线做检查）。";
            error->detail += "lane_id:";
            error->detail += lane_id1;
            error->detail += ",lane_id:";
            error->detail += lane_id2;
            error->detail += "有交点";

            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_016(const string &lane_id,
                                                                const vector<shared_ptr<NodeError>> &ptr_error_nodes) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_016);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "车道中心线前后结点重复,前后点xy坐标相同（z值可能相同）";
            error->detail += "lane_id:";
            error->detail += lane_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail += ",索引点";
                error->detail += to_string(error_node->index);
                error->detail += ",坐标(";
                error->detail += to_string(error_node->ptr_coord->lng_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_coord->lat_);
                error->detail += ",";
                error->detail += to_string(error_node->ptr_coord->z_);
                error->detail += ")";
            }
            error->detail += "重复";

            return error;
        }

        DCAdasError::DCAdasError(const string &checkModel) : DCError(checkModel) {

        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_001(long road_id, string f_adas_node_id,
                                                                string t_adas_node_id, double distance) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_001);
            error->checkName = "相邻属性点间的距离不等于1米，除去与终点距离可能小于1米。";
            error->detail += "road_id:";
            error->detail += std::to_string(road_id);
            error->detail += ",属性点adas_node_id:";
            error->detail += f_adas_node_id;
            error->detail += ",";
            error->detail += t_adas_node_id;
            error->detail += "距离:";
            error->detail += to_string(distance);
            error->detail += "米";


            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_002(long road_id, const shared_ptr<DCCoord> &ptr_coord,
                                                                long index, double distance) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_002);
            error->checkName = "属性点与拟合曲线属性点的距离不大于10厘米。";
            error->detail += "road_id:";
            error->detail += std::to_string(road_id);
            error->detail += ",属性点索引:";
            error->detail += to_string(index);
            error->detail += ",点坐标:";
            error->detail += to_string(ptr_coord->lng_);
            error->detail += ",";
            error->detail += to_string(ptr_coord->lat_);
            error->detail += ",";
            error->detail += to_string(ptr_coord->z_);
            error->detail += ",与拟合曲线属性点距离:";
            error->detail += to_string(distance);
            error->detail += "米。";

            return error;
        }

        string DCAdasError::toString() {
            return detail;
        }

        DCFieldError::DCFieldError(const string &checkModel) : DCError(checkModel) {

        }

        string DCFieldError::toString() {
            return detail;
        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_019(const string &detail) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>(CHECK_ITEM_KXS_ORG_019);
            error->checkName = "字段范围检查";
            error->detail = detail;

            return error;
        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_020(const string &file) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>(CHECK_ITEM_KXS_ORG_020);
            error->checkName = "缺少数据文件，或者数据为空";
            error->detail = file;

            return error;
        }

        bool DCFieldError::check_file(const string &file) {
            return file == "HD_DIVIDER" || file == "HD_DIVIDER_ATTRIBUTE" || file == "HD_DIVIDER_NODE" ||
                   file == "HD_LANE"  ||
                   file == "HD_LANE_GROUP" || file == "HD_R_LANE_GROUP" || file == "ROAD" ||
                   file == "ROAD_NODE";
        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_024(const string &type,
                                                                  const string &id, const set<long> &index) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>(CHECK_ITEM_KXS_ORG_024);
            error->checkName = "结点坐标有效性检查";
            error->detail = type;
            error->detail += ":";
            error->detail += id;
            error->sourceId = id;
            for (auto idx : index) {
                error->detail += ",索引点";
                error->detail += to_string(idx);
            }

            return error;
        }


    }
}

