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

using namespace kd::api;

namespace kd {
    namespace dc {
        LengthCheck::LengthCheck() {
            length_precise_ = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LENGTH_PRECISE);
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
            checkItemInfo->checkId = CHECK_ITEM_KXS_DIFF_001;
            size_t  total = 0;

            double osmLength = 0.0;
            double kxfLength = 0.0;
            //read osm data
            const map<long, shared_ptr<KDSData>> &kdsDataMap = resource_manager_->getKdsData(MODEL_NAME_ROAD);
            for (const auto &kdsData : kdsDataMap) {
                shared_ptr<KDSRoad> kdsRoad = std::static_pointer_cast<KDSRoad>(kdsData.second);
                osmLength += kdsRoad->LENGTH;
            }
            //read kxf data
            string fileName = kxf_base_path_ + "/ROAD";
            ShpData shpObj(fileName);
            if (shpObj.isInit()) {
                size_t recordNums = shpObj.getRecords();
                for (size_t i = 0; i < recordNums; i++) {
                    SHPObject *shpObject = shpObj.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;
                    int nVertices = shpObject->nVertices;
                    vector<shared_ptr<DCCoord>>coords;
                    for (size_t i = 0; i < nVertices; i++) {
                        shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                        coord->lng_ = shpObject->padfX[idx];
                        coord->lat_ = shpObject->padfY[idx];
                        coord->z_ = shpObject->padfZ[idx];
                        coords.push_back(coord);
                    }
                    kxfLength += GeosObjUtil::get_length_of_coords(coords);
                }
            }


            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
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