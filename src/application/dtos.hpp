#pragma once

#include <cstddef>
#include <string>

namespace cinema::application {

struct SessionAnalytics {
    std::string sessionId;
    std::string movieTitle;
    std::string startAt;
    std::size_t soldTickets{};
    double occupancyRate{};
    double revenue{};
};

struct MovieDemandView {
    std::string movieId;
    std::string movieTitle;
    double averageOccupancy{};
    std::size_t sessionsCount{};
};

}  // namespace cinema::application