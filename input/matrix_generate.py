import numpy as np
import argparse

def generate_communication_matrix(num_nodes, chunk_sizes, sparsity=0.1, symmetric=False, diagonal_zero=True):
    """
    生成随机通信矩阵
    :param num_nodes: 节点数量 (n x n 矩阵)
    :param chunk_sizes: 可选的数据块大小列表 (如 [256, 512, 1024])
    :param sparsity: 稀疏度 (0-1之间，表示有多少比例的通信是0)
    :param symmetric: 是否生成对称矩阵 (模拟双向对称通信)
    :param diagonal_zero: 对角线是否强制为0 (节点不给自己发数据)
    :return: n x n 的通信矩阵
    """
    # 初始化全0矩阵
    matrix = np.zeros((num_nodes, num_nodes), dtype=int)
    
    # 生成随机索引（跳过对角线）
    rows, cols = np.indices((num_nodes, num_nodes))
    if diagonal_zero:
        mask = (rows != cols)  # 排除对角线
    else:
        mask = np.ones((num_nodes, num_nodes), dtype=bool)
    
    # 应用稀疏度
    if sparsity > 0:
        sparsity_mask = np.random.random((num_nodes, num_nodes)) > sparsity
        mask = mask & sparsity_mask
    
    # 填充随机数据块大小
    matrix[mask] = np.random.choice(chunk_sizes, size=np.sum(mask))
    
    # 如果需要对称矩阵
    if symmetric:
        matrix = np.triu(matrix)  # 取上三角
        matrix = matrix + matrix.T - np.diag(matrix.diagonal())  # 构建对称矩阵
    
    return matrix

def save_matrix_to_file(matrix, filename):
    """保存矩阵到文件"""
    np.savetxt(filename, matrix, fmt='%d', delimiter='\t')
    print(f"矩阵已保存到 {filename}")

def print_matrix(matrix):
    """打印矩阵"""
    np.set_printoptions(linewidth=120, threshold=np.inf)
    print("生成的通信矩阵：")
    print(matrix)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="生成随机通信矩阵")
    parser.add_argument("--nodes", type=int, default=8, help="节点数量")
    parser.add_argument("--output", type=str, default="MeshChunkMatrix.txt", help="输出文件名")
    parser.add_argument("--sizes", nargs='+', type=int, default=[0, 256, 512, 1024, 2048], help="可选的数据块大小")
    parser.add_argument("--sparsity", type=float, default=0.2, help="稀疏度 (0-1)")
    parser.add_argument("--symmetric", action="store_true", help="生成对称矩阵")
    args = parser.parse_args()

    matrix = generate_communication_matrix(
        num_nodes=args.nodes,
        chunk_sizes=args.sizes,
        sparsity=args.sparsity,
        symmetric=args.symmetric,
        diagonal_zero=True
    )

    # print_matrix(matrix)
    save_matrix_to_file(matrix, args.output)