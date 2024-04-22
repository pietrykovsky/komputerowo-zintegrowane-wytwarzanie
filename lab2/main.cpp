#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <chrono>
#include <sstream>
#include <vector>

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

int calculatePenalty(const Task& task) 
{
    int delay = task.completionTime - task.executionTime;
    if (delay > 0) {
        return delay * task.penaltyWeight;
    }
    return 0;
}

int calculateTotalPenalty(const Data& dataset) 
{
    int totalPenalty = 0;
    for (int i = 0; i < dataset.numberOfTasks; ++i) {
        totalPenalty += calculatePenalty(dataset.tasks[i]);
    }
    return totalPenalty;
}

struct TaskInfo {
    int prevIndex;
    int minCost;
};

Result dynamicProgramming(const std::list<Task>& tasks) {
    int n = tasks.size();
    std::vector<Task> taskVec(tasks.begin(), tasks.end());
    
    std::vector<TaskInfo> v(n + 1); //minimalne koszty oraz indeksy poprzednich zadań
    std::vector<int> F(n + 1, 0); // minimalne koszty każdego zadania
    std::vector<int> pi(n + 1); //optymalne rozwiązanie
    
    int index = 1;
    
    for (const auto& task : tasks) {
    int minIndex = -1;
    int minCost = std::numeric_limits<int>::max();
    
    for (const auto& prevTask : tasks) {
        int newCost = F[prevTask.id] + prevTask.penaltyWeight * (task.completionTime - task.executionTime);
        if (newCost < minCost || (newCost == minCost && prevTask.id < minIndex)) {
            minCost = newCost;
            minIndex = prevTask.id;
        }
    }
    
    v[task.id].prevIndex = minIndex;
    v[task.id].minCost = minCost;
    F[task.id] = minCost;
}
    // for (const auto& task : taskVec) { //dla każdego zadania z taska
    //     int minIndex = -1;
    //     int minCost = std::numeric_limits<int>::max(); // maxint
        
    //     for (int j = 0; j < index; ++j) { // bierzemy 1 zadanie to leci raz, koszt zadania się oblicza
    //         int newCost = F[j] + task.penaltyWeight * (task.completionTime - task.executionTime);
    //         if (newCost < minCost) { // jeżeli nowy koszt zadania jest mniejszy niż poprzedni minimalny koszt to zamieniamy 
    //             minCost = newCost;
    //             minIndex = j; //ustawiamy też minimalny index na ten który ma minimalny koszt
    //         }
    //     }
        
    //     v[index].prevIndex = minIndex; // przypisujemy to jako poprzedni index ( tylko nie wiem czy minIndex jest tu dobry)
    //     v[index].minCost = minCost; //minimalny koszt przypisujemy 
    //     F[index] = F[minIndex] + task.penaltyWeight * (task.completionTime - task.executionTime); //do tablicy którą będziemy wykorzystywać przypisujemy sam koszt 
    //     ++index; // iterujemy 
    // }
    
    int i = n;
    while (i > 0) {
        pi[i] = v[i].prevIndex;
        std::cout<<v[i].prevIndex <<"_";
        std::cout<< i <<std::endl;
        --i;
    }
    
    std::string taskSequence;
    for (int j = 1; j <= n; ++j) {
        taskSequence += std::to_string(pi[j]) + " ";
    }
    
    return {F[n], taskSequence};
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    const std::string DATA_PATH = "data.txt";
    std::list<Data> datasets = *loadDataFile(DATA_PATH);
    
    for (auto& dataset : datasets) 
    {
        std::cout << "\nDataset " << dataset.id << ":\n";
        Result result = dynamicProgramming(std::list<Task>(dataset.tasks, dataset.tasks + dataset.numberOfTasks));
        std::cout << "Optimal Result:\nTime = " << result.time << ", Task Sequence = " << result.taskSequence << std::endl;
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    printExecutionTime(duration);
    return 0;
}


