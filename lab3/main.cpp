#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <algorithm>
#include <numeric>

struct Task {
    int id;
    std::vector<int> processingTimes;
};

struct Result {
    int optimalTime;
    std::string optimalSequence;
};

struct Data {
    int id;
    int numberOfTasks;
    int numberOfMachines;
    std::vector<Task> tasks;
    Result result;
};

Task readTask(std::istream& in, int taskID, int numMachines) {
    Task task;
    task.id = taskID;
    int time;
    for (int i = 0; i < numMachines; ++i) {
        in >> time;
        task.processingTimes.push_back(time);
    }
    return task;
}

Result readOptimalResult(std::istream& in) 
{
    Result result;
    std::string line;

    std::getline(in, line);
    std::istringstream optStream(line);
    optStream >> result.optimalTime;

    std::getline(in, result.optimalSequence);

    return result;
}

std::vector<Data> loadDataFromFile(const std::string& filePath) {
    std::vector<Data> datasets;
    std::ifstream file(filePath);
    std::string line;
    int datasetID = 0;

    while (getline(file, line)) {
        if (line.find("data.") != std::string::npos) {
            Data dataset;
            dataset.id = ++datasetID;
            file >> dataset.numberOfTasks >> dataset.numberOfMachines;

            dataset.tasks.reserve(dataset.numberOfTasks);
            for (int i = 0; i < dataset.numberOfTasks; ++i) {
                Task task = readTask(file, i + 1, dataset.numberOfMachines);
                dataset.tasks.push_back(task);
            }

            std::string dummyLine = "";
            while (dummyLine.find("neh:") == std::string::npos) 
            {
                std::getline(file, dummyLine);
            }
            dataset.result = readOptimalResult(file);
            datasets.push_back(dataset);

            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return datasets;
}

std::vector<int> extract_column(const std::vector<std::vector<int>>& matrix, int col) {
    std::vector<int> column(matrix.size());
    for (size_t i = 0; i < matrix.size(); ++i) {
        column[i] = matrix[i][col];
    }
    return column;
}

void forward_propagate(std::vector<std::vector<int>>& times) {
    for (size_t i = 1; i < times.size(); ++i) {
        for (size_t j = 1; j < times[i].size(); ++j) {
            times[i][j] += std::max(times[i-1][j], times[i][j-1]);
        }
    }
}

void backward_propagate(std::vector<std::vector<int>>& times) {
    for (int i = times.size() - 2; i >= 0; --i) {
        for (int j = times[i].size() - 2; j >= 0; --j) {
            times[i][j] += std::max(times[i+1][j], times[i][j+1]);
        }
    }
}

int calculateCmax(const std::vector<std::vector<int>>& processing_times, const std::vector<int>& order) {
    int num_tasks = processing_times.size();
    int num_machines = processing_times[0].size();
    std::vector<std::vector<int>> times(num_tasks, std::vector<int>(num_machines, 0));

    for (size_t i = 0; i < order.size(); ++i) {
        int task_idx = order[i] - 1;
        for (int j = 0; j < num_machines; ++j) {
            if (i == 0 && j == 0) {
                times[i][j] = processing_times[task_idx][j];
            } else if (i == 0) {
                times[i][j] = times[i][j-1] + processing_times[task_idx][j];
            } else if (j == 0) {
                times[i][j] = times[i-1][j] + processing_times[task_idx][j];
            } else {
                times[i][j] = std::max(times[i-1][j], times[i][j-1]) + processing_times[task_idx][j];
            }
        }
    }

    forward_propagate(times);
    backward_propagate(times);
    return times[num_tasks-1][num_machines-1];
}

std::vector<int> sort_tasks(const std::vector<Task>& tasks) {
    std::vector<int> order(tasks.size());
    for (size_t i = 0; i < tasks.size(); ++i) {
        order[i] = tasks[i].id;
    }
    std::sort(order.begin(), order.end(), [&tasks](int a, int b) {
        int sumA = std::accumulate(tasks[a-1].processingTimes.begin(), tasks[a-1].processingTimes.end(), 0);
        int sumB = std::accumulate(tasks[b-1].processingTimes.begin(), tasks[b-1].processingTimes.end(), 0);
        return sumA > sumB;
    });
    return order;
}

std::vector<int> accelerated_qneh(const std::vector<Task>& tasks) {
    std::vector<int> order = sort_tasks(tasks);
    std::vector<int> optimal_order;

    for (size_t i = 0; i < order.size(); ++i) {
        int best_Cmax = std::numeric_limits<int>::max();
        size_t best_pos = 0;

        for (size_t j = 0; j <= optimal_order.size(); ++j) {
            std::vector<int> temp_order = optimal_order;
            temp_order.insert(temp_order.begin() + j, order[i]);
            std::vector<std::vector<int>> processing_times(tasks.size(), std::vector<int>(tasks[0].processingTimes.size()));
            for (size_t k = 0; k < tasks.size(); ++k) {
                processing_times[k] = tasks[k].processingTimes;
            }
            int Cmax = calculateCmax(processing_times, temp_order);
            if (Cmax < best_Cmax) {
                best_Cmax = Cmax;
                best_pos = j;
            }
        }

        optimal_order.insert(optimal_order.begin() + best_pos, order[i]);
    }

    return optimal_order;
}

int main() {
    const std::string filePath = "neh.data.txt";
    std::vector<Data> datasets = loadDataFromFile(filePath);

    for (const auto& dataset : datasets) {
        std::cout << "Dataset " << dataset.id << ":\n";
        std::cout << "Optimal Time: " << dataset.result.optimalTime << "\n";
        std::cout << "Optimal Sequence: ";
        for (auto task : dataset.result.optimalSequence) {
            std::cout << task << " ";
        }
        std::cout << "\n";

        std::vector<int> order = accelerated_qneh(dataset.tasks);
        std::cout << "QNEH Order: ";
        for (int task : order) {
            std::cout << task << " ";
        }
        std::cout << "\n";

        std::vector<std::vector<int>> processing_times(dataset.tasks.size(), std::vector<int>(dataset.tasks[0].processingTimes.size()));
        for (size_t i = 0; i < dataset.tasks.size(); ++i) {
            processing_times[i] = dataset.tasks[i].processingTimes;
        }
        int qneh_time = calculateCmax(processing_times, order);
        std::cout << "QNEH Optimal Time: " << qneh_time << "\n";
    }

    return 0;
}
