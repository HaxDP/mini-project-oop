#pragma once

#include <optional>
#include <stdexcept>
#include <string>

namespace cinema::domain {

class OperationResult {
public:
    static OperationResult Success() { return OperationResult(true, ""); }
    static OperationResult Failure(std::string error) {
        return OperationResult(false, std::move(error));
    }

    bool isSuccess() const { return success_; }
    const std::string& error() const { return error_; }

private:
    OperationResult(bool success, std::string error)
        : success_(success), error_(std::move(error)) {}

    bool success_;
    std::string error_;
};

template <typename T>
class Result {
public:
    static Result<T> Success(T value) {
        return Result<T>(true, std::move(value), "");
    }

    static Result<T> Failure(std::string error) {
        return Result<T>(false, std::nullopt, std::move(error));
    }

    bool isSuccess() const { return success_; }

    const T& value() const {
        if (!success_ || !value_.has_value()) {
            throw std::logic_error("Значення недоступне для помилкового результату");
        }
        return value_.value();
    }

    T& value() {
        if (!success_ || !value_.has_value()) {
            throw std::logic_error("Значення недоступне для помилкового результату");
        }
        return value_.value();
    }

    const std::string& error() const { return error_; }

private:
    Result(bool success, std::optional<T> value, std::string error)
        : success_(success), value_(std::move(value)), error_(std::move(error)) {}

    bool success_;
    std::optional<T> value_;
    std::string error_;
};

}  // namespace cinema::domain