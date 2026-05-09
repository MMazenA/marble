# Marble

Mono-repo for stock ingestion, back testing, and analysis

## Requirements
- C++23
- CMake Version ^3.29
- Docker ^27.1.2
- Ninja ^1.13.1
- Clang ^17 
- MSVC
  - GCC Not Tested

## Build and Run
1. CD into the root `/marble`
2. Give executable permissions 
    ```bash
    chmod +x start.sh
    ```
3. Execute run command
    ```bash
    ./start.sh
    ```  
## Dev
The idea is that quarry will handle data retrieval, chisel controls backtesting, and gallery will present findings. 


### Common Commands
| Name    | Command |
| -------- | ------- |
| Build Target  |`cmake --build --preset dev --target help`   |
| Run Tests  |`ctest --preset dev`   |
| Clear Build  |`rm -rf build`   |
| Install Dependencies  |`cmake --preset dev`   |
| GrpcUI | `grpcui -plaintext localhost:50051` |
| Profiling | `codesign --force --sign - --timestamp=none --entitlements debug.plist ./build/executable` |

### Generate Dependency Graphs
```bash
cd build
cmake --graphviz=project.dot ..
dot -T svg project.dot -o project.svg
```

