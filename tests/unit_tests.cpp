#include <filesystem>
#include <iostream>

#include "../src/application/cinema_service.hpp"
#include "../src/domain/pricing.hpp"
#include "../src/infrastructure/file_repositories.hpp"
#include "test_framework.hpp"

using cinema::application::CinemaService;
using cinema::domain::DynamicPricingStrategy;
using cinema::infrastructure::RepositoryFactory;

namespace {

std::filesystem::path resetTestDirectory(const std::string& name) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

CinemaService createService(const std::filesystem::path& path,
                           cinema::infrastructure::RepositoryBundle& repositories,
                           DynamicPricingStrategy& pricing) {
    repositories = RepositoryFactory::createFileRepositories(path.string());
    return CinemaService(*repositories.movieRepository,
                         *repositories.hallRepository,
                         *repositories.sessionRepository,
                         *repositories.reservationRepository,
                         *repositories.ticketRepository,
                         pricing);
}

}  // namespace

int main() {
    cinema::tests::TestContext context;

    {
        auto storage = resetTestDirectory("cinema-oop-unit-1");
        cinema::infrastructure::RepositoryBundle repositories;
        DynamicPricingStrategy pricing;
        auto service = createService(storage, repositories, pricing);

        const auto invalidMovie = service.createMovie("", "Драма", 120);
        EXPECT_TRUE(context,
                    !invalidMovie.isSuccess(),
                    "Порожня назва фільму має повертати помилку");
    }

    {
        auto storage = resetTestDirectory("cinema-oop-unit-2");
        cinema::infrastructure::RepositoryBundle repositories;
        DynamicPricingStrategy pricing;
        auto service = createService(storage, repositories, pricing);

        const auto movieId = service.createMovie("Інтерстеллар", "Sci-Fi", 169).value();
        const auto hallId = service.createHall("Червоний", 3, 4).value();
        const auto sessionId =
            service.scheduleSession(movieId, hallId, "2026-05-01T19:00", 220.0).value();

        const auto first = service.reserveSeat(sessionId, 1, 1, "Ірина");
        const auto second = service.reserveSeat(sessionId, 1, 1, "Олег");

        EXPECT_TRUE(context, first.isSuccess(), "Перше бронювання має бути успішним");
        EXPECT_TRUE(context,
                    !second.isSuccess(),
                    "Повторне бронювання того самого місця має завершитися помилкою");
    }

    {
        auto storage = resetTestDirectory("cinema-oop-unit-3");
        cinema::infrastructure::RepositoryBundle repositories;
        DynamicPricingStrategy pricing;
        auto service = createService(storage, repositories, pricing);

        const auto movieId = service.createMovie("Дюна", "Sci-Fi", 155).value();
        const auto hallId = service.createHall("Синій", 2, 3).value();
        const auto sessionId =
            service.scheduleSession(movieId, hallId, "2026-05-02T20:00", 180.0).value();
        const auto reservation = service.reserveSeat(sessionId, 1, 2, "Марта").value();

        const auto payment = service.purchaseReservation(reservation.id(), reservation.finalPrice() - 10.0);
        EXPECT_TRUE(context,
                    !payment.isSuccess(),
                    "Недостатня сума оплати має повертати помилку");
    }

    if (context.failedCount() == 0) {
        std::cout << "Всі unit-тести пройдено.\n";
        return 0;
    }

    std::cout << "Знайдено помилок: " << context.failedCount() << "\n";
    return 1;
}