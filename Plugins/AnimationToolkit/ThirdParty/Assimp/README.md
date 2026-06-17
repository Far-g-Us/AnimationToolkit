# Assimp (Open Asset Import Library) Installation

## About Assimp

Assimp is a free, open-source library that supports 40+ 3D formats including:
- FBX (.fbx)
- glTF 2.0 (.gltf, .glb)
- BVH (.bvh)
- OBJ (.obj)
- COLLADA (.dae)
- 3DS (.3ds)
- Blender (.blend)
- And many more...

## Download

### Option 1: Pre-built binaries (recommended)

Download from: https://github.com/assimp/assimp/releases

Look for: `assimp-5.4.3-win64.zip` (or latest version)

### Option 2: Build from source

```bash
git clone https://github.com/assimp/assimp.git
cd assimp
cmake -S . -B build -DBUILD_SHARED_LIBS=ON
cmake --build build --config Release
cmake --build build --config Debug
```

## Installation Steps

### If using pre-built binaries:

1. Extract the downloaded zip
2. Copy files to this directory structure:

```
ThirdParty/Assimp/
├── include/
│   └── assimp/
│       ├── importer.hpp
│       ├── scene.h
│       ├── postprocess.h
│       └── ... (all headers from include/assimp/)
├── lib/
│   ├── dll/
│   │   └── assimp-vc143-mt.dll
│   └── Win64/
│       ├── Release/
│       │   └── assimp-vc143-mt.lib
│       └── Debug/
│           └── assimp-vc143-mtd.lib
```

### If building from source:

1. Copy headers from `assimp/include/` to `ThirdParty/Assimp/include/`
2. Copy `assimp.lib` to `ThirdParty/Assimp/lib/Win64/Release/`
3. Copy `assimpd.lib` to `ThirdParty/Assimp/lib/Win64/Debug/`
4. Copy `assimp.dll` to `ThirdParty/Assimp/lib/dll/`

## Supported Formats

| Format | Extension | Import | Export |
|--------|-----------|--------|--------|
| FBX | .fbx | Yes | Yes |
| glTF 2.0 | .gltf, .glb | Yes | Yes |
| BVH | .bvh | Yes | No |
| OBJ | .obj | Yes | Yes |
| COLLADA | .dae | Yes | Yes |
| 3DS | .3ds | Yes | Yes |
| Blender | .blend | Yes | No |
| STL | .stl | Yes | Yes |
| PLY | .ply | Yes | Yes |
| X3D | .x3d | Yes | No |

## Troubleshooting

### "Assimp not found" error

1. Make sure the `ThirdParty/Assimp/` directory exists
2. Check that headers are in `include/assimp/`
3. Check that DLL is in `lib/dll/`

### Linker errors

1. Make sure the library filename matches: `assimp-vc143-mt.lib`
2. Check platform is Win64
3. Verify the library is not corrupted

### DLL not found at runtime

1. Make sure `assimp-vc143-mt.dll` is in `lib/dll/`
2. For packaged builds, the DLL is automatically copied to `Binaries/Win64/`
3. The DLL will be automatically loaded at runtime

## Packaging for Distribution

The plugin automatically handles DLL distribution:
- For Editor: DLL is loaded from ThirdParty directory
- For Packaged Builds: DLL is copied to Binaries directory

## License

Assimp is licensed under the BSD 3-Clause License.
See: https://github.com/assimp/assimp/blob/master/LICENSE
