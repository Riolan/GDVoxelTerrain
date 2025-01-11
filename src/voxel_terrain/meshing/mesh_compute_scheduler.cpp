#include "mesh_compute_scheduler.h"
#include "chunk_mesh_data.h"
#include "jar_voxel_terrain.h"
#include "surface_nets/surface_nets.h"
#include "voxel_octree_node.h"

MeshComputeScheduler::MeshComputeScheduler(int maxConcurrentTasks)
    : _maxConcurrentTasks(maxConcurrentTasks), _activeTasks(0), _totalTris(0), _prevTris(0)
{
}

void MeshComputeScheduler::enqueue(VoxelOctreeNode *node)
{
    ChunksToAdd.push(new ScheduledChunk(node));
}

void MeshComputeScheduler::process(JarVoxelTerrain *terrain)
{
    _prevTris = _totalTris;
    if (!terrain->is_building())
    {
        process_queue(terrain);
    }
    while (!ChunksToProcess.empty())
    {
        std::pair<VoxelOctreeNode *, ChunkMeshData *> tuple;
        if (ChunksToProcess.try_pop(tuple))
        {
            auto [node, chunkMeshData] = tuple;
            node->update_chunk(terrain, chunkMeshData);
        }
    }
}

void MeshComputeScheduler::process_queue(JarVoxelTerrain *terrain)
{
    while (!ChunksToAdd.empty())
    {
        if (_activeTasks >= _maxConcurrentTasks)
            return;
        ScheduledChunk *chunk;
        if (ChunksToAdd.try_pop(chunk))
        {
            _activeTasks++;
            run_task(terrain, *chunk);
        }
        else
            return;
    }
}

void MeshComputeScheduler::run_task(const JarVoxelTerrain *terrain, ScheduledChunk &chunk)
{
    //std::thread([this, terrain, chunk]() {

    int triCount = 0;

    auto meshCompute = SurfaceNets(terrain, chunk);
    ChunkMeshData *chunkMeshData = meshCompute.generate_mesh_data(terrain);

    if (chunkMeshData != nullptr)
    {
        // if (JarVoxelTerrain::render_details())
        // {
        //     chunkMeshData->instantiate_details();
        // }
        ChunksToProcess.push(std::make_pair(chunk.node, chunkMeshData));
    }
    _activeTasks--;

    //}).detach();
}

void MeshComputeScheduler::clear_queue()
{
    ChunksToAdd.clear();
    ChunksToProcess.clear();
}
