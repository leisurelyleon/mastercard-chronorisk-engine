#include "LargeTabularModel.cpp"
#include "GraphNeuralNetwork.cpp"
#include <future>
#include <chrono>
#include <iostream>

using namespace chronorisk::fraud;

int main() {
    std::cout << "=========================================================\n";
    std::cout << "     MASTERCARD CHRONORISK: ASYNC FRAUD SCORER           \n";
    std::cout << "=========================================================\n";

    LargeTabularModel tabular_model("models/fraud_ltm_v4.onnx");
    GraphNeuralNetwork gnn_model;

    // A simulated incoming transaction
    TransactionFeatures tx_features{ .amount_usd = 12500.0f, .time_since_last_tx = 300.0f, .distance_from_home = 600.0f, .merchant_category_risk = 0.8f, .velocity_1hr = 5.0f };
    uint64_t cardholder_id = 8472910499; // Ends in 99, triggering our mock GNN ring detection
    uint64_t merchant_id = 11203040;

    std::cout << "[SCORER] Inbound ISO 20022 message parsed. Initiating 50ms deadline.\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    // Spawn inference tasks concurrently
    std::future<float> ltm_future = std::async(std::launch::async, [&]() {
        return tabular_model.PredictRiskScore(tx_features);
    });

    std::future<float> gnn_future = std::async(std::launch::async, [&]() {
        return gnn_model.EvaluateSubgraphRisk(cardholder_id, merchant_id);
    });

    // Wait with a strict SLA timeout of 40ms to leave network transmission buffer
    auto timeout = std::chrono::milliseconds(40);
    bool ltm_ready = ltm_future.wait_for(timeout) == std::future_status::ready;
    bool gnn_ready = gnn_future.wait_for(timeout) == std::future_status::ready;

    if (ltm_ready && gnn_ready) {
        float ltm_score = ltm_future.get();
        float gnn_score = gnn_future.get();
        
        // Ensemble the models: 40% Tabular, 60% Graph Topology
        float final_risk_score = (ltm_score * 0.4f) + (gnn_score * 0.6f);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "  -> LTM Score: " << ltm_score << "\n";
        std::cout << "  -> GNN Score: " << gnn_score << "\n";
        std::cout << "  -> Final Ensemble Risk: " << final_risk_score << "\n";
        std::cout << "[SCORER] Computation completed in " << elapsed_ms << "ms.\n";
        
        if (final_risk_score > 0.75f) {
            std::cout << "[SCORER] ACTION: TRANSACTION DECLINED (Fraud Threshold Exceeded).\n";
        } else {
            std::cout << "[SCORER] ACTION: TRANSACTION APPROVED.\n";
        }
    } else {
        std::cout << "[SCORER] CRITICAL: 50ms SLA BREACH. Falling back to rule-based engine.\n";
    }

    return 0;
}
