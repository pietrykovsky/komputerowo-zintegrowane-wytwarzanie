#include <iostream>
#include <fstream>

struct Task
{
    int id;
    int preparationTime;
    int executionTime;
    int deliveryTime;
};

Task* loadTasks(const std::string& filePath, int& numberOfTasks) {
    std::ifstream dataFile(filePath);
    if (!dataFile) {
        std::cerr << "Failed to open " << filePath << std::endl;
        numberOfTasks = 0; // Set number of tasks to 0 to indicate failure
        return nullptr;
    }
    
    dataFile >> numberOfTasks; // Read the number of records
    Task* tasks = new Task[numberOfTasks]; // Dynamically allocate array for tasks
    
    for (int i = 0; i < numberOfTasks; ++i) {
        tasks[i].id = i+1;
        dataFile >> tasks[i].preparationTime >> tasks[i].executionTime >> tasks[i].deliveryTime;
    }
    
    dataFile.close();
    return tasks;
}

void printTask(const Task& task) {
    std::cout << "Task " << task.id << ": Prep Time = " << task.preparationTime
              << ", Exec Time = " << task.executionTime
              << ", Delivery Time = " << task.deliveryTime << std::endl;
}

void printTaskArray(const Task* tasks, int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; ++i) {
        printTask(tasks[i]);
    }
}

struct Data
{
    Task* tasks;
    int numberOfTasks;
};

Data* loadDataFiles(int dataFilesCount) {
    Data* data = new Data[dataFilesCount];
    for (int i = 0; i < dataFilesCount; ++i) {
        std::string filePath = "data" + std::to_string(i+1) + ".txt";
        data[i].tasks = loadTasks(filePath, data[i].numberOfTasks);
    }
    return data;
}

int main() {
    int DATA_FILES_COUNT = 4;
    Data* data = loadDataFiles(DATA_FILES_COUNT);
    for (int i = 0; i<DATA_FILES_COUNT; i++)
    {
        std::cout << "Data file " << i+1 << ":\n";
        printTaskArray(data[i].tasks, data[i].numberOfTasks);
    }
    
    return 0;
}
