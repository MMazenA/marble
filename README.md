# Marble

Mono-repo for stock ingestion, back testing, and analysis



### Commands so i don't forget them


| Name    | Command |
| -------- | ------- |
| Enter container  | `docker exec -it marble_kafka bash`   |
| Create Topic  | `bash /usr/bin/kafka-topics --create --topic test-topic --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1`   |
| Producing  | `/usr/bin/kafka-console-producer --broker-list localhost:9092 --topic test-topic`   |
| Consuming | `/usr/bin/kafka-console-consumer --bootstrap-server localhost:9092 --topic test-topic --from-beginning`    |
| View Topics    | `/usr/bin/kafka-topics --list --bootstrap-server localhost:9092`    |
| Consume Low Latency    | `/usr/bin/kafka-console-producer --bootstrap-server localhost:9092 --topic test-topic --producer-property linger.ms=0`    |






### Thoughts

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
    - 
