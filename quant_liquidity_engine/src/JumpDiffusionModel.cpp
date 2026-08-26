#pragma once
#include <random>
#include <cmath>
#include <vector>

namespace chronorisk::quant {

// Merton's Jump-Diffusion Model for FX rate volatility spikes:
// $$dS_t = (\mu - \lambda k) S_t dt + \sigma S_t dW_t + S_t J dN_t$$
struct JumpDiffusionParameters {
    double spot_rate;        // Initial currency conversion spot price ($S_0$)
    double drift;            // Expected annual drift ($\mu$)
    double volatility;       // Continuous diffusion volatility ($\sigma$)
    double jump_intensity;   // Poisson arrival rate of unexpected liquidity events ($\lambda$)
    double jump_mean;        // Expected log-jump magnitude ($\mu_J$)
    double jump_std_dev;     // Standard deviation of jump size ($\sigma_J$)
};

class JumpDiffusionModel {
private:
    JumpDiffusionParameters params_;
    std::mt19937_64 rng_;
    std::normal_distribution<double> standard_normal_;
    std::poisson_distribution<int> poisson_dist_;

public:
    explicit JumpDiffusionModel(const JumpDiffusionParameters& params, uint64_t seed = 42)
        : params_(params),
          rng_(seed),
          standard_normal_(0.0, 1.0),
          poisson_dist_(params.jump_intensity) {}

    // Simulates a single path of length steps over time horizon T (in years)
    std::vector<double> SimulatePath(double time_horizon_years, size_t steps) {
        std::vector<double> path(steps + 1, 0.0);
        path[0] = params_.spot_rate;

        const double dt = time_horizon_years / static_cast<double>(steps);
        const double sqrt_dt = std::sqrt(dt);
        const double k = std::exp(params_.jump_mean + 0.5 * params_.jump_std_dev * params_.jump_std_dev) - 1.0;
        const double drift_adj = (params_.drift - params_.jump_intensity * k - 0.5 * params_.volatility * params_.volatility) * dt;

        for (size_t t = 1; t <= steps; ++t) {
            double current_price = path[t - 1];
            double z = standard_normal_(rng_);
            double diffusion = params_.volatility * sqrt_dt * z;

            // Sample Poisson jumps
            int num_jumps = poisson_dist_(rng_);
            double jump_factor = 0.0;
            for (int j = 0; j < num_jumps; ++j) {
                double jump_size = standard_normal_(rng_) * params_.jump_std_dev + params_.jump_mean;
                jump_factor += jump_size;
            }

            path[t] = current_price * std::exp(drift_adj + diffusion + jump_factor);
        }

        return path;
    }
};

} // namespace chronorisk::quant
