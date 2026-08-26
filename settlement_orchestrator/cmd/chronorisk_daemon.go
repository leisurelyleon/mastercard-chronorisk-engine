package main

import (
	"log"
	"os"
	"os/signal"
	"sync"
	"syscall"

	"chronorisk_orchestrator/internal"
)

func main() {
	log.Println("=========================================================")
	log.Println(" MASTERCARD CHRONORISK: DISTRIBUTED SETTLEMENT DAEMON    ")
	log.Println("=========================================================")

	// Initialize channels for inter-goroutine communication
	txChannel := make(chan internal.TransactionEvent, 10000)
	
	var wg sync.WaitGroup

	// 1. Boot the Prometheus & gRPC Telemetry server
	wg.Add(1)
	go func() {
		defer wg.Done()
		internal.StartTelemetryServer(":9090")
	}()

	// 2. Boot the High-Throughput Kafka Consumer
	wg.Add(1)
	go func() {
		defer wg.Done()
		internal.ConsumeTransactionStream(txChannel)
	}()

	// 3. Boot the Two-Phase Commit State Machine Processor
	wg.Add(1)
	go func() {
		defer wg.Done()
		internal.ProcessSettlements(txChannel)
	}()

	// Graceful shutdown handling
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	<-sigChan

	log.Println("[DAEMON] Shutdown signal received. Draining queues...")
	close(txChannel)
	wg.Wait()
	log.Println("[DAEMON] Graceful shutdown complete.")
}
