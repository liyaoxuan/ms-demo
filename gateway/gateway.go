// gateway.go
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"net/http"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	pb "github.com/liyaoxuan/ms-demo/gateway/proto" // 替换为实际的proto包路径
)

type ComputeRequest struct {
	TimeA int32 `json:"time_a"`
	TimeB int32 `json:"time_b"`
	TimeC int32 `json:"time_c"`
}

type ComputeResponse struct {
	Success bool `json:"success"`
}

type ServiceAClient struct {
	conn   *grpc.ClientConn
	client pb.ServiceAClient
}

func NewServiceAClient(addr string) (*ServiceAClient, error) {
	//conn, err := grpc.Dial(addr,
	//	grpc.WithTransportCredentials(insecure.NewCredentials()),
	//	grpc.WithTimeout(5*time.Second),
	//)
	conn, err := grpc.NewClient(addr, 
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		return nil, fmt.Errorf("failed to connect: %v", err)
	}

	return &ServiceAClient{
		conn:   conn,
		client: pb.NewServiceAClient(conn),
	}, nil
}

func (c *ServiceAClient) Compute(ctx context.Context, timeA int32, timeB int32, timeC int32) (bool, error) {
	req := &pb.RequestA{TimeA: timeA, TimeB: timeB, TimeC: timeC}
	res, err := c.client.Compute(ctx, req)
	if err != nil {
		return false, fmt.Errorf("gRPC call failed: %v", err)
	}
	return res.Success, nil
}

func main() {
	var (
		gatewayPort  = flag.Int("port", 8080, "Gateway HTTP port")
		serviceAAddr = flag.String("serviceA", "svc_a:50050", "ServiceA address")
	)
	flag.Parse()

	// 初始化gRPC客户端
	client, err := NewServiceAClient(*serviceAAddr)
	if err != nil {
		log.Fatalf("Failed to create gRPC client: %v", err)
	}
	defer client.conn.Close()

	// 配置HTTP路由
	http.HandleFunc("/api/compute", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
			return
		}

		var req ComputeRequest
		if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
			http.Error(w, "Invalid JSON format", http.StatusBadRequest)
			return
		}

		success, err := client.Compute(r.Context(), req.TimeA, req.TimeB, req.TimeC)
		if err != nil {
			log.Printf("gRPC error: %v", err)
			http.Error(w, "Service unavailable", http.StatusBadGateway)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(ComputeResponse{Success: success})
	})

	log.Printf("Gateway listening on :%d", *gatewayPort)
	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%d", *gatewayPort), nil))
}
