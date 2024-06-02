#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <limits>

using namespace std;


struct Job {
    int jobId;
    vector<int> processingTimes;
    int totalProcessingTime;
};


//sort jobs by total processing time
vector<int> getSortedJobOrder(const vector<Job>& tasks) {
    vector<Job> sortedTasks = tasks;
    stable_sort(sortedTasks.begin(), sortedTasks.end(), [](const Job& a, const Job& b) {
        return a.totalProcessingTime > b.totalProcessingTime;
    });

    vector<int> jobOrder;
    transform(sortedTasks.begin(), sortedTasks.end(), back_inserter(jobOrder), [](const Job& task) {
        return task.jobId - 1;
    });

    return jobOrder;
}


//extract a column from a matrix
vector<int> getColumn(const vector<vector<int>>& matrix, int col) {
    int numRows = matrix.size();
    vector<int> column(numRows, 0);
    for (int i = 0; i < numRows; ++i) {
        column[i] = matrix[i][col];
    }
    return column;
}


//propagate times forward
void propagateForward(const vector<Job>& tasks, const vector<int>& jobOrder, vector<vector<int>>& forwardMatrix, int start = 0) {
    int numMachines = tasks[0].processingTimes.size();
    vector<int> previous(numMachines, 0);
    if (start != 0) {
        previous = getColumn(forwardMatrix, jobOrder[start - 1]);
    }
    for (int i = start; i < jobOrder.size(); i++) {
        int time = 0;
        for (int m = 0; m < numMachines; m++) {
            time = max(time, previous[m]) + tasks[jobOrder[i]].processingTimes[m];
            forwardMatrix[m][jobOrder[i]] = time;
            previous[m] = time;
        }
    }
}


//propagate times backward
void propagateBackward(const vector<Job>& tasks, const vector<int>& jobOrder, vector<vector<int>>& backwardMatrix, int start = -1) {
    if (jobOrder.empty()) {
        return;
    }
    int numMachines = tasks[0].processingTimes.size();
    vector<int> previous(numMachines, 0);
    if (start != jobOrder.size() - 1) {
        previous = getColumn(backwardMatrix, jobOrder[start + 1]);
    }
    for (int i = start; i >= 0; i--) {
        int time = 0;
        for (int m = numMachines - 1; m >= 0; m--) {
            time = max(time, previous[m]) + tasks[jobOrder[i]].processingTimes[m];
            backwardMatrix[m][jobOrder[i]] = time;
            previous[m] = time;
        }
    }
}


//calculate Cmax
int calculateCmax(const Job& task, const vector<vector<int>>& forwardMatrix, const vector<vector<int>>& backwardMatrix, int pos, const vector<int>& jobOrder) {
    int time = 0;
    int cmax = 0;
    int numJobs = jobOrder.size();
    int numMachines = task.processingTimes.size();
    vector<int> taskTimes(numMachines, 0);
    vector<int> previous;
    if (pos != 0) {
        previous = getColumn(forwardMatrix, jobOrder[pos - 1]);
    } else {
        previous = vector<int>(numMachines, 0);
    }
    for (int m = 0; m < numMachines; m++) {
        time = max(time, previous[m]) + task.processingTimes[m];
        taskTimes[m] = time;
        if (pos < numJobs) {
            int value = taskTimes[m] + backwardMatrix[m][jobOrder[pos]];
            if (value > cmax) {
                cmax = value;
            }
        }
    }
    if (pos == numJobs) {
        return time;
    }
    return cmax;
}


// Optimized NEH algorithm
vector<int> optimizedNEH(const vector<Job>& tasks) {
    vector<int> initialOrder = getSortedJobOrder(tasks);
    vector<int> finalOrder;
    int numJobs = tasks.size();
    int numMachines = tasks[0].processingTimes.size();
    int bestPos = 0;
    vector<vector<int>> forwardMatrix(numMachines, vector<int>(numJobs, 0));
    vector<vector<int>> backwardMatrix(numMachines, vector<int>(numJobs, 0));

    for (int jobIndex : initialOrder) {
        int currentJobs = finalOrder.size();
        int minCmax = numeric_limits<int>::max();
        propagateForward(tasks, finalOrder, forwardMatrix, bestPos);
        propagateBackward(tasks, finalOrder, backwardMatrix, bestPos);
        for (int i = 0; i < currentJobs + 1; i++) {
            int c = calculateCmax(tasks[jobIndex], forwardMatrix, backwardMatrix, i, finalOrder);
            if (c < minCmax) {
                minCmax = c;
                bestPos = i;
            }
        }
        finalOrder.insert(finalOrder.begin() + bestPos, jobIndex);
    }
    return finalOrder;
}


//compute final Cmax
int finalCmax(const vector<Job>& tasks, const vector<int>& jobOrder) {
    int numMachines = tasks[0].processingTimes.size();
    int cmax = 0;
    vector<int> previous(numMachines, 0);
    for (int jobIndex : jobOrder) {
        int time = 0;
        for (int m = 0; m < numMachines; m++) {
            time = max(time, previous[m]) + tasks[jobIndex].processingTimes[m];
            previous[m] = time;
            cmax = time;
        }
    }
    return cmax;
}


// Basic NEH algorithm
vector<int> basicNEH(const vector<Job>& tasks) {
    vector<int> initialOrder = getSortedJobOrder(tasks);
    vector<int> finalOrder;
    int numJobs = tasks.size();
    int numMachines = tasks[0].processingTimes.size();
    int bestPos = 0;

    for (int jobIndex : initialOrder) {
        int currentJobs = finalOrder.size();
        int minCmax = numeric_limits<int>::max();
        for (int i = 0; i < currentJobs + 1; i++) {
            vector<int> newOrder = finalOrder;
            newOrder.insert(newOrder.begin() + i, jobIndex);
            int c = finalCmax(tasks, newOrder);
            if (c < minCmax) {
                minCmax = c;
                bestPos = i;
            }
        }
        finalOrder.insert(finalOrder.begin() + bestPos, jobIndex);
    }
    return finalOrder;
}


int main() {
    string filePath = "neh.data.txt";
    ifstream file(filePath);
    vector<vector<Job>> datasets;
    string line;

    if (!file.is_open()) {
        cerr << "Error: Cannot open file: " << filePath << endl;
        return 1;
    }

    vector<Job> currentDataset;
    bool isSavingData = false;
    int taskCounter = 0;

    while (getline(file, line)) {
        if (line.empty()) {
            isSavingData = false;
            if (!currentDataset.empty()) {
                datasets.push_back(currentDataset);
                currentDataset.clear();
            }
            continue;
        }
        if (line.find("data.") != string::npos) {
            isSavingData = true;
            taskCounter = 0;
        } else {
            if (isSavingData) {
                if (taskCounter == 0) {
                    taskCounter++;
                    continue;
                }
                istringstream iss(line);
                int num;
                int totalTimes = 0;
                vector<int> taskTimes;
                while (iss >> num) {
                    totalTimes += num;
                    taskTimes.push_back(num);
                }
                currentDataset.push_back({taskCounter, taskTimes, totalTimes});
                taskCounter++;
            }
        }
    }
    if (!currentDataset.empty()) {
        datasets.push_back(currentDataset);
    }

    file.close();

    int dataStart = 0;
    int dataEnd = 120;
    cout << "Results for NEH" << endl;

    chrono::duration<double> totalExecutionTime = chrono::duration<double>::zero();
    for (int i = dataStart; i <= dataEnd; i++) {
        cout << "data." << i << ": Cmax: ";
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = basicNEH(datasets[i]);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        totalExecutionTime += duration;
        cout << finalCmax(datasets[i], result) << " ";
        cout << "Execution Time: " << duration.count() << " seconds" << endl;
    }
    cout << "Total execution time for NEH: " << totalExecutionTime.count() << " seconds" << endl;

    cout << "Results for Optimized NEH (QNEH)" << endl;
    totalExecutionTime = chrono::duration<double>::zero();
    for (int i = dataStart; i <= dataEnd; i++) {
        cout << "data." << i << ": Cmax: ";
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = optimizedNEH(datasets[i]);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        totalExecutionTime += duration;
        cout << finalCmax(datasets[i], result) << " ";
        cout << "Execution Time: " << duration.count() << " seconds" << endl;
    }
    cout << "Total execution time for QNEH: " << totalExecutionTime.count() << " seconds" << endl;

    return 0;
}
