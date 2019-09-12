//
// Created by ubuntu on 2019/9/9.
//

#include "util/distance.h"


namespace kd {
    namespace dc {

        const double PI = 3.141592653589793;
        const double deg2rad = PI / 180.0;
        #define EARTH_2R			1273346728.7768236971495736012709F
        #define FACTOR_ARC_RAD		5729577951.3082320876798154814105F

        double GisToolSetGetLonLatDist(long coor1x, long coor1y, long coor2x, long coor2y)
        {
            if (coor1x == coor2x && coor1y == coor2y) {
                return 0;
            }

            double dlon = (coor2x - coor1x) / FACTOR_ARC_RAD;
            double dlat = (coor2y - coor1y) / FACTOR_ARC_RAD;
            double sindlat2 = sin(dlat / 2.0);
            double sindlon2 = sin(dlon / 2.0);
            double a = sindlat2 * sindlat2 + cos(coor1y / FACTOR_ARC_RAD) * cos(coor2y / FACTOR_ARC_RAD) * sindlon2 * sindlon2;
            return EARTH_2R * asin(sqrt(a));
        }

        double Distance::distance(const shared_ptr<DCCoord> pt1, const shared_ptr<DCCoord> pt2) {
            return GisToolSetGetLonLatDist(pt1->x_ * 100000000.0, pt1->y_ * 100000000.0,
                                           pt2->x_ * 100000000.0, pt2->y_ * 100000000.0);
        }

        double Distance::distance(const shared_ptr<DCCoord> pt,
                                  const vector<shared_ptr<DCCoord>>& line,
                                  shared_ptr<DCCoord> pFoot,
                                  int32_t* pSeg) {
            double dMinDistance = INT_MAX;
            int ptNum = (int)line.size();

            for (int i = 0; i < ptNum - 1; i++) {
                shared_ptr<DCCoord> a = line[i];
                shared_ptr<DCCoord> b = line[i + 1];

                shared_ptr<DCCoord> c = make_shared<DCCoord>();
                shared_ptr<DCCoord> ab = make_shared<DCCoord>();
                ab->y_ = b->y_  - a->y_ ;
                ab->x_ = b->x_  - a->x_ ;
                shared_ptr<DCCoord> ac = make_shared<DCCoord>();
                ac->y_ = pt->y_ - a->y_;
                ac->x_ = pt->x_ - a->x_ ;

                double cosLat = cos(pt->y_ * deg2rad);
                double f = ab->x_ * 100000000.0 * ac->x_ * 100000000.0 * cosLat * cosLat +
                           ab->y_ * 100000000.0 * ac->y_ * 100000000.0;
                double d = ab->x_ * 100000000.0 * ab->x_ * 100000000 * cosLat * cosLat +
                           ab->y_ * 100000000.0 * ab->y_ * 100000000.0;

                double dDis = INT_MAX;
                if (abs(d) < 0.0000001) {
                    // dDis=0;
                    dDis = distance(a, pt);
                } else if (f < 0) {
                    // Distance(a, c);
                    dDis = distance(a, pt);
                } else if (f > d) {
                    // Distance(b, c)
                    dDis = distance(b, pt);
                } else {
                    double newf = f / d;
                    //use our own defined function to adjust the accuracy
                    c->x_ = a->x_ + newf * ab->x_;
                    c->y_ = a->y_ + newf * ab->y_ ;
                    dDis = distance(c, pt);
                }

                if (dDis < dMinDistance) {
                    dMinDistance = dDis;
                    if (pSeg) {
                        *pSeg = i;
                    }

                    if (pFoot) {
                        if (abs(d) < 0.0000001) {
                            pFoot->x_ = a->x_;
                            pFoot->y_ = a->y_;
                        } else if (f < 0) {
                            pFoot->x_ = a->x_;
                            pFoot->y_ = a->y_;
                        } else if (f > d) {
                            pFoot->x_ = b->x_;
                            pFoot->y_ = b->y_;
                            // *pFoot = i + 1;
                        } else {
                            pFoot->x_ = c->x_;
                            pFoot->y_ = c->y_;
                        }
                    }
                }
            }

            return dMinDistance;
        }


        double Distance::distance(const shared_ptr<DCCoord> pt,
                                  const vector<shared_ptr<DCDividerNode>>& nodes,
                                  shared_ptr<DCCoord> pFoot,
                                  int32_t* pSeg) {
            double dMinDistance = INT_MAX;
            int ptNum = (int)nodes.size();

            for (int i = 0; i < ptNum - 1; i++) {
                shared_ptr<DCCoord> a = nodes[i]->coord_;
                shared_ptr<DCCoord> b = nodes[i + 1]->coord_;

                shared_ptr<DCCoord> c = make_shared<DCCoord>();
                shared_ptr<DCCoord> ab = make_shared<DCCoord>();
                ab->y_ = b->y_  - a->y_ ;
                ab->x_ = b->x_  - a->x_ ;
                shared_ptr<DCCoord> ac = make_shared<DCCoord>();
                ac->y_ = pt->y_ - a->y_;
                ac->x_ = pt->x_ - a->x_ ;

                double cosLat = cos(pt->y_ * deg2rad);
                double f = ab->x_ * 100000000.0 * ac->x_ * 100000000.0 * cosLat * cosLat +
                           ab->y_ * 100000000.0 * ac->y_ * 100000000.0;
                double d = ab->x_ * 100000000.0 * ab->x_ * 100000000 * cosLat * cosLat +
                           ab->y_ * 100000000.0 * ab->y_ * 100000000.0;

                double dDis = INT_MAX;
                if (abs(d) < 0.0000001) {
                    // dDis=0;
                    dDis = distance(a, pt);
                } else if (f < 0) {
                    // Distance(a, c);
                    dDis = distance(a, pt);
                } else if (f > d) {
                    // Distance(b, c)
                    dDis = distance(b, pt);
                } else {
                    double newf = f / d;
                    //use our own defined function to adjust the accuracy
                    c->x_ = a->x_ + newf * ab->x_;
                    c->y_ = a->y_ + newf * ab->y_ ;
                    dDis = distance(c, pt);
                }

                if (dDis < dMinDistance) {
                    dMinDistance = dDis;
                    if (pSeg) {
                        *pSeg = i;
                    }

                    if (pFoot) {
                        if (abs(d) < 0.0000001) {
                            pFoot->x_ = a->x_;
                            pFoot->y_ = a->y_;
                        } else if (f < 0) {
                            pFoot->x_ = a->x_;
                            pFoot->y_ = a->y_;
                        } else if (f > d) {
                            pFoot->x_ = b->x_;
                            pFoot->y_ = b->y_;
                            // *pFoot = i + 1;
                        } else {
                            pFoot->x_ = c->x_;
                            pFoot->y_ = c->y_;
                        }
                    }
                }
            }

            return dMinDistance;
        }




    }
}
