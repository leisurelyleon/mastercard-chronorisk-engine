package internal

import (
	"log"
)

// Processes transactions through a mocked Two-Phase Commit (2PC) protocol
func ProcessSettlements(txChan <-chan TransactionEvent) {
	log.Println("[STATE-MACHINE] Two-Phase Commit Settlement Engine Active.")

	for tx := range txChan {
		if tx.RiskScore > 0.75 {
			log.Printf("[STATE-MACHINE] TX %s REJECTED. Fraud threshold exceeded (Score: %.2f).\n", tx.TransactionID, tx.RiskScore)
			continue
		}

		// Phase 1: Prepare (Locking funds conceptually in the C++ Liquidity Engine)
		log.Printf("[STATE-MACHINE] Phase 1: PREPARE lock for TX %s\n", tx.TransactionID)
		
		// Phase 2: Commit (Updating the distributed ledger)
		log.Printf("[STATE-MACHINE] Phase 2: COMMIT ledger entry for TX %s\n", tx.TransactionID)
		
		RecordTelemetrySuccess()
	}
}
