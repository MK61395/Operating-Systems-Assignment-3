#include <iostream>
#include <fstream>
#include <pthread.h>
#include <mutex>
using namespace std;

const int NUM_ACCOUNTS = 2;
double accounts[NUM_ACCOUNTS] = {1000.0, 2000.0}; // Initial account balances
std::mutex accountMutex[NUM_ACCOUNTS]; // One mutex per account

void deductFromAccount(int accountIndex, double amount) {
    lock_guard<mutex> lock(accountMutex[accountIndex]);
    accounts[accountIndex] -= amount;
}

void addToAccount(int accountIndex, double amount) {
    lock_guard<mutex> lock(accountMutex[accountIndex]);
    accounts[accountIndex] += amount;
}

void calculateTransactionFees(double totalTransaction) {
    double transactionFee = totalTransaction * 0.02; // 2% of the total transaction

    // Deduct transaction fee from account 0
    deductFromAccount(0, transactionFee);
}

void updateTransactionHistory() {
    ofstream historyFile("transaction_history.txt", std::ios::app);
    if (historyFile.is_open()) {
        historyFile << "Transaction Details:\n";
        for (int i = 0; i < NUM_ACCOUNTS; ++i) {
            historyFile << "Account " << i << " Balance: " << accounts[i] << "\n";
        }
        historyFile << "\n";
        historyFile.close();
    } else {
        cerr << "Unable to open transaction history file.\n";
    }
}

void sendEmailNotifications(int senderIndex, int receiverIndex, double amount) {
    // Implementation of sending email notifications
    lock(accountMutex[senderIndex], accountMutex[receiverIndex]);
    lock_guard<mutex> lockSender(accountMutex[senderIndex], adopt_lock);
    lock_guard<mutex> lockReceiver(accountMutex[receiverIndex], adopt_lock);

    // Print email notification details
    cout << "Email sent: Transaction Details\n";
    cout << "Sender Balance: " << accounts[senderIndex] << "\n";
    cout << "Receiver Balance: " << accounts[receiverIndex] << "\n";
    cout << "Amount: " << amount << "\n";
}

void* deductThread(void* arg) {
    double* data = reinterpret_cast<double*>(arg);
    deductFromAccount(0, data[0]); // Deduct from account 0
    return nullptr;
}

void* addToThread(void* arg) {
    double* data = reinterpret_cast<double*>(arg);
    addToAccount(1, data[0]); // Add to account 1
    return nullptr;
}

void* calculateFeesThread(void* arg) {
    double* data = reinterpret_cast<double*>(arg);
    calculateTransactionFees(data[0]);
    return nullptr;
}

void* updateHistoryThread(void* /*arg*/) {
    updateTransactionHistory();
    return nullptr;
}

void* sendEmailThread(void* arg) {
    double* data = reinterpret_cast<double*>(arg);
    sendEmailNotifications(0, 1, data[0]);
    return nullptr;
}

int main() {
    // Take user input for transaction amount, initial sender balance, and initial receiver balance
    double transactionAmount, senderInitialBalance, receiverInitialBalance;
    cout << "Enter transaction amount: ";
    cin >> transactionAmount;
    cout << "Enter initial sender balance: ";
    cin >> senderInitialBalance;
    cout << "Enter initial receiver balance: ";
    cin >> receiverInitialBalance;

    // Set initial account balances based on user input
    accounts[0] = senderInitialBalance;
    accounts[1] = receiverInitialBalance;

    pthread_t thread1, thread2, thread3, thread4, thread5;

    double data[] = {transactionAmount};

    // Create threads for each transaction type
    pthread_create(&thread1, nullptr, deductThread, data);        // Deduct from account 0
    pthread_create(&thread2, nullptr, addToThread, data);         // Add to account 1
    pthread_create(&thread3, nullptr, calculateFeesThread, data); // Calculate fees
    pthread_create(&thread4, nullptr, updateHistoryThread, nullptr);
    pthread_create(&thread5, nullptr, sendEmailThread, data);

    // Join threads to wait for them to finish
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);
    pthread_join(thread3, nullptr);
    pthread_join(thread4, nullptr);
    pthread_join(thread5, nullptr);

    // Print final account balances
    cout << "Final Account Balances:\n";
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        cout << "Account " << i << ": " << accounts[i] << "\n";
    }

    return 0;
}

