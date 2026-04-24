#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models.hpp"

namespace cinema::domain {

class IMovieRepository {
public:
    virtual ~IMovieRepository() = default;
    virtual void upsert(const Movie& movie) = 0;
    virtual std::optional<Movie> findById(const std::string& id) const = 0;
    virtual std::vector<Movie> listAll() const = 0;
};

class IHallRepository {
public:
    virtual ~IHallRepository() = default;
    virtual void upsert(const Hall& hall) = 0;
    virtual std::optional<Hall> findById(const std::string& id) const = 0;
    virtual std::vector<Hall> listAll() const = 0;
};

class ISessionRepository {
public:
    virtual ~ISessionRepository() = default;
    virtual void upsert(const Session& session) = 0;
    virtual std::optional<Session> findById(const std::string& id) const = 0;
    virtual std::vector<Session> listAll() const = 0;
};

class IReservationRepository {
public:
    virtual ~IReservationRepository() = default;
    virtual void upsert(const Reservation& reservation) = 0;
    virtual std::optional<Reservation> findById(const std::string& id) const = 0;
    virtual std::vector<Reservation> listAll() const = 0;
};

class ITicketRepository {
public:
    virtual ~ITicketRepository() = default;
    virtual void add(const Ticket& ticket) = 0;
    virtual std::optional<Ticket> findById(const std::string& id) const = 0;
    virtual std::vector<Ticket> listAll() const = 0;
};

}  // namespace cinema::domain