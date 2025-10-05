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
| Run Tests  |`ctest --test-dir ./build/quarry`   |
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





## Random


- Need to have main loop
- Each stock opens its own thread that handles its input
  - Need to make sure there are no duplicates
- Main loop should not be stopped to add or remove stocks
  - Stocks to track should be externally added (gallery service)

---

I want to save my strategy information in postgres, but its going to be unstructured data most likely, maybe assign an id to each strategy?

### Kafka
- Where and when do kafka topics get made?
    - dockerfile in repo root?

### Chisel
- Strategy Table: 
    - Auto Increment ID
    - name
    - daily_average_return
    - strategy_time
    - created_at
    - updated_at

- Strategy Abstract class (can expose to python later):
    -  buy_or_sell : boolean
    -  quantity : int
    -  when_to_execute: datetime
