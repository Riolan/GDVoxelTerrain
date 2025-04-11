# Godot Voxel Terrain Plugin

![Banner Image](https://github.com/JorisAR/GDVoxelTerrain/blob/main/banner.png?raw=true)

This project adds a smooth voxel terrain system to godot. 
More precisely, it uses an octree to store an SDF, which is then meshed using a custom version of surface nets. Level of detail systems are in place for large viewing distances.


## Getting Started

### Installation

- Build the source using scons to your target platforms.
- move the contents of the addons folder to the addons folder in your project.

## Usage

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

In particular, here are some areas of interest:
- Optimizing LOD connections, i.e. remove the overdraw/duplicate triangle generation.
- Multi-materials
- Optimize generating colliders
- Add more interesting SDF options
    - Combine SDFs
    - Better noise based SDFs for more realistic terrain
- Documentation & icons
- Navigation system
- Dual contouring extension

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
