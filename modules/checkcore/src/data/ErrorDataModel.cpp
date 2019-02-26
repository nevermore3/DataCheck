//
// Created by gaoyanhong on 2018/3/29.
//

#include <data/ErrorDataModel.h>

#include "DataCheckConfig.h"

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCkError
        /////////////////////////////////////////////////////////////////////////////////////////

        DCError::DCError(string checkModel) {
            checkModel_ = checkModel;
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

        string DCAttCheckError::getHeader() {
            return "checkModel,modelName,fieldName,recordId,errorDesc";
        }

        string DCAttCheckError::toString() {
            stringstream ss;
            ss << "\"" << checkModel_ << ":" << checkDesc_ << "\"," << modelName_ << "," << fieldName_ << ","
               << recordId_ << "," << errorDesc_;
            return ss.str();
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCRelationCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCRelationCheckError::DCRelationCheckError(string checkModel) : DCError(checkModel) {

        }

        DCRelationCheckError::DCRelationCheckError(string checkModel, string modelName, string fieldName,
                                                   string refModelName, string refFieldName) :
                DCError(checkModel) {
            modelName_ = modelName;
            fieldName_ = fieldName;
            refModelName_ = refModelName;
            refFieldName_ = refFieldName;
        }

        string DCRelationCheckError::getHeader() {
            return "checkModel,modelName,fieldName,refModelName,refFieldName,recordValue,errorDesc";
        }

        string DCRelationCheckError::toString() {
            stringstream ss;
            ss << "\"" << checkModel_ << ":" << checkDesc_ << "\"," << modelName_ << "," << fieldName_ << ","
               << refModelName_;
            ss << "," << refFieldName_ << "," << recordValue_ << "," << errorDesc_;
            return ss.str();
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCDividerCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCDividerCheckError::DCDividerCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCDividerCheckError::getHeader() {
            return "checkModel,dividerId,attId,nodeId,lng,lat,z,errorDesc";
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
            if (att != nullptr) {
                error->nodeId_ = att->dividerNode_->id_;
                error->attId_ = att->id_;
                error->lng_ = att->dividerNode_->coord_.lng_;
                error->lat_ = att->dividerNode_->coord_.lat_;
                error->z_ = att->dividerNode_->coord_.z_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
            }

            return error;
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByNode(string checkModel, shared_ptr<DCDivider> div,
                                          shared_ptr<DCDividerNode> node) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            if (div == nullptr)
                return error;

            error->dividerId_ = div->id_;

            if (node != nullptr) {
                error->nodeId_ = node->id_;
                error->attId_ = "";
                error->lng_ = node->coord_.lng_;
                error->lat_ = node->coord_.lat_;
                error->z_ = node->coord_.z_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
            }

            return error;
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByNode(string checkModel, string nodeId, double lng, double lat, double z) {
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            error->dividerId_ = "";
            error->nodeId_ = nodeId;
            error->attId_ = "";
            error->lng_ = lng;
            error->lat_ = lat;
            error->z_ = z;

            return error;
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneCheckError::DCLaneCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCLaneCheckError::getHeader() {
            return "checkModel,laneId,leftDividerId,rightDividerId,attId,nodeId,lng,lat,z,errorDesc";
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
                error->lng_ = att->dividerNode_->coord_.lng_;
                error->lat_ = att->dividerNode_->coord_.lat_;
                error->z_ = att->dividerNode_->coord_.z_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
            }

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
                error->lng_ = node->coord_.lng_;
                error->lat_ = node->coord_.lat_;
                error->z_ = node->coord_.z_;
            } else {
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
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

        string DCSqlCheckError::getHeader() {
            return "checkModel,modelName,fieldName,recordId,errorDesc";
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

        string DCLaneGroupCheckError::getHeader() {
            return "";
        }

        string DCLaneGroupCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long s_index, long e_index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-005");
            error->checkLevel_ = LEVEL_WARNING;
            error->checkDesc_ = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的范围是" +
                            to_string(s_index) + "," + to_string(e_index) + "," +
                            "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_005(string road_id, long index, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-005");
            error->checkLevel_ = LEVEL_WARNING;
            error->checkDesc_ = "自动生成二维路网时，车道组要对道路全覆盖";
            error->detail = "roadid:" + road_id + "未全被车道组覆盖.未覆盖的节点" +
                            to_string(index) + "," + "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_006(string road_id, string lg1,
                                                                                    long s_index1, long e_index1,
                                                                                    string lg2, long s_index2,
                                                                                    long e_index2, bool is_positive) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-006");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "ValidityRange之间不重叠或交叉";
            error->detail = "roadid:" + road_id + "上的车道组关联关系有交叉." +
                            lg1 + ":" + to_string(s_index1) + "," + to_string(e_index1) + "," +
                            lg2 + ":" + to_string(s_index2) + "," + to_string(e_index2) + "," +
                            "lane group direction " + to_string(is_positive);

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_004(string divider_id,
                                                                                    set<string> lane_groups) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-004");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "一条普通车道线存在于多个车道组中。两个车道组共用的双向车道线除外";
            error->detail += "divider_id:";
            error->detail += divider_id;
            error->detail += "存在于分组";
            for (const auto &lg : lane_groups) {
                error->detail += lg;
                error->detail += " ";
            }

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_002(string lane_group_id) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-002");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "分组中车道编号从零开始，从内向外递增。编号连续，不缺失，不重复。";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "车道编号异常";

            return error;
        }

        shared_ptr<DCLaneGroupCheckError>
        DCLaneGroupCheckError::createByKXS_03_001(string lane_group_id, const vector<string> &dividers) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-001");
            error->checkLevel_ = LEVEL_WARNING;
            error->checkDesc_ = "同一个车道组内，单根车道线的长度同组内车道线平均长度不应该偏差超过";
            error->checkDesc_ += to_string((int) (DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::DIVIDER_LENGTH_RATIO) * 100));
            error->checkDesc_ += "%以上。";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "的divider长度异常：";
            for (const auto &div : dividers) {
                error->detail += div;
                error->detail += " ";
            }

            return error;
        }

        shared_ptr<DCLaneGroupCheckError> DCLaneGroupCheckError::createByKXS_03_027(string lane_group_id) {
            shared_ptr<DCLaneGroupCheckError> error = make_shared<DCLaneGroupCheckError>("KXS-03-027");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "车道组没有打断，不应该存在既是入口又是出口的组";
            error->detail += "lane_group_id:";
            error->detail += lane_group_id;
            error->detail += "车道组没有打断";

            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneGroupTopoCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneGroupTopoCheckError::DCLaneGroupTopoCheckError(const string &checkModel) : DCError(checkModel) {

        }

        string DCLaneGroupTopoCheckError::getHeader() {
            return "";
        }

        string DCLaneGroupTopoCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_04_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>("KXS-04-001");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "若车道组之间连通，则其内部的道路必然连通。";
            error->detail += "lane group id:";
            error->detail += lg_id1;
            error->detail += ",lane group id:";
            error->detail += lg_id2;
            error->detail += "道路不连通";

            return error;
        }

        shared_ptr<DCLaneGroupTopoCheckError> DCLaneGroupTopoCheckError::createByKXS_05_001(string lg_id1,
                                                                                            string lg_id2) {
            shared_ptr<DCLaneGroupTopoCheckError> error = make_shared<DCLaneGroupTopoCheckError>("KXS-05-001");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "如果车道组之间是“封闭”的，则车道组之间的车道不会存在孤立的车道。";
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

        string DCRoadCheckError::getHeader() {
            return "";
        }

        string DCRoadCheckError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCRoadCheckError> DCRoadCheckError::createByKXS_04_002(const string &road_id,
                                                                          const string &lane_group_id) {
            shared_ptr<DCRoadCheckError> error = make_shared<DCRoadCheckError>("KXS-04-002");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "道路与车道组的两个车道边缘线无交叉点。双向道路除外。";
            error->detail += "road_id:";
            error->detail += road_id;
            error->detail += ",lane group id:";
            error->detail += lane_group_id;
            error->detail += "边缘线有交叉";

            return error;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLaneError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCLaneError::DCLaneError(const string &checkModel) : DCError(checkModel) {

        }

        string DCLaneError::getHeader() {
            return "";
        }

        string DCLaneError::toString() {
            stringstream ss;
            ss << detail;
            return ss.str();
        }

        shared_ptr<DCLaneError> DCLaneError::createByKXS_05_002(const string &lane_id,
                                                                const string &divider_id) {
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>("KXS-05-002");
            error->checkLevel_ = LEVEL_WARNING;
            error->checkDesc_ = "车道中心线与车道线在5米内不能存在两个交点（只检查组内车道线有共点的做检查）。";
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
            shared_ptr<DCLaneError> error = make_shared<DCLaneError>("KXS-05-003");
            error->checkLevel_ = LEVEL_ERROR;
            error->checkDesc_ = "车道中心线与本组的车道边缘线无交叉点。";
            error->detail += "lane_id:";
            error->detail += lane_id;
            error->detail += ",divider_id:";
            error->detail += divider_id;
            error->detail += "边缘线有交点";

            return error;
        }

        DCAdasError::DCAdasError(const string &checkModel) : DCError(checkModel) {

        }

        shared_ptr<DCAdasError> DCAdasError::createByKXS_07_001(long road_id, string f_adas_node_id,
                                                                string t_adas_node_id, double distance) {
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>("KXS-07-001");
            error->checkDesc_ = "相邻属性点间的距离不等于1米，除去与终点距离可能小于1米。";
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
            shared_ptr<DCAdasError> error = make_shared<DCAdasError>("KXS-07-002");
            error->checkDesc_ = "属性点与拟合曲线属性点的距离不大于10厘米。";
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

        string DCAdasError::getHeader() {
            return std::__cxx11::string();
        }

        DCFieldError::DCFieldError(const string &checkModel) : DCError(checkModel) {

        }

        string DCFieldError::toString() {
            return detail;
        }

        string DCFieldError::getHeader() {
            return std::__cxx11::string();
        }

        shared_ptr<DCFieldError> DCFieldError::createByKXS_01_019(const string &detail) {
            shared_ptr<DCFieldError> error = make_shared<DCFieldError>("KXS-01-019");
            error->checkDesc_ = "字段范围检查";
            error->detail = detail;

            return error;
        }
    }
}

