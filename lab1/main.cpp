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

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    const int DATA_FILES_COUNT = 4;
    const std::string DATA_DIR_PATH = "data/";
    Data* data = loadDataFiles(DATA_DIR_PATH, DATA_FILES_COUNT);
    for (int i = 0; i<DATA_FILES_COUNT; i++)
    {
        std::cout << "\nData file " << i+1 << ":\n";
        printTaskArray(data[i].tasks, data[i].numberOfTasks);
        
        std::cout << "\nScheduled tasks for data file " << i+1 << ":\n";
        auto scheduledTasks = schrageSchedule(data[i].tasks, data[i].numberOfTasks);
        std::cout << getScheduledTasksSequence(scheduledTasks, data[i].numberOfTasks) << std::endl;
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    printExecutionTime(duration);
    return 0;
}
