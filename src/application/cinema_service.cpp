#include "cinema_service.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace cinema::application {

CinemaService::CinemaService(domain::IMovieRepository& movieRepository,
                             domain::IHallRepository& hallRepository,
                             domain::ISessionRepository& sessionRepository,
                             domain::IReservationRepository& reservationRepository,
                             domain::ITicketRepository& ticketRepository,
                             const domain::IPricingStrategy& pricingStrategy)
    : movieRepository_(movieRepository),
      hallRepository_(hallRepository),
      sessionRepository_(sessionRepository),
      reservationRepository_(reservationRepository),
      ticketRepository_(ticketRepository),
      pricingStrategy_(pricingStrategy) {}

domain::Result<std::string> CinemaService::createMovie(const std::string& title,
                                                       const std::string& genre,
                                                       int durationMinutes) {
    if (title.empty() || genre.empty()) {
        return domain::Result<std::string>::Failure("Назва та жанр фільму є обов'язковими");
    }

    if (durationMinutes <= 0) {
        return domain::Result<std::string>::Failure("Тривалість фільму має бути більшою за 0");
    }

    const auto id = nextId("MOV");
    movieRepository_.upsert(domain::Movie(id, title, genre, durationMinutes));
    return domain::Result<std::string>::Success(id);
}

domain::Result<std::string> CinemaService::createHall(const std::string& name,
                                                      int rows,
                                                      int seatsPerRow) {
    if (name.empty()) {
        return domain::Result<std::string>::Failure("Назва залу є обов'язковою");
    }

    if (rows <= 0 || seatsPerRow <= 0) {
        return domain::Result<std::string>::Failure("Кількість рядів і місць має бути більшою за 0");
    }

    const auto id = nextId("HAL");
    hallRepository_.upsert(domain::Hall(id, name, rows, seatsPerRow));
    return domain::Result<std::string>::Success(id);
}

domain::Result<std::string> CinemaService::scheduleSession(const std::string& movieId,
                                                           const std::string& hallId,
                                                           const std::string& startAt,
                                                           double basePrice) {
    if (!movieRepository_.findById(movieId).has_value()) {
        return domain::Result<std::string>::Failure("Фільм не знайдено");
    }

    const auto hall = hallRepository_.findById(hallId);
    if (!hall.has_value()) {
        return domain::Result<std::string>::Failure("Зал не знайдено");
    }

    if (startAt.empty()) {
        return domain::Result<std::string>::Failure("Час початку сеансу є обов'язковим");
    }

    if (basePrice <= 0.0) {
        return domain::Result<std::string>::Failure("Базова ціна має бути більшою за 0");
    }

    const auto id = nextId("SES");
    sessionRepository_.upsert(domain::Session(id, movieId, hallId, startAt, hall->totalSeats(), basePrice));
    return domain::Result<std::string>::Success(id);
}

domain::Result<domain::Reservation> CinemaService::reserveSeat(const std::string& sessionId,
                                                                int row,
                                                                int number,
                                                                const std::string& customerName) {
    if (customerName.empty()) {
        return domain::Result<domain::Reservation>::Failure("Ім'я клієнта не може бути порожнім");
    }

    auto session = sessionRepository_.findById(sessionId);
    if (!session.has_value()) {
        return domain::Result<domain::Reservation>::Failure("Сеанс не знайдено");
    }

    const auto hall = hallRepository_.findById(session->hallId());
    if (!hall.has_value()) {
        return domain::Result<domain::Reservation>::Failure("Зал для сеансу не знайдено");
    }

    const domain::Seat seat{row, number};
    if (!hall->contains(seat)) {
        return domain::Result<domain::Reservation>::Failure("Місце не існує у вказаному залі");
    }

    if (!session->canReserveSeat(seat)) {
        return domain::Result<domain::Reservation>::Failure("Місце вже зайняте або заброньоване");
    }

    const auto tickets = ticketRepository_.listAll();
    const auto purchasedByCustomer = static_cast<std::size_t>(std::count_if(
        tickets.begin(), tickets.end(), [&](const domain::Ticket& ticket) {
            return ticket.customerName() == customerName;
        }));

    const double finalPrice = pricingStrategy_.calculatePrice(
        *session, session->basePrice(), purchasedByCustomer, isWeekendDate(session->startAt()));

    session->reserveSeat(seat);
    sessionRepository_.upsert(*session);

    const auto reservation = domain::Reservation(nextId("RES"),
                                                 sessionId,
                                                 seat,
                                                 customerName,
                                                 finalPrice,
                                                 domain::ReservationStatus::Active,
                                                 session->startAt());
    reservationRepository_.upsert(reservation);
    return domain::Result<domain::Reservation>::Success(reservation);
}

domain::Result<domain::Ticket> CinemaService::purchaseReservation(const std::string& reservationId,
                                                                   double paymentAmount) {
    auto reservation = reservationRepository_.findById(reservationId);
    if (!reservation.has_value()) {
        return domain::Result<domain::Ticket>::Failure("Бронювання не знайдено");
    }

    if (reservation->status() != domain::ReservationStatus::Active) {
        return domain::Result<domain::Ticket>::Failure("Бронювання вже оброблено");
    }

    if (paymentAmount + 1e-9 < reservation->finalPrice()) {
        return domain::Result<domain::Ticket>::Failure("Недостатня сума оплати");
    }

    auto session = sessionRepository_.findById(reservation->sessionId());
    if (!session.has_value()) {
        return domain::Result<domain::Ticket>::Failure("Сеанс не знайдено");
    }

    session->markSeatAsSold(reservation->seat());
    sessionRepository_.upsert(*session);

    reservation->setStatus(domain::ReservationStatus::Purchased);
    reservationRepository_.upsert(*reservation);

    const auto ticket = domain::Ticket(nextId("TIC"),
                                       reservation->id(),
                                       reservation->sessionId(),
                                       reservation->seat(),
                                       reservation->customerName(),
                                       paymentAmount,
                                       session->startAt());
    ticketRepository_.add(ticket);

    return domain::Result<domain::Ticket>::Success(ticket);
}

domain::OperationResult CinemaService::cancelReservation(const std::string& reservationId) {
    auto reservation = reservationRepository_.findById(reservationId);
    if (!reservation.has_value()) {
        return domain::OperationResult::Failure("Бронювання не знайдено");
    }

    if (reservation->status() != domain::ReservationStatus::Active) {
        return domain::OperationResult::Failure("Скасувати можна лише активне бронювання");
    }

    auto session = sessionRepository_.findById(reservation->sessionId());
    if (!session.has_value()) {
        return domain::OperationResult::Failure("Сеанс не знайдено");
    }

    session->releaseSeat(reservation->seat());
    sessionRepository_.upsert(*session);

    reservation->setStatus(domain::ReservationStatus::Cancelled);
    reservationRepository_.upsert(*reservation);

    return domain::OperationResult::Success();
}

std::vector<domain::Movie> CinemaService::listMovies() const {
    return movieRepository_.listAll();
}

std::vector<domain::Hall> CinemaService::listHalls() const {
    return hallRepository_.listAll();
}

std::vector<domain::Session> CinemaService::listSessions() const {
    return sessionRepository_.listAll();
}

std::vector<domain::Reservation> CinemaService::listReservations() const {
    return reservationRepository_.listAll();
}

std::vector<domain::Ticket> CinemaService::listTickets() const {
    return ticketRepository_.listAll();
}

std::string CinemaService::nextId(const std::string& prefix) {
    std::ostringstream stream;
    stream << prefix << std::setfill('0') << std::setw(5) << idCounter_++;
    return stream.str();
}

bool CinemaService::isWeekendDate(const std::string& isoDateTime) {
    if (isoDateTime.size() < 10U) {
        return false;
    }

    int year = std::stoi(isoDateTime.substr(0, 4));
    int month = std::stoi(isoDateTime.substr(5, 2));
    const int day = std::stoi(isoDateTime.substr(8, 2));

    if (month < 3) {
        month += 12;
        --year;
    }

    const int k = year % 100;
    const int j = year / 100;

    const int h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) + (5 * j)) % 7;
    return h == 0 || h == 1;
}

}  // namespace cinema::application