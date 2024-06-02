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

vector<int> sortJobsByTotalTime(const vector<Job>& jobs) {
    vector<Job> sortedJobs = jobs;
    stable_sort(sortedJobs.begin(), sortedJobs.end(), [](const Job& a, const Job& b) {
        return a.totalProcessingTime > b.totalProcessingTime;
    });

    vector<int> jobOrder;

    transform(sortedJobs.begin(), sortedJobs.end(), back_inserter(jobOrder), [](const Job& job) {
        return job.jobId - 1;
    });

    return jobOrder;
}

vector<int> extractColumn(const vector<vector<int>>& matrix, int col) {
    int rows = matrix.size();
    vector<int> column(rows, 0);
    for (int i = 0; i < rows; ++i){
        column[i] = matrix[i][col];
    }
    return column;
}

void forwardPropagation(const vector<Job>& jobs, const vector<int>& jobOrder, vector<vector<int>>& forwardMatrix, int start = 0) {
    int machines = jobs[0].processingTimes.size();
    vector<int> previous(machines, 0);
    if (start != 0){
        previous = extractColumn(forwardMatrix, jobOrder[start-1]);
    }
    for (int i = start; i < jobOrder.size(); i++) {
        int time = 0;
        for (int m = 0; m < machines; m++) {
            time = max(time, previous[m]) + jobs[jobOrder[i]].processingTimes[m];
            forwardMatrix[m][jobOrder[i]] = time;
            previous[m] = time;
        }
    }
}

void backwardPropagation(const vector<Job>& jobs, const vector<int>& jobOrder, vector<vector<int>>& backwardMatrix, int start = -1) {
    if (jobOrder.size() == 0) {
        return;
    }
    int machines = jobs[0].processingTimes.size();
    vector<int> previous(machines, 0);
    if (start != jobOrder.size()-1){
        previous = extractColumn(backwardMatrix, jobOrder[start+1]);
    }
    for (int i = start; i >= 0; i--) {
        int time = 0;
        for (int m = machines-1; m >= 0; m--) {
            time = max(time, previous[m]) + jobs[jobOrder[i]].processingTimes[m];
            backwardMatrix[m][jobOrder[i]] = time;
            previous[m] = time;
        }
    }
}

int computeCmax(const Job& job, const vector<vector<int>>& forwardMatrix, const vector<vector<int>>& backwardMatrix, int pos, const vector<int>& jobOrder) {
    int time = 0;
    int cmax = 0;
    int totalJobs = jobOrder.size();
    int machines = job.processingTimes.size();
    vector<int> jobTimes(machines, 0);
    vector<int> previous;
    if (pos != 0){
        previous = extractColumn(forwardMatrix, jobOrder[pos-1]);
    } else {
        previous = vector<int>(machines, 0);
    }
    for (int m = 0; m < machines; m++) {
        time = max(time, previous[m]) + job.processingTimes[m];
        jobTimes[m] = time;
        if (pos < totalJobs) {
            int value = jobTimes[m] + backwardMatrix[m][jobOrder[pos]];
            if (value > cmax){
                cmax = value;
            }
        }
    }
    if (pos == totalJobs){
        return time;
    } 
    return cmax; 
}

vector<int> optimizedNEH(const vector<Job>& jobs) {
    vector<int> initialOrder = sortJobsByTotalTime(jobs);
    vector<int> finalOrder;
    int totalJobs = jobs.size();
    int machines = jobs[0].processingTimes.size();
    int bestPos = 0;
    vector<vector<int>> forwardMatrix(machines, vector<int>(totalJobs, 0));
    vector<vector<int>> backwardMatrix(machines, vector<int>(totalJobs, 0)); 

    for (int jobIndex : initialOrder) {
        int currentJobs = finalOrder.size();
        int minCmax = numeric_limits<int>::max();
        forwardPropagation(jobs, finalOrder, forwardMatrix, bestPos);
        backwardPropagation(jobs, finalOrder, backwardMatrix, bestPos);
        for (int i = 0; i < currentJobs + 1; i++){
            int c = computeCmax(jobs[jobIndex], forwardMatrix, backwardMatrix, i, finalOrder);
            if (c < minCmax) {
                minCmax = c;
                bestPos = i;
            }
        }
        finalOrder.insert(finalOrder.begin() + bestPos, jobIndex);
    }
    return finalOrder;
}

int computeFinalCmax(const vector<Job>& jobs, const vector<int>& jobOrder) {
    int machines = jobs[0].processingTimes.size();
    int cmax = 0;
    vector<int> previous(machines, 0);
    for (int jobIndex : jobOrder) {
        int time = 0;
        for (int m = 0; m < machines; m++) {
            time = max(time, previous[m]) + jobs[jobIndex].processingTimes[m];
            previous[m] = time;
            cmax = time;
        }
    }
    return cmax;
}

vector<int> basicNEH(const vector<Job>& jobs) {
    vector<int> initialOrder = sortJobsByTotalTime(jobs);
    vector<int> finalOrder;
    int totalJobs = jobs.size();
    int machines = jobs[0].processingTimes.size();
    int bestPos = 0;

    for (int jobIndex : initialOrder) {
        int currentJobs = finalOrder.size();
        int minCmax = numeric_limits<int>::max();
        for (int i = 0; i < currentJobs + 1; i++) {
            vector<int> newOrder = finalOrder;
            newOrder.insert(newOrder.begin() + i, jobIndex);
            int c = computeFinalCmax(jobs, newOrder);
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
    vector<vector<Job>> allDatasets;
    string line;

    if (!file.is_open()) {
        cerr << "Failed to open file: " << filePath << endl;
        return 1;
    }

    vector<Job> currentDataset;
    bool isSavingData = false;
    int taskCounter = 0;

    while (getline(file, line)) {
        if (line.empty()) {
            isSavingData = false;
            if (!currentDataset.empty()) {
                allDatasets.push_back(currentDataset);
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
        allDatasets.push_back(currentDataset);
    }

    file.close();

    int dataStart = 0;
    int dataEnd = 120;
    cout << "NEH Results" << endl;

    chrono::duration<double> totalExecutionTime = chrono::duration<double>::zero();
    for (int i = dataStart; i <= dataEnd; i++) {
        cout << "data." << i << ": Cmax: ";
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = basicNEH(allDatasets[i]);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        totalExecutionTime += duration;
        cout << computeFinalCmax(allDatasets[i], result) << " ";
        cout << "Czas: " << duration.count() << endl;
    }
    cout << "Czas dzialania programu NEH: " << totalExecutionTime.count() << " s" << endl;

    cout << "QNEH Results" << endl;
    totalExecutionTime = chrono::duration<double>::zero();
    for (int i = dataStart; i <= dataEnd; i++) {
        cout << "data." << i << ": Cmax: ";
        auto start = chrono::high_resolution_clock::now();
        vector<int> result = optimizedNEH(allDatasets[i]);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        totalExecutionTime += duration;
        cout << computeFinalCmax(allDatasets[i], result) << " ";
        cout << "Czas: " << duration.count() << endl;
    }
    cout << "Czas dzialania programu QNEH: " << totalExecutionTime.count() << " s" << endl;

    return 0;
}
