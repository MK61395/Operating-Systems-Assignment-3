#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

// Max stud
const int MAX_STUDENTS = 20;

struct Student {
    string name;
    int rollNumber;
    string mainCourse;
    int grade;
};

// Global variables for shared data
Student studentData[MAX_STUDENTS];
float averageGrade;
const int counselingThreshold = 40;  // threshold
Student studentsNeedCounseling[MAX_STUDENTS];
Student sortedStudents[MAX_STUDENTS];

// Mutexes for synchronization
pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t avgGradeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counselingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sortingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sortedStudentsMutex = PTHREAD_MUTEX_INITIALIZER;

// (Process A)
void* calculateAverageGrade(void* arg) {
    float totalGrade = 0;

    // Calculate the total grade
    for (int i = 0; i < MAX_STUDENTS; ++i) {
        pthread_mutex_lock(&dataMutex);
        totalGrade += studentData[i].grade;
        pthread_mutex_unlock(&dataMutex);
    }

    // Calculate the average grade
    pthread_mutex_lock(&avgGradeMutex);
    averageGrade = totalGrade / MAX_STUDENTS;
    pthread_mutex_unlock(&avgGradeMutex);

    pthread_exit(nullptr);
}

// (Process B)
void* findStudentsNeedCounseling(void* arg) {
    int counselingCount = 0;

    // Find students who need counseling
    for (int i = 0; i < MAX_STUDENTS; ++i) {
        pthread_mutex_lock(&dataMutex);
        if (studentData[i].grade < counselingThreshold) {
            pthread_mutex_lock(&counselingMutex);
            studentsNeedCounseling[counselingCount] = studentData[i];
            ++counselingCount;
            pthread_mutex_unlock(&counselingMutex);
        }
        pthread_mutex_unlock(&dataMutex);
    }

    pthread_exit(nullptr);
}

// (Process C)
void* sortStudents(void* arg) {
    // Copy data to sortedStudents array
    for (int i = 0; i < MAX_STUDENTS; ++i) {
        pthread_mutex_lock(&dataMutex);
        pthread_mutex_lock(&sortingMutex);
        pthread_mutex_lock(&sortedStudentsMutex);

        // Copy the student to sortedStudents array
        sortedStudents[i] = studentData[i];

        pthread_mutex_unlock(&sortedStudentsMutex);
        pthread_mutex_unlock(&sortingMutex);
        pthread_mutex_unlock(&dataMutex);
    }

    // Perform stable sorting on the sortedStudents array in descending order
    sort(sortedStudents, sortedStudents + MAX_STUDENTS, [](const Student& a, const Student& b) {
        return a.grade > b.grade;
    });

    // Store the sorted studs
    ofstream sortedFile("sorted_students.txt");
    if (!sortedFile) {
        cerr << "Error opening sorted_students.txt file." << endl;
        pthread_exit(nullptr);
    }

    for (int i = 0; i < MAX_STUDENTS; ++i) {
        pthread_mutex_lock(&sortedStudentsMutex);
        sortedFile << sortedStudents[i].name << " - " << sortedStudents[i].grade << endl;
        pthread_mutex_unlock(&sortedStudentsMutex);
    }

    sortedFile.close();

    pthread_exit(nullptr);
}

int main() {
    // Read student data from the file
    ifstream inputFile("student_data.txt");
    if (!inputFile) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    string line;
    int studentCount = 0;
    while (getline(inputFile, line) && studentCount < MAX_STUDENTS) {
        istringstream iss(line);
        Student& student = studentData[studentCount];
        iss >> student.name >> student.rollNumber >> student.mainCourse >> student.grade;
        ++studentCount;
    }

    // Create threads for each process
    pthread_t avgGradeThread, counselingThread, sortingThread;

    // Create thread for Process A (calculate average grade)
    if (pthread_create(&avgGradeThread, nullptr, calculateAverageGrade, nullptr) != 0) {
        cerr << "Error creating thread for Process A." << endl;
        return 1;
    }

    // Create thread for Process B (find students who need counseling)
    if (pthread_create(&counselingThread, nullptr, findStudentsNeedCounseling, nullptr) != 0) {
        cerr << "Error creating thread for Process B." << endl;
        return 1;
    }

    // Create thread for Process C (sort students based on grade)
    if (pthread_create(&sortingThread, nullptr, sortStudents, nullptr) != 0) {
        cerr << "Error creating thread for Process C." << endl;
        return 1;
    }

    // Wait for threads to finish
    pthread_join(avgGradeThread, nullptr);
    pthread_join(counselingThread, nullptr);
    pthread_join(sortingThread, nullptr);

    // Output or further processing
    cout << "Average Grade: " << averageGrade << endl;

    cout << "Students needing counseling:" << endl;
    for (int i = 0; i < MAX_STUDENTS; ++i) {
        cout << studentsNeedCounseling[i].name << " - " << studentsNeedCounseling[i].grade << endl;
    }

    cout << "Sorted Students have been stored in 'sorted_students.txt'." << endl;
    return 0;
}

