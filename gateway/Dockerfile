# 构建阶段 
FROM ubuntu:22.04 AS builder 
 
# 安装系统依赖 
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    git \
    curl \
    wget \
    unzip \
    vim \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* 
 
# 设置环境变量 
ENV GO_VERSION=1.23.1 \
    PROTOC_VERSION=27.2 \
    TZ=Asia/Shanghai \
    GOPATH=/go


# 安装特定版本Protobuf编译器 
RUN wget -qO protoc.zip  https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOC_VERSION}/protoc-${PROTOC_VERSION}-linux-x86_64.zip\
    && unzip protoc.zip  -d /usr/local \
    && rm protoc.zip  
 
# 安装Go语言环境
RUN wget -qO go.tar.gz  https://dl.google.com/go/go${GO_VERSION}.linux-amd64.tar.gz  && \
    tar -C /usr/local -xzf go.tar.gz  && \
    rm go.tar.gz  
 
ENV PATH="/usr/local/go/bin:${GOPATH}/bin:${PATH}"

# 安装Go工具链 
RUN go install google.golang.org/protobuf/cmd/protoc-gen-go@latest  && \
    go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest  
 
# 设置工作目录 
WORKDIR /app 
 
# 复制项目文件 
COPY proto/ ./proto/
COPY *.go .
COPY go.mod  .
COPY go.sum  .
 
# 生成gRPC代码 
RUN protoc --proto_path=proto \
    --go_out=paths=source_relative:./proto \
    --go-grpc_out=paths=source_relative:./proto \
    proto/service.proto
#    && mv service.pb.go service_grpc.pb.go proto
 
# 下载依赖 
RUN go mod download 
 
# 编译可执行文件 
RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-w -s" -o gateway .
 
## 运行阶段 
#FROM ubuntu:22.04 
#
## 设置运行时环境 
#ENV TZ=Asia/Shanghai 
#RUN apt-get update && apt-get install -y --no-install-recommends \
#    ca-certificates \
#    tzdata \
#    && ln -fs /usr/share/zoneinfo/${TZ} /etc/localtime \
#    && dpkg-reconfigure -f noninteractive tzdata \
#    && apt-get clean \
#    && rm -rf /var/lib/apt/lists/* 
# 
## 从构建阶段复制文件 
#COPY --from=builder /app/gateway /app/
#COPY --from=builder /go/pkg/mod /go/pkg/mod 
# 
## 设置工作目录和用户 
#WORKDIR /app 
 
# 暴露端口 
EXPOSE 8080 
 
# 启动命令 
CMD ["/app/gateway"]