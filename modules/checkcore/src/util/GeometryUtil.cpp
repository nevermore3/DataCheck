#include <cmath>
#include "geom/geo_util.h"
#include <util/GeometryUtil.h>

#define PI 3.1415926535897932384626433832795
#define EARTH_RADIUS 6378.137 //地球半径 KM

using namespace std;
double rad(double d) {
    return d * PI / 180.0;
}

 double GeometryUtil::distance(double x1, double y1, double x2, double y2){
     double radLat1;
     double radLat2;
     double a;
     double b;

     radLat1 = rad(y1);
     radLat2 = rad(y2);
     a = radLat1 - radLat2;
     b = rad(x1) - rad(x2);

     double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
     s = s * EARTH_RADIUS;
     s = s * 1000;
     return s;
 }

 double GeometryUtil::UT_GetMapDistance(double dx1, double dy1, double dx2, double dy2){
     double length = 0.0;
     double dx = dx2 - dx1;
     double dy = dy2 - dy1;
     double sx = cos(dy1 * 0.01745329252f);
     length = sqrt(dx * dx * sx * sx + dy * dy) * 111195.0;
     return length;
 }

bool GeometryUtil::calLaneSameDir(double firstNode1X,double firstNode1Y,double firstNode2X,double firstNode2Y,
                                         double secondNode1X,double secondNode1Y,double secondNode2X,double secondNode2Y, double angleLimit) {
    double thetaFirst = geo::geo_util::calcAngle(firstNode1X, firstNode1Y, firstNode2X, firstNode2Y);
    double thetaSencond = geo::geo_util::calcAngle(secondNode1X, secondNode1Y, secondNode2X, secondNode2Y);

    double angleDiff = fabs(thetaFirst - thetaSencond);
    if (angleDiff > PI) {
        angleDiff = 2 * PI - angleDiff;
    }
    double angle =  180/PI * angleDiff;
    bool isSameDir = true;
    if(angle > angleLimit){
        isSameDir = false;
    }
    return isSameDir;
}

double GeometryUtil::getAngleDiff(double cx1, double cy1, double cx2, double cy2,
                                  double cx3, double cy3, double cx4, double cy4) {
    double thetaFirst = geo::geo_util::calcAngle(cx1, cy1, cx2, cy2);
    double thetaSencond = geo::geo_util::calcAngle(cx3, cy3, cx4, cy4);

    double angleDiff = fabs(thetaFirst - thetaSencond);
    if (angleDiff > PI) {
        angleDiff = 2 * PI - angleDiff;
    }
    return angleDiff;
}

double GeometryUtil::getAngleDiff(double angle1, double angle2) {
    double angleDiff = fabs(angle1 - angle2);
    if (angleDiff > PI) {
        angleDiff = 2 * PI - angleDiff;
    }
    return angleDiff;
}

double GeometryUtil::getAngleDiffWithSign(double cx1, double cy1, double cx2, double cy2,
                                          double cx3, double cy3, double cx4, double cy4) {
    double thetaFirst = geo::geo_util::calcAngle(cx1, cy1, cx2, cy2);
    double thetaSencond = geo::geo_util::calcAngle(cx3, cy3, cx4, cy4);

    double angleDiff = thetaFirst - thetaSencond;
    if (angleDiff > PI) {
        angleDiff = 2 * PI - angleDiff;
    }
    if (angleDiff < -PI) {
        angleDiff = (2 * PI + angleDiff) * -1;
    }
    return angleDiff;
}

double GeometryUtil::getDistance(double dx1, double dy1, double dx2, double dy2) {
    double dx = dx2 - dx1;
    double dy = dy2 - dy1;
    double dis = sqrt(dx * dx  + dy * dy);
    return dis;
}

double GeometryUtil::getDistance(const geos::geom::Coordinate * coord1, const geos::geom::Coordinate * coord2){
    return getDistance(coord1->x, coord1->y, coord2->x, coord2->y );
}

std::tuple<double, double> GeometryUtil::normalVectorPoint(double x0, double y0, double x1, double y1, bool isRight, bool isEnd, double len){
    double x2 = 0, y2 = 0;
    double dx = fabs(x1-x0);
    double dy = fabs(y1-y0);
    double epsilong = 1.0E-8;
    double dirFlag = isRight ? 1.0 : -1.0;

    if(dx < epsilong){
        if(y1 > y0){
            x2 = x1 + dirFlag*len;
            y2 = y1;
        }else{
            x2 = x1 - dirFlag*len;
            y2 = y1;
        }
        return make_tuple(x2, y2);
    }

    if(dy < epsilong){
        if(x1 > x0){
            x2 = x1;
            y2 = y1 - dirFlag*len;
        }else{
            x2 = x1;
            y2 = y1 + dirFlag*len;
        }
        return make_tuple(x2, y2);
    }

    double theta = std::atan(dy/dx);
    double flagx = 1.0, flagy = 1.0;
    if(y1 < y0)
        flagx = -1;
    if(x1 > x0)
        flagy = -1;

    if(isEnd){
        x2 = x1 + dirFlag*flagx * len * sin(theta);
        y2 = y1 + dirFlag*flagy * len * cos(theta);
    } else{
        x2 = x0 + dirFlag*flagx * len * sin(theta);
        y2 = y0 + dirFlag*flagy * len * cos(theta);
    }


    return make_tuple(x2, y2);
}


int GeometryUtil::calPTOrentationOfLine(double x0, double y0, double x1, double y1, double ptx, double pty) {
    double tempValue =(y0 - y1)*ptx + (x1 - x0)* pty +( x0 * y1) - (x1 * y0);
    //tempValue>0，说明点在线的左边，小于在右边，等于则在线上
    if (tempValue > 0){
        return 1;
    } else if(tempValue == 0){
        return 0;
    } else{
        return -1;
    }
}

void GeometryUtil::InterpolationValue(int num, double value1, double value2, vector<double> &results) {
    double delta = (value2 - value1) / (num - 1);
    results.emplace_back(value1);
    for (int i = 1; i < num - 1; i++) {
        double temp_value = value1 + i * delta;
        results.emplace_back(temp_value);
    }
    results.emplace_back(value2);
}

bool GeometryUtil::getCircleCenter(double x1,double y1,double x2,double y2,double x3,double y3,double& x_center,double& y_center){
    double a = x1 - x2;
    double b = y1 - y2;
    double c = x1 - x3;
    double d = y1 - y3;
    double e = ((x1 * x1 - x2 * x2) + (y1 * y1 - y2 * y2)) / 2.0;
    double f = ((x1 * x1 - x3 * x3) + (y1 * y1 - y3 * y3)) / 2.0;
    double det = b * c - a * d;

    if( fabs(det) < 1e-5) {
        return false;
    }
    x_center = -(d * e - b * f) / det;
    y_center = -(a * f - c * e) / det;
    return true;
}
