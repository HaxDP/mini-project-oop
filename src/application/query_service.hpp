#pragma once

#include <cstddef>
#include <vector>

#include "../domain/contracts.hpp"
#include "dtos.hpp"

namespace cinema::application {

class QueryService {
public:
    QueryService(const domain::IMovieRepository& movieRepository,
                 const domain::ISessionRepository& sessionRepository,
                 const domain::ITicketRepository& ticketRepository);

    std::vector<SessionAnalytics> topSessionsByRevenue(std::size_t limit) const;
    std::vector<MovieDemandView> moviesByOccupancy(double minOccupancy) const;

private:
    const domain::IMovieRepository& movieRepository_;
    const domain::ISessionRepository& sessionRepository_;
    const domain::ITicketRepository& ticketRepository_;
};

}  // namespace cinema::application