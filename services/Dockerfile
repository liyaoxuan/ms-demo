# 使用 Ubuntu 22.04 作为基础镜像
FROM ubuntu:22.04

# 设置环境变量，避免交互式安装时的提示
ENV DEBIAN_FRONTEND=noninteractive

ENV GRPC_INSTALL_DIR=/usr/local

# 安装系统依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    libcurl4-openssl-dev \
    libssl-dev \
    pkg-config \
    autoconf \
    libtool \
    nginx \
    vim \
    && rm -rf /var/lib/apt/lists/*


# 安装 gRPC 插件
RUN mkdir -p /grpc && cd /grpc \
    && git clone --recurse-submodules -b v1.66.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc . \
    && mkdir -p cmake/build \
    && cd cmake/build \
    && cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF CMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$GRPC_INSTALL_DIR ../.. \
    && make -j$(nproc) \
    && make install

# 设置工作目录
WORKDIR /app

# 复制代码到容器中
COPY service.proto service_a.cpp service_b.cpp service_c.cpp CMakeLists.txt common.cmake .

# 编译代码
RUN cd /app \
    && mkdir -p cmake/build \
    && cd cmake/build \
    && cmake -DCMAKE_INSTALL_PREFIX=$GRPC_INSTALL_DIR ../.. \
    && make -j$(nproc) \
    && cp service_a service_b service_c /usr/local/bin
    
# 复制 Nginx 配置文件
#COPY nginx.conf /etc/nginx/nginx.conf

# 暴露端口
#EXPOSE 80
EXPOSE 50050
#EXPOSE 50051
#EXPOSE 50052

## 启动脚本
#COPY start_services.sh /start_services.sh
#RUN chmod +x /start_services.sh

# 设置启动命令
#CMD ["/start_services.sh"]
CMD ["/bin/bash"]