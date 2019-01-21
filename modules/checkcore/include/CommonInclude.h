//
// Created by gaoyanhong on 2018/3/14.
//
// 本文件用于引用常用的系统文件、工具类等

#ifndef AUTOHDMAP_FUSIONCORE_COMMONINCLUDE_H
#define AUTOHDMAP_FUSIONCORE_COMMONINCLUDE_H

//system
#include <string>
#include <map>
#include <unordered_map>
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

#include "cppsqlite3/Database.h"
#include "cppsqlite3/Statement.h"
#include "cppsqlite3/Query.h"

#include "tinyxml.h"
using namespace std;

//thirdparty -- Eigen
#include <Eigen/Eigen>
using namespace Eigen;

//thirdparty -- opencv
//#include <opencv2/opencv.hpp>



#endif //AUTOHDMAP_FUSIONCORE_COMMONINCLUDE_H
