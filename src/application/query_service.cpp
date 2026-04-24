#include "query_service.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <unordered_map>

namespace cinema::application {

QueryService::QueryService(const domain::IMovieRepository& movieRepository,
                           const domain::ISessionRepository& sessionRepository,
                           const domain::ITicketRepository& ticketRepository)
    : movieRepository_(movieRepository),
      sessionRepository_(sessionRepository),
      ticketRepository_(ticketRepository) {}

std::vector<SessionAnalytics> QueryService::topSessionsByRevenue(std::size_t limit) const {
    const auto movies = movieRepository_.listAll();
    const auto sessions = sessionRepository_.listAll();
    const auto tickets = ticketRepository_.listAll();

    std::unordered_map<std::string, std::string> movieTitleById;
    for (const auto& movie : movies) {
        movieTitleById[movie.id()] = movie.title();
    }

    std::unordered_map<std::string, std::size_t> soldBySession;
    std::unordered_map<std::string, double> revenueBySession;

    for (const auto& ticket : tickets) {
        soldBySession[ticket.sessionId()] += 1U;
        revenueBySession[ticket.sessionId()] += ticket.paidAmount();
    }

    std::vector<SessionAnalytics> analytics;
    analytics.reserve(sessions.size());

    for (const auto& session : sessions) {
        analytics.push_back(SessionAnalytics{
            session.id(),
            movieTitleById[session.movieId()],
            session.startAt(),
            soldBySession[session.id()],
            session.occupancyRate(),
            revenueBySession[session.id()]});
    }

    std::ranges::sort(analytics, [](const SessionAnalytics& left, const SessionAnalytics& right) {
        return left.revenue > right.revenue;
    });

    if (limit < analytics.size()) {
        analytics.resize(limit);
    }

    return analytics;
}

std::vector<MovieDemandView> QueryService::moviesByOccupancy(double minOccupancy) const {
    const auto movies = movieRepository_.listAll();
    const auto sessions = sessionRepository_.listAll();

    std::unordered_map<std::string, std::vector<double>> occupancyByMovie;
    for (const auto& session : sessions) {
        occupancyByMovie[session.movieId()].push_back(session.occupancyRate());
    }

    auto filtered = movies | std::views::filter([&](const domain::Movie& movie) {
                        const auto iterator = occupancyByMovie.find(movie.id());
                        if (iterator == occupancyByMovie.end() || iterator->second.empty()) {
                            return false;
                        }

                        const double total = std::accumulate(iterator->second.begin(),
                                                             iterator->second.end(),
                                                             0.0);
                        const double average = total / static_cast<double>(iterator->second.size());
                        return average >= minOccupancy;
                    }) |
                    std::views::transform([&](const domain::Movie& movie) {
                        const auto& values = occupancyByMovie[movie.id()];
                        const double total = std::accumulate(values.begin(), values.end(), 0.0);
                        return MovieDemandView{movie.id(),
                                               movie.title(),
                                               total / static_cast<double>(values.size()),
                                               values.size()};
                    });

    std::vector<MovieDemandView> result;
    for (const auto& entry : filtered) {
        result.push_back(entry);
    }

    std::ranges::sort(result, [](const MovieDemandView& left, const MovieDemandView& right) {
        return left.averageOccupancy > right.averageOccupancy;
    });

    return result;
}

}  // namespace cinema::application