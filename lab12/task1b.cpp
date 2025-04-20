// поиск статически оптимального BST за O(n^2) : оптимизация Кнута-Яо

#include <iostream>
#include <vector>
#include <limits>

// восстанавливаем обход дерева
std::vector<size_t> recurrentDSATRansversal(std::vector<size_t>& answer,
                                            const std::vector<std::vector<size_t>>& opt_root,
                                            size_t left, size_t right,
                                            size_t left_index) {
    if (left == right) {
        return answer;
    }
    if (left == right - 1) {
        answer[left_index] = left + 1;
        return answer;
    }

    size_t current_root = opt_root[left][right];
    answer[left_index] = current_root + 1;
    recurrentDSATRansversal(answer, opt_root, left, current_root, left_index + 1);
    if (current_root != right) {
        recurrentDSATRansversal(answer, opt_root, current_root + 1, right, left_index + current_root - left + 1);
    }
    return answer;
}

// возвращаем оптимальную стоимость и конфигурацию BST (DSA)
std::pair<float, std::vector<size_t>> getOptTree(size_t number, const std::vector<float>& success_probabilities,
                                                 const std::vector<float>& not_success_probabilities) {
    // динамика по подотрезкам dp[l][r] - отптимальная стоимость для [l; r)
    std::vector<std::vector<float>> dp(number, std::vector<float>(
            number + 1, std::numeric_limits<float>::infinity()));
    // храним оптимальный корень для [l; r)
    std::vector<std::vector<size_t>> opt_root(number, std::vector<size_t>(
            number + 1, 0));

    // подсчитаем префиксные суммы
    std::vector<float> success_prefix(number + 1, 0);
    std::vector<float> not_success_prefix(number + 2, 0);
    for (size_t i = 0; i < number; ++i) {
        success_prefix[i + 1] = success_prefix[i] + success_probabilities[i];
    }
    for (size_t i = 0; i < number + 1; ++i) {
        not_success_prefix[i + 1] = not_success_prefix[i] + not_success_probabilities[i];
    }

    // инициализация
    for (size_t left = 0; left < number; ++left) {
        dp[left][left + 1] = success_probabilities[left] + not_success_probabilities[left] + not_success_probabilities[left + 1];
    }
    for (size_t left = 0; left < number; ++left) {
        opt_root[left][left + 1] = left;
    }

    // подсчет dp
    float current_cost;
    for (int left = static_cast<int>(number) - 2; left >= 0; --left) {
        for (size_t right = left + 1; right < number + 1; ++right) {
            // добавляем оптимизацию
            size_t from = opt_root[left][right - 1];
            size_t to = opt_root[left + 1][right];
            for (size_t root = from; root <= to; ++root) {
                current_cost = (left == root ? 0 : dp[left][root]) + (root + 1 < right ? dp[root + 1][right] : 0) + (success_prefix[right] - success_prefix[left]) +
                               (not_success_prefix[right + 1] - not_success_prefix[left]);
                if (dp[left][right] > current_cost) {
                    opt_root[left][right] = root;
                    dp[left][right] = current_cost;
                }
            }
        }
    }

    //  восстановление ответа

    std::vector<size_t> dsa_transversal (number);

    return {dp[0][number], recurrentDSATRansversal(dsa_transversal, opt_root, 0, number, 0)};
}


int main() {
    size_t number;
    std::cin >> number;
    std::vector<float> success_probabilities(number);
    std::vector<float> not_success_probabilities(number + 1);
    for (size_t i = 0; i < number; ++i) {
        std::cin >> success_probabilities[i];
    }
    for (size_t i = 0; i < number + 1; ++i) {
        std::cin >> not_success_probabilities[i];
    }
    auto result = getOptTree(number, success_probabilities, not_success_probabilities);
    std::cout << result.first << '\n';
    for (auto x: result.second) {
        std::cout << x << ' ';
    }
}


/*

 Пример:
 Ввод:
 4
 0.7 0.1 0.1 0.1
 0.01 0.01 0.01 0.01 0.02

 Вывод:

 1.66
 1 3 2 4

 */