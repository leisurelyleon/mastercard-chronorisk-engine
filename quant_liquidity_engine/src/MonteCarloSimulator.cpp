#pragma once
#include "JumpDiffusionModel.cpp"
#include <vector>
#include <numeric>
#include <algorithm>
#include <future>
#include <thread>

namespace chronorisk::quant {

struct MonteCarloResult {
    double expected_future_rate;
    double value_at_risk_99;      // 99% VaR metric
    double expected_shortfall_99; // Conditional VaR (CVaR)
    double liquidity_drawdown_max;
};

class MonteCarloSimulator {
public:
    static MonteCarloResult RunParallelSimulation(
        const JumpDiffusionParameters& params,
        double horizon_years,
        size_t steps,
        size_t total_simulations,
        unsigned int num_threads = std::thread::hardware_concurrency()
    ) {
        const size_t sims_per_thread = total_simulations / num_threads;
        std::vector<std::future<std::vector<double>>> futures;

        for (unsigned int i = 0; i < num_threads; ++i) {
            futures.push_back(std::async(std::launch::async, [params, horizon_years, steps, sims_per_thread, i]() {
                JumpDiffusionModel model(params, 1337 + i * 997);
                std::vector<double> terminal_rates;
                terminal_rates.reserve(sims_per_thread);

                for (size_t s = 0; s < sims_per_thread; ++s) {
                    auto path = model.SimulatePath(horizon_years, steps);
                    terminal_rates.push_back(path.back());
                }
                return terminal_rates;
            }));
        }

        std::vector<double> all_terminals;
        all_terminals.reserve(total_simulations);

        for (auto& f : futures) {
            auto chunk = f.get();
            all_terminals.insert(all_terminals.end(), chunk.begin(), chunk.end());
        }

        std::sort(all_terminals.begin(), all_terminals.end());

        // Calculate statistics
        double sum = std::accumulate(all_terminals.begin(), all_terminals.end(), 0.0);
        double mean = sum / static_cast<double>(all_terminals.size());

        size_t var_idx = static_cast<size_t>(all_terminals.size() * 0.01);
        double var_99 = params.spot_rate - all_terminals[var_idx];

        double cvar_sum = std::accumulate(all_terminals.begin(), all_terminals.begin() + var_idx, 0.0);
        double cvar_99 = params.spot_rate - (cvar_sum / static_cast<double>(var_idx));

        return MonteCarloResult{
            .expected_future_rate = mean,
            .value_at_risk_99 = var_99,
            .expected_shortfall_99 = cvar_99,
            .liquidity_drawdown_max = params.spot_rate - all_terminals.front()
        };
    }
};

} // namespace chronorisk::quant
