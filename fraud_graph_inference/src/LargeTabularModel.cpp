#pragma once
#include <vector>
#include <string>
#include <iostream>

namespace chronorisk::fraud {

struct TransactionFeatures {
    float amount_usd;
    float time_since_last_tx;
    float distance_from_home;
    float merchant_category_risk;
    float velocity_1hr;
};

class LargeTabularModel {
public:
    LargeTabularModel(const std::string& onnx_model_path) {
        std::cout << "[LTM-INFERENCE] Loading Tabular ONNX Model from: " << onnx_model_path << "\n";
        // Implementation would bind to Ort::Session (ONNX Runtime)
    }

    // Returns a risk probability between 0.0 (Safe) and 1.0 (High Risk)
    float PredictRiskScore(const TransactionFeatures& features) {
        // Mocking the tensor input creation and ONNX session run
        std::vector<float> input_tensor = {
            features.amount_usd,
            features.time_since_last_tx,
            features.distance_from_home,
            features.merchant_category_risk,
            features.velocity_1hr
        };

        // Simulated inference math (would normally be executed on GPU/NPU)
        float base_risk = (features.amount_usd > 10000.0f) ? 0.6f : 0.1f;
        if (features.distance_from_home > 500.0f && features.time_since_last_tx < 3600.0f) {
            base_risk += 0.3f; // Impossible travel velocity anomaly
        }

        return std::min(base_risk, 1.0f);
    }
};

} // namespace chronorisk::fraud
