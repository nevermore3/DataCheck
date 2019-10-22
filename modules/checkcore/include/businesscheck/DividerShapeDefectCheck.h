#ifndef AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
#include "IMapProcessor.h"

namespace kd {
namespace dc {

/**
 * 车道线形状缺失检查
 * 对应检查项：kxs-01-005, kxs-01-006
 */
class DividerShapeDefectCheck : public IMapProcessor{

public:

    /**
     * 获得对象唯一标识
     * @return 对象标识
     */
    virtual string getId() override ;

    /**
     * 进行任务处理
     * @param mapDataManager 地图数据
     * @return 操作是否成功
     */
    virtual bool execute(shared_ptr<MapDataManager> data_manager,
                         shared_ptr<CheckErrorOutput> error_output) override;

private:
    void Check_kxs_01_005();

    void Check_kxs_01_006();

    void check_kxs(const std::string& limit_key, const std::string& check_model,
                   EnumDividerAttributeType div_attr_type);

    void checkShapeDefect(string checkModel, double distLimit,
                          shared_ptr<DCDivider> div, int beginNodexIdx,
                          int endNodeIdx, bool nodeDirection,int &subTotal);

    void getComparePair(shared_ptr<DCDivider> div, int begin, int end,
                        bool direction, vector<pair<int,int>> & pairs);

private:
    const string id = "divider_shape_defect_check";
};

}  // namespace dc
}  // namespace kd

#endif //AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
