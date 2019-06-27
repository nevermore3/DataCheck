//
// Created by gaoyanhong on 2018/3/30.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H

#include "IMapProcessor.h"

namespace kd {
namespace dc {

/**
 * 车道线形状规范化检查
 * 对应检查项：JH_C_7,    JH_C_8,     JH_C_9
 * 检查项新ID: kxs-01-011,kxs-01-012,kxs-01-013
 */
class DividerShapeNormCheck : public IMapProcessor{

public:

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
                         shared_ptr<CheckErrorOutput> error_output) override ;
private:

    /**
     * KXS-01-011
     * 平滑度检查
     * @param dc_divider
     */
    void Check_kxs_01_011();

    /**
     * KXS-01-012
     * 结点间距检查
     * @param dc_divider
     */
    void Check_kxs_01_012();

    /**
     * KXS-01-013
     * 高程检查
     * @param dc_divider
     */
    void Check_kxs_01_013();

private:
    const string id = "divider_shape_norm_check";
};

}  // namespace dc
}  // namespace kd

#endif //AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H
