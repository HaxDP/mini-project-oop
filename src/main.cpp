#include <iostream>

#include "application/cinema_service.hpp"
#include "application/query_service.hpp"
#include "domain/pricing.hpp"
#include "infrastructure/file_repositories.hpp"
#include "presentation/cli.hpp"

int main() {
    try {
        auto repositories = cinema::infrastructure::RepositoryFactory::createFileRepositories("storage");
        cinema::domain::DynamicPricingStrategy pricingStrategy;

        cinema::application::CinemaService cinemaService(*repositories.movieRepository,
                                                         *repositories.hallRepository,
                                                         *repositories.sessionRepository,
                                                         *repositories.reservationRepository,
                                                         *repositories.ticketRepository,
                                                         pricingStrategy);

        cinema::application::QueryService queryService(*repositories.movieRepository,
                                                       *repositories.sessionRepository,
                                                       *repositories.ticketRepository);

        cinema::presentation::ConsoleApp app(cinemaService, queryService);
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Критична помилка: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}