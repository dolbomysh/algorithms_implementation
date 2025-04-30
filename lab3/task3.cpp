#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <unordered_map>

struct Job {
    size_t job_number;
    int duration;
    int deadline;
};

bool compare_deadlines(const Job& a, const Job& b) {
    return a.deadline < b.deadline;
}

std::string get_state_key(const std::vector<bool>& scheduled, int current_time) {
    std::string key;
    for (bool b : scheduled) key += b ? '1' : '0';
    key += std::to_string(current_time);
    return key;
}

int dp(const std::vector<Job>& jobs, std::vector<bool>& scheduled,
                int current_time, std::unordered_map<std::string, int>& memo) {
    std::string key = get_state_key(scheduled, current_time);
    if (memo.count(key)) return memo[key];

    int min_tardiness = INT_MAX;
    bool all_scheduled = true;

    for (int i = 0; i < jobs.size(); ++i) {
        if (!scheduled[i]) {
            all_scheduled = false;
            scheduled[i] = true;
            int completion_time = current_time + jobs[i].duration;
            int tardiness = std::max(0, completion_time - jobs[i].deadline);

            tardiness += dp(jobs, scheduled, completion_time, memo);

            min_tardiness = std::min(min_tardiness, tardiness);
            scheduled[i] = false;
        }
    }

    return memo[key] = (all_scheduled ? 0 : min_tardiness);
}

std::vector<Job> reconstruct_schedule(const std::vector<Job>& jobs,
                                 const std::unordered_map<std::string, int>& memo) {
    std::vector<bool> scheduled(jobs.size(), false);
    int current_time = 0;
    std::vector<Job> result;

    while (result.size() < jobs.size()) {
        for (int i = 0; i < jobs.size(); ++i) {
            if (!scheduled[i]) {
                std::vector<bool> new_scheduled = scheduled;
                new_scheduled[i] = true;
                int new_time = current_time + jobs[i].duration;
                std::string key = get_state_key(new_scheduled, new_time);

                if (memo.at(key) + std::max(0, new_time - jobs[i].deadline) ==
                    memo.at(get_state_key(scheduled, current_time))) {
                    result.push_back(jobs[i]);
                    scheduled[i] = true;
                    current_time = new_time;
                    break;
                }
            }
        }
    }

    return result;
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<Job> jobs(n);

    for (size_t i = 0; i < n; ++i) {
        jobs[i].job_number = i + 1;
        std::cin >> jobs[i].duration;
        std::cin >> jobs[i].deadline;
    }

    sort(jobs.begin(), jobs.end(), compare_deadlines);

    std::vector<bool> scheduled(jobs.size(), false);
    std::unordered_map<std::string, int> memo;

    int min_tardiness = dp(jobs, scheduled, 0, memo);
    std::vector<Job> optimal_schedule = reconstruct_schedule(jobs, memo);

    std::cout << "Оптимальный порядок работ: ";
    for (const auto& job : optimal_schedule) {
        std::cout << job.job_number << " ";
    }
    std::cout << "\nСумма запаздываний: " << min_tardiness << std::endl;

}

/*
 Ввод
 4
 3 4
 2 5
 4 7
 1 3
 Вывод
 Оптимальный порядок работ: 4 1 2 3
 Сумма запаздываний: 4
 */