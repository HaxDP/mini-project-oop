#pragma once

#include <stdexcept>
#include <string>

namespace cinema::domain {

class DomainError : public std::runtime_error {
public:
    explicit DomainError(const std::string& message) : std::runtime_error(message) {}
};

class ApplicationError : public std::runtime_error {
public:
    explicit ApplicationError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace cinema::domain