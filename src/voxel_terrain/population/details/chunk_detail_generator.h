/*
class chunk_detail_generator
{
  private:
  public:
    chunk_detail_generator(Array<TerrainDetail> &details, JarVoxelTerrain &terrain);
    ~chunk_detail_generator();
};

chunk_detail_generator::chunk_detail_generator()
{
}

chunk_detail_generator::~chunk_detail_generator()
{
}

Array<MultiMesh> CreateDetailMultiMeshes(const ChunkMeshData &chunkMeshData)
{
    _verts = chunkMeshData.MeshArray[(int) Mesh.ArrayType.Vertex].AsVector3Array();
    _indices = chunkMeshData.MeshArray[(int) Mesh.ArrayType.Index].AsInt32Array();
    _normals = chunkMeshData.MeshArray[(int) Mesh.ArrayType.Normal].AsVector3Array();
    _colors = chunkMeshData.MeshArray[(int) Mesh.ArrayType.Color].AsColorArray();
    
    var chunkCenter = chunkMeshData.Bounds.GetCenter();
    if (Details.Length <= 0)
        return Array.Empty<MultiMesh>();
    var ts = new List<Transform3D>[Details.Length];
    for (var index = 0; index < ts.Length; index++)
    {
        ts[index] = new List<Transform3D>();
    }

    for (var i = 0; i < _indices.Length; i += 3)
    {
        var vertA = _indices[i];
        var vertB = _indices[i + 1];
        var vertC = _indices[i + 2];

        var posA = _verts[vertA];
        var posB = _verts[vertB];
        var posC = _verts[vertC];

        var normA = _normals[vertA];
        var normB = _normals[vertB];
        var normC = _normals[vertC];

        var colA = _colors[vertA];
        var colB = _colors[vertB];
        var colC = _colors[vertC];

        // Calculate the area of the triangle
        var edge1 = posB - posA;
        var edge2 = posC - posA;
        var area = edge1.Cross(edge2).Length() * 0.5f;

        // sample detail:

        for (var d = 0; d < Details.Length; d++)
        {
            var detail = Details[d];
            if (chunkMeshData.LoD > detail.MaxLoD)
                continue;
            var n = area * detail.Density;
            var nInt = Mathf.FloorToInt(n);
            var nFloat = DeterministicFloat((posA + posB + posC) * (1 + d));
            nInt += nFloat < (n - nInt) ? 1 : 0;

            for (var j = 0; j < nInt; j++)
            {
                // sample 2 barycentric coordinates, calculate the last
                var r1 = DeterministicFloat(posA * (1 + d));
                var r2 = DeterministicFloat(posB * (1 + d));
                if (r1 + r2 >= 1)
                {
                    r1 = 1 - r1;
                    r2 = 1 - r2;
                }

                var position = posA + r1 * edge1 + r2 * edge2;
                var globalPosition = chunkCenter + position;
                if (!detail.IsInRange(globalPosition.Y) || !detail.SampleMask(globalPosition) ||
                    !detail.IsRightBiome(WorldBiomes.GetBiomeAt(globalPosition).BiomeId))
                    continue;

                var up = normA + r1 * (normB - normA) + r2 * (normC - normA);
                var color = colA + r1 * (colB - colA) + r2 * (colC - colA);
                up = up.Normalized();

                var dot = up.Dot(Vector3.Up);
                float minDot = detail.MaxSlope;

                if (!detail.LegalNormal(up) || color.R > 0.5)
                    continue;
                dot = (dot - minDot) / (1 - minDot);
                // this scales it down when on off slope i think?
                var height = Mathf.Pow(dot, 0.25f);

                if (!detail.AlignWithNormal)
                    up = Vector3.Up;

                var r3 = DeterministicFloat(posC * (1 + d));
                var r4 = DeterministicFloat((posA + posC) * (1 + d));

                ts[d].Add(TerrainPopulator.BuildTransform(
                    position, up, Vector3.One * Mathf.Lerp(detail.MinimumScale, detail.MaximumScale, r4),
                    r3 * 2 * float.Pi));
            }
        }
    }

    // todo omit details that are empty!

    var meshes = new List<MultiMesh>();

    for (var d = 0; d < Details.Length; d++)
    {
        var detail = Details[d];
        foreach (var lod in detail.Visuals)
        {
            var mesh = new MultiMesh(){TransformFormat = MultiMesh.TransformFormatEnum.Transform3D, Mesh = lod.Mesh,
                                       InstanceCount = ts[d].Count};

            for (var i = 0; i < ts[d].Count; i++)
            {
                mesh.SetInstanceTransform(i, ts[d][i]);
            }

            meshes.Add(mesh);
        }
    }

    return meshes.ToArray();
}*/