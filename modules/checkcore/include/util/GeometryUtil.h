
#ifndef AUTOHDMAP_COMPILE_GEOMETRYUTIL_H
#define AUTOHDMAP_COMPILE_GEOMETRYUTIL_H

typedef struct Point
{
    double x;
    double y;
}Point;

#include <tuple>
#include <geos/geom/Coordinate.h>

#include <ProductDataModel.hpp>

class GeometryUtil{
public:
    static double distance(double x1, double y1, double x2, double y2);

    static double UT_GetMapDistance(double dx1, double dy1, double dx2, double dy2);

    /**
     * 判断两条线是否方向相同
     * @return
     */
    static bool calLaneSameDir(double firstNode1X,double firstNode1Y,double firstNode2X,double firstNode2Y,
                        double secondNode1X,double secondNode1Y,double secondNode2X,double secondNode2Y, double angleLimit = 90.0);

    //计算两条线段质检的角度差，返回值为角度差的弧度值
    static double getAngleDiff(double cx1, double cy1, double cx2, double cy2,
                               double cx3, double cy3, double cx4, double cy4);

    static double getAngleDiff(double angle1, double angle2);

    //计算两条线段质检的角度差，返回值为角度差的弧度值，带符号，正数代表向逆时针针旋转，负数代表顺时针旋转
    static double getAngleDiffWithSign(double cx1, double cy1, double cx2, double cy2,
                                       double cx3, double cy3, double cx4, double cy4);

    /**
     * 计算平面距离
     * @param dx1
     * @param dy1
     * @param dx2
     * @param dy2
     * @return
     */
    static double getDistance(double dx1, double dy1, double dx2, double dy2);

    //计算两个geos Coordinate之间的距离
    static double getDistance(const geos::geom::Coordinate * coord1, const geos::geom::Coordinate * coord2);

    /**
     * 计算一条线段的垂线
     * @param x0 起点x
     * @param y0
     * @param x1 尾点x
     * @param y1
     * @param isRight  是否往右侧做垂线，否则左侧
     * @param isEnd   是否已尾点做垂线，否则首点
     * @param len
     * @return
     */
    static std::tuple<double, double> normalVectorPoint(double x0, double y0, double x1, double y1, bool isRight, bool isEnd, double len = 30.0);

    /**
     * 计算一个点在线段点左侧还是右侧
     * @param x0  线段起点x
     * @param y0  线段起点y
     * @param x1  线段终点x
     * @param y1  线段终点y
     * @param x2  判断点x
     * @param y2  判断点y
     * @return  返回值为1则在左侧，0为在线段上，-1为右侧
     */
    static int calPTOrentationOfLine(double x0, double y0, double x1, double y1, double ptx, double pty);

    /**
     * 等分插值
     * @param num 数量
     * @param value1 起始值
     * @param value2 终止值
     * @param results 结果集合
     */
    static void InterpolationValue(int num, double value1, double value2, vector<double>& results);


    /**
     * 计算圆心
     * @param[in] x1 第一点x坐标
     * @param[in] y1 第一点y坐标
     * @param[in] x2 第二点x坐标
     * @param[in] y2 第二点y坐标
     * @param[in] x3 第三点x坐标
     * @param[in] y3 第三点y坐标
     * @param[out] x_center 圆心x坐标
     * @param[out] y_center 圆心y坐标
     * @return true表示圆，false表示三点共线
     */
    static bool getCircleCenter(double x1,double y1,double x2,double y2,double x3,double y3,double& x_center,double& y_center);

};
#endif //AUTOHDMAP_COMPILE_GEOMETRYUTIL_H
