package internal

import (
	"log"
	"time"
)

type TransactionEvent struct {
	TransactionID string
	AmountCents   uint64
	Currency      string
	RiskScore     float32
}

// Simulates consuming a high-throughput Kafka topic containing risk-scored transactions
func ConsumeTransactionStream(txChan chan<- TransactionEvent) {
	log.Println("[KAFKA] Connected to topic: chronorisk.clearing.events")
	
	// Mocking a continuous stream of incoming transactions
	for i := 0; i < 5; i++ {
		time.Sleep(200 * time.Millisecond)
		
		event := TransactionEvent{
			TransactionID: "TX-99882231",
			AmountCents:   1250000, // $12,500.00
			Currency:      "USD",
			RiskScore:     0.12,
		}
		
		log.Printf("[KAFKA] Ingested TX %s for %s %d\n", event.TransactionID, event.Currency, event.AmountCents)
		txChan <- event
	}
}
