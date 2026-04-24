#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace cinema::tests {

class TestContext {
public:
    void expectTrue(bool condition, const std::string& message) {
        if (!condition) {
            ++failed_;
            std::cout << "[FAIL] " << message << "\n";
        }
    }

    template <typename T, typename U>
    void expectEqual(const T& left, const U& right, const std::string& message) {
        if (!(left == right)) {
            ++failed_;
            std::cout << "[FAIL] " << message << " (очікувалось: " << right
                      << ", отримано: " << left << ")\n";
        }
    }

    int failedCount() const { return failed_; }

private:
    int failed_{0};
};

}  // namespace cinema::tests

#define EXPECT_TRUE(ctx, cond, msg) (ctx).expectTrue((cond), (msg))
#define EXPECT_EQ(ctx, left, right, msg) (ctx).expectEqual((left), (right), (msg))