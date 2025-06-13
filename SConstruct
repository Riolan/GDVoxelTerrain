#!/usr/bin/env python
import os
import sys
# from scons_compiledb import compile_db # Import the compile_db function # Call the compile_db function to enable compile_commands.json generation 
# compile_db()


# Import the SConstruct from godot-cpp
env = SConscript("godot-cpp/SConstruct")

# Add necessary include directories
env.Append(CPPPATH=[
    "src/glm/",
    "src/utility/",
    "src/",
    "src/sdf/",
    "src/voxel_terrain/",
    "src/voxel_terrain/meshing",
    "src/voxel_terrain/meshing/adaptive_surface_nets",
    "src/voxel_terrain/world",
    "src/voxel_terrain/population",
    "src/voxel_terrain/population/details",
    "src/voxel_terrain/population/features",
])

# # Add main source files
sources = Glob("src/*.cpp") + Glob("src/utility/*.cpp") + Glob("src/sdf/*.cpp") + \
      Glob("src/voxel_terrain/*.cpp") + Glob("src/voxel_terrain/meshing/*.cpp") + \
        Glob("src/voxel_terrain/meshing/adaptive_surface_nets/*.cpp") + \
            Glob("src/voxel_terrain/meshing/stitched_surface_nets/*.cpp") +\
            Glob("src/voxel_terrain/population/*.cpp") + Glob("src/voxel_terrain/population/details/*.cpp") + \
            Glob("src/voxel_terrain/population/features/*.cpp")

#compiler flags
if env['PLATFORM'] == 'windows':
    if env['CXX'] == 'x86_64-w64-mingw32-g++':
        env.Append(CXXFLAGS=['-std=c++11'])  # Example flags for MinGW
    elif env['CXX'] == 'cl':
        env.Append(CXXFLAGS=['/EHsc'])  # Apply /EHsc for MSVC

print(env['PLATFORM'])

if env['platform'] == 'linux':
    print("Configuring for Linux/POSIX...")
    # On Linux, pkg-config is the standard and best way to find libraries.
    try:
        env.ParseConfig('pkg-config --cflags --libs tbb')

        print("Added tbb.")
    except OSError:
        print("Error: 'pkg-config --cflags --libs tbb' failed.")
        print("Please ensure libtbb-dev is installed and pkg-config can find it.")
        Exit(1)

# Handle different platforms
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "project/addons/jar_voxel_terrain/bin/jar_voxel_terrain.{}.{}.framework/jar_voxel_terrain.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "project/addons/jar_voxel_terrain/bin/jar_voxel_terrain.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "project/addons/jar_voxel_terrain/bin/jar_voxel_terrain.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "project/addons/jar_voxel_terrain/bin/jar_voxel_terrain{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
