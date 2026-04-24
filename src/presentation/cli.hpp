#pragma once

#include "../application/cinema_service.hpp"
#include "../application/query_service.hpp"

namespace cinema::presentation {

class ConsoleApp {
public:
    ConsoleApp(application::CinemaService& cinemaService,
               application::QueryService& queryService);

    void run();

private:
    void showMenu() const;
    void handleCreateMovie();
    void handleCreateHall();
    void handleCreateSession();
    void handleReserveSeat();
    void handlePurchaseReservation();
    void handleCancelReservation();
    void handleAnalytics() const;
    void handleReferenceData() const;

    application::CinemaService& cinemaService_;
    application::QueryService& queryService_;
};

}  // namespace cinema::presentation