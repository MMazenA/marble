# Marble

Mono-repo for stock ingestion, back testing, and analysis

## Requirements
- C++23
- CMake Version ^3.29
- Docker ^27.1.2
- Ninja ^1.13.1

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


### Common CMake Commands
| Name    | Command |
| -------- | ------- |
| Build Target  |`cmake --build --preset dev --target help`   |
| Run Tests  |`ctest --preset dev`   |
| Clear Build  |`rm -rf build`   |
| Install Dependencies  |`cmake --preset dev`   |

### Generate Dependency Graphs
```bash
cd build
cmake --graphviz=project.dot ..
dot -T svg project.dot -o project.svg
```





### Kafka Commands


| Name    | Command |
| -------- | ------- |
| Enter container  | `docker exec -it marble_kafka bash`   |
| Create Topic  | `bash /usr/bin/kafka-topics --create --topic test-topic --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1`   |
| Producing  | `/usr/bin/kafka-console-producer --broker-list localhost:9092 --topic test-topic`   |
| Consuming | `/usr/bin/kafka-console-consumer --bootstrap-server localhost:9092 --topic test-topic --from-beginning`    |
| View Topics    | `/usr/bin/kafka-topics --list --bootstrap-server localhost:9092`    |
| Consume Low Latency    | `/usr/bin/kafka-console-producer --bootstrap-server localhost:9092 --topic test-topic --producer-property linger.ms=0`    |
