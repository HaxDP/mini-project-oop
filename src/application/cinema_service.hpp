#pragma once

#include <string>
#include <vector>

#include "../domain/contracts.hpp"
#include "../domain/pricing.hpp"
#include "../domain/result.hpp"

namespace cinema::application {

class CinemaService {
public:
    CinemaService(domain::IMovieRepository& movieRepository,
                  domain::IHallRepository& hallRepository,
                  domain::ISessionRepository& sessionRepository,
                  domain::IReservationRepository& reservationRepository,
                  domain::ITicketRepository& ticketRepository,
                  const domain::IPricingStrategy& pricingStrategy);

    domain::Result<std::string> createMovie(const std::string& title,
                                            const std::string& genre,
                                            int durationMinutes);

    domain::Result<std::string> createHall(const std::string& name,
                                           int rows,
                                           int seatsPerRow);

    domain::Result<std::string> scheduleSession(const std::string& movieId,
                                                const std::string& hallId,
                                                const std::string& startAt,
                                                double basePrice);

    domain::Result<domain::Reservation> reserveSeat(const std::string& sessionId,
                                                    int row,
                                                    int number,
                                                    const std::string& customerName);

    domain::Result<domain::Ticket> purchaseReservation(const std::string& reservationId,
                                                       double paymentAmount);

    domain::OperationResult cancelReservation(const std::string& reservationId);

    std::vector<domain::Movie> listMovies() const;
    std::vector<domain::Hall> listHalls() const;
    std::vector<domain::Session> listSessions() const;
    std::vector<domain::Reservation> listReservations() const;
    std::vector<domain::Ticket> listTickets() const;

private:
    std::string nextId(const std::string& prefix);
    static bool isWeekendDate(const std::string& isoDateTime);

    domain::IMovieRepository& movieRepository_;
    domain::IHallRepository& hallRepository_;
    domain::ISessionRepository& sessionRepository_;
    domain::IReservationRepository& reservationRepository_;
    domain::ITicketRepository& ticketRepository_;
    const domain::IPricingStrategy& pricingStrategy_;

    unsigned long idCounter_{1};
};

}  // namespace cinema::application