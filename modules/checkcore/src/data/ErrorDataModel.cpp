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

        string DCError::toString() {
            return detail_;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // SplitCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        SplitCheckError::SplitCheckError(string checkId_) :DCError( checkId_) {

        }

        string SplitCheckError::toString() {
            stringstream ss;
            ss << "\"" << checkId << ":" << checkName<<"\","<<detail_;
            return ss.str();
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
                error->coord->x_=0;
                error->coord->y_=0;
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
                error->coord->x_=0;
                error->coord->y_=0;
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
            error->coord->x_ = lng;
            error->coord->y_ = lat;
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
                errNodeInfo->x_ = error_node->ptr_current_coord->x_;
                errNodeInfo->y_ = error_node->ptr_current_coord->y_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->y_);
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
                errNodeInfo->x_ = error_node->ptr_current_coord->x_;
                errNodeInfo->y_ = error_node->ptr_current_coord->y_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->previous);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_previous_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->z_);
                error->detail_ += ")";
                error->detail_ += "与索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->y_);
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
                errNodeInfo->x_ = error_node->ptr_current_coord->x_;
                errNodeInfo->y_ = error_node->ptr_current_coord->y_;
                errNodeInfo->z_ = error_node->ptr_current_coord->z_;
                errNodeInfo->dataId = error_node->id;
                errNodeInfo->dataType = DATA_TYPE_WAY;
                errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
                error->errNodeInfo.emplace_back(errNodeInfo);
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->previous);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_previous_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->z_);
                error->detail_ += ")";
                error->detail_ += "与索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->y_);
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

        shared_ptr<DCDividerCheckError> DCDividerCheckError::createByKXS_01_030(long dividerID, long index,
                                                                                const shared_ptr<DCCoord> &coord,
                                                                                int level){
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(CHECK_ITEM_KXS_ORG_030);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "Divider点完备性检查.";
            error->detail_ += "Divider ID:";
            error->detail_ += std::to_string(dividerID);
            error->detail_ += ",属性点索引:";
            error->detail_ += to_string(index);
            if (level == 1) {
                error->detail_ += ",Divider节点周围1.5米内找不到HD_DIVIDER_SCH";
            } else if (level == 2) {
                error->detail_ += ",车道线起点和终点之处缺失HD_DIVIDER_SCH";
            }
            error->coord = coord;
            return error;
        }


        shared_ptr<DCDividerCheckError> DCDividerCheckError::createByKXS_01_029(long nodeID,
                                                                                shared_ptr<DCCoord> coord) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(CHECK_ITEM_KXS_ORG_029);
            error->checkName = "DIVIDER_SCH与关联DIVIDER距离检查。";
            error->detail_ += "DIVIDER_SCH ID:";
            error->detail_ += std::to_string(nodeID);
            error->detail_ += ",DIVIDER_SCH点离DIVIDER的垂直距离超过10cm";

            error->coord = coord;
            return error;

        }
        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneCheckError::DCLaneCheckError(string checkModel) : DCError(checkModel) {

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
                error->coord->x_=0;
                error->coord->y_=0;
                error->coord->z_=0;
            }

            error->taskId_ = lane->task_id_;
            error->flag = lane->flag_;
            error->dataKey_ = DATA_TYPE_LANE+lane->task_id_+DATA_TYPE_LAST_NUM;

            return error;
        }
        shared_ptr<DCLaneCheckError>
        DCLaneCheckError::createByKXS_05_008(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCDivider> leftDiv, shared_ptr<DCDivider> rightDiv) {

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
            error->sourceId = lane->id_;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(leftDiv->nodes_[0]->coord_);
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
            errNodeInfo->dataId = leftDiv->id_;
            error->errNodeInfo.emplace_back(errNodeInfo);

            shared_ptr<ErrNodeInfo> errNodeInfo1 = make_shared<ErrNodeInfo>(rightDiv->nodes_[0]->coord_);
            errNodeInfo1->dataType = DATA_TYPE_WAY;
            errNodeInfo1->dataLayer = MODEL_NAME_DIVIDER;
            errNodeInfo1->dataId = rightDiv->id_;
            error->errNodeInfo.emplace_back(errNodeInfo1);
            error->coord = leftDiv->nodes_[0]->coord_;

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
            error->sourceId = error->laneId_;
            error->taskId_ = lane->task_id_;
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
                error->coord->x_=0;
                error->coord->y_=0;
                error->coord->z_=0;
            }
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_LANE;
            error->errNodeInfo.emplace_back(errNodeInfo);
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


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneGroupCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneGroupCheckError::DCLaneGroupCheckError(const string &checkModel) : DCError(checkModel) {

        }


        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long s_index, long e_index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_005);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail_ = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的范围是" +
                            to_string(s_index) + "," + to_string(e_index) + "," +
                            "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_005);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail_ = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的节点" +
                            to_string(index) + "," ;
            error->sourceId = road_id;
            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_006(string road_id, string lg1,
                                                                                    long s_index1, long e_index1,
                                                                                    string lg2, long s_index2,
                                                                                    long e_index2,string taskId,bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_006);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "ValidityRange之间不重叠或交叉";
            error->detail_ = "roadid:" + road_id + "上的车道组关联关系有交叉." +
                            lg1 + ":" + to_string(s_index1) + "," + to_string(e_index1) + "," +
                            lg2 + ":" + to_string(s_index2) + "," + to_string(e_index2);

            error->sourceId = road_id;
            error->coord = make_shared<DCCoord>();
            error->coord->x_=0;
            error->coord->y_=0;
            error->coord->z_ = 0;
            error->taskId_ = taskId;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_NODE;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER_NODE;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_004(string divider_id,
                                                                                    set<string> lane_groups) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_004);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "一条普通车道线存在于多个车道组中。两个车道组共用的双向车道线除外";
            error->detail_ += "divider_id:";
            error->detail_ += divider_id;
            error->detail_ += "存在于分组";
            error->sourceId = divider_id;
            for (const auto &lg : lane_groups) {
                error->detail_ += lg;
                error->detail_ += " ";
            }
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
            errNodeInfo->x_ = 0;
            errNodeInfo->y_ = 0;
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
            error->detail_ += "lane_group_id:";
            error->detail_ += lane_group_id;
            error->detail_ += "车道编号异常";

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
            error->detail_ += "lane_group_id:";
            error->detail_ += lane_group_id;
            error->detail_ += "的divider长度异常：";
            for (const auto &div : dividers) {
                error->detail_ += div;
                error->detail_ += " ";

                shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>();
                errNodeInfo->x_ = 0;
                errNodeInfo->y_ = 0;
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
            error->detail_ += "lane_group_id:";
            error->detail_ += lane_group_id;
            error->detail_ += "车道组没有打断";
            error->coord = make_shared<DCCoord>();
            error->coord->y_=0;
            error->coord->x_=0;
            error->coord->z_=0;
            error->sourceId = lane_group_id;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_RELATION;
            errNodeInfo->dataLayer = MODEL_NAME_LANE_GROUP;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_003( shared_ptr<DCDivider> div) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_003);
            error->checkName = "车道线不存在于车道组中";
            error->detail_ += "divider_id:";
            error->detail_ += div->id_;
            error->detail_ += "是孤立的";
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
                error->coord->x_ = 0;
                error->coord->y_ = 0;
                error->coord->z_ = 0;
                errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
                errNodeInfo->dataId="";
            }
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }
        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_028(shared_ptr<DCLaneGroup> laneGroup,string daId, shared_ptr<DCCoord> coord) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_028);
            error->checkName = "车道组是否属于虚拟路口检查";
            error->detail_ += "车道组[";
            error->detail_ += laneGroup->id_;
            error->detail_ += "]的IS_VIR需为1（虚拟路口）,";
            error->detail_ += "辅助DA为["+daId+"]";
            error->taskId_ = laneGroup->task_id_;
            error->flag = laneGroup->flag_;
            error->coord = make_shared<DCCoord>();
            shared_ptr<ErrNodeInfo> errNodeInfo;
            error->coord = coord;
            errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataId = laneGroup->id_;
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_LANE;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }
        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_029(shared_ptr<DCLaneGroup> laneGroup1,shared_ptr<DCLaneGroup> laneGroup2) {
        shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>(CHECK_ITEM_KXS_LG_029);
        error->checkName = "车道组是否属于虚拟路口检查";
        error->detail_ += "车道组[";
        error->detail_ += laneGroup1->id_;
        error->detail_ += "]和车道组[";
        error->detail_ += laneGroup2->id_;
        error->detail_ += "]都是虚拟路口，不能相连";
        error->taskId_ = laneGroup1->task_id_;
        error->flag = laneGroup1->flag_;
        error->coord = make_shared<DCCoord>();
        shared_ptr<ErrNodeInfo> errNodeInfo;
        error->coord = make_shared<DCCoord>();
        error->coord->x_ = 0;
        error->coord->y_ = 0;
        error->coord->z_ = 0;
        errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
        errNodeInfo->dataId = laneGroup1->id_;
        errNodeInfo->dataType = DATA_TYPE_WAY;
        errNodeInfo->dataLayer = MODEL_NAME_LANE;
        error->errNodeInfo.emplace_back(errNodeInfo);
        return error;
    }
        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneGroupTopoCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneGroupTopoCheckError::DCLaneGroupTopoCheckError(const string &checkModel) : DCError(checkModel) {

        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_04_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>(
                    CHECK_ITEM_KXS_ROAD_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "若车道组之间连通，则其内部的道路必然连通。";
            error->detail_ += "lane group id:";
            error->detail_ += lg_id1;
            error->detail_ += "与lane group id:";
            error->detail_ += lg_id2;
            error->detail_ += "之间的道路不连通";
            error->coord = make_shared<DCCoord>();
            error->coord->y_=0;
            error->coord->x_=0;
            error->coord->z_=0;
            error->sourceId = lg_id1;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_RELATION;
            errNodeInfo->dataLayer = MODEL_NAME_LANE_GROUP;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_05_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>(
                    CHECK_ITEM_KXS_LANE_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "如果车道组之间是“封闭”的，则车道组之间的车道不会存在孤立的车道。";
            error->detail_ += "lane group id:";
            error->detail_ += lg_id1;
            error->detail_ += "与lane group id:";
            error->detail_ += lg_id2;
            error->detail_ += "车道中心线不连通";
            error->coord = make_shared<DCCoord>();
            error->coord->y_=0;
            error->coord->x_=0;
            error->coord->z_=0;
            error->sourceId = lg_id1;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_RELATION;
            errNodeInfo->dataLayer = MODEL_NAME_LANE_GROUP;
            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCRoadCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCRoadCheckError::DCRoadCheckError(const string &checkModel) : DCError(checkModel) {

        }


        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_002(const string &road_id,
                                                                          const string &lane_group_id) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_002);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "道路与车道组的两个车道边缘线应无交叉点。双向道路除外。";
            error->detail_ += "road_id:";
            error->detail_ += road_id;
            error->detail_ += "与lane group id:";
            error->detail_ += lane_group_id;
            error->detail_ += "边缘线有交叉";
            error->sourceId = road_id;

            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_003(const string &road_id,
                                                                          vector<NodeCheck> &error_index_pair) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_003);
            error->checkName = "道路高程突变>±10厘米/米。";
            error->detail_ += "road_id:";
            error->detail_ += road_id;
            error->detail_ += ",结点索引:";
            for (auto index_pair : error_index_pair) {
                error->detail_ += to_string(index_pair.pre_index);
                error->detail_ += ",";
                error->detail_ += to_string(index_pair.index);
                error->detail_ += ",高度差：";
                error->detail_ += to_string(index_pair.diff_height);
                error->detail_ += ",距离：";
                error->detail_ += to_string(index_pair.distance);
                error->detail_ += " ";
            }

            return error;
        }

        shared_ptr<DCRoadCheckError>
        DCRoadCheckError::createByKXS_04_006(const string &road_id,
                                             const vector<shared_ptr<NodeError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_006);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "道路前后结点重复,前后点xy坐标相同（z值可能相同）";
            error->detail_ += "road_id:";
            error->detail_ += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->index);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_coord->z_);
                error->detail_ += ")";
            }
            error->detail_ += "重复";

            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_007(const string &road_id,
                                                                          const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_007);
            error->checkName = "道路结点出现拐点，或者角度过大";
            error->detail_ += "road_id:";
            error->detail_ += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->z_);
                error->detail_ += ")";
                error->detail_ += "前后点角度：";
                error->detail_ += to_string(error_node->angle);
            }
            error->sourceId =road_id;
            error->coord = ptr_error_nodes[0]->ptr_current_coord;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(error->coord);
            errNodeInfo->dataType = DATA_TYPE_WAY;
            errNodeInfo->dataLayer = MODEL_NAME_ROAD;
            errNodeInfo->dataId = ptr_error_nodes[0]->id;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_008(const string &road_id,
                                                                          const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>(CHECK_ITEM_KXS_ROAD_008);
            error->checkName = "道路形态点间距过近";
            error->detail_ += "road_id:";
            error->detail_ += road_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->previous);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_previous_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_previous_coord->z_);
                error->detail_ += ")";
                error->detail_ += "与索引点";
                error->detail_ += to_string(error_node->current);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_current_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_current_coord->z_);
                error->detail_ += ")";
                error->detail_ += "距离：";
                error->detail_ += to_string(error_node->distance);

                shared_ptr<ErrNodeInfo> errNodeInfo1 = make_shared<ErrNodeInfo>(error_node->ptr_current_coord);
                errNodeInfo1->dataType = DATA_TYPE_WAY;
                errNodeInfo1->dataLayer = MODEL_NAME_ROAD;
                errNodeInfo1->dataId = error_node->id;
            }
            error->detail_ += "距离过近";
            error->sourceId = road_id;
            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneError::DCLaneError(const string &checkModel) : DCError(checkModel) {

        }


        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_002(const string &lane_id,
                                                                const string &divider_id) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_002);
            error->checkLevel_ = LEVEL_WARNING;
            error->checkName = "车道中心线与车道线在5米内不能存在两个交点（只检查组内车道线有共点的做检查）。";
            error->detail_ += "lane_id:";
            error->detail_ += lane_id;
            error->detail_ += ",divider_id:";
            error->detail_ += divider_id;
            error->detail_ += "在";
            error->detail_ += to_string(
                    DataCheckConfig::getInstance().getPropertyI(DataCheckConfig::LANE_INTERSECT_LENGTH));
            error->detail_ += "米内存在两个或存在多于2个交点";

            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_003(const string taskid,const string dataKey,const string dataType,const string dataLayer,shared_ptr<DCCoord>  coord,const string &lane_id,
                                                                const string &divider_id) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_003);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "车道中心线与本组的车道边缘线存在交叉点。";
            error->detail_ += "lane_id:";
            error->detail_ += lane_id;
            error->detail_ += "与divider_id:";
            error->detail_ += divider_id;
            error->detail_ += "边缘线有交点";
            error->sourceId = divider_id;
            error->taskId_ = taskid;
            error->dataKey_ = dataKey;
            if(coord == nullptr){
                coord = make_shared<DCCoord>();
                coord->x_ = 0;
                coord->y_ = 0;
                coord->z_ = 0;
            }
            error->coord = coord;
            shared_ptr<ErrNodeInfo> errNodeInfo = make_shared<ErrNodeInfo>(coord);
            errNodeInfo->dataType = DATA_TYPE_NODE;
            errNodeInfo->dataLayer = MODEL_NAME_DIVIDER;
            error->errNodeInfo.emplace_back(errNodeInfo);
            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_015(const string &lane_id1, const string &lane_id2,shared_ptr<DCCoord> coord) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_015 );
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "检查同组内中心线与中心线是否存在交叉问题（组内有共点的中心线做检查）。";
            error->detail_ += "lane_id:";
            error->detail_ += lane_id1;
            error->detail_ += ",lane_id:";
            error->detail_ += lane_id2;
            error->detail_ += "有交点："+to_string(coord->x_)+","+to_string(coord->y_);
            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_016(const string &lane_id,
                                                                const vector<shared_ptr<NodeError>> &ptr_error_nodes) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_016);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "车道中心线前后结点重复,前后点xy坐标相同（z值可能相同）";
            error->detail_ += "lane_id:";
            error->detail_ += lane_id;
            for (const auto &error_node : ptr_error_nodes) {
                error->detail_ += ",索引点";
                error->detail_ += to_string(error_node->index);
                error->detail_ += ",坐标(";
                error->detail_ += to_string(error_node->ptr_coord->x_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_coord->y_);
                error->detail_ += ",";
                error->detail_ += to_string(error_node->ptr_coord->z_);
                error->detail_ += ")";
            }
            error->detail_ += "重复";

            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_017(const string &lane_id,
                                                                const vector<shared_ptr<NodeCompareError>> &errorArray) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_017);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "同一条车道中心线上连续三个节点构成的夹角（绝对值）不能小于165度 (可配置)";
            error->detail_ += "lane_id:";
            error->detail_ += lane_id;
            for (const auto &errorNode : errorArray) {
                error->detail_ += ", { 3个坐标点的索引为: (";
                error->detail_ += to_string(errorNode->previous);
                error->detail_ += ",";
                error->detail_ += to_string(errorNode->current);
                error->detail_ += ",";
                error->detail_ += to_string(errorNode->next);
                error->detail_ += ")";
                error->detail_ += "角度为 ：";
                error->detail_ += to_string(errorNode->angle);
                error->detail_ += "}";
            }
            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_018(long fromLaneID, long toLaneID, double angle) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_018);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "两条相交且有车道拓扑关系的车道中心线最近的形状点构成的夹角（绝对值）不能小于170度(可配置)";
            error->detail_ += " 进入车道 ID :";
            error->detail_ += to_string(fromLaneID);
            error->detail_ += ", 退出车道 ID :";
            error->detail_ += to_string(toLaneID);
            error->detail_ += ", 夹角为 : ";
            error->detail_ += to_string(angle);
            return error;
        }


        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_020(long laneID, long index,
                                                                const shared_ptr<DCCoord> &coord, int level) {

            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_020);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "LANE点完备性检查。";
            error->detail_ += "Lane_id:";
            error->detail_ += std::to_string(laneID);
            error->detail_ += ",属性点索引:";
            error->detail_ += to_string(index);
            if (level == 1) {
                error->detail_ += ",Lane节点周围1.5米内找不到HD_LANE_SCH";
            } else if (level == 2) {
                error->detail_ += ",车道中心线起点和终点之处缺失HD_LANE_SCH";
            }
            error->coord = coord;
            return error;
        }


        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_022(long nodeID, shared_ptr<DCCoord> coord) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_022);
            error->checkName = "LANE_SCH与关联LANE距离检查。";
            error->detail_ += "LANE_SCH ID:";
            error->detail_ += std::to_string(nodeID);
            error->detail_ += ",LANE_SCH点离LANE的垂直距离超过10cm";

            error->coord = coord;
            return error;
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_023(string lane_id,string divider_id,double dis,shared_ptr<DCCoord> coord){
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>(CHECK_ITEM_KXS_LANE_023);
            error->checkName = CHECK_ITEM_KXS_LANE_023_DESC;
            error->checkLevel_ = LEVEL_ERROR;

            error->detail_ += "车道中心线 "+lane_id;
            error->detail_ += "距离车行道边缘线"+divider_id;
            error->detail_ += "小于"+to_string(dis)+"米";
            error->detail_ +=",位置:"+to_string(coord->x_)+","+to_string(coord->y_);
            error->coord = coord;
            return error;
        }

        DCAdasError::DCAdasError(const string &checkModel) : DCError(checkModel) {

        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_001(long road_id, string f_adas_node_id,
                                                                string t_adas_node_id, double distance) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_001);
            error->checkName = "相邻属性点间的距离不等于1米，除去与终点距离可能小于1米。";
            error->detail_ += "road_id:";
            error->detail_ += std::to_string(road_id);
            error->detail_ += ",属性点adas_node_id:";
            error->detail_ += f_adas_node_id;
            error->detail_ += ",";
            error->detail_ += t_adas_node_id;
            error->detail_ += "距离:";
            error->detail_ += to_string(distance);
            error->detail_ += "米";


            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_002(long road_id, const shared_ptr<DCCoord> &ptr_coord,
                                                                long index, double distance) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_002);
            error->checkName = "属性点与拟合曲线属性点的距离不大于10厘米。";
            error->detail_ += "road_id:";
            error->detail_ += std::to_string(road_id);
            error->detail_ += ",属性点索引:";
            error->detail_ += to_string(index);
            error->detail_ += ",点坐标:";
            error->detail_ += to_string(ptr_coord->x_);
            error->detail_ += ",";
            error->detail_ += to_string(ptr_coord->y_);
            error->detail_ += ",";
            error->detail_ += to_string(ptr_coord->z_);
            error->detail_ += ",与拟合曲线属性点距离:";
            error->detail_ += to_string(distance);
            error->detail_ += "米。";

            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_003(long road_id, long index,
                                                                const shared_ptr<DCCoord> &ptr_coord, int level) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_003);
            error->checkName = "ADAS_NODE点完备性检查。";
            error->detail_ += "road_id:";
            error->detail_ += std::to_string(road_id);
            error->detail_ += ",属性点索引:";
            error->detail_ += to_string(index);
            if (level == 1) {
                error->detail_ += ",道路节点周围1米内找不到ADAS_NODE";
            } else if (level == 2) {
                error->detail_ += ",道路起点和终点之处缺失ADAS_NODE";
            } else if (level == 3) {
                error->detail_ += ",DAS_NODE点离ROAD的垂直距离超过10cm";
            }
            error->coord = ptr_coord;

            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_005(long adas_node_id, shared_ptr<DCCoord> ptr_coord) {
            double adas_max_curvature = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::ADAS_NODE_MAX_CURVATURE);

            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_005);
            error->checkName = "ADAS_NODE曲率值域检查。";
            error->detail_ += "adas_node_id:";
            error->detail_ += std::to_string(adas_node_id);
            error->detail_ += ",曲率的绝对值不能大于";
            error->detail_ += to_string(adas_max_curvature);

            error->coord = ptr_coord;

            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_007(long adas_node_id, shared_ptr<DCCoord> ptr_coord) {
            double adas_max_slope = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::ADAS_NODE_MAX_SLOPE);

            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_007);
            error->checkName = "ADAS_NODE坡度值域检查。";
            error->detail_ += "adas_node_id:";
            error->detail_ += std::to_string(adas_node_id);
            error->detail_ += ",ADAS_NODE的坡度的绝对值不能超过";
            error->detail_ += to_string(adas_max_slope);

            error->coord = ptr_coord;

            return error;
        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_008(long adas_node_id, shared_ptr<DCCoord> ptr_coord) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>(CHECK_ITEM_KXS_ADAS_008);
            error->checkName = "ADAS_NODE与关联ROAD距离检查。";
            error->detail_ += "adas_node_id:";
            error->detail_ += std::to_string(adas_node_id);
            error->detail_ += ",ADAS_NODE点离ROAD的垂直距离超过10cm";

            error->coord = ptr_coord;

            return error;
        }


        DCFieldError::DCFieldError(const string &checkModel) : DCError(checkModel) {

        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_019(const string &detail) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>(CHECK_ITEM_KXS_ORG_019);
            error->checkName = "字段范围检查";
            error->detail_ = detail;

            return error;
        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_020(const string &file) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>(CHECK_ITEM_KXS_ORG_020);
            error->checkName = "缺少数据文件，或者数据为空";
            error->detail_ = file;

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
            error->detail_ = type;
            error->detail_ += ":";
            error->detail_ += id;
            error->sourceId = id;
            for (auto idx : index) {
                error->detail_ += ",索引点";
                error->detail_ += to_string(idx);
            }

            return error;
        }


        shared_ptr<DCLengthCheckError> DCLengthCheckError::createByLength(const double osmLength,
                                                                          const double kxfLength,
                                                                          const string &name) {
            shared_ptr<DCLengthCheckError>error = make_shared<DCLengthCheckError>(CHECK_ITEM_KXS_LENGTH_001);
            string head = "母库中的" + name + "和kxf中的" + name + "长度总和一致性检查";
            error->checkName = head;
            string content = "OSM格式中" + name + "长度为 : " + to_string(osmLength) + "\t";
            content += "KXF格式中"+ name + "长度为 : " + to_string(kxfLength);
            error->detail_ = content;
            return error;
        }

        shared_ptr<DCCountCheckError> DCCountCheckError::createByKXS_09_001(size_t osmCount, size_t kxfCount,
                                                                            const string &name) {
            shared_ptr<DCCountCheckError>error = make_shared<DCCountCheckError>(CHECK_ITEM_KXS_COUNT_001);
            string head = "母库中的" + name + "和kxf中的" + name + "数量一致性检查";
            error->checkName = head;
            string content = "OSM格式中" + name + "数量为 : " + to_string(osmCount) + "\t";
            content += "KXF格式中"+ name + "数量为 : " + to_string(kxfCount);
            error->detail_ = content;
            return error;
        }



        shared_ptr<DCTableDescError> DCTableDescError::createByKXS_10_001(const string &detail) {
            shared_ptr<DCTableDescError> error = make_shared<DCTableDescError>(CHECK_ITEM_KXS_NORM_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "KXF表描述检查";
            error->detail_ = detail;
            return error;
        }


        shared_ptr<DCForeignKeyCheckError> DCForeignKeyCheckError::createByKXS_01_026(string &modelName,
                                                                                      string &fieldName) {
            shared_ptr<DCForeignKeyCheckError> error = make_shared<DCForeignKeyCheckError>(CHECK_ITEM_KXS_ORG_026);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "外键在规格中存在, 在数据中不存在";
            error->detail_ += "{表名 : ";
            error->detail_ += modelName;
            error->detail_ += ",";
            error->detail_ += "外键 : ";
            error->detail_ += fieldName;
            error->detail_ += "}";
            return error;
        }

        shared_ptr<DCForeignKeyCheckError> DCForeignKeyCheckError::createByKXS_01_027(string &tableName,
                                                                                      string &foreignKeyName,
                                                                                      string &value,
                                                                                      string &relationTableName,
                                                                                      string &relationFieldName) {
            shared_ptr<DCForeignKeyCheckError> error = make_shared<DCForeignKeyCheckError>(CHECK_ITEM_KXS_ORG_027);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "外键的完备性检查, 记录不可缺失";
            error->detail_ += "{ 表名 : ";
            error->detail_ += tableName;
            error->detail_ += ", 外键 : ";
            error->detail_ += foreignKeyName;
            error->detail_ += ", 的值 :";
            error->detail_ += value;
            error->detail_ += ", 在关联表 : ";
            error->detail_ += relationTableName;
            error->detail_ += ", 关联字段 : ";
            error->detail_ += relationFieldName;
            error->detail_ += " 中不存在其数值 }";
            return error;
        }


        shared_ptr<DCAttributeCheckError> DCAttributeCheckError::createByKXS_10_002(const string &detail) {
            shared_ptr<DCAttributeCheckError> error = make_shared<DCAttributeCheckError>(CHECK_ITEM_KXS_NORM_002);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "坡度变化的平滑检查";
            error->detail_ = detail;
            return error;
        }


        shared_ptr<DCSCHInfoError> DCSCHInfoError::createByKXS_01_031(string name, string objID, double value,
                                                                      double threshold, shared_ptr<DCCoord> &coord) {
            shared_ptr<DCSCHInfoError> error = make_shared<DCSCHInfoError>(CHECK_ITEM_KXS_ORG_031);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "属性点的曲率值检查";
            error->detail_ += "表名: ";
            error->detail_ += name;
            error->detail_ += "相关联的对象ID :";
            error->detail_ += objID;
            error->detail_ += ", 曲率为 :";
            error->detail_ += to_string(value);
            error->detail_ +=", 超过了阈值 :";
            error->detail_ += to_string(threshold);
            error->coord = coord;
            return error;
        }

        shared_ptr<DCSCHInfoError> DCSCHInfoError::createByKXS_01_032(long objID, int index1, int index2, double dis,
                                                                      double threshold, string name) {
            shared_ptr<DCSCHInfoError> error = make_shared<DCSCHInfoError>(CHECK_ITEM_KXS_ORG_032);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = "属性点之间距离不超过1.3m";
            error->detail_ += "表名 :";
            error->detail_ += name;
            error->detail_ += ", Object ID: ";
            error->detail_ += std::to_string(objID);
            error->detail_ += ",属性点索引: ";
            error->detail_ += to_string(index1);
            error->detail_ += "和";
            error->detail_ += to_string(index2);
            error->detail_ += ", 的距离为: ";
            error->detail_ += to_string(dis);
            error->detail_ += ", 超过规定的 ";
            error->detail_ += to_string(threshold);
            return error;
        }

        shared_ptr<PolyLineError> PolyLineError::createByKXS_011_02(string line_id,shared_ptr<DCCoord> coord){
            shared_ptr<PolyLineError> error = make_shared<PolyLineError>(CHECK_ITEM_KXS_LINE_001);
            error->checkLevel_ = LEVEL_ERROR;
            error->checkName = CHECK_ITEM_KXS_LINE_001_DESC;
            error->detail_ += "HD_R_LO_ROAD表中停止线TYPE=2,LO_ID=[";
            error->detail_ +=line_id;
            error->detail_ +="]关联的ROAD错误!";
            error->coord = coord;
            return error;
        }

        string PolyLineError::toString() {
            return detail_;
        }
    }
}

