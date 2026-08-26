#include "SIMD_MathCore.cpp"
#include "JumpDiffusionModel.cpp"
#include "MonteCarloSimulator.cpp"
#include "LiquidityGraph.cpp"
#include <iostream>
#include <chrono>

using namespace chronorisk;

int main() {
    std::cout << "=========================================================\n";
    std::cout << "     MASTERCARD CHRONORISK: QUANT LIQUIDITY ENGINE       \n";
    std::cout << "=========================================================\n";

    // 1. Initialize Currency Corridor Liquidity Graph
    liquidity::LiquidityGraph liquidity_graph;
    liquidity_graph.RegisterCurrencyCorridor("USD", 500'000'000'00ULL); // $500M Reserve
    liquidity_graph.RegisterCurrencyCorridor("EUR", 420'000'000'00ULL); // 420M EUR Reserve
    liquidity_graph.RegisterCurrencyCorridor("GBP", 280'000'000'00ULL); // 280M GBP Reserve

    std::cout << "[CHRONORISK-CORE] Liquidity corridors initialized.\n";

    // 2. Setup Jump-Diffusion Parameters for USD/EUR corridor
    quant::JumpDiffusionParameters usd_eur_params{
        .spot_rate = 1.0850,
        .drift = 0.015,
        .volatility = 0.082,
        .jump_intensity = 3.5,     // ~3.5 significant jumps per year
        .jump_mean = -0.005,
        .jump_std_dev = 0.025
    };

    std::cout << "[CHRONORISK-CORE] Initiating Parallel Monte Carlo Liquidity Stress-Test...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    // 3. Execute 250,000 Path Monte Carlo Simulation across all CPU cores
    auto results = quant::MonteCarloSimulator::RunParallelSimulation(
        usd_eur_params,
        1.0 / 252.0, // 1-day clearing horizon
        50,          // 50 intra-day discretization steps
        250'000      // 250k stochastic paths
    );

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "=========================================================\n";
    std::cout << " SIMULATION COMPLETED IN: " << elapsed_ms << " ms\n";
    std::cout << " Expected 1-Day Future Rate : " << results.expected_future_rate << "\n";
    std::cout << " Value at Risk 99% (VaR)    : " << results.value_at_risk_99 << "\n";
    std::cout << " Expected Shortfall 99%     : " << results.expected_shortfall_99 << "\n";
    std::cout << " Max Liquidity Drawdown     : " << results.liquidity_drawdown_max << "\n";
    std::cout << "=========================================================\n";

    return 0;
}
