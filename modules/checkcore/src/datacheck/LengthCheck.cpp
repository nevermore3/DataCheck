//
// Created by ubuntu on 2019/8/21.
//

#include <datacheck/LengthCheck.h>
#include <util/FileUtil.h>
#include <storage/CheckTaskInput.h>
#include <util/StringUtil.h>
#include "parsers/OSMDataParser.hpp"
#include "IMapProcessor.h"
#include "util/GeosObjUtil.h"
#include "data/ErrorDataModel.h"
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include "model/ConsistencyCheckModel.h"
#include "data/ResourceDataManager.h"
using namespace kd::api;

namespace kd {
    namespace dc {
        LengthCheck::LengthCheck() {
            length_precise_ = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LENGTH_PRECISE);
            ConsistencyCheckModel *model = ConsistencyCheckModel::GetInstance();
            model->LoadLengthCheckModel();
            length_check_model_ = model->length_check_model_;
        }

        LengthCheck::~LengthCheck() {
        }

        string LengthCheck::getId() {
            return id_;
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
            if (length_check_model_.empty()) {
                LOG(ERROR)<<"length_check_model is null";
            } else {
                for (const auto &checkModel : length_check_model_) {
                    CheckLength(checkModel);
                }
            }
            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void LengthCheck::CheckLength(const shared_ptr<DataCheckModel> &checkModel) {
            double osmLength = 0.0;
            double kxfLength = 0.0;

            shared_ptr<ResourceManager> resourceManager = ResourceDataManager::GetInstance()->resource_manager_;
            const map<long, shared_ptr<KDSData>> &kdsDataMap = resourceManager->getKdsData(checkModel->modelName);
            if (kdsDataMap.empty()) {
                LOG(ERROR) << "Can't Open ModelFile :"<<checkModel->modelName;
                return;
            }
            for (const auto &kdsData : kdsDataMap) {
                shared_ptr<KDSWay> kdsWay = std::static_pointer_cast<KDSWay>(kdsData.second);

                if (!checkModel->modelLengthKey.empty()) {
                    osmLength += kdsWay->getPropertyDouble(checkModel->modelLengthKey);
                } else {
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
            }

            //read kxf data
            string fileName = checkModel->fileName;
            ShpData shpObj(fileName);
            if (!shpObj.isInit()) {
                LOG(ERROR) << "Can't Open KXF file :"<< checkModel->fileName;
                return;
            }
            size_t recordNums = shpObj.getRecords();
            for (size_t i = 0; i < recordNums; i++) {
                SHPObject *shpObject = shpObj.readShpObject(i);
                if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                    continue;
                if (!checkModel->shpLengthKey.empty()) {
                    kxfLength += shpObj.readDoubleField(i, checkModel->shpLengthKey);
                } else {
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
            }

            double ratio = (abs(osmLength - kxfLength) / osmLength ) * 100;

            if (ratio > length_precise_) {
                shared_ptr<DCLengthCheckError> error = DCLengthCheckError::createByLength(osmLength, kxfLength,
                                                                                          checkModel->modelName);
                error_output()->saveError(error);
            }

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