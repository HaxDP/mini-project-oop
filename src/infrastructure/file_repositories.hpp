#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../domain/contracts.hpp"

namespace cinema::infrastructure {

class FileMovieRepository final : public domain::IMovieRepository {
public:
    explicit FileMovieRepository(std::string filePath);

    void upsert(const domain::Movie& movie) override;
    std::optional<domain::Movie> findById(const std::string& id) const override;
    std::vector<domain::Movie> listAll() const override;

private:
    void load();
    void save() const;

    std::string filePath_;
    std::unordered_map<std::string, domain::Movie> items_;
};

class FileHallRepository final : public domain::IHallRepository {
public:
    explicit FileHallRepository(std::string filePath);

    void upsert(const domain::Hall& hall) override;
    std::optional<domain::Hall> findById(const std::string& id) const override;
    std::vector<domain::Hall> listAll() const override;

private:
    void load();
    void save() const;

    std::string filePath_;
    std::unordered_map<std::string, domain::Hall> items_;
};

class FileSessionRepository final : public domain::ISessionRepository {
public:
    explicit FileSessionRepository(std::string filePath);

    void upsert(const domain::Session& session) override;
    std::optional<domain::Session> findById(const std::string& id) const override;
    std::vector<domain::Session> listAll() const override;

private:
    void load();
    void save() const;

    std::string filePath_;
    std::unordered_map<std::string, domain::Session> items_;
};

class FileReservationRepository final : public domain::IReservationRepository {
public:
    explicit FileReservationRepository(std::string filePath);

    void upsert(const domain::Reservation& reservation) override;
    std::optional<domain::Reservation> findById(const std::string& id) const override;
    std::vector<domain::Reservation> listAll() const override;

private:
    void load();
    void save() const;

    std::string filePath_;
    std::unordered_map<std::string, domain::Reservation> items_;
};

class FileTicketRepository final : public domain::ITicketRepository {
public:
    explicit FileTicketRepository(std::string filePath);

    void add(const domain::Ticket& ticket) override;
    std::optional<domain::Ticket> findById(const std::string& id) const override;
    std::vector<domain::Ticket> listAll() const override;

private:
    void load();
    void save() const;

    std::string filePath_;
    std::unordered_map<std::string, domain::Ticket> items_;
};

struct RepositoryBundle {
    std::unique_ptr<domain::IMovieRepository> movieRepository;
    std::unique_ptr<domain::IHallRepository> hallRepository;
    std::unique_ptr<domain::ISessionRepository> sessionRepository;
    std::unique_ptr<domain::IReservationRepository> reservationRepository;
    std::unique_ptr<domain::ITicketRepository> ticketRepository;
};

class RepositoryFactory {
public:
    static RepositoryBundle createFileRepositories(const std::string& storageDirectory);
};

}  // namespace cinema::infrastructure