# Backtest



### Commands so i don't forget them

#### Producer
- Enter container
    ```bash 
    docker exec -it backtest_kafka bash
    ```
- Creating topic named `test-topic`
    ```bash
    /usr/bin/kafka-topics --create --topic test-topic --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1
    ```
- Producing:
    ```bash
    /usr/bin/kafka-console-producer --broker-list localhost:9092 --topic test-topic
    ```

- Consuming:
    ```bash
    /usr/bin/kafka-console-consumer --bootstrap-server localhost:9092 --topic test-topic --from-beginning
    ```

- View all topics:
    ```bash
    /usr/bin/kafka-topics --list --bootstrap-server localhost:9092
    ```

- Producing with low latency:
    ```bash
    /usr/bin/kafka-console-producer --bootstrap-server localhost:9092 --topic test-topic --producer-property linger.ms=0
    ```

