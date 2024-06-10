#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <climits>

using namespace std;

struct WorkUnit {
    int unitID;
    vector<int> durations;
    int totalDuration;
};

vector<int> shuffleOrder(const vector<int>& originalOrder) {
    vector<int> modifiedOrder = originalOrder;
    int firstPosition = rand() % originalOrder.size();
    int secondPosition;
    do {
        secondPosition = rand() % originalOrder.size();
    } while (firstPosition == secondPosition);
    swap(modifiedOrder[firstPosition], modifiedOrder[secondPosition]);
    return modifiedOrder;
}

int computeMaxDuration(const vector<WorkUnit>& units, const vector<int>& sequence) {
    int machineCount = units[0].durations.size();
    int maxDuration = 0;
    vector<int> lastCompletion(machineCount, 0);
    for (int idx : sequence) {
        int duration = 0;
        for (int m = 0; m < machineCount; m++) {
            duration = max(duration, lastCompletion[m]) + units[idx].durations[m];
            lastCompletion[m] = duration;
            maxDuration = duration;
        }
    }
    return maxDuration;
}

vector<int> performAnnealing(const vector<WorkUnit>& units, int cycles, double initialTemp, double reductionFactor) {
    vector<int> sequence(units.size());
    ofstream output("results.csv", ofstream::out);

    for (size_t i = 0; i < sequence.size(); i++) {
        sequence[i] = i;
    }

    int currentMax = computeMaxDuration(units, sequence);

    srand(static_cast<unsigned int>(time(nullptr)));

    double temperature = initialTemp;

    for (int i = 0; i < cycles; i++) {
        vector<int> newSequence = shuffleOrder(sequence);

        int newMax = computeMaxDuration(units, newSequence);
        int change = newMax - currentMax;

        if (change < 0) {
            sequence = newSequence;
            currentMax = newMax;
        } else {
            double chance = exp(-change / temperature);
            if (static_cast<double>(rand()) / RAND_MAX < chance) {
                sequence = newSequence;
                currentMax = newMax;
            }
        }
        if (i % 100 == 0) {
            output << i << ", " << currentMax << endl;
        }

        temperature *= reductionFactor;
    }
    output.close();
    return sequence;
}

pair<double, double> defineTemperatures(int maxChange, int minChange) {
    if (maxChange <= 0 || minChange <= 0) {
        throw invalid_argument("maxChange and minChange must be greater than 0.");
    }
    double upperTemp = -maxChange / log(0.9);
    double lowerTemp = -minChange / log(0.1);
    return {upperTemp, lowerTemp};
}

double determineCoolingRate(double upperTemp, double lowerTemp, int cycles) {
    if (upperTemp <= 0 || lowerTemp <= 0) {
        throw invalid_argument("upperTemp and lowerTemp must be greater than 0.");
    }
    return pow(lowerTemp / upperTemp, 1.0 / cycles);
}

pair<int, int> computeDeltaExtremes(const vector<WorkUnit>& units, int alterations) {
    vector<int> sequence(units.size());
    for (size_t i = 0; i < sequence.size(); i++) sequence[i] = i;

    int currentMax = computeMaxDuration(units, sequence);
    int maxDelta = INT_MIN;
    int minDelta = INT_MAX;

    for (int i = 0; i < alterations; i++) {
        vector<int> newSequence = shuffleOrder(sequence);
        int newMax = computeMaxDuration(units, newSequence);
        int delta = abs(newMax - currentMax);

        if (delta >= maxDelta) maxDelta = delta;
        if (delta <= minDelta) minDelta = delta;

        currentMax = newMax;
    }

    if (minDelta == 0) minDelta = 1;
    return {maxDelta, minDelta};
}

int main() {
    string filePath = "neh.data.txt";
    ifstream dataFile(filePath);
    vector<vector<WorkUnit>> dataSets;
    string fileLine;

    if (!dataFile.is_open()) {
        cerr << "Failed to open file: " << filePath << endl;
        return 1;
    }

    vector<WorkUnit> currentData;
    bool shouldSave = false;
    int itemCounter = 0;

    while (getline(dataFile, fileLine)) {
        if (fileLine.empty()) {
            shouldSave = false;
            if (!currentData.empty()) {
                dataSets.push_back(currentData);
                currentData.clear();
            }
            continue;
        }

        if (fileLine.find("data.") != string::npos) {
            shouldSave = true;
            itemCounter = 0;
        } else {
            if (shouldSave) {
                if (itemCounter == 0) {
                    itemCounter++;
                    continue;
                }
                istringstream lineStream(fileLine);
                int number;
                int sumDurations = 0;
                vector<int> taskDurations;
                while (lineStream >> number) {
                    sumDurations += number;
                    taskDurations.push_back(number);
                }
                currentData.push_back({itemCounter, taskDurations, sumDurations});
                itemCounter++;
            }
        }
    }

    if (!currentData.empty()) {
        dataSets.push_back(currentData);
    }

    dataFile.close();

    int startData = 100;
    int endData = 110;

    cout << "Simulated Annealing - Results" << endl;

    chrono::duration<double> totalTime = chrono::duration<double>::zero();

    int totalCycles = 100000;

    for (int i = startData; i <= endData; i++) {
        auto extremes = computeDeltaExtremes(dataSets[i], 1000);

        auto temps = defineTemperatures(extremes.first, extremes.second);
        
        double startTemp = temps.first;
        double coolRate = determineCoolingRate(startTemp, temps.second, totalCycles);

        auto startTime = chrono::high_resolution_clock::now();
        vector<int> result = performAnnealing(dataSets[i], totalCycles, startTemp, coolRate);
        auto endTime = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = endTime - startTime;

        totalTime += duration;

        cout << "data." << i << ": Cmax: " << computeMaxDuration(dataSets[i], result) << " ";
        cout << "| Time: " << duration.count() << " s ";
        cout << "| Start Temp: " << startTemp << " | Cooling Rate: " << coolRate << endl;
    }

    cout << "Total Execution Time: " << totalTime.count() << " s" << endl;

    return 0;
}
