#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_set>

namespace cinema::domain {

struct Seat {
    int row{};
    int number{};

    std::string toKey() const {
        return std::to_string(row) + ":" + std::to_string(number);
    }

    static Seat fromKey(const std::string& key) {
        const auto delimiterPos = key.find(':');
        if (delimiterPos == std::string::npos) {
            return Seat{};
        }

        Seat seat;
        seat.row = std::stoi(key.substr(0, delimiterPos));
        seat.number = std::stoi(key.substr(delimiterPos + 1));
        return seat;
    }
};

class Movie {
public:
    Movie() = default;

    Movie(std::string id, std::string title, std::string genre, int durationMinutes)
        : id_(std::move(id)),
          title_(std::move(title)),
          genre_(std::move(genre)),
          durationMinutes_(durationMinutes) {}

    const std::string& id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& genre() const { return genre_; }
    int durationMinutes() const { return durationMinutes_; }

private:
    std::string id_;
    std::string title_;
    std::string genre_;
    int durationMinutes_{};
};

class Hall {
public:
    Hall() = default;

    Hall(std::string id, std::string name, int rows, int seatsPerRow)
        : id_(std::move(id)),
          name_(std::move(name)),
          rows_(rows),
          seatsPerRow_(seatsPerRow) {}

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    int rows() const { return rows_; }
    int seatsPerRow() const { return seatsPerRow_; }

    std::size_t totalSeats() const {
        return static_cast<std::size_t>(rows_ * seatsPerRow_);
    }

    bool contains(const Seat& seat) const {
        return seat.row >= 1 && seat.row <= rows_ && seat.number >= 1 &&
               seat.number <= seatsPerRow_;
    }

private:
    std::string id_;
    std::string name_;
    int rows_{};
    int seatsPerRow_{};
};

class Session {
public:
    Session() = default;

    Session(std::string id,
            std::string movieId,
            std::string hallId,
            std::string startAt,
            std::size_t totalSeats,
            double basePrice)
        : id_(std::move(id)),
          movieId_(std::move(movieId)),
          hallId_(std::move(hallId)),
          startAt_(std::move(startAt)),
          totalSeats_(totalSeats),
          basePrice_(basePrice) {}

    const std::string& id() const { return id_; }
    const std::string& movieId() const { return movieId_; }
    const std::string& hallId() const { return hallId_; }
    const std::string& startAt() const { return startAt_; }
    std::size_t totalSeats() const { return totalSeats_; }
    double basePrice() const { return basePrice_; }

    bool canReserveSeat(const Seat& seat) const {
        const auto key = seat.toKey();
        return reservedSeats_.count(key) == 0 && soldSeats_.count(key) == 0;
    }

    void reserveSeat(const Seat& seat) {
        reservedSeats_.insert(seat.toKey());
    }

    void releaseSeat(const Seat& seat) {
        reservedSeats_.erase(seat.toKey());
    }

    void markSeatAsSold(const Seat& seat) {
        const auto key = seat.toKey();
        reservedSeats_.erase(key);
        soldSeats_.insert(key);
    }

    double occupancyRate() const {
        if (totalSeats_ == 0U) {
            return 0.0;
        }
        return static_cast<double>(soldSeats_.size() + reservedSeats_.size()) /
               static_cast<double>(totalSeats_);
    }

    const std::unordered_set<std::string>& reservedSeats() const {
        return reservedSeats_;
    }

    const std::unordered_set<std::string>& soldSeats() const { return soldSeats_; }

    void restoreSeats(std::unordered_set<std::string> reserved,
                      std::unordered_set<std::string> sold) {
        reservedSeats_ = std::move(reserved);
        soldSeats_ = std::move(sold);
    }

private:
    std::string id_;
    std::string movieId_;
    std::string hallId_;
    std::string startAt_;
    std::size_t totalSeats_{};
    double basePrice_{};
    std::unordered_set<std::string> reservedSeats_;
    std::unordered_set<std::string> soldSeats_;
};

enum class ReservationStatus { Active, Purchased, Cancelled };

class Reservation {
public:
    Reservation() = default;

    Reservation(std::string id,
                std::string sessionId,
                Seat seat,
                std::string customerName,
                double finalPrice,
                ReservationStatus status,
                std::string createdAt)
        : id_(std::move(id)),
          sessionId_(std::move(sessionId)),
          seat_(seat),
          customerName_(std::move(customerName)),
          finalPrice_(finalPrice),
          status_(status),
          createdAt_(std::move(createdAt)) {}

    const std::string& id() const { return id_; }
    const std::string& sessionId() const { return sessionId_; }
    const Seat& seat() const { return seat_; }
    const std::string& customerName() const { return customerName_; }
    double finalPrice() const { return finalPrice_; }
    ReservationStatus status() const { return status_; }
    const std::string& createdAt() const { return createdAt_; }

    void setStatus(ReservationStatus status) { status_ = status; }

private:
    std::string id_;
    std::string sessionId_;
    Seat seat_;
    std::string customerName_;
    double finalPrice_{};
    ReservationStatus status_{ReservationStatus::Active};
    std::string createdAt_;
};

class Ticket {
public:
    Ticket() = default;

    Ticket(std::string id,
           std::string reservationId,
           std::string sessionId,
           Seat seat,
           std::string customerName,
           double paidAmount,
           std::string issuedAt)
        : id_(std::move(id)),
          reservationId_(std::move(reservationId)),
          sessionId_(std::move(sessionId)),
          seat_(seat),
          customerName_(std::move(customerName)),
          paidAmount_(paidAmount),
          issuedAt_(std::move(issuedAt)) {}

    const std::string& id() const { return id_; }
    const std::string& reservationId() const { return reservationId_; }
    const std::string& sessionId() const { return sessionId_; }
    const Seat& seat() const { return seat_; }
    const std::string& customerName() const { return customerName_; }
    double paidAmount() const { return paidAmount_; }
    const std::string& issuedAt() const { return issuedAt_; }

private:
    std::string id_;
    std::string reservationId_;
    std::string sessionId_;
    Seat seat_;
    std::string customerName_;
    double paidAmount_{};
    std::string issuedAt_;
};

}  // namespace cinema::domain