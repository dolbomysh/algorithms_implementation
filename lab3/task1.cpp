// Поиск оптимального порядка выполнения работ, при учете времени выполниния и стоимости на одной машине.
// Сложность O(nlogn)

#include <vector>
#include <iostream>
#include <algorithm>

struct Job {
    size_t job_number;
    size_t start;
    size_t deadline;
    int cost;
};

bool compare_deadlines(const Job& first, const Job& second) {
    return first.deadline < second.deadline;
}

// поиск последней работы с временем завершения не превосходящим время страта текущей
int jobs_upper_bound(const std::vector<Job>& jobs, int index) {
    int left = 0;
    int right = index - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (jobs[mid].deadline <= jobs[index].start) {
            if (mid + 1 <= right && jobs[mid + 1].deadline <= jobs[index].start)
                left = mid + 1;
            else
                return mid;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

std::pair<int, std::vector<size_t>> OptimalJobs(std::vector<Job> jobs) {
    std::sort(jobs.begin(), jobs.end(), compare_deadlines);

    // dp[i] - лучшая стоимость для первых i работ
    std::vector<int> dp(jobs.size());
    std::vector<int> previous(jobs.size(), -1);
    std::vector<bool> chosen(jobs.size(), false);

    for (int i = 0; i < jobs.size(); ++i) {
        int current_cost = jobs[i].cost;
        int prev_job = jobs_upper_bound(jobs, i);

        if (prev_job != -1)
            current_cost += dp[prev_job];

        if (i == 0 || current_cost > dp[i - 1]) {
            dp[i] = current_cost;
            previous[i] = prev_job;
            chosen[i] = true;
        } else {
            dp[i] = dp[i - 1];
            previous[i] = i - 1;
        }
    }

    // восстановление ответа
    std::vector<size_t> result_jobs;
    int index = static_cast<int>(jobs.size() - 1);
    while (index >= 0) {
        if (chosen[index]) {
            result_jobs.push_back(jobs[index].job_number);
        }
        index = previous[index];
    }

    std::reverse(result_jobs.begin(), result_jobs.end());
    return {dp[dp.size() - 1], result_jobs};
}
int main() {
    size_t jobs_number;
    std::cin >> jobs_number;
    std::vector<Job> jobs(jobs_number);

    for (size_t i = 0; i < jobs_number; ++i) {
        jobs[i].job_number = i + 1;
        std::cin >> jobs[i].start;
        std::cin >> jobs[i].deadline;
        std::cin >> jobs[i].cost;
    }

    auto best_jobs = OptimalJobs(jobs);

    std::cout << "Наибольшая возможная стоимость равна " << best_jobs.first << std::endl;
    std::cout << "Достигается при порядке выпонения ";
    for (size_t index : best_jobs.second) {
        std::cout << index << ' ';
    }
}

/*
  Ввод:
  5
  1 3 1
  4 6 3
  2 5 3
  1 4 2
  4 10 2
  Вывод:
  Наибольшая возможная стоимость равна 5
  Достигается при порядке выпонения 4 2
 */