//
// Created by ubuntu on 19-9-27.
//

#ifndef AUTOHDMAP_DATACHECK_SHP_FILE_LOAD_UTIL_H
#define AUTOHDMAP_DATACHECK_SHP_FILE_LOAD_UTIL_H

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <Poco/StringTokenizer.h>
#include <Poco/File.h>
#include "data/DividerGeomModel.h"
using namespace std;
using namespace kd::dc;
class ShpFileLoad {
public:
    /**
     * 获取线要素对象
     * @param type  线对象类型
     * 0：未调查1：停车让行线2：减速让行线3：停止线4：道路边缘5：左道路边缘
     * 6：右道路边缘7：纵向减速标线8：公交车道专用线9：实线10：虚线
     * 11：可变导向线12：路缘石13：防护栏14：沟99：其它
     * @param polylines 解析结果
     */
    static void GetPolyline(long type,std::map<string,shared_ptr<DCPolyline>> &polylines);

    static void GetRLoRoad(long type,set<string> line_id,map<string,shared_ptr<DCRLORoad>> &r_lo_road);
};


#endif //AUTOHDMAP_DATACHECK_SHP_FILE_LOAD_UTIL_H
