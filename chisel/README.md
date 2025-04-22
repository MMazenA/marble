# Chisel

Back testing and realtime strategy engine


### Commands so i don't forget them


| Name    | Command |
| -------- | ------- |
| Enter container  | `docker exec -it backtest_kafka bash`   |
| Create Topic  | `bash /usr/bin/kafka-topics --create --topic test-topic --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1`   |
| Producing  | `/usr/bin/kafka-console-producer --broker-list localhost:9092 --topic test-topic`   |
| Consuming | `/usr/bin/kafka-console-consumer --bootstrap-server localhost:9092 --topic test-topic --from-beginning`    |
| View Topics    | `/usr/bin/kafka-topics --list --bootstrap-server localhost:9092`    |
| Consume Low Latency    | `/usr/bin/kafka-console-producer --bootstrap-server localhost:9092 --topic test-topic --producer-property linger.ms=0`    |
