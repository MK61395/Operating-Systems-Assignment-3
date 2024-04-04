#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <unistd.h>

const int NUM_TABLES = 3;
const int SEATS_PER_TABLE = 5;
const int TOTAL_PHILOSOPHERS = NUM_TABLES * SEATS_PER_TABLE;
int FOOD_QUANTITY = 10; // Adjust this value to control the simulation duration

std::mutex forkMutex[NUM_TABLES * SEATS_PER_TABLE];
sem_t tableSem[NUM_TABLES];
sem_t foodBowlSem;

void philosopher(int philosopherId) {
    int table = philosopherId / SEATS_PER_TABLE;
    int seat = philosopherId % SEATS_PER_TABLE;

    while (true) {
        // Thinking
        std::cout << "Philosopher at Table " << table + 1 << ", Seat " << seat + 1 << " is thinking." << std::endl;
        usleep(rand() % 1000000); // Simulate thinking by sleeping for a random time

        // Pick up left fork
        std::unique_lock<std::mutex> leftForkLock(forkMutex[philosopherId]);
        std::cout << "Philosopher at Table " << table + 1 << ", Seat " << seat + 1 << " picked up left fork." << std::endl;

        // Pick up right fork
        int rightSeat = (seat + 1) % SEATS_PER_TABLE;
        sem_wait(&tableSem[table]); // Wait for the table semaphore
        std::unique_lock<std::mutex> rightForkLock(forkMutex[table * SEATS_PER_TABLE + rightSeat]);
        std::cout << "Philosopher at Table " << table + 1 << ", Seat " << seat + 1 << " picked up right fork." << std::endl;

        // Check food availability
        sem_wait(&foodBowlSem);
        if (FOOD_QUANTITY <= 0) {
            sem_post(&foodBowlSem); // Release the bowl semaphore
            break; // Stop the philosopher when there is no more food
        }

        // Eating
        std::cout << "Philosopher at Table " << table + 1 << ", Seat " << seat + 1 << " is eating." << std::endl;
        usleep(rand() % 1000000); // Simulate eating by sleeping for a random time
        FOOD_QUANTITY--;

        // Release forks and food bowl
        leftForkLock.unlock();
        rightForkLock.unlock();
        sem_post(&tableSem[table]); // Release the table semaphore
        sem_post(&foodBowlSem); // Release the bowl semaphore

        std::cout << "Philosopher at Table " << table + 1 << ", Seat " << seat + 1 << " released forks." << std::endl;
    }
}

int main() {
    // Initialize semaphores
    for (int i = 0; i < NUM_TABLES; ++i) {
        sem_init(&tableSem[i], 0, SEATS_PER_TABLE - 1); // Each table has one less fork than seats
    }
    sem_init(&foodBowlSem, 0, 1); // Initialize food bowl semaphore with 1 (available)

    // Create philosopher threads
    std::thread philosopherThreads[TOTAL_PHILOSOPHERS];
    for (int i = 0; i < TOTAL_PHILOSOPHERS; ++i) {
        philosopherThreads[i] = std::thread(philosopher, i);
    }

    // Join philosopher threads
    for (int i = 0; i < TOTAL_PHILOSOPHERS; ++i) {
        philosopherThreads[i].join();
    }

    // Destroy semaphores
    for (int i = 0; i < NUM_TABLES; ++i) {
        sem_destroy(&tableSem[i]);
    }
    sem_destroy(&foodBowlSem);

    return 0;
}

