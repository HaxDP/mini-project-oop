#include <filesystem>
#include <iostream>

#include "../src/application/cinema_service.hpp"
#include "../src/application/query_service.hpp"
#include "../src/domain/pricing.hpp"
#include "../src/infrastructure/file_repositories.hpp"
#include "test_framework.hpp"

using cinema::application::CinemaService;
using cinema::application::QueryService;
using cinema::domain::DynamicPricingStrategy;
using cinema::infrastructure::RepositoryFactory;

namespace {

std::filesystem::path resetTestDirectory() {
    const auto path = std::filesystem::temp_directory_path() / "cinema-oop-integration";
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

}  // namespace

int main() {
    cinema::tests::TestContext context;
    const auto storage = resetTestDirectory();

    {
        auto repositories = RepositoryFactory::createFileRepositories(storage.string());
        DynamicPricingStrategy pricing;
        CinemaService service(*repositories.movieRepository,
                             *repositories.hallRepository,
                             *repositories.sessionRepository,
                             *repositories.reservationRepository,
                             *repositories.ticketRepository,
                             pricing);

        const auto movieId = service.createMovie("Початок", "Трилер", 148).value();
        const auto hallId = service.createHall("Прем'єра", 2, 2).value();
        const auto sessionId =
            service.scheduleSession(movieId, hallId, "2026-05-03T18:30", 210.0).value();

        const auto reservationResult = service.reserveSeat(sessionId, 2, 2, "Анна");
        EXPECT_TRUE(context, reservationResult.isSuccess(), "Бронювання має бути успішним");

        const auto invalidSeat = service.reserveSeat(sessionId, 9, 1, "Анна");
        EXPECT_TRUE(context,
                    !invalidSeat.isSuccess(),
                    "Бронювання неіснуючого місця має завершуватися помилкою");

        const auto purchaseResult =
            service.purchaseReservation(reservationResult.value().id(), reservationResult.value().finalPrice());
        EXPECT_TRUE(context, purchaseResult.isSuccess(), "Оплата має бути успішною");
    }

    {
        auto repositories = RepositoryFactory::createFileRepositories(storage.string());
        QueryService queries(*repositories.movieRepository,
                             *repositories.sessionRepository,
                             *repositories.ticketRepository);

        const auto topSessions = queries.topSessionsByRevenue(3);
        EXPECT_EQ(context,
                  static_cast<int>(topSessions.size()),
                  1,
                  "Після повторного завантаження має бути один сеанс в аналітиці");

        const auto demand = queries.moviesByOccupancy(0.10);
        EXPECT_EQ(context,
                  static_cast<int>(demand.size()),
                  1,
                  "Запит LINQ-подібної вибірки має повернути один фільм");
    }

    if (context.failedCount() == 0) {
        std::cout << "Всі integration-тести пройдено.\n";
        return 0;
    }

    std::cout << "Знайдено помилок: " << context.failedCount() << "\n";
    return 1;
}