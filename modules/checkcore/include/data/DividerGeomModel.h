//
// Created by gaoyanhong on 2018/2/26.
//

#ifndef AUTOHDMAP_FUSIONCORE_FUSIONMODEL_H
#define AUTOHDMAP_FUSIONCORE_FUSIONMODEL_H

//参考"高精数据规格格式规范v3.5e_20180226_jiaojie"

#include "CommonInclude.h"

//third party
#include <geos/geom/LineString.h>

using namespace geos::geom;


namespace kd {
    namespace dc {

        enum EnumDividerAttributeType {
            DA_TYPE_UNINVESTIGATED = 0,  //0：未调查
            DA_TYPE_ROAD_EDGE = 1,  //1：车行道边缘线
            DA_TYPE_WHITE_DOTTED = 2,  //2：白虚线
            DA_TYPE_WHITE_SOLID = 3,  //3：白实线
            DA_TYPE_BUS_LANE = 4,  //4：公交专用车道线
            DA_TYPE_INOUT_LANE = 5,  //5：道路出入口标线
            DA_TYPE_PENDING_AREA_LANE = 6,  //6：左弯待转区线
            DA_TYPE_VARIABLE_DIRECTION = 7,  //7：可变导向车道线
            DA_TYPE_TIDAL_LANE = 8,  //8：潮汐车道线
            DA_TYPE_YELLOW_DOTTED = 9,  //9：黄虚线
            DA_TYPE_YELLOW_SOLID = 10, //10：黄实线
            DA_TYPE_DECELERATION = 11, //11：纵向减速标线
            DA_TYPE_DUAL_YELLOW_DOTTED = 12, //12：双黄虚线
            DA_TYPE_DUAL_YELLOW_SOLID = 13, //13：双黄实线
            DA_TYPE_WHITE_LSOLID_RDOT = 14, //14：白左实右虚线
            DA_TYPE_YELLOW_LSOLID_RDOT = 15, //15：黄左实右虚线
            DA_TYPE_WHITE_RSOLID_LDOT = 16, //16：白右实左虚线
            DA_TYPE_YELLOW_RSOLID_LDOT = 17, //17：黄右实左虚线
            DA_TYPE_HOV_LANE = 18,  //18：HOV专用车道线
//#define DA_TYPE_       19：路缘石
//#define DA_TYPE_        20：墙
//#define DA_TYPE_       21：隧道墙
//#define DA_TYPE_        22：屏障
//#define DA_TYPE_        23：隔音屏障
//#define DA_TYPE_       24：线缆屏障
//#define DA_TYPE_        25：防护栏
//#define DA_TYPE_        26：栅栏
//#define DA_TYPE_        27：可通行路缘石
//#define DA_TYPE_        28：道路终点
//#define DA_TYPE_        29：悬崖
//#define DA_TYPE_        30：沟
//#define DA_TYPE_        31：其他屏障
//#define DA_TYPE_       32：其他"

            DA_TYPE_STATION_MARKING = 33,  //33：停靠站标线
            DA_TYPE_LEFT_EDGE = 34,  //34：车行道左边缘线
            DA_TYPE_RIGHT_EDGE = 35,  //35：车行道右边缘线
            DA_TYPE_EMERGENCE_LANE = 36  //36：应急车道

        };


        enum EnumDividerAttributeDriveRule {
            DA_DRIVE_RULE_UNINVESTIGATED = 0, //0：未调查
            DA_DRIVE_RULE_SOLID = 1, //1：实线
            DA_DRIVE_RULE_DOTTED = 2, //2：虚线
            DA_DRIVE_RULE_LSOLID_RDOT = 3, //3：左实右虚
            DA_DRIVE_RULE_RSOLID_LDOT = 4 //4：右实左虚
        };


        enum EnumDividerNodeType {
            DN_DASH_TYPE_SOLID = 0, //0：实线
            DN_DASH_TYPE_DOT_START = 1, //1：虚线起点
            DN_DASH_TYPE_DOT_END = 2, //2：虚线终点
            DN_DASH_TYPE_DOT_MIDDLE = 3 //3：虚线中间点
        };

        //车道线方向信息
        enum EnumDividerDirection{
            DIV_DIR_BI_DIRECTION = 1, //双向
            DIV_DIR_POSITIVE_DIRECTION = 2, //正向
            DIV_DIR_NEGATIVE_DIRECTION = 3, //逆向
            DIV_DIR_CLOSED_IN_BOTH_DIRECTION = 4 //双向禁行
        };


        class DCModel {
        public:
            DCModel() : valid_(true) {}

        public:
            string id_;

            //对象是否有效，默认true代表有效，当删除时，可将本变量置为false来实现逻辑删除
            bool valid_;

            string task_id_;

            string flag_;

        };


        /**
         * 坐标
         */
        class DCCoord {
        public:
            double lng_;
            double lat_;
            double z_;

            bool operator==(const DCCoord b) const{
                return (lng_==b.lng_ && lat_==b.lat_ && z_==b.z_);
            }

            bool operator<(const DCCoord b) const{
                if (lng_ < b.lng_)
                    return true;
                else if (lng_ > b.lng_)
                    return false;
                else if (lat_ < b.lat_)
                    return true;
                else if (lat_ > b.lat_)
                    return false;
                else if (z_ < b.z_)
                    return true;
                else if (z_ > b.z_)
                    return false;
                return false;
            }
        };

        class DCDividerNode;

        class DCDividerAttribute;

        class DCDivider;

        class DCLane;

        class DCLaneAttribute;

        class DCRoad;


        /**
         * 道路分隔线节点
         */
        class DCDividerNode : public DCModel {
        public:
            DCDividerNode() {
                dashType_ = 0; //
            }

        public:
            //坐标信息
            shared_ptr<DCCoord> coord_;

            //虚线起终点类型: "0：实线, 1：虚线起点, 2：虚线终点, 3：虚线中间点"
            long dashType_;

        };

        /**
         * 道路分隔线
         */
        class DCDivider : public DCModel {
        public:
            DCDivider() {
                dividerNo_ = -1;
                direction_ = 2; //
                rLine_ = 0; //
                tollFlag_ = 0; //
                fromNodeId_ = -1; //
                toNodeId_ = -1; //

                line_ = nullptr;
                len_ = 0.0;
            }

            /**
             * 对对象有效性进行检查
             * @return true代表有效，false代表无效
             */
            bool checkValid();

            /**
             * 查询对象是否有效
             * @return true代表有效，false代表无效，如被逻辑删除了
             */
            bool isValid();

            /**
             * 序列化
             * @return
             */
            string toString();

            /**
             * 按照关联的节点编号对属性变化点进行排序
             */
            void sortAtts();

            /**
              * 获得节点的索引
              * @param node 属性变化点
              * @return -1代表未找到，大于等于零的数代表节点在线中的索引
              */
            int getAttNodeIndex(shared_ptr<DCDividerNode> node);

            /**
             * 创建几何对象，用于后期的空间判断
             * @return 创建是否成功
             */
            bool buildGeometryInfo();

        public:
            //道路分隔线编号
            long dividerNo_;

            //方向："1：双向， 2：正向，3：逆向，4：双向禁行"
            long direction_;

            //是否参考线："0：非车道参考线，1：车道参考线"
            long rLine_;

            //收费站分隔线标识:"0：普通分隔线, 1：收费站分隔线"
            long tollFlag_;

            //开始节点
            string fromNodeId_;

            //结束节点
            string toNodeId_;

            //所有节点对象，同数字化方向相同
            std::vector<shared_ptr<DCDividerNode>> nodes_;

            //所有属性变化点，最终是按照节点的顺序，从索引小到大排序
            std::vector<shared_ptr<DCDividerAttribute>> atts_;

            //geos线对象，用于空间运算判断
            shared_ptr<geos::geom::LineString> line_;

            //对象的长度
            double len_;
        };


        /**
         * 道路分隔线属性变化点
         */
        class DCDividerAttribute : public DCModel {

        public:
            DCDividerAttribute() {
                virtual_ = 0; //
                color_ = 1; //
                type_ = 0; //
                driveRule_ = 0; //
                material_ = 1; //
                width_ = 0; //
                dividerNode_ = nullptr;
            }

            //判断两个属性变化点对象是否是同类型属性
            bool typeSame(shared_ptr<DCDividerAttribute> dividerAtt);

            /**
             * 判断两个属性变化点对象的属性一致
             * @param dividerAtt 被判断的属性变化点对象
             * @return true代表相同，false代表不同
             */
            //bool valueSame(shared_ptr<DCDividerAttribute> dividerAtt);

            /**
             * 拷贝属性变化点的基本属性
             * @param srcDividerAtt 原始属性变化点
             * @return 拷贝是否成功
             */
            bool copyBaseInfo(shared_ptr<DCDividerAttribute> srcDividerAtt);

        public:

            //虚拟分隔线类型: "0：非虚拟分隔线, 1：路口虚拟分隔线, 2：缺失虚拟分隔线, 3：人工虚拟分隔线"
            long virtual_;

            //分隔线颜色: "0：未调查,1：白色,2：黄色,3：橙色,4：蓝色,5：（虚拟线）无颜色"
            long color_;

            //分隔线类型: "0：未调查, 1：车行道边缘线, 2：白虚线, 3：白实线, 4：公交专用车道线, 5：道路出入口标线
            //, 6：左弯待转区线, 7：可变导向车道线, 8：潮汐车道线, 9：黄虚线, 10：黄实线
            //, 11：纵向减速标线, 12：双黄虚线, 13：双黄实线, 14：白左实右虚线, 15：黄左实右虚线
            //, 16：白右实左虚线, 17：黄右实左虚线, 18：HOV专用车道线, 19：路缘石, 20：墙
            //, 21：隧道墙, 22：屏障, 23：隔音屏障, 24：线缆屏障, 25：防护栏
            //, 26：栅栏, 27：可通行路缘石, 28：道路终点, 29：悬崖, 30：沟
            //, 31：其他屏障, 32：其他"
            long type_;

            //通行类型: "0：未调查, 1：实线, 2：虚线, 3：左实右虚, 4：右实左虚"
            long driveRule_;

            //分隔线材质："0：未调查, 1：路标漆, 2：振动凸起标线, 3：振动凸起标线与路标漆混合, 4：（虚拟线）无材质"
            long material_;

            //分隔线宽度："0：0CM, 1：15CM, 2：20CM, 3：25CM, 4：45CM, 5：50CM, 6：55CM, 7：60CM"
            long width_;

            //属性变化点关联的节点
            shared_ptr<DCDividerNode> dividerNode_;
        };


        /**
         * 车道对象
         */
        class DCLane : public DCModel {
        public:
            DCLane():road_(nullptr), leftDivider_(nullptr), rightDivider_(nullptr),
                     leftDivSNode_(nullptr), leftDivENode_(nullptr),
                     rightDivSNode_(nullptr), rightDivENode_(nullptr){}


            /**
             * 按照关联的节点编号对属性变化点进行排序
             */
            void sortAtts();

            /**
              * 获得节点的索引
              * @param node 属性变化点
              * @return -1代表未找到，大于等于零的数代表节点在线中的索引
              */
            int getAttNodeIndex(shared_ptr<DCDividerNode> node);

            /**
              * 获得通行方向的首尾节点
              * @param left 左侧车道标识
              * @param start 首节点标识
              * @return nullptr代表无法获取到节点，
              */
            shared_ptr<DCDividerNode> getPassDividerNode(bool left, bool start);

        public:
            //车道编号
            long laneNo_;

            //关联道路
            shared_ptr<DCRoad> road_;

            //关联左侧车道分隔线
            shared_ptr<DCDivider> leftDivider_;

            //关联右侧车道分隔线
            shared_ptr<DCDivider> rightDivider_;

            //车道属性变化点
            vector<shared_ptr<DCLaneAttribute>> atts_;

            //线坐标对象
            vector<shared_ptr<DCCoord>> coords_;

        private:
            shared_ptr<DCDividerNode> leftDivSNode_;
            shared_ptr<DCDividerNode> leftDivENode_;
            shared_ptr<DCDividerNode> rightDivSNode_;
            shared_ptr<DCDividerNode> rightDivENode_;
        };

        class DCLaneAttribute : public DCModel {

        public:
            //判断两个属性变化点对象是否是同类型属性
            bool typeSame(shared_ptr<DCLaneAttribute> laneAtt);

        public:
            //车道类型 "0：未调查, 1：普通车道, 2：停车道, 3：进入车道, 4：退出车道, 5：进入退出车道
            // 6：连接车道, 7：专用车道, 8：潮汐车道, 9：应急车道, 10：可变导向车道, 11：收费站车道
            //, 12：HOV车道, 13：摩托车道, 14：自行车道, 99：其他车道"
            long laneType_;

            //车道子类型"收费站车道：, 0：普通, 1：ETC
            //        应急车道：, 0：普通, 1：导流带车道
            //        进入车道：, 0：加速车道
            //        退出车道：, 0：减速车道
            //        进入退出车道, 0：加速减速车道
            //        连接车道：, 0：加速车道, 1：减速车道"
            long subType_;

            //车道方向 "1：双向, 2：正向, 3：逆向, 4：双向禁行"
            long direction_;

            //车道宽度 0～99.999999
            double width_;

            //最高限速
            long maxSpeed_;

            //最低限速
            long minSpeed_;

            //车道分歧点类型 "-1：非分离合并点, 0：未调查, 1：车道合并, 2：车道分离"
            long smType_;

            //车道通行状态 "0：正常通行(默认), 1：建设中, 2：禁止通行"
            long status_;

            //关联的车道线节点信息
            shared_ptr<DCDividerNode> dividerNode_;

        };

        /**
         * 复杂路口
         */
        class DCCNode : public DCModel{
        public:

        };

        /**
         * 道路节点对象
         */
        class DCRoadNode{
        public:
            DCRoadNode():cNode_(nullptr){}
        public:

            //关联复杂路口
            shared_ptr<DCCNode> cNode_;

        };

        /**
         * 道路对象
         */
        class DCRoad : public DCModel {
        public:
            DCRoad():fNode_(nullptr), tNode_(nullptr){}

        public:
            /**
             * 创建几何对象，用于后期的空间判断
             * @return 创建是否成功
             */
            bool buildGeometryInfo();
        public:
            //通行方向
            long direction_;

            //关联开始节点
            shared_ptr<DCRoadNode> fNode_;

            //关联结束节点
            shared_ptr<DCRoadNode> tNode_;

            // 关联开始节点
            string f_node_id;

            // 关联结束节点
            string t_node_id;

            //道路路线编号
            string routeNo_;

            //车道数
            long sLanes_;

            //节点
            vector<shared_ptr<DCCoord>> nodes_;

            //geos线对象，用于空间运算判断
            shared_ptr<geos::geom::LineString> line_;

            //对象的长度
            double len_;
        };

        /**
         * 复杂路口道路拓扑关系
         */
        class DCCNodeConn : public DCModel{
        public:
            DCCNodeConn():fRoad_(nullptr), cNode_(nullptr), tRoad_(nullptr){}

        public:
            //进入道路
            shared_ptr<DCRoad> fRoad_;

            //关联节点
            shared_ptr<DCCNode> cNode_;

            //退出道路
            shared_ptr<DCRoad> tRoad_;

            //通达标识 "0：能通达（默认）, 1：不能通达"
            long accessabel_;
        };

        /**
         * 简单节点道路拓扑关系
         */
        class DCNodeConn : public DCModel{
        public:
            DCNodeConn():fRoad_(nullptr), roadNode_(nullptr), tRoad_(nullptr){}

        public:
            //进入道路
            shared_ptr<DCRoad> fRoad_;

            //关联节点
            shared_ptr<DCRoadNode> roadNode_;

            //退出道路
            shared_ptr<DCRoad> tRoad_;

            //通达标识 "0：能通达（默认）, 1：不能通达"
            long accessabel_;

        };


        /**
         * 路口
         */
        class DCJunction : public DCModel{

        public:
            //坐标信息
            DCCoord coord_;
        };

        /**
         * 车道组
         */
        class DCLaneGroup : public DCModel{

        public:
            DCLaneGroup(): road_(nullptr){}

            /**
             * 根据车道编号对车道进行排序
             */
            void sortLanes();

        public:
            //道路
            shared_ptr<DCRoad> road_;

            // 是否是虚拟组
            int is_virtual_;

            // 车道组方向
            int direction_;

            //组成车道组的所有车道，经过排序
            vector<shared_ptr<DCLane>> lanes_;
        };


        /**
         * 线对象信息
         */
        class DCObjectPL : public DCModel{

        public:
            /**
             * 创建几何对象，用于后期的空间判断
             * @return 创建是否成功
             */
            bool buildGeometryInfo();

        public:
            //类型
            long type_;

            //子类型
            long subType_;

            //材质
            long material_;

            //颜色
            long color_;

            //所有坐标信息
            vector<shared_ptr<DCCoord>> coords_;

            //geos线对象，用于空间运算判断
            shared_ptr<geos::geom::LineString> line_;
        };

        /**
         * 车道拓扑关系
         */
        class DCLaneConnectivity : public DCModel{

        public:
            //节点类型"0：路口, 1：道路分歧, 3：车道分歧"
            long nodeType_;

            //关联节点
            long nodeId_;

            //关联进入车道
            long fLaneId_;

            //关联退出车道
            long tLaneId_;

        };


        //////////////////////////////////
        // 辅助信息，为了提高检查速度
        //////////////////////////////////
        /**
         * 共点节点信息
         */
        class DCDividerTopoNode {
        public:
            //节点id
            string nodeId_;

            //节点经度
            double lng_;

            //节点纬度
            double lat_;

            //节点高程
            double z_;

            //所有fromnode指向本节点的车道线，key和value都是车道线id
            map<string, string> startRels_;

            //所有tonode指向本节点的车道线，key和value都是车道线id
            map<string, string> endRels_;
        };

        //////////////////////////////////
        // Adas信息
        //////////////////////////////////
        /**
         * adas_node信息
         */
        class AdasNode : public DCModel {
        public:
            long road_id_;

            long road_node_idx_;

            long adas_node_id_;

            double curvature_;

            double slope_;

            double heading_;

            //坐标信息
            shared_ptr<DCCoord> coord_;
        };

        /**
         * adas_node_fitting信息
         */
        class AdasNodeFitting : public DCModel {
        public:
            long road_id_;

            long node_index_;

            double curvature_;

            double slope_;

            //坐标信息
            shared_ptr<DCCoord> coord_;
        };

        /**
         * 线参数
         */
        class CurvatureLine {
        public:
            double ratio_;

            double intercept_;

            long x_axis_based_;
        };

        /**
         * 圆弧参数
         */
        class CurvatureCircle {
        public:
            double radius_;

            double center_x_;

            double center_y_;

            double center_dir_;
        };

        /**
         * 回旋曲线参数
         */
        class CurvatureCurve {
        public:
            double theta0_;

            double theta1_;

            double arc_len_;

            double curvature0_;

            double curvature1_;

            double x0_;

            double y0_;

            double x1_;

            double y1_;

            long x_axis_based_;
        };

        /**
         * adas_node_curvaature信息
         */
        class AdasNodeCurvature : public DCModel {
        public:
            long road_id_;

            long node_num_;

            long seg_index_;

            long from_node_;

            long to_node_;

            long type_;

            CurvatureLine curvature_line_;

            CurvatureCircle curvature_circle_;

            CurvatureCurve curvature_curve_;

            double offset_x_;

            double offset_y_;

            std::vector<shared_ptr<DCCoord>> nodes_;
        };

        /**
       * adas_node_curvaature信息
       */
        class AdasNodeSlope : public DCModel {
        public:
            long road_id_;

            long node_num_;

            long seg_index_;

            long from_node_;

            long to_node_;

            double ratio_;

            double intercept_;

            std::vector<shared_ptr<DCCoord>> nodes_;
        };
    }
}

#endif //AUTOHDMAP_FUSIONCORE_FUSIONMODEL_H
