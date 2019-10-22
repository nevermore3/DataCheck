

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

    void preCheck();

    void clearMeomery();
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
    void check_kxs_03_028();
    /**
     * 两个类型为虚拟路口的车道组不能相连
     */
    void check_kxs_03_029();
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

    void BuildLaneGroup2Lanes();

    void BuildLaneConn();
private:
    string id_ = "lane_group_check";
    ///map<lg_id,set<lane_id>>
    map<long,set<long>> map_lg_id_to_lanes;
    ///map<lane_id,set<lane_id>>
    map<long,set<long>> map_lane_id_to_lanes;
    ///map<lane_id,lg_id>
    map<long,long> map_lane_id_to_lg_id;
};

}  // namespace dc
}  // namespace kd

#endif //AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H
