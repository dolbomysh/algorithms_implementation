#include <iostream>
#include <vector>
#include <algorithm>

struct Job {
    size_t job_number;
    size_t process_time;
    size_t deadline;
    int weight;
};

bool compare_deadlines(const Job& first, const Job& second) {
    return first.deadline < second.deadline;
}

bool compare_weights(const Job& first, const Job& second) {
    return first.weight > second.weight;
}

std::pair<int, std::vector<size_t>> OptimalJobs(std::vector<Job> jobs) {
    size_t jobs_number = jobs.size();
    std::sort(jobs.begin(), jobs.end(), compare_deadlines);

    size_t total_time = 0;
    for (const Job& job : jobs) {
        total_time += job.process_time;
    }

    std::vector<int> dp(total_time + 1, 0);
    std::vector<std::vector<bool>> used(jobs_number + 1, std::vector<bool>(total_time + 1, false));

    // проход dp
    for (size_t i = 0; i < jobs_number; ++i) {
        for (int t = total_time; t >= 0; --t) {
            if (t + jobs[i].process_time <= jobs[i].deadline) {
                int new_val = dp[t] + jobs[i].weight;
                if (new_val > dp[t + jobs[i].process_time]) {
                    dp[t + jobs[i].process_time] = new_val;
                    used[i + 1][t + jobs[i].process_time] = true;
                }
            }
        }
    }

    // найдем максимальный вес и время
    int max_weight = 0;
    int best_time = 0;
    for (size_t t = 0; t <= total_time; ++t) {
        if (dp[t] > max_weight) {
            max_weight = dp[t];
            best_time = t;
        }
    }

    // множество в срок
    std::vector<Job> on_time_jobs;
    std::vector<bool> on_time_indices(jobs_number, false);
    int time = best_time;
    for (int i = jobs_number; i >= 1; --i) {
        if (used[i][time]) {
            on_time_jobs.push_back(jobs[i - 1]);
            on_time_indices[i - 1] = true;
            time -= jobs[i - 1].process_time;
        }
    }
    std::reverse(on_time_jobs.begin(), on_time_jobs.end());

    // множество опаздывающих
    std::vector<Job> late_jobs;
    for (size_t i = 0; i < jobs_number; ++i) {
        if (!on_time_indices[i]) {
            late_jobs.push_back(jobs[i]);
        }
    }

    std::sort(late_jobs.begin(), late_jobs.end(), compare_weights);

    // итоговое расписание
    std::vector<size_t> schedule;
    for (const Job& job : on_time_jobs) {
        schedule.push_back(job.job_number);
    }
    for (const Job& job : late_jobs) {
        schedule.push_back(job.job_number);
    }

    return {max_weight, schedule};
}

int main() {
    size_t jobs_number;
    std::cin >> jobs_number;
    std::vector<Job> jobs(jobs_number);

    for (size_t i = 0; i < jobs_number; ++i) {
        jobs[i].job_number = i + 1;
        std::cin >> jobs[i].process_time >> jobs[i].deadline >> jobs[i].weight;
    }

    auto result = OptimalJobs(jobs);
    std::cout << "Максимальный суммарный вес незапаздывающих задач: " << result.first << std::endl;
    std::cout << "Оптимальное расписание: ";
    for (size_t job_id : result.second) {
        std::cout << job_id << " ";
    }
    std::cout << std::endl;
}

/*
 Ввод:
 6
 3 6 20
 2 7 25
 4 10 15
 1 5 10
 6 12 50
 2 9 30

 Вывод:
 Максимальный суммарный вес незапаздывающих задач: 115
 Оптимальное расписание: 4 2 6 5 1 3
 */