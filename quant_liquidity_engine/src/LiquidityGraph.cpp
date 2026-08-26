#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <mutex>

namespace chronorisk::liquidity {

struct CorridorNode {
    std::string currency_code;
    int64_t available_clearing_reserves; // Stored in cents/base units
    int64_t pending_inflow;
    int64_t pending_outflow;
};

class LiquidityGraph {
private:
    std::unordered_map<std::string, CorridorNode> corridor_nodes_;
    mutable std::shared_mutex graph_mutex_;

public:
    void RegisterCurrencyCorridor(const std::string& currency, int64_t initial_reserves) {
        std::unique_lock lock(graph_mutex_);
        corridor_nodes_[currency] = CorridorNode{
            .currency_code = currency,
            .available_clearing_reserves = initial_reserves,
            .pending_inflow = 0,
            .pending_outflow = 0
        };
    }

    bool CheckAndLockLiquidity(const std::string& currency, int64_t amount_cents) {
        std::unique_lock lock(graph_mutex_);
        auto it = corridor_nodes_.find(currency);
        if (it == corridor_nodes_.end()) {
            return false;
        }

        if (it->second.available_clearing_reserves >= amount_cents) {
            it->second.available_clearing_reserves -= amount_cents;
            it->second.pending_outflow += amount_cents;
            return true;
        }
        return false;
    }

    void CommitSettlement(const std::string& from_currency, const std::string& to_currency, int64_t from_amount, int64_t to_amount) {
        std::unique_lock lock(graph_mutex_);
        auto it_from = corridor_nodes_.find(from_currency);
        auto it_to = corridor_nodes_.find(to_currency);

        if (it_from != corridor_nodes_.end()) {
            it_from->second.pending_outflow -= from_amount;
        }
        if (it_to != corridor_nodes_.end()) {
            it_to->second.available_clearing_reserves += to_amount;
        }
    }

    int64_t GetAvailableReserves(const std::string& currency) const {
        std::shared_lock lock(graph_mutex_);
        auto it = corridor_nodes_.find(currency);
        return (it != corridor_nodes_.end()) ? it->second.available_clearing_reserves : 0;
    }
};

} // namespace chronorisk::liquidity
