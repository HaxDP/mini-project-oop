#include "file_repositories.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace cinema::infrastructure {
namespace {

std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;

    while (std::getline(stream, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

std::string joinSeats(const std::unordered_set<std::string>& seats) {
    std::string result;
    bool first = true;

    for (const auto& seat : seats) {
        if (!first) {
            result += ',';
        }
        result += seat;
        first = false;
    }

    return result;
}

std::unordered_set<std::string> parseSeats(const std::string& value) {
    std::unordered_set<std::string> seats;
    for (const auto& item : split(value, ',')) {
        if (!item.empty()) {
            seats.insert(item);
        }
    }
    return seats;
}

void ensureParentDirectory(const std::string& filePath) {
    const std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
}

std::string statusToString(domain::ReservationStatus status) {
    switch (status) {
        case domain::ReservationStatus::Active:
            return "active";
        case domain::ReservationStatus::Purchased:
            return "purchased";
        case domain::ReservationStatus::Cancelled:
            return "cancelled";
    }
    return "active";
}

domain::ReservationStatus parseStatus(const std::string& value) {
    if (value == "purchased") {
        return domain::ReservationStatus::Purchased;
    }
    if (value == "cancelled") {
        return domain::ReservationStatus::Cancelled;
    }
    return domain::ReservationStatus::Active;
}

}  // namespace

FileMovieRepository::FileMovieRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

void FileMovieRepository::upsert(const domain::Movie& movie) {
    items_[movie.id()] = movie;
    save();
}

std::optional<domain::Movie> FileMovieRepository::findById(const std::string& id) const {
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::Movie> FileMovieRepository::listAll() const {
    std::vector<domain::Movie> result;
    result.reserve(items_.size());
    for (const auto& [_, item] : items_) {
        result.push_back(item);
    }
    return result;
}

void FileMovieRepository::load() {
    items_.clear();
    std::ifstream input(filePath_);
    if (!input.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto parts = split(line, '\t');
        if (parts.size() < 4U) {
            continue;
        }
        items_[parts[0]] = domain::Movie(parts[0], parts[1], parts[2], std::stoi(parts[3]));
    }
}

void FileMovieRepository::save() const {
    ensureParentDirectory(filePath_);
    std::ofstream output(filePath_, std::ios::trunc);
    for (const auto& [_, item] : items_) {
        output << item.id() << '\t' << item.title() << '\t' << item.genre() << '\t'
               << item.durationMinutes() << '\n';
    }
}

FileHallRepository::FileHallRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

void FileHallRepository::upsert(const domain::Hall& hall) {
    items_[hall.id()] = hall;
    save();
}

std::optional<domain::Hall> FileHallRepository::findById(const std::string& id) const {
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::Hall> FileHallRepository::listAll() const {
    std::vector<domain::Hall> result;
    result.reserve(items_.size());
    for (const auto& [_, item] : items_) {
        result.push_back(item);
    }
    return result;
}

void FileHallRepository::load() {
    items_.clear();
    std::ifstream input(filePath_);
    if (!input.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto parts = split(line, '\t');
        if (parts.size() < 4U) {
            continue;
        }
        items_[parts[0]] = domain::Hall(parts[0], parts[1], std::stoi(parts[2]), std::stoi(parts[3]));
    }
}

void FileHallRepository::save() const {
    ensureParentDirectory(filePath_);
    std::ofstream output(filePath_, std::ios::trunc);
    for (const auto& [_, item] : items_) {
        output << item.id() << '\t' << item.name() << '\t' << item.rows() << '\t'
               << item.seatsPerRow() << '\n';
    }
}

FileSessionRepository::FileSessionRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

void FileSessionRepository::upsert(const domain::Session& session) {
    items_[session.id()] = session;
    save();
}

std::optional<domain::Session> FileSessionRepository::findById(const std::string& id) const {
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::Session> FileSessionRepository::listAll() const {
    std::vector<domain::Session> result;
    result.reserve(items_.size());
    for (const auto& [_, item] : items_) {
        result.push_back(item);
    }
    return result;
}

void FileSessionRepository::load() {
    items_.clear();
    std::ifstream input(filePath_);
    if (!input.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto parts = split(line, '\t');
        if (parts.size() < 8U) {
            continue;
        }

        domain::Session session(parts[0],
                               parts[1],
                               parts[2],
                               parts[3],
                               static_cast<std::size_t>(std::stoull(parts[4])),
                               std::stod(parts[5]));
        session.restoreSeats(parseSeats(parts[6]), parseSeats(parts[7]));
        items_[parts[0]] = session;
    }
}

void FileSessionRepository::save() const {
    ensureParentDirectory(filePath_);
    std::ofstream output(filePath_, std::ios::trunc);
    for (const auto& [_, item] : items_) {
        output << item.id() << '\t' << item.movieId() << '\t' << item.hallId() << '\t'
               << item.startAt() << '\t' << item.totalSeats() << '\t' << item.basePrice() << '\t'
               << joinSeats(item.reservedSeats()) << '\t' << joinSeats(item.soldSeats()) << '\n';
    }
}

FileReservationRepository::FileReservationRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

void FileReservationRepository::upsert(const domain::Reservation& reservation) {
    items_[reservation.id()] = reservation;
    save();
}

std::optional<domain::Reservation> FileReservationRepository::findById(const std::string& id) const {
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::Reservation> FileReservationRepository::listAll() const {
    std::vector<domain::Reservation> result;
    result.reserve(items_.size());
    for (const auto& [_, item] : items_) {
        result.push_back(item);
    }
    return result;
}

void FileReservationRepository::load() {
    items_.clear();
    std::ifstream input(filePath_);
    if (!input.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto parts = split(line, '\t');
        if (parts.size() < 8U) {
            continue;
        }

        const domain::Seat seat{std::stoi(parts[2]), std::stoi(parts[3])};
        items_[parts[0]] = domain::Reservation(parts[0],
                                               parts[1],
                                               seat,
                                               parts[4],
                                               std::stod(parts[5]),
                                               parseStatus(parts[6]),
                                               parts[7]);
    }
}

void FileReservationRepository::save() const {
    ensureParentDirectory(filePath_);
    std::ofstream output(filePath_, std::ios::trunc);

    for (const auto& [_, item] : items_) {
        output << item.id() << '\t' << item.sessionId() << '\t' << item.seat().row << '\t'
               << item.seat().number << '\t' << item.customerName() << '\t' << item.finalPrice()
               << '\t' << statusToString(item.status()) << '\t' << item.createdAt() << '\n';
    }
}

FileTicketRepository::FileTicketRepository(std::string filePath)
    : filePath_(std::move(filePath)) {
    load();
}

void FileTicketRepository::add(const domain::Ticket& ticket) {
    items_[ticket.id()] = ticket;
    save();
}

std::optional<domain::Ticket> FileTicketRepository::findById(const std::string& id) const {
    const auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::Ticket> FileTicketRepository::listAll() const {
    std::vector<domain::Ticket> result;
    result.reserve(items_.size());
    for (const auto& [_, item] : items_) {
        result.push_back(item);
    }
    return result;
}

void FileTicketRepository::load() {
    items_.clear();
    std::ifstream input(filePath_);
    if (!input.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto parts = split(line, '\t');
        if (parts.size() < 8U) {
            continue;
        }

        const domain::Seat seat{std::stoi(parts[3]), std::stoi(parts[4])};
        items_[parts[0]] = domain::Ticket(parts[0],
                                          parts[1],
                                          parts[2],
                                          seat,
                                          parts[5],
                                          std::stod(parts[6]),
                                          parts[7]);
    }
}

void FileTicketRepository::save() const {
    ensureParentDirectory(filePath_);
    std::ofstream output(filePath_, std::ios::trunc);

    for (const auto& [_, item] : items_) {
        output << item.id() << '\t' << item.reservationId() << '\t' << item.sessionId() << '\t'
               << item.seat().row << '\t' << item.seat().number << '\t' << item.customerName()
               << '\t' << item.paidAmount() << '\t' << item.issuedAt() << '\n';
    }
}

RepositoryBundle RepositoryFactory::createFileRepositories(const std::string& storageDirectory) {
    const auto base = std::filesystem::path(storageDirectory);

    RepositoryBundle bundle;
    bundle.movieRepository = std::make_unique<FileMovieRepository>((base / "movies.db").string());
    bundle.hallRepository = std::make_unique<FileHallRepository>((base / "halls.db").string());
    bundle.sessionRepository = std::make_unique<FileSessionRepository>((base / "sessions.db").string());
    bundle.reservationRepository =
        std::make_unique<FileReservationRepository>((base / "reservations.db").string());
    bundle.ticketRepository = std::make_unique<FileTicketRepository>((base / "tickets.db").string());

    return bundle;
}

}  // namespace cinema::infrastructure