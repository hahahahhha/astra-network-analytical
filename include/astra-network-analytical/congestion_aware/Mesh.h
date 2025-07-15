/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#pragma once

#include "common/Type.h"
#include "congestion_aware/BasicTopology.h"

using namespace NetworkAnalytical;

namespace NetworkAnalyticalCongestionAware {

class Mesh final : public BasicTopology {
    public:
        /**
         * @param rows 网格行数
         * @param cols 网格列数 
         * @param bandwidth 链路带宽
         * @param latency 链路延迟
         * @param wrap_around 是否启用环形连接（Torrus拓扑）
         */
        Mesh(int rows, int cols, 
             Bandwidth bandwidth, 
             Latency latency,
             bool wrap_around = false) noexcept;
    
        [[nodiscard]] Route route(DeviceId src, DeviceId dest) const noexcept override;
    
    private:
        const int rows_;
        const int cols_;
        const bool wrap_around_;
    
        // 将二维坐标转换为设备ID
        [[nodiscard]] DeviceId coord_to_id(int x, int y) const noexcept;
        
        // 将设备ID转换为二维坐标
        [[nodiscard]] std::pair<int, int> id_to_coord(DeviceId id) const noexcept;
    };
    
}  // namespace NetworkAnalyticalCongestionAware
