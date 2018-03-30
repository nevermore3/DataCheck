#ifndef AUTOHDMAP_FUSIONCORE_KDGEOUTIL_H
#define AUTOHDMAP_FUSIONCORE_KDGEOUTIL_H


#include "CommonInclude.h"

//third party
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
using namespace geos::geom;

namespace kd {
    namespace automap {

        /**
         * 线类型
         */
        class LineRelation {
        public:
            int index; //坐标点索引
            bool relation; //线与线的关系, true代表包含在buffer内，false代表不在buffer内
        };


        /**
         * 空间距离相关
         */
        class KDGeoLenUtil{

        };




        /**
         * 空间运算相关方法，
         * 注意当涉及到距离计算时，都要求传入坐标是UTM坐标
         */
        class KDGeoUtil {

        public:

            //////////////////////////////////////////////////////////////////////////////////////
            //  长度计算相关
            //////////////////////////////////////////////////////////////////////////////////////
            /**
             * 计算空间两个点的三维距离
             * @param start 起点
             * @param end 终点
             * @return 距离值，单位为米
             */
            static double get3DDistance(const geos::geom::Coordinate *start, const geos::geom::Coordinate *end);


            /**
             * 计算空间两个点的三维距离
             * @param start
             * @param end
             * @return
             */
            static double get3DDistance(const geos::geom::Point *start, const geos::geom::Point *end);

            /**
             * 计算空间两个点的平面距离
             * @param start
             * @param end
             * @return
             */
            static double getDistance(const geos::geom::Point *start, const geos::geom::Point *end);

            /**
             * 计算空间两个点的平面距离
             * @param start
             * @param end
             * @return
             */
            static double getDistance(const geos::geom::Coordinate *start, const geos::geom::Coordinate *end);

            /**
             * 计算空间两个点的高度差
             * @param start
             * @param end
             * @return 高度差，负值代表start低于end, 正值代表start高于end
             */
            static double getZDistance(const geos::geom::Point *start, const geos::geom::Point *end);


            /**
              * 获得线的指定范围的长度
              * @param line  线对象
              * @param beginIndex  点开始索引
              * @param endIndex 点结束索引
              * @return 长度值，如果参数有问题，返回值为-1.0
              */
            static double getLength(geos::geom::LineString *line, int beginIndex = -1, int endIndex = -1);


            /**
             * 计算中间点的z值
             * @param start
             * @param end
             * @param middle
             * @return
             */
            static bool calZValue(const geos::geom::Coordinate *start, const geos::geom::Coordinate *end,
                                  geos::geom::Coordinate *middle);


            //////////////////////////////////////////////////////////////////////////////////////
            //  空间关系计算相关
            //////////////////////////////////////////////////////////////////////////////////////

            /**
             * 判断空间的两条线段是否相交
             * @param start1 第一条线段的起点
             * @param end1 第一条线段的终点
             * @param start2 第二条线段的起点
             * @param end2 第二条线段的终点
             * @param sameLimit 两点位置一致的判断容差
             * @return bool代表相交，false代表不相交
             */
            static bool isLineSegmentCross(const geos::geom::Coordinate *start1, const geos::geom::Coordinate *end1,
                                           const geos::geom::Coordinate *start2, const geos::geom::Coordinate *end2,
                                           double sameLimit = 0.01);


            /**
             * 判断两条线是否相交
             * @param line1 线1
             * @param line2 线2
             * @param sameLimit 相交容差判断
             * @return true代表相交, false代表不相交
             */
            static bool
            isLineCross(geos::geom::LineString *line1, geos::geom::LineString *line2, double sameLimit = 0.01);

            /**
             * 判断两条线是否相交
             * @param line1
             * @param geom_buffer
             * @param line2
             * @param sameLimit
             * @return
             */
            static bool
            isLineCross(geos::geom::LineString *line1, geos::geom::Geometry *geom_buffer, geos::geom::LineString *line2,
                        double sameLimit);

            /**
             * 统计两条线相交的部分
             * @param line1 线1
             * @param line2 线2
             * @param vecLine1 第一条线在第二条线缓冲区内的索引，成对出现，返回值
             * @param vecLine2 第二条线在第一条线缓冲区内的索引，成对出现，返回值
             * @param buffer 缓冲区大小，单位为米
             * @return 操作是否成功
             */
            static bool getCorssPart(geos::geom::LineString * line1, geos::geom::LineString * line2, vector<int> & vecLine1, vector<int> & vecLine2, double buffer);


            static bool getPedalInfo(geos::geom::LineString * line, geos::geom::Coordinate * coord, double buffer, int & index, double & len);


            /**
             * 判断第二条在第一条线的缓冲区内
             * @param line1
             * @param geom_buffer
             * @param line2
             * @param sameLimit
             * @return
             */
            static bool isLineInBuffer(geos::geom::LineString *line1, geos::geom::Geometry *geom_buffer,
                                       geos::geom::LineString *line2, double sameLimit);

            /**
             * 判断两条线是否相邻
             * @param line1 第一条线
             * @param line2 第二条线
             * @param startIndex
             * @param endIndex
             * @param buffer 缓冲区大小，单位为米，用于均值判断
             * @return true代表相邻，false代表不相邻
             */
            static bool isLineAdjoin(const geos::geom::LineString *line1, const geos::geom::LineString *line2,
                                     int & startIndex, int & endIndex, double buffer);

            /**
             * 判断两个线的空间关系: 前提是两个线的方向一致，并且不相交，不重叠
             * @param line1 线1
             * @param line2 线2
             * @return 1代表line2在line1的左侧，2代表line2在line1的右侧, -1代表计算失败
             */
            static int calLineRelation(geos::geom::LineString *line1, geos::geom::LineString *line2);

            /**
             * 判断两条线首末点之间的关系
             * 注：两个线都不是环线
             * @param line1
             * @param line2
             * @param sameLimit
             * @return 1:起点相连,2:终点相连, 3:起点、终点都相连, 4:第一条线的起点和第二条线的终点相连
             *        ,5:第一条线的终点和第二条线的起点相连, 6:两条线两个端点均首尾相连, -1:两条线的起终点不相连
             */
            static int calLineStartEndRelation(geos::geom::LineString *line1, geos::geom::LineString *line2,
                                               double sameLimit = 0.01);


            /**
             * 判断线是否为环
             * @param line 线
             * @param sameLimit 两点位置一致的判断容差
             * @return true代表为闭合环，false代表不是环
             */
            static bool isLoopLine(geos::geom::LineString *line, double sameLimit = 0.01);


            /**
             * 判断两条线的方向是否相同
             * @param line1 线1
             * @param line2 线2
             * @return true线方向相同，false线
             */
            static bool checkLineDirection(geos::geom::LineString *line1, geos::geom::LineString *line2);


            static bool calIntersectPartsExt(const geos::geom::LineString *line1,
                                             const geos::geom::LineString *line2,
                                             std::vector<shared_ptr<LineRelation>> &lineRelations, double bufferLen);


            static bool coordInZBuffer(geos::geom::CoordinateSequence *cs, const geos::geom::Coordinate &coord, double bufferLen);


            /**
             * 计算点到点集的最近距离
             * @param cs 点集
             * @param PtA 待检测点
             * @param PtB 垂足点，或者线段的起点和终点
             * @param PtC 线段的两个坐标点，存储格式为x1,y1,x2,y2
             * @param min_index 最近线段的索引
             * @return 点到"垂足"的距离
             */
            static double pt2LineDist(CoordinateSequence *cs, double PtA[], double PtB[], double PtC[], int &min_index);


            /**
             * 判断点同线段是否相交
             * @param point 点对象
             * @param buffer 点对象的缓冲区
             * @param line 线对象
             * @param bufferLen 判断缓冲区大小，默认为1.0米
             * @return 是否相交
             */
            static bool isPointLineCorss(const geos::geom::Point *point, const geos::geom::Geometry *buffer,
                                         const geos::geom::LineString *line, double bufferLen = 1.0);


            /**
             * 判断两条线是否平行
             * @param start1
             * @param end1
             * @param start2
             * @param end2
             * @return
             */
            static bool isLineParallel(const geos::geom::Coordinate * start1, const geos::geom::Coordinate * end1,
                                       const geos::geom::Coordinate * start2, const geos::geom::Coordinate * end2, double angleLimit = 0.01);
            /**
             * 计算两条直线的二维交点
             * @param start1
             * @param end1
             * @param start2
             * @param end2
             * @param crossPt 交点
             * @return
             */
            static bool get2DCrossPoint(const geos::geom::Coordinate * start1, const geos::geom::Coordinate * end1,
                                        const geos::geom::Coordinate * start2, const geos::geom::Coordinate * end2,
                                        geos::geom::Coordinate * crossPt);


            //////////////////////////////////
            //根据两个点创建追踪三角形
            static geos::geom::Polygon *
            getTraceTriGeometry(double startx, double starty, double endx, double endy, int type);

            static geos::geom::Polygon *
            getTraceTriGeometry(double startx, double starty, double endx, double endy, double trace_Angle,
                                double trace_Length);

            //围绕中心点进行坐标旋转
            static void
            rotate(double centx, double centy, double x, double y, double radius, double *newx, double *newy);

            //计算两点间距离
            static double distance(double x1, double y1, double x2, double y2);

            static double distanceLL(double lng1, double lat1, double lng2, double lat2);

            static double pt2LineDist(vector<shared_ptr<Eigen::Vector3d>> &points, double PtA[],
                                      double PtB[], double PtC[], int &min_index);

//            static double pt2LineDist(geos::geom::CoordinateSequence * cs, double PtA[],
//                                      double PtB[], double PtC[], int &min_index);


            static int pt2LineSegmentRelation(double Line[], double PtA[], double PtB[]);

            static int
            pt2LineSegmentRelation(Eigen::Vector3d *lineStart, Eigen::Vector3d *lineEnd, Eigen::Vector3d *point);

            static bool calIntersectParts(const shared_ptr<geos::geom::LineString> &srcLine,
                                          const shared_ptr<geos::geom::LineString> &dstLine,
                                          vector<shared_ptr<LineRelation>> &lineRelations, double bufferLen);

            static bool IsLineStringIntersect(const shared_ptr<geos::geom::LineString> &srcLine,
                                              const shared_ptr<geos::geom::LineString> &dstLine,
                                              double bufferLen);


            static bool getDifferencePoint(const geos::geom::Coordinate * start, const geos::geom::Coordinate * end,
                                           geos::geom::Coordinate * diffPoint, double length);

            static bool getDifferencePoint(double x1, double y1, double x2, double y2, double & x3, double & y3, double length);


            //根据两个点创建追踪三角形
            //static geos::geom::Polygon * getTraceTriGeometry(double startx, double starty, double endx, double endy,int type);

            //static geos::geom::Polygon * getTraceTriGeometry(double startx, double starty, double endx, double endy, double trace_Angle, double trace_Length);

            //围绕中心点进行坐标旋转
            //static void rotate(double centx, double centy, double x, double y, double radius, double *newx, double *newy);

            //计算两点间距离
            //static double distance(double x1, double y1, double x2, double y2);

//            static double pt2LineDist(vector<shared_ptr<Eigen::Vector3d>> & points, double PtA[],
//                               double PtB[], double PtC[], int &min_index);

//            static double pt2LineDist(geos::geom::CoordinateSequence * cs, double PtA[],
//                                      double PtB[], double PtC[], int &min_index);
//
//
//            static int pt2LineSegmentRelation(double Line[], double PtA[], double PtB[]);
//
//            static int pt2LineSegmentRelation(Eigen::Vector3d * lineStart, Eigen::Vector3d * lineEnd, Eigen::Vector3d * point);
//
//            static bool calIntersectParts(const shared_ptr<geos::geom::LineString> & srcLine,
//                                          const shared_ptr<geos::geom::LineString> & dstLine,
//                                          vector<shared_ptr<LineRelation>> & lineRelations, double bufferLen);
//
//            static bool IsLineStringIntersect(const shared_ptr<geos::geom::LineString> & srcLine,
//                                              const shared_ptr<geos::geom::LineString> & dstLine,
//                                              double bufferLen);



        private:

//            static double calLineZMean(const shared_ptr<geos::geom::LineString> & srcLine,
//                                       int beginIndex, int endIndex);
//
//            static bool checkIntersect(const shared_ptr<geos::geom::LineString> & srcLine,
//                                       int beginIndex, int endIndex,
//                                       const shared_ptr<geos::geom::LineString> & dstLine,
//                                       double bufferLen);

            static double calLineZMean(const shared_ptr<geos::geom::LineString> &srcLine,
                                       int beginIndex, int endIndex);

            static bool checkIntersect(const shared_ptr<geos::geom::LineString> &srcLine,
                                       int beginIndex, int endIndex,
                                       const shared_ptr<geos::geom::LineString> &dstLine,
                                       double bufferLen);
        };


        /**
         * 线拟合工具类
         */
        class KDGLineFittingUtil{

        public:

            /**
             *
             * @param key_point
             * @param n
             * @param A
             * @return
             */
            static bool polynomialCurveFit(std::vector<cv::Point3d> & key_point, int n, cv::Mat& A);

        };


    }
}

#endif //AUTOHDMAP_FUSIONCORE_KDGEOUTIL_H
