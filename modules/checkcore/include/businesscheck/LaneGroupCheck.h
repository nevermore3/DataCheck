//
// Created by zhangxingang on 19-6-17.
//

#ifndef AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H
#define AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H

#include "IMapProcessor.h"

namespace kd {
namespace dc {
class LaneGroupCheck : public IMapProcessor {
public:
    ~LaneGroupCheck() override;

    string getId() override;

    bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

private:

    /**
     * 车道组内的divider检查
     */
    void Check_kxs_03_001();

    /**
     * 车道线不存在于车道组中
     */
    void Check_kxs_03_003();

    /**
     * 车道组与DIVIDER关系检查
     */
    void Check_kxs_03_004();
    /**
     * 车道组是否属于虚拟路口检查
     */
    void check_kxs_03_028_029();
    /**
     * 车道组编号检查
     * @param mapDataManager
     * @param errorOutput
     * @param lane_group
     * @param ptr_dividers
     */
    void check_divider_no(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput,
                          const string &lane_group, const vector<shared_ptr<DCDivider>> &ptr_dividers);

    bool check_divider_no(const vector<shared_ptr<DCDivider>> &ptr_dividers, bool is_front, bool direction);

    /**
     * 车道组内divider长度的检查
     * @param mapDataManager
     * @param errorOutput
     * @param lane_group
     * @param ptr_dividers
     */
    void check_divider_length(const string &lane_group, const vector<shared_ptr<DCDivider>> &ptr_dividers);

    bool checkDaTypeAndVirtual(long type_,long virtual_,long is_vir_);
private:
    string id_ = "lane_group_check";
};

}  // namespace dc
}  // namespace kd

#endif //AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H
