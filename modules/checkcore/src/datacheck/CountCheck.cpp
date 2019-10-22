
#include "datacheck/CountCheck.h"
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
        CountCheck::CountCheck() {
            ConsistencyCheckModel *model = ConsistencyCheckModel::GetInstance();
            //载入数目检查模型
            model->LoadCountCheckModel();
            count_check_model_ = model->count_check_model_;
        }

        string CountCheck::getId() {
            return id_;
        }

        bool CountCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            set_error_output(errorOutput);
            //数目一致性检查
            Check_kxs_08_002();
            return true;
        }

        void CountCheck::Check_kxs_08_002() {
            shared_ptr<CheckItemInfo> checkItemInfo = make_shared<CheckItemInfo>();
            checkItemInfo->checkId = CHECK_ITEM_KXS_COUNT_001;
            size_t  total = 0;
            if (count_check_model_.empty()) {
                LOG(ERROR) << "count_check_model_ is null";
            } else {
                for (const auto &checkModel : count_check_model_) {
                    CheckCount(checkModel);
                }
            }
            checkItemInfo->totalNum = total;
            error_output()->addCheckItemInfo(checkItemInfo);
        }

        void CountCheck::CheckCount(const shared_ptr<DataCheckModel> &checkModel) {
            size_t osmCount = 0;
            size_t kxfCount = 0;

            shared_ptr<ResourceManager> resourceManager = ResourceDataManager::GetInstance()->resource_manager_;
            const map<long, shared_ptr<KDSData>> &kdsDataMap = resourceManager->getKdsData(checkModel->modelName);
            if (kdsDataMap.empty()) {
                LOG(ERROR) << "Can't Open ModelFile :" << checkModel->modelName;
                return;
            }
            osmCount = kdsDataMap.size();

            string fileName = checkModel->fileName;
            ShpData shpObj(fileName);
            if (!shpObj.isInit()) {
                LOG(ERROR) << "Can't Open KXF file :"<< checkModel->fileName;
                return;
            }
            kxfCount = shpObj.getRecords();
            cout<<" Count "<<checkModel->modelName<<"\t"<<osmCount<<"\t"<<kxfCount<<endl;
            if (osmCount != kxfCount) {
                shared_ptr<DCCountCheckError> error = DCCountCheckError::createByKXS_09_001(osmCount, kxfCount,
                                                                                          checkModel->modelName);
                error_output()->saveError(error);
            }
        }
    }
}
