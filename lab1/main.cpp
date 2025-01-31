#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <limits>
#include <vector>
#include <chrono>

struct Task
{
    int id;
    int preparationTime;
    int executionTime;
    int deliveryTime;
};

Task* loadTasks(const std::string& filePath, int& numberOfTasks) 
{
    std::ifstream dataFile(filePath);
    if (!dataFile) 
    {
        std::cerr << "Failed to open " << filePath << std::endl;
        numberOfTasks = 0; // Set number of tasks to 0 to indicate failure
        return nullptr;
    }
    
    dataFile >> numberOfTasks; // Read the number of records
    Task* tasks = new Task[numberOfTasks]; // Dynamically allocate array for tasks
    
    for (int i = 0; i < numberOfTasks; ++i) 
    {
        tasks[i].id = i+1;
        dataFile >> tasks[i].preparationTime >> tasks[i].executionTime >> tasks[i].deliveryTime;
    }
    
    dataFile.close();
    return tasks;
}

void printTask(const Task& task) 
{
    std::cout << "Task " << task.id << ": Prep Time = " << task.preparationTime
              << ", Exec Time = " << task.executionTime
              << ", Delivery Time = " << task.deliveryTime << std::endl;
}

void printTaskArray(const Task* tasks, int numberOfTasks) 
{
    for (int i = 0; i < numberOfTasks; ++i) 
    {
        printTask(tasks[i]);
    }
}

struct Data
{
    Task* tasks;
    int numberOfTasks;
};

Data* loadDataFiles(std::string dirPath, int dataFilesCount) 
{
    Data* data = new Data[dataFilesCount];
    for (int i = 0; i < dataFilesCount; ++i) 
    {
        std::string filePath = dirPath + "data" + std::to_string(i+1) + ".txt";
        data[i].tasks = loadTasks(filePath, data[i].numberOfTasks);
    }
    return data;
}

Task* sortRSchedule(Task* tasks, int numberOfTasks) 
{
    Task* scheduledTasks = new Task[numberOfTasks];
    for (int i = 0; i < numberOfTasks; ++i) 
    {
        scheduledTasks[i] = tasks[i];
    }

    std::sort(
        scheduledTasks, 
        scheduledTasks + numberOfTasks, 
        [](const Task& a, const Task& b) // criterion function: sum of execution time and delivery time
        {
            return a.preparationTime < b.preparationTime;
        }
    );
    return scheduledTasks;
}


Task* insertLongestPrepTime(Task* tasks, int numberOfTasks) { //only for data2
    Task* scheduledTasks = new Task[numberOfTasks];
    std::vector<Task> tasksVec(tasks, tasks + numberOfTasks);
    int currentTime = 0;
    int scheduledCount = 0;

    auto compareByPreparationTime = [](const Task& a, const Task& b) {
        return a.preparationTime < b.preparationTime; // Ascending order for preparation time
    };

    auto compareByExecutionTime = [](const Task& a, const Task& b) {
        return a.executionTime < b.executionTime; // Ascending order for execution time
    };

    // Sort tasks by preparation time
    std::sort(tasksVec.begin(), tasksVec.end(), compareByPreparationTime);

    // Copy tasks sorted by preparation time into scheduledTasks
    for (const Task& task : tasksVec) {
        scheduledTasks[scheduledCount++] = task;
    }

    // Find task with the longest preparation time
    auto itHighestPreparationTime = std::max_element(tasksVec.begin(), tasksVec.end(),
        [](const Task& a, const Task& b) { return a.preparationTime < b.preparationTime; });

    if (itHighestPreparationTime != tasksVec.end()) {
        Task taskWithHighestPreparationTime = *itHighestPreparationTime;

        // Find the sum of execution times closest to the highest preparation time
        int closestExecutionSum = 0;
        int minDiff = INT_MAX;
        int count = 0;

        for (const Task& task : tasksVec) {
            int diff = std::abs(closestExecutionSum - taskWithHighestPreparationTime.preparationTime);
            if (diff < minDiff) {
                minDiff = diff;
                closestExecutionSum = closestExecutionSum+task.executionTime;
                count++;
            }
        }

        // Find the index where to insert the task with the highest preparation time
        int insertionIndex = count;

        // Shift elements in scheduledTasks to make space for the task with highest preparation time
        for (int i = scheduledCount; i > insertionIndex; --i) {
            scheduledTasks[i] = scheduledTasks[i - 1];
        }

        // Insert the task with the highest preparation time
        scheduledTasks[insertionIndex] = taskWithHighestPreparationTime;
    }

    return scheduledTasks;
}



Task* schrageSchedule(Task* tasks, int numberOfTasks) {
    Task* scheduledTasks = new Task[numberOfTasks];
    std::vector<Task> tasksVec(tasks, tasks + numberOfTasks);
    std::vector<Task> readyQueue;
    int currentTime = 0;
    int scheduledCount = 0;

    auto compareByDeliveryTime = [](const Task& a, const Task& b) {
        return a.deliveryTime > b.deliveryTime; // Descending order for delivery time
    };

    auto compareByPreparationTime = [](const Task& a, const Task& b) {
        return a.preparationTime < b.preparationTime; // Ascending order for preparation time
    };

    // Initial sort by preparation time to find the first task(s) to start
    std::sort(tasksVec.begin(), tasksVec.end(), compareByPreparationTime);
    
    while (scheduledCount < numberOfTasks) {
        // Move tasks ready to be processed to the ready queue
        for (auto it = tasksVec.begin(); it != tasksVec.end();) {
            if (it->preparationTime <= currentTime) {
                readyQueue.push_back(*it);
                it = tasksVec.erase(it); // Remove from tasksVec and add to readyQueue
            } else {
                ++it;
            }
        }

        // If ready queue is empty, advance time to the next task's preparation time
        if (readyQueue.empty() && !tasksVec.empty()) {
            currentTime = tasksVec.front().preparationTime;
            continue;
        }

        // Sort ready queue by delivery time to select the task with the highest delivery time
        std::sort(readyQueue.begin(), readyQueue.end(), compareByDeliveryTime);
        Task selectedTask = readyQueue.front();
        readyQueue.erase(readyQueue.begin());

        // "Process" selected task
        scheduledTasks[scheduledCount++] = selectedTask;
        currentTime += selectedTask.executionTime;
    }

    return scheduledTasks;
}

std::string getScheduledTasksSequence(const Task* scheduledTasks, int numberOfTasks) 
{
    std::string solution = "";
    for (int i = 0; i < numberOfTasks; ++i) 
    {
        solution += std::to_string(scheduledTasks[i].id);
        if (i < numberOfTasks - 1) 
        {
            solution += " ";
        }
    }
    return solution;
}


void printExecutionTime(std::chrono::milliseconds duration)
{
    auto milliseconds = duration.count();
    auto seconds = milliseconds / 1000;
    milliseconds = milliseconds % 1000;

    // Output the duration in seconds and milliseconds
    std::cout << "\nExecution time: " << seconds << " seconds " << milliseconds << " milliseconds" << std::endl;
}

int calculateCmax(const Task* scheduledTasks, int numberOfTasks) {
    int currentTime = 0;
    int cmax = 0;

    for (int i = 0; i < numberOfTasks; ++i) {
        const Task& task = scheduledTasks[i];
        
        int startTime = std::max(currentTime, task.preparationTime);
        int finishTime = startTime + task.executionTime + task.deliveryTime;
        currentTime = startTime + task.executionTime;
        
        cmax = std::max(cmax, finishTime);
    }

    return cmax;
}

std::string getTotalCmax(int* cmaxData, int dataFilesCount) 
{
    int totalCmax = 0;
    for (int i = 0; i < dataFilesCount; ++i) 
    {
        totalCmax += cmaxData[i];
    }
    return std::to_string(totalCmax);
}

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    const int DATA_FILES_COUNT = 4;
    const std::string DATA_DIR_PATH = "data/";
    Data* data = loadDataFiles(DATA_DIR_PATH, DATA_FILES_COUNT);
    int* cmaxData = new int[DATA_FILES_COUNT];
    for (int i = 0; i<DATA_FILES_COUNT; i++)
    {
        std::cout << "\nData file " << i+1 << ":\n";
        printTaskArray(data[i].tasks, data[i].numberOfTasks);
        
        std::cout << "\nScheduled tasks for data file " << i+1 << ":\n";
        auto scheduledTasks = insertLongestPrepTime(data[i].tasks, data[i].numberOfTasks);
        std::cout << getScheduledTasksSequence(scheduledTasks, data[i].numberOfTasks) << std::endl;

        auto cmax = calculateCmax(scheduledTasks, data[i].numberOfTasks);
        cmaxData[i] = cmax;
        std::cout << "Cmax = " << cmax << std::endl;
    }
    std::cout << "\nTotal Cmax: " << getTotalCmax(cmaxData, DATA_FILES_COUNT) << std::endl;
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    printExecutionTime(duration);
    return 0;
}


//sprawko - opis problemu + oznaczenia??? żebyśmy stosowali jeden typ oznaczeń
//struktury danych - jak dane rozw jest zaimplementowane w kodzie xd?
//algorytmy jakie zastosowaliśmy 
//testy
//tabela z wynikami
