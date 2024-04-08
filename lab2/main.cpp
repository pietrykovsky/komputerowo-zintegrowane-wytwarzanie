#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <chrono>
#include <sstream>

/*
1 maszyna
n zadan
czas produkcji | waga kary | czas wymaganej daty

to do:
- napisanie funkcji obliczajacej kare:
kara = opoznienie * waga kary
opoznienie - czas zakonczenia - czas wymaganej daty
- napisanie funkcji obliczajacej calkowita kare w datasecie
*/

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

void printOptimalResult(const Result& result) 
{
    std::cout << "\nOptimal Result:\nTime = " << result.time << ", Task Sequence = " << result.taskSequence << std::endl;
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

Result readOptimalResult(std::istream& in) {
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


int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    const std::string DATA_PATH = "data.txt";
    std::list<Data> datasets = *loadDataFile(DATA_PATH);
    
    for (auto dataset = datasets.begin(); dataset != datasets.end(); ++dataset) 
    {
        std::cout << "\nDataset " << dataset->id << ":\n";
        printTaskArray(dataset->tasks, dataset->numberOfTasks);
        printOptimalResult(dataset->optimalResult);
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    printExecutionTime(duration);
    return 0;
}
