/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "congestion_aware/Mesh.h"
#include <cassert>
#include <algorithm>
using namespace NetworkAnalyticalCongestionAware;

Mesh::Mesh(int rows, int cols, 
           Bandwidth bandwidth,
           Latency latency,
           bool wrap_around) noexcept
    : BasicTopology(rows * cols,       // 总NPU数
                   rows * cols,       // 总设备数（假设每个节点一个设备）
                   bandwidth,
                   latency),
      rows_(rows),
      cols_(cols),
      wrap_around_(wrap_around)
{
    assert(rows * cols > 0);
    assert(bandwidth > 0);
    assert(latency >= 0);
    const bool bidirectional = true;

    // 连接内部节点（水平和垂直方向）
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            const DeviceId current = coord_to_id(i, j);
            
            // 水平连接（如果不是最后一列）
            if (i < cols - 1) {
                connect(current, coord_to_id(i + 1, j), 
                    bandwidth, latency, bidirectional);
            }
            
            // 垂直连接（如果不是最后一行）
            if (j < rows - 1) {
                connect(current, coord_to_id(i, j + 1), 
                    bandwidth, latency, bidirectional);
            }
        }
    }

    // 如果需要环形连接（Torrus拓扑）
    if (wrap_around) {
        // 水平环形连接（最左和最右列）
        for (int j = 0; j < rows; j++) {
            connect(coord_to_id(0, j), coord_to_id(cols - 1, j),
                bandwidth, latency, bidirectional);
        }
        
        // 垂直环形连接（最上和最下行）
        for (int i = 0; i < cols; i++) {
            connect(coord_to_id(i, 0), coord_to_id(i, rows - 1),
                bandwidth, latency, bidirectional);
        }
    }

}

Route Mesh::route(DeviceId src, DeviceId dest) const noexcept {
    // assert npus are in valid range
    assert(0 <= src && src < npus_count);
    assert(0 <= dest && dest < npus_count);

    auto [src_x, src_y] = id_to_coord(src);
    auto [dest_x, dest_y] = id_to_coord(dest);

    Route path;
    path.push_back(devices[src]);

    // X轴方向路由
    while (src_x != dest_x) {
        int step = (dest_x > src_x) ? 1 : -1;
        if (wrap_around_ && abs(dest_x - src_x) > cols_ / 2) {
            step *= -1;
        }
        src_x = (src_x + step + cols_) % cols_;
        path.push_back(devices[coord_to_id(src_x, src_y)]);
    }

    // Y轴方向路由
    while (src_y != dest_y) {
        int step = (dest_y > src_y) ? 1 : -1;
        if (wrap_around_ && abs(dest_y - src_y) > rows_ / 2) {
            step *= -1;
        }
        src_y = (src_y + step + rows_) % rows_;
        path.push_back(devices[coord_to_id(src_x, src_y)]);
    }

    return path;
}

DeviceId Mesh::coord_to_id(int x, int y) const noexcept {
    return y * cols_ + x;
}

std::pair<int, int> Mesh::id_to_coord(DeviceId id) const noexcept {
    return {id % cols_, id / cols_};
}