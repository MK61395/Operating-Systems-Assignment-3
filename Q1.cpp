#include <iostream>
#include <queue>
#include <string>

using namespace std;

#define MAX_TEAM_SIZE 100
#define TIME_QUANTUM 3

class Task {
public:
    string name;
    string type;
    int priority;
    string requiredSkillSet;
    int totalCompletionTime; 
    int remainingExecutionTime; 

    Task(const string& n, const string& t, int p, const string& skillSet, int time)
        : name(n), type(t), priority(p), requiredSkillSet(skillSet), totalCompletionTime(time), remainingExecutionTime(time) {}
};

class TeamMember {
public:
    string name;
    string skillSet;
};

class TaskManagementSystem {
private:
    queue<Task> taskQueue;
    TeamMember teamMembers[MAX_TEAM_SIZE];
    int teamSize;
    int currentMemberIndex;

public:
    TaskManagementSystem() : teamSize(0), currentMemberIndex(0) {}

    void addTask(const Task& task) {
        taskQueue.push(task);
    }

    void addTeamMember(const TeamMember& member) {
        if (teamSize >= MAX_TEAM_SIZE) {
            cout << "Team is full. Cannot add more members." << endl;
            return;
        }
        teamMembers[teamSize++] = member;
    }

    void assignTasks() {
        while (!taskQueue.empty()) {
            Task& task = taskQueue.front();
            TeamMember& member = teamMembers[currentMemberIndex];

            if (member.skillSet == task.requiredSkillSet) {
                cout << "Assigning task " << task.name << " to " << member.name <<endl;

                // Determine the time to execute (considering time quantum or remaining execution time)
                int executionTime = min(TIME_QUANTUM, task.remainingExecutionTime);

                // Update task and member
                task.remainingExecutionTime -= executionTime;

                // Move to the next task or team member
                if (task.remainingExecutionTime == 0) {
                    taskQueue.pop(); // Task completed
                } else {
                    currentMemberIndex = (currentMemberIndex + 1) % teamSize; // Move to the next team member
                }
            } else {
                // Move to the next team member
                currentMemberIndex = (currentMemberIndex + 1) % teamSize;
            }
        }
    }
};

int main() {
    TaskManagementSystem system;

    // Add team members
    system.addTeamMember({"Kashif", "Software Engineer"});
    system.addTeamMember({"Bilal", "Software Tester"});
    system.addTeamMember({"Salal", "Project Manager"});
    system.addTeamMember({"Mini", "Software Engineer"});
    system.addTeamMember({"Usman", "Software Tester"});
    system.addTeamMember({"Huzi", "Project Manager"});

    // Add tasks with total completion time
    system.addTask({"Task 1", "Software Development", 1, "Software Engineer", 10});
    system.addTask({"Task 2", "Software Testing", 2, "Software Tester", 6});
    system.addTask({"Task 3", "Project Management", 3, "Project Manager", 7});

    // Assign tasks
    system.assignTasks();

    return 0;
}
