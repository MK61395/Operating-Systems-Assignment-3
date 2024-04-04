#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

const int S = 5000;
const int M = 3;

struct Task {
    int id;
    int value;
    int arrivalTime;
    int units;
    int unitIds[5];
    int totalUnits;
};

struct ThreadData {
    Task task;
    int unitId;
};

void* unit(void* arg);

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queueNotFull = PTHREAD_COND_INITIALIZER;

const int MAX_QUEUE_SIZE = 100;
Task taskQueue[MAX_QUEUE_SIZE];
int front = 0, rear = 0;

void enqueue(const Task& task) {
    taskQueue[rear] = task;
    rear = (rear + 1) % MAX_QUEUE_SIZE;
}

Task dequeue() {
    Task task = taskQueue[front];
    front = (front + 1) % MAX_QUEUE_SIZE;
    return task;
}

void* reception(void* arg) {
    // Read input file and enqueue tasks
    ifstream inputFile("tasks.txt");
    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        Task task;
        iss >> task.id >> task.value >> task.arrivalTime >> task.units;
        task.totalUnits = 0; // Initialize totalUnits to 0

        // Enqueue the task
        pthread_mutex_lock(&queueMutex);
        enqueue(task);
        pthread_cond_signal(&queueNotEmpty);
        pthread_mutex_unlock(&queueMutex);
    }

    pthread_exit(NULL);
}

void* dispatcher(void* arg) {
    while (true) {
        pthread_mutex_lock(&queueMutex);
        while (front == rear) {
            // Queue is empty, wait for tasks
            pthread_cond_wait(&queueNotEmpty, &queueMutex);
        }

        Task task = dequeue();
        pthread_mutex_unlock(&queueMutex);

        // Process the task
        for (int i = 0; i < task.units; ++i) {
            ThreadData* threadData = new ThreadData{task, i};
            pthread_t thread;
            pthread_create(&thread, NULL, unit, (void*)threadData);
            // Wait for the thread to finish
            pthread_join(thread, NULL);
        }
    }
}

void* unit(void* arg) {
    ThreadData* threadData = (ThreadData*)arg;
    Task task = threadData->task;
    int unitId = threadData->unitId;

    // Perform the specific operation based on unitId
    switch (unitId) {
        case 0:
            task.value = (task.value + 5) % S;
            break;
        case 1:
            task.value = (task.value * 7) % S;
            break;
        case 2:
            // Assuming task.value is non-negative for simplicity
            task.value = (task.value * task.value * task.value * task.value * task.value) % M;
            break;
        case 3:
            task.value -= 100;
            break;
        case 4:
            task.value = task.value * task.value;
            break;
    }

    // Update task structure with unitId and TaskValue
    task.unitIds[unitId] = unitId;
    task.totalUnits++;

    // Notify the Print room
    pthread_mutex_lock(&queueMutex);
    enqueue(task);
    pthread_cond_signal(&queueNotFull);
    pthread_mutex_unlock(&queueMutex);

    delete threadData;
    pthread_exit(NULL);
}

void* printRoom(void* arg) {
    while (true) {
        pthread_mutex_lock(&queueMutex);
        while (front == rear) {
            // Queue is empty, wait for tasks
            pthread_cond_wait(&queueNotFull, &queueMutex);
        }

        // Print the TaskValues in FCFS order
        Task task = dequeue();
        cout << "TaskID: " << task.id << " CompletionTime: " << task.arrivalTime
                  << " UnitIds: ";
        for (int i = 0; i < task.totalUnits; ++i) {
            cout << task.unitIds[i] << " ";
        }
        cout << " TaskValue1: " << task.value << endl;

        pthread_mutex_unlock(&queueMutex);
    }
}

int main() {
    pthread_t receptionThread, dispatcherThread, printRoomThread;

    // Create threads
    pthread_create(&receptionThread, NULL, reception, NULL);
    pthread_create(&dispatcherThread, NULL, dispatcher, NULL);
    pthread_create(&printRoomThread, NULL, printRoom, NULL);

    // Join threads
    pthread_join(receptionThread, NULL);
    pthread_join(dispatcherThread, NULL);
    pthread_join(printRoomThread, NULL);

    return 0;
}
