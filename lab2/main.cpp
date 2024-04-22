#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <chrono>
#include <sstream>
#include <vector>

struct Task
{
    int id;
    int executionTime;
    int penaltyWeight;
    int completionTime;
};

struct Result
{
    int time;
    std::string taskSequence;
};

struct Data
{
    int id;
    int numberOfTasks;
    Task* tasks;
    Result optimalResult;
};

Task readTask(std::istream& in);
Result readOptimalResult(std::istream& in);
Data createDataset(std::istream& in, int id, int numberOfTasks);


void printTask(const Task& task) 
{
    std::cout << "Task " << task.id << ": Exec Time = " << task.executionTime
              << ", Penalty Rate = " << task.penaltyWeight
              << ", Completion Time = " << task.completionTime << std::endl;
}

void printTaskArray(const Task* tasks, int numberOfTasks) 
{
    for (int i = 0; i < numberOfTasks; ++i) 
    {
        printTask(tasks[i]);
    }
}

void printExecutionTime(std::chrono::milliseconds duration)
{
    auto milliseconds = duration.count();
    auto seconds = milliseconds / 1000;
    milliseconds = milliseconds % 1000;

    // Output the duration in seconds and milliseconds
    std::cout << "\nExecution time: " << seconds << " seconds " << milliseconds << " milliseconds" << std::endl;
}

void printResult(const Result& result) 
{
    std::cout << "Result:\nTime = " << result.time << ", Task Sequence = " << result.taskSequence << std::endl;
}

std::list<Data>* loadDataFile(std::string filePath) 
{
    std::list<Data>* datasets = new std::list<Data>();
    std::ifstream dataFile(filePath);
    std::string line;

    while(dataFile >> line) 
    {
        if(line.find("data.") != std::string::npos) 
        { // Check if the line indicates a new dataset
            int id = std::stoi(line.substr(5)); // Extract the dataset ID
            int numberOfTasks;
            dataFile >> numberOfTasks; // Read the number of tasks

            Data dataset = createDataset(dataFile, id, numberOfTasks);
            datasets->push_back(dataset);
        }
    }
    return datasets;
}

// Reads and returns a single task from the input stream
Task readTask(std::istream& in) 
{
    Task task;
    in >> task.executionTime >> task.penaltyWeight >> task.completionTime;
    return task;
}

Result readOptimalResult(std::istream& in) 
{
    Result result;
    std::string line;

    // Read the optimal time
    std::getline(in, line); // Move to the next line to read the optimal time
    std::istringstream optStream(line);
    optStream >> result.time;

    // Read the task sequence on the next line
    std::getline(in, result.taskSequence);

    return result;
}


// Creates and returns a Data object by reading from the input stream
Data createDataset(std::istream& in, int id, int numberOfTasks) 
{
    Data dataset;
    dataset.id = id;
    dataset.numberOfTasks = numberOfTasks;
    dataset.tasks = new Task[numberOfTasks];

    for(int i = 0; i < numberOfTasks; ++i) 
    {
        dataset.tasks[i] = readTask(in);
        dataset.tasks[i].id = i + 1;
    }

    // Skip lines until the optimal result is found
    std::string dummyLine = "";
    while (dummyLine.find("opt:") == std::string::npos) 
    {
        std::getline(in, dummyLine);
    }

    dataset.optimalResult = readOptimalResult(in);

    return dataset;
}

int calculatePenalty(const Task& task) 
{
    int delay = task.completionTime - task.executionTime;
    if (delay > 0) 
    {
        return delay * task.penaltyWeight;
    }
    return 0;
}

int calculateTotalPenalty(const Data& dataset) 
{
    int totalPenalty = 0;
    for (int i = 0; i < dataset.numberOfTasks; ++i) 
    {
        totalPenalty += calculatePenalty(dataset.tasks[i]);
    }
    return totalPenalty;
}

Result scheduleTasks(const Task* tasks, int numberOfTasks) 
{
    // Convert array to vector for easier manipulation
    std::vector<Task> taskVector(tasks, tasks + numberOfTasks);
    
    // Sort tasks based on a heuristic, smallest penalty weight or other criteria could be considered
    std::sort(taskVector.begin(), taskVector.end(), [](const Task& a, const Task& b) 
    {
        return a.completionTime < b.completionTime; // Earliest due date first
    });

    int n = taskVector.size();
    std::vector<int> dp(1 << n, INT_MAX); // DP array to store minimum penalty for each subset of tasks
    dp[0] = 0; // Base case: no tasks scheduled, no penalty
    std::vector<int> lastTask(1 << n, -1); // To track the last task in optimal sequence

    // Iterate over each subset of tasks
    for (int mask = 0; mask < (1 << n); ++mask) 
    {
        if (dp[mask] == INT_MAX) continue; // Skip infeasible states

        int currentTime = 0; // Current time is the sum of execution times of tasks in the current subset
        for (int i = 0; i < n; ++i) 
        {
            if (mask & (1 << i)) 
            {
                currentTime += taskVector[i].executionTime;
            }
        }

        // Consider adding each task not yet in the subset
        for (int i = 0; i < n; ++i) 
        {
            if (!(mask & (1 << i))) 
            {
                int nextMask = mask | (1 << i);
                int finishTime = currentTime + taskVector[i].executionTime;
                int tardiness = std::max(0, finishTime - taskVector[i].completionTime);
                int penalty = tardiness * taskVector[i].penaltyWeight;

                if (dp[nextMask] > dp[mask] + penalty) {
                    dp[nextMask] = dp[mask] + penalty;
                    lastTask[nextMask] = i;
                }
            }
        }
    }

    // Reconstruct the optimal task sequence using `lastTask`
    std::vector<int> sequence;
    int currentMask = (1 << n) - 1;
    while (currentMask) 
    {
        int taskIdx = lastTask[currentMask];
        sequence.push_back(taskVector[taskIdx].id);
        currentMask &= ~(1 << taskIdx); // Remove the last task added from currentMask
    }
    std::reverse(sequence.begin(), sequence.end()); // Reverse to get the order from start to finish

    // Convert task sequence to string
    std::stringstream ss;
    for (int id : sequence) 
    {
        ss << id << " ";
    }

    return {dp[(1 << n) - 1], ss.str().substr(0, ss.str().size() - 1)}; // Remove the last space
}

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    const std::string DATA_PATH = "data.txt";
    std::list<Data> datasets = *loadDataFile(DATA_PATH);
    
    for (auto& dataset : datasets) 
    {
        std::cout << "\nDataset " << dataset.id << ":\n";
        printTaskArray(dataset.tasks, dataset.numberOfTasks);
        int totalPenalty = calculateTotalPenalty(dataset);
        std::cout << "Total Penalty: " << totalPenalty << std::endl;
        std::cout << "Optimal ";
        printResult(dataset.optimalResult);
        std::cout << "Received ";
        Result result = scheduleTasks(dataset.tasks, dataset.numberOfTasks);
        printResult(result);
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    printExecutionTime(duration);
    return 0;
}
