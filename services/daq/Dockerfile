# 使用 Debian Slim 作為基礎映像
FROM debian:bullseye-slim

# 安裝必要的套件與 RabbitMQ-C 庫
RUN apt-get update && apt-get install -y \
    wget \
    g++ \
    build-essential \
    git \
    libssl-dev \
    librabbitmq-dev \
    pkg-config \
    ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# 安裝最新版本的 CMake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.27.6/cmake-3.27.6-linux-x86_64.tar.gz && \
    tar -xzf cmake-3.27.6-linux-x86_64.tar.gz --strip-components=1 -C /usr/local && \
    rm cmake-3.27.6-linux-x86_64.tar.gz

# 設定工作目錄
WORKDIR /daq

# 複製程式碼與庫文件
COPY main.cpp .
COPY hsdaql.h .
COPY libhsdaq.a .
COPY libhsdaq.so .
COPY windows2linux.h .

# 編譯程式，鏈接 RabbitMQ-C 和 Hsdaq 庫
#RUN g++ -std=c++14 -o main main.cpp -I/usr/include/rabbitmq-c -L/usr/local/lib -L/daq/ -lrabbitmq -lhsdaq -lpthread
RUN g++ -o main main.cpp -I. -L. -lhsdaq -lrabbitmq -lpthread

# 設定執行時動態庫路徑
ENV LD_LIBRARY_PATH=/usr/local/lib:/daq/:$LD_LIBRARY_PATH

# 預設執行命令
CMD ["./main"]