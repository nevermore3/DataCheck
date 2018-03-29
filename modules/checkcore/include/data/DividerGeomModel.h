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
            DA_TYPE_HOV_LANE = 18  //18：HOV专用车道线
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


        enum EnumOperType {
            OPER_TYPE_AUTOMATIC = 1, //1：自动化
            OPER_TYPE_MANUAL = 2 //2：人工
        };


        class KDModel {
        public:
            KDModel() : valid(true), visible_(true) {}

        public:
            string id;

            string version_;

            bool visible_;

            //对象是否有效，默认true代表有效，当删除时，可将本变量置为false来实现逻辑删除
            bool valid;

            //
            std::string changeset_;

            //
            std::string timestamp_;

            //模型名称
            std::string modelName_;

            //任务id
            string taskId_;

            //批次信息
            string batch_;

            //序号信息
            string seq_;

        };


        /**
         * 坐标
         */
        class KDCoord {
        public:
            double lng;
            double lat;
            double z;
        };

        class KDDividerNode;
        class KDMeasureInfo;

        class KDDividerAttribute;

        class KDDivider;


        /**
         * 道路分隔线节点
         */
        class KDDividerNode : public KDModel {
        public:
            KDDividerNode() {
                dashType_ = 0; //
                flag_ = 1; //
                measureInfo = nullptr;
            }

        public:

            //坐标信息
            KDCoord coord;

            //虚线起终点类型: "0：实线, 1：虚线起点, 2：虚线终点, 3：虚线中间点"
            long dashType_;

            //自动化标识:"1：自动化, 2：人工"
            long flag_;

            //对象量测信息
            shared_ptr<KDMeasureInfo> measureInfo;

        };

        class KDMeasureInfo : public KDModel{
        public:
            string method_;

            string parameter_;

            string flag_;
        };


        /**
         * 道路分隔线
         */
        class KDDivider : public KDModel {
        public:
            KDDivider() {
                dividerNo_ = -1;
                direction_ = 2; //
                rLine_ = 0; //
                tollFlag_ = 0; //
                source_ = 1; //
                sDate_ = -1; //
                flag_ = 1;//
                taskId_ = -1;

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
            int getAttNodeIndex(shared_ptr<KDDividerNode> node);

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

            //作业员
            string operator_;

            //生产参考资料: "1：图像, 2：图像点云, 3：第三方, 4：其它"
            long source_;

            //参考资料时效
            long sDate_;

            //自动化标识："1：自动化， 2：人工"
            long flag_;

            //所有节点对象
            std::vector<shared_ptr<KDDividerNode>> nodes_;

            //所有属性变化点
            std::vector<shared_ptr<KDDividerAttribute>> atts_;

            //geos线对象，用于空间运算判断
            shared_ptr<geos::geom::LineString> line_;

            //对象的长度
            double len_;
        };


        /**
         * 道路分隔线属性变化点
         */
        class KDDividerAttribute : public KDModel {

        public:
            KDDividerAttribute() {
                virtual_ = 0; //
                color_ = 1; //
                type_ = 0; //
                driveRule_ = 0; //
                material_ = 1; //
                width_ = 0; //
                source_ = 1; //
                sDate_ = -1; //
                flag_ = 1;

                divider_ = nullptr;
                dividerNode_ = nullptr;
            }

            //判断两个属性变化点对象是否是同类型属性
            bool typeSame(shared_ptr<KDDividerAttribute> dividerAtt);

            /**
             * 判断两个属性变化点对象的属性一致
             * @param dividerAtt 被判断的属性变化点对象
             * @return true代表相同，false代表不同
             */
            bool valueSame(shared_ptr<KDDividerAttribute> dividerAtt);

            /**
             * 拷贝属性变化点的基本属性
             * @param srcDividerAtt 原始属性变化点
             * @return 拷贝是否成功
             */
            bool copyBaseInfo(shared_ptr<KDDividerAttribute> srcDividerAtt);

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

            //作业员
            string operator_;

            //生产参考资料: "1：图像, 2：图像点云, 3：第三方, 4：其它"
            long source_;

            //参考资料时效
            long sDate_;

            //自动化标识："1：自动化， 2：人工"
            long flag_;

            //属性变化点关联的车道线
            shared_ptr<KDDivider> divider_;

            //属性变化点关联的节点
            shared_ptr<KDDividerNode> dividerNode_;
        };


    }
}

#endif //AUTOHDMAP_FUSIONCORE_FUSIONMODEL_H
