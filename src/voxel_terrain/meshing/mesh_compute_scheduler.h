#ifndef MESH_COMPUTE_SCHEDULER_H
#define MESH_COMPUTE_SCHEDULER_H

#include "voxel_octree_node.h"
#include <atomic>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_priority_queue.h>

#include <functional>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <thread>
#include "utility/thread_pool.h"

using namespace godot;

class JarVoxelTerrain;


struct ChunkComparator {
    bool operator()(const VoxelOctreeNode *a, const VoxelOctreeNode *b) const {
        return a->get_lod() > b->get_lod();
    }
};

class MeshComputeScheduler
{
  private:
    tbb::concurrent_priority_queue<VoxelOctreeNode*, ChunkComparator> ChunksToAdd;
    tbb::concurrent_queue<std::pair<VoxelOctreeNode*, ChunkMeshData*>> ChunksToProcess;

    std::atomic<int> _activeTasks;
    int _maxConcurrentTasks;

    ThreadPool threadPool;

    // Debug variables
    int _totalTris;
    int _prevTris;

    void process_queue(JarVoxelTerrain &terrain);
    void run_task(const JarVoxelTerrain &terrain, VoxelOctreeNode &chunk);

  public:
    MeshComputeScheduler(int maxConcurrentTasks);
    void enqueue(VoxelOctreeNode &node);
    void process(JarVoxelTerrain &terrain);
    void clear_queue();

    bool is_meshing()
    {
        return !ChunksToAdd.empty();
    }
};

#endif // MESH_COMPUTE_SCHEDULER_H
