# Code_Nhom-15_Lap-trinh-tien-hoa
## Dataset có sẵn nằm trong thu mục csp_dataset
## Chạy code

1. Chỉnh sửa tham số trong file csp.cpp
```
freopen("result/40/output5a.txt", "w", stdout); -> output file
Problem problem = Problem::from_file("dataset_csp/problem5a.csp"); -> input file
int num_run = 20; -> số lần chạy
option.population_size = 40; -> số ca thể
option.max_generation = 1000; -> số thế hệ
option.elitist_percentage = 0.5; -> % số cá thể cha mẹ được chọn sang thế hệ tiếp theo
```

2. Chạy file csp.cpp
```
cd ~/kafka/kafka_2.13-3.9.0/
bin/zookeeper-server-start.sh config/zookeeper.properties
bin/kafka-server-start.sh config/server.properties
bin/kafka-topics.sh --create --topic test --bootstrap-server <<YOUR IP ADDRESS>>:9092 
bin/kafka-topics.sh --create --topic testOutput --bootstrap-server <<YOUR IP ADDRESS>>:9092 
```
3. Kết quả thu được nằm ở trong thư mục result
