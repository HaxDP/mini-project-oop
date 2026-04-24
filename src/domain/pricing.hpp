#pragma once

#include <algorithm>
#include <cstddef>

#include "models.hpp"

namespace cinema::domain {

class IPricingStrategy {
public:
    virtual ~IPricingStrategy() = default;
    virtual double calculatePrice(const Session& session,
                                  double basePrice,
                                  std::size_t customerPurchasedTickets,
                                  bool weekend) const = 0;
};

class DynamicPricingStrategy final : public IPricingStrategy {
public:
    double calculatePrice(const Session& session,
                          double basePrice,
                          std::size_t customerPurchasedTickets,
                          bool weekend) const override {
        double multiplier = 1.0;
        const double occupancy = session.occupancyRate();

        if (occupancy >= 0.70) {
            multiplier *= 1.25;
        } else if (occupancy >= 0.40) {
            multiplier *= 1.10;
        }

        if (weekend) {
            multiplier *= 1.15;
        }

        if (customerPurchasedTickets >= 3U) {
            multiplier *= 0.90;
        }

        return std::max(1.0, basePrice * multiplier);
    }
};

}  // namespace cinema::domain