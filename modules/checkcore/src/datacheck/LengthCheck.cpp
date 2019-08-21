//
// Created by ubuntu on 2019/8/21.
//

#include <datacheck/LengthCheck.h>
#include <util/FileUtil.h>
#include <storage/JsonDataInput.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"
#include "IMapProcessor.h"
#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
using namespace kd::api;

namespace kd {
    namespace dc {
        LengthCheck::LengthCheck() {
            length_precise_ = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LENGTH_PRECISE);
            LoadCheckModel();
        }

        LengthCheck::~LengthCheck() {
        }

        string LengthCheck::getId() {
            return id_;
        }

        void LengthCheck::LoadCheckModel() {
            string checkFile = DataCheckConfig::getInstance().getProperty(DataCheckConfig::LENGTH_CHECK_FILE);
            Poco::File in_dir(checkFile);
            if (!in_dir.exists()) {
                LOG(ERROR) << checkFile << " is not exists!";
                return;
            }
            try {
                std::filebuf in;
                if (!in.open(checkFile, std::ios::in)) {
                    LOG(ERROR) << "fail to open task file : " << checkFile;
                    return;
                }

                Poco::JSON::Parser parser;
                std::istream iss(&in);
                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootObj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr))
                    rootObj = result.extract<Poco::JSON::Object::Ptr>();

                //获得基本信息
                string taskName = rootObj->getValue<string>("taskName");
                string dataPath = rootObj->getValue<string>("dataPath");
                //获得任务信息
                if (!rootObj->has("tasks"))
                    return;

                Poco::JSON::Array::Ptr taskArray = rootObj->getArray("tasks");
                size_t totalCount = taskArray->size();
                for (size_t i = 0 ; i < totalCount ; i++) {
                    Dynamic::Var value = taskArray->get(i);
                    Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                    string fileName = nodeObj->getValue<string>("fileName");
                    string fileType = nodeObj->getValue<string>("fileType");
                    string modelName = nodeObj->getValue<string>("modelName");

                    shared_ptr<DCTask> dcTask = make_shared<DCTask>();
                    dcTask->name = modelName;
                    dcTask->fileName = dataPath + fileName;
                    dcTask->fileType = fileType;
                    dcTask->modelName = modelName;
                    length_check_model_.push_back(dcTask);
                }
            }catch (Exception &e) {
                LOG(ERROR) << e.what();
                return;
            }
        }

        /*
         * 检查母库和kxf数据中
         * road、 divider、 object_PL、 barrier、 bridge
         * 长度之间的差异
         */
        void LengthCheck::Check_kxs_08_001() {

            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_LENGTH_001;
            size_t  total = 0;

            for (const auto &task : length_check_model_) {
                CheckLength(task->modelName, task->fileName);
            }


            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void LengthCheck::CheckLength(const string &modelName, const string &fileName) {
            double osmLength = 0.0;
            double kxfLength = 0.0;

            //read osm data
            const map<long, shared_ptr<KDSData>> &kdsDataMap = resource_manager_->getKdsData(modelName);
            for (const auto &kdsData : kdsDataMap) {
                shared_ptr<KDSWay> kdsWay = std::static_pointer_cast<KDSWay>(kdsData.second);
                vector<shared_ptr<DCCoord>>coords;
                for (const auto &node : kdsWay->nodes) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->lng_ = node->x;
                    coord->lat_ = node->y;
                    coord->z_ = node->z;
                    coords.push_back(coord);
                }
                osmLength += GeosObjUtil::get_length_of_coords(coords);
            }

            //read kxf data
            ShpData shpObj(fileName);
            if (!shpObj.isInit()) {
                //todo error info
            }
            size_t recordNums = shpObj.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpObj.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                    continue;
                size_t nVertices = shpObject->nVertices;
                vector<shared_ptr<DCCoord>>coords;
                for (size_t index = 0; index < nVertices; index++) {
                    shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                    coord->lng_ = shpObject->padfX[index];
                    coord->lat_ = shpObject->padfY[index];
                    coord->z_ = shpObject->padfZ[index];
                    coords.push_back(coord);
                }
                kxfLength += GeosObjUtil::get_length_of_coords(coords);
            }
            // compare & output

        }


        bool LengthCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                  shared_ptr<CheckErrorOutput> errorOutput) {
            set_error_output(errorOutput);

            //if (CheckItemValid(CHECK_ITEM_KXS_DIFF_001)) {
            Check_kxs_08_001();
            //}
            return true;
        }
    }
}