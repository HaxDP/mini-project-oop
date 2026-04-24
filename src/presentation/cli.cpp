#include "cli.hpp"

#include <iostream>
#include <limits>
#include <string>

namespace cinema::presentation {
namespace {

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

}  // namespace

ConsoleApp::ConsoleApp(application::CinemaService& cinemaService,
                       application::QueryService& queryService)
    : cinemaService_(cinemaService), queryService_(queryService) {}

void ConsoleApp::run() {
    bool running = true;

    while (running) {
        showMenu();
        int option = -1;
        std::cin >> option;

        if (std::cin.fail()) {
            std::cin.clear();
            clearInputBuffer();
            std::cout << "Помилка: введіть номер пункту меню.\n\n";
            continue;
        }

        clearInputBuffer();

        switch (option) {
            case 1:
                handleCreateMovie();
                break;
            case 2:
                handleCreateHall();
                break;
            case 3:
                handleCreateSession();
                break;
            case 4:
                handleReserveSeat();
                break;
            case 5:
                handlePurchaseReservation();
                break;
            case 6:
                handleCancelReservation();
                break;
            case 7:
                handleAnalytics();
                break;
            case 8:
                handleReferenceData();
                break;
            case 0:
                running = false;
                std::cout << "Завершення роботи. Дані збережено.\n";
                break;
            default:
                std::cout << "Невідомий пункт меню.\n";
                break;
        }

        std::cout << "\n";
    }
}

void ConsoleApp::showMenu() const {
    std::cout << "===== Кінотеатр OOP =====\n";
    std::cout << "1. Додати фільм\n";
    std::cout << "2. Додати зал\n";
    std::cout << "3. Створити сеанс\n";
    std::cout << "4. Забронювати місце\n";
    std::cout << "5. Викупити бронювання\n";
    std::cout << "6. Скасувати бронювання\n";
    std::cout << "7. Показати аналітику\n";
    std::cout << "8. Показати довідники\n";
    std::cout << "0. Вихід\n";
    std::cout << "Оберіть дію: ";
}

void ConsoleApp::handleCreateMovie() {
    std::string title;
    std::string genre;
    int duration = 0;

    std::cout << "Назва фільму: ";
    std::getline(std::cin, title);
    std::cout << "Жанр: ";
    std::getline(std::cin, genre);
    std::cout << "Тривалість (хв): ";
    std::cin >> duration;
    clearInputBuffer();

    const auto result = cinemaService_.createMovie(title, genre, duration);
    if (result.isSuccess()) {
        std::cout << "Успіх: фільм створено з ID " << result.value() << "\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handleCreateHall() {
    std::string name;
    int rows = 0;
    int seatsPerRow = 0;

    std::cout << "Назва залу: ";
    std::getline(std::cin, name);
    std::cout << "Кількість рядів: ";
    std::cin >> rows;
    std::cout << "Місць у ряду: ";
    std::cin >> seatsPerRow;
    clearInputBuffer();

    const auto result = cinemaService_.createHall(name, rows, seatsPerRow);
    if (result.isSuccess()) {
        std::cout << "Успіх: зал створено з ID " << result.value() << "\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handleCreateSession() {
    std::string movieId;
    std::string hallId;
    std::string startAt;
    double basePrice = 0.0;

    std::cout << "ID фільму: ";
    std::getline(std::cin, movieId);
    std::cout << "ID залу: ";
    std::getline(std::cin, hallId);
    std::cout << "Початок (YYYY-MM-DDTHH:MM): ";
    std::getline(std::cin, startAt);
    std::cout << "Базова ціна: ";
    std::cin >> basePrice;
    clearInputBuffer();

    const auto result = cinemaService_.scheduleSession(movieId, hallId, startAt, basePrice);
    if (result.isSuccess()) {
        std::cout << "Успіх: сеанс створено з ID " << result.value() << "\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handleReserveSeat() {
    std::string sessionId;
    int row = 0;
    int number = 0;
    std::string customerName;

    std::cout << "ID сеансу: ";
    std::getline(std::cin, sessionId);
    std::cout << "Ряд: ";
    std::cin >> row;
    std::cout << "Місце: ";
    std::cin >> number;
    clearInputBuffer();
    std::cout << "Клієнт: ";
    std::getline(std::cin, customerName);

    const auto result = cinemaService_.reserveSeat(sessionId, row, number, customerName);
    if (result.isSuccess()) {
        std::cout << "Успіх: бронювання " << result.value().id() << " на суму "
                  << result.value().finalPrice() << " грн\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handlePurchaseReservation() {
    std::string reservationId;
    double payment = 0.0;

    std::cout << "ID бронювання: ";
    std::getline(std::cin, reservationId);
    std::cout << "Сума оплати: ";
    std::cin >> payment;
    clearInputBuffer();

    const auto result = cinemaService_.purchaseReservation(reservationId, payment);
    if (result.isSuccess()) {
        std::cout << "Успіх: квиток " << result.value().id() << " оформлено\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handleCancelReservation() {
    std::string reservationId;

    std::cout << "ID бронювання: ";
    std::getline(std::cin, reservationId);

    const auto result = cinemaService_.cancelReservation(reservationId);
    if (result.isSuccess()) {
        std::cout << "Успіх: бронювання скасовано\n";
    } else {
        std::cout << "Помилка: " << result.error() << "\n";
    }
}

void ConsoleApp::handleAnalytics() const {
    std::cout << "Топ сеансів за виторгом:\n";
    const auto sessions = queryService_.topSessionsByRevenue(5);
    if (sessions.empty()) {
        std::cout << "Аналітика ще порожня.\n";
    }

    for (const auto& session : sessions) {
        std::cout << "- " << session.sessionId << " | " << session.movieTitle << " | "
                  << session.startAt << " | продано: " << session.soldTickets
                  << " | виторг: " << session.revenue << " грн\n";
    }

    std::cout << "\nФільми з середньою заповненістю >= 30%:\n";
    const auto movies = queryService_.moviesByOccupancy(0.30);
    if (movies.empty()) {
        std::cout << "Немає фільмів, що відповідають критерію.\n";
    }

    for (const auto& movie : movies) {
        std::cout << "- " << movie.movieTitle << " | середня заповненість: "
                  << (movie.averageOccupancy * 100.0) << "%\n";
    }
}

void ConsoleApp::handleReferenceData() const {
    std::cout << "Фільми:\n";
    for (const auto& movie : cinemaService_.listMovies()) {
        std::cout << "- " << movie.id() << " | " << movie.title() << " | " << movie.genre()
                  << " | " << movie.durationMinutes() << " хв\n";
    }

    std::cout << "\nЗали:\n";
    for (const auto& hall : cinemaService_.listHalls()) {
        std::cout << "- " << hall.id() << " | " << hall.name() << " | рядів: " << hall.rows()
                  << " | місць в ряду: " << hall.seatsPerRow() << "\n";
    }

    std::cout << "\nСеанси:\n";
    for (const auto& session : cinemaService_.listSessions()) {
        std::cout << "- " << session.id() << " | movie=" << session.movieId()
                  << " | hall=" << session.hallId() << " | " << session.startAt()
                  << " | базова ціна: " << session.basePrice() << "\n";
    }
}

}  // namespace cinema::presentation