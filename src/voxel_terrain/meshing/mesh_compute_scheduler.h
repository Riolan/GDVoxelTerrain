#ifndef MESH_COMPUTE_SCHEDULER_H
#define MESH_COMPUTE_SCHEDULER_H

#include "voxel_octree_node.h"
#include <atomic>
#include <concurrent_priority_queue.h>
#include <concurrent_queue.h>
#include <functional>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <thread>

using namespace godot;

class JarVoxelTerrain;

struct ScheduledChunk
{
  public:
    VoxelOctreeNode &node;

    ScheduledChunk(VoxelOctreeNode &node) : node(node)
    {
    }

    bool operator<(const VoxelOctreeNode &other) const
    {
        return node.LoD > other.LoD;
    }
};

class MeshComputeScheduler
{
  private:
    // concurrency::concurrent_queue<ScheduledChunk*> ChunksToAdd;
    concurrency::concurrent_priority_queue<ScheduledChunk*> ChunksToAdd;
    concurrency::concurrent_queue<std::pair<VoxelOctreeNode*, ChunkMeshData*>> ChunksToProcess;

    std::atomic<int> _activeTasks;
    int _maxConcurrentTasks;

    // Debug variables
    int _totalTris;
    int _prevTris;

    void process_queue(JarVoxelTerrain &terrain);
    void run_task(const JarVoxelTerrain &terrain, ScheduledChunk &chunk);

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
