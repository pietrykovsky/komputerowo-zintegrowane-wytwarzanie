#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

// Structure to hold task information
struct Task {
    int id;
    std::vector<int> processingTimes;
};

// Structure to store the result including optimal time and sequence
struct Result {
    int optimalTime;
    std::string optimalSequence;
};

// Structure to hold all data for one dataset
struct Data {
    int id;
    int numberOfTasks;
    int numberOfMachines;
    std::vector<Task> tasks;
    Result result;
};

// Function to read a single task
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

// Function to parse the optimal sequence and time
Result readOptimalResult(std::istream& in) 
{
    Result result;
    std::string line;

    // Read the optimal time
    std::getline(in, line); // Move to the next line to read the optimal time
    std::istringstream optStream(line);
    optStream >> result.optimalTime;

    // Read the task sequence on the next line
    std::getline(in, result.optimalSequence);

    return result;
}

// Function to load data from the provided file
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

            // Read each task
            dataset.tasks.reserve(dataset.numberOfTasks);
            for (int i = 0; i < dataset.numberOfTasks; ++i) {
                Task task = readTask(file, i + 1, dataset.numberOfMachines);
                dataset.tasks.push_back(task);
            }

            // Skip lines until the optimal result is found
            std::string dummyLine = "";
            while (dummyLine.find("neh:") == std::string::npos) 
            {
                std::getline(file, dummyLine);
            }
            // Read and parse the optimal result
            dataset.result = readOptimalResult(file);
            datasets.push_back(dataset);

            // Skip any trailing newlines after each data block
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return datasets;
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
    }

    return 0;
}
