/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "common/EventQueue.h"
#include "common/NetworkParser.h"
#include "congestion_aware/Chunk.h"
#include "congestion_aware/Helper.h"
#include <iostream>
#include <fstream>
#include <cassert>
using namespace NetworkAnalytical;
using namespace NetworkAnalyticalCongestionAware;

void chunk_arrived_callback(void* const event_queue_ptr) {
    // typecast event_queue_ptr
    auto* const event_queue = static_cast<EventQueue*>(event_queue_ptr);

    // print chunk arrival time
    const auto current_time = event_queue->get_current_time();
    std::cout << "A chunk arrived at destination at time: " << current_time << " ns" << std::endl;
}
std::vector<std::vector<int>> read_send_matrix(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::vector<int>> matrix;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int value;
        while (iss >> value) {
            row.push_back(value);
        }
        matrix.push_back(row);
    }
    return matrix;
}

void simulate_all_to_all(
    const std::vector<std::vector<int>>& send_matrix,
    Topology* topology,
    EventQueue* event_queue
) {
    int npus_count = send_matrix.size();

    for (int i = 0; i < npus_count; i++) {
        for (int j = 0; j < npus_count; j++) {
            if (i == j) continue;  // 跳过自身

            int chunk_size = send_matrix[i][j];
            if (chunk_size <= 0) continue;  // 不发送 0 或负值数据

            // 获取路由
            auto route = topology->route(i, j);

            // 创建模拟 chunk（假设 Chunk 只需要 size 和 route）
            auto chunk = std::make_unique<Chunk>(
                chunk_size,
                route,
                chunk_arrived_callback,
                static_cast<void*>(event_queue)
            );

            // 发送
            topology->send(std::move(chunk));
        }
    }
}
int main() {
    // Instantiate shared resources
    const auto event_queue = std::make_shared<EventQueue>();
    Topology::set_event_queue(event_queue);

    // Parse network config and create topology
    const auto network_parser = NetworkParser("../input/Mesh.yml");
    const auto topology = construct_topology(network_parser);
    const auto npus_count = topology->get_npus_count();
    const auto devices_count = topology->get_devices_count();

    auto send_matrix = read_send_matrix("../input/MeshChunkMatrix.txt");
    assert(send_matrix.size()==npus_count);
    simulate_all_to_all(send_matrix, topology.get(), event_queue.get());
    // // message settings
    // const auto chunk_size = 1'048'576;  // 1 MB

    // // Run All-Gather
    // for (int i = 0; i < npus_count; i++) {
    //     for (int j = 0; j < npus_count; j++) {
    //         if (i == j) {
    //             continue;
    //         }

    //         // crate a chunk
    //         auto route = topology->route(i, j);
    //         auto* event_queue_ptr = static_cast<void*>(event_queue.get());
    //         auto chunk = std::make_unique<Chunk>(chunk_size, route, chunk_arrived_callback, event_queue_ptr);

    //         // send a chunk
    //         topology->send(std::move(chunk));
    //     }
    // }

    // Run simulation
    while (!event_queue->finished()) {
        event_queue->proceed();
    }

    // Print simulation result
    const auto finish_time = event_queue->get_current_time();
    std::cout << "Total NPUs Count: " << npus_count << std::endl;
    std::cout << "Total devices Count: " << devices_count << std::endl;
    std::cout << "Simulation finished at time: " << finish_time << " ns" << std::endl;

    return 0;
}
