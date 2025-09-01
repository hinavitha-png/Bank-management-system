#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 100
#define MAX_PASS 50
#define MAX_ADDR 200
#define MAX_PHONE 20
#define MAX_EMAIL 100

// --------------------------- STRUCTURES ---------------------------

typedef struct {
    int day, month, year;
    int hour, min, sec;
} DateTime;

typedef struct {
    int accNumber;
    char name[MAX_NAME];
    int age;
    char gender;
    char address[MAX_ADDR];
    char phone[MAX_PHONE];
    char email[MAX_EMAIL];
    char password[MAX_PASS];
    double balance;
    double loan;
} Account;

typedef struct {
    int accNumber;
    char type[20];       // deposit, withdraw, transfer
    double amount;
    DateTime datetime;
    int targetAcc;       // for transfers
} Transaction;

// --------------------------- GLOBALS ---------------------------

FILE *fAccounts, *fTransactions;
int lastAccNumber = 1000;

// --------------------------- UTILITIES ---------------------------

DateTime currentDateTime() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DateTime dt;
    dt.day = tm.tm_mday;
    dt.month = tm.tm_mon + 1;
    dt.year = tm.tm_year + 1900;
    dt.hour = tm.tm_hour;
    dt.min = tm.tm_min;
    dt.sec = tm.tm_sec;
    return dt;
}

void printDateTime(DateTime dt) {
    printf("%02d/%02d/%04d %02d:%02d:%02d",
           dt.day, dt.month, dt.year, dt.hour, dt.min, dt.sec);
}

// --------------------------- ACCOUNT FUNCTIONS ---------------------------

void saveAccount(Account acc) {
    fAccounts = fopen("accounts.dat", "ab");
    fwrite(&acc, sizeof(Account), 1, fAccounts);
    fclose(fAccounts);
}

Account *findAccount(int accNumber) {
    fAccounts = fopen("accounts.dat", "rb");
    if (!fAccounts) return NULL;
    Account *acc = malloc(sizeof(Account));
    while (fread(acc, sizeof(Account), 1, fAccounts)) {
        if (acc->accNumber == accNumber) {
            fclose(fAccounts);
            return acc;
        }
    }
    fclose(fAccounts);
    free(acc);
    return NULL;
}

void updateAccount(Account updated) {
    fAccounts = fopen("accounts.dat", "rb+");
    Account acc;
    while (fread(&acc, sizeof(Account), 1, fAccounts)) {
        if (acc.accNumber == updated.accNumber) {
            fseek(fAccounts, -sizeof(Account), SEEK_CUR);
            fwrite(&updated, sizeof(Account), 1, fAccounts);
            break;
        }
    }
    fclose(fAccounts);
}

void createAccount() {
    Account acc;
    acc.accNumber = ++lastAccNumber;

    printf("Enter full name: ");
    getchar();
    fgets(acc.name, MAX_NAME, stdin);
    acc.name[strcspn(acc.name, "\n")] = '\0';

    printf("Enter age: ");
    scanf("%d", &acc.age);

    printf("Enter gender (M/F): ");
    scanf(" %c", &acc.gender);

    printf("Enter address: ");
    getchar();
    fgets(acc.address, MAX_ADDR, stdin);
    acc.address[strcspn(acc.address, "\n")] = '\0';

    printf("Enter phone: ");
    scanf("%s", acc.phone);

    printf("Enter email: ");
    scanf("%s", acc.email);

    printf("Set account password: ");
    scanf("%s", acc.password);

    acc.balance = 0;
    acc.loan = 0;

    saveAccount(acc);
    printf("\nAccount created successfully! Your account number is %d\n", acc.accNumber);
}

int login(int accNumber, char *password) {
    Account *acc = findAccount(accNumber);
    if (acc && strcmp(acc->password, password) == 0) {
        free(acc);
        return 1;
    }
    if (acc) free(acc);
    return 0;
}

// --------------------------- TRANSACTION FUNCTIONS ---------------------------

void saveTransaction(Transaction t) {
    fTransactions = fopen("transactions.dat", "ab");
    fwrite(&t, sizeof(Transaction), 1, fTransactions);
    fclose(fTransactions);
}

void addTransaction(int accNumber, char *type, double amount, int targetAcc) {
    Transaction t;
    t.accNumber = accNumber;
    strcpy(t.type, type);
    t.amount = amount;
    t.datetime = currentDateTime();
    t.targetAcc = targetAcc;
    saveTransaction(t);
}

void viewTransactions(int accNumber) {
    fTransactions = fopen("transactions.dat", "rb");
    if (!fTransactions) {
        printf("No transactions found.\n");
        return;
    }
    Transaction t;
    printf("\n--- Transaction History ---\n");
    while (fread(&t, sizeof(Transaction), 1, fTransactions)) {
        if (t.accNumber == accNumber) {
            printf("[%s] ", t.type);
            printf("Amount: %.2f ", t.amount);
            if (strcmp(t.type, "transfer") == 0)
                printf("-> Acc %d ", t.targetAcc);
            printf(" on ");
            printDateTime(t.datetime);
            printf("\n");
        }
    }
    fclose(fTransactions);
}

// --------------------------- BANKING OPERATIONS ---------------------------

void deposit(int accNumber, double amount) {
    Account *acc = findAccount(accNumber);
    if (!acc) return;
    acc->balance += amount;
    updateAccount(*acc);
    addTransaction(accNumber, "deposit", amount, -1);
    printf("Deposit successful. New Balance: %.2f\n", acc->balance);
    free(acc);
}

void withdraw(int accNumber, double amount) {
    Account *acc = findAccount(accNumber);
    if (!acc) return;
    if (acc->balance < amount) {
        printf("Insufficient balance.\n");
        free(acc);
        return;
    }
    acc->balance -= amount;
    updateAccount(*acc);
    addTransaction(accNumber, "withdraw", amount, -1);
    printf("Withdrawal successful. New Balance: %.2f\n", acc->balance);
    free(acc);
}

void transfer(int fromAcc, int toAcc, double amount) {
    Account *from = findAccount(fromAcc);
    Account *to = findAccount(toAcc);
    if (!from || !to) {
        printf("Invalid account(s).\n");
        if (from) free(from);
        if (to) free(to);
        return;
    }
    if (from->balance < amount) {
        printf("Insufficient balance.\n");
        free(from); free(to);
        return;
    }
    from->balance -= amount;
    to->balance += amount;
    updateAccount(*from);
    updateAccount(*to);
    addTransaction(fromAcc, "transfer", amount, toAcc);
    addTransaction(toAcc, "deposit", amount, fromAcc);
    printf("Transfer successful. New Balance: %.2f\n", from->balance);
    free(from); free(to);
}

void checkBalance(int accNumber) {
    Account *acc = findAccount(accNumber);
    if (!acc) return;
    printf("Account Balance: %.2f\n", acc->balance);
    free(acc);
}

void applyLoan(int accNumber, double amount) {
    Account *acc = findAccount(accNumber);
    if (!acc) return;
    acc->loan += amount;
    acc->balance += amount;
    updateAccount(*acc);
    addTransaction(accNumber, "loan", amount, -1);
    printf("Loan of %.2f approved. Balance updated: %.2f\n", amount, acc->balance);
    free(acc);
}

void repayLoan(int accNumber, double amount) {
    Account *acc = findAccount(accNumber);
    if (!acc) return;
    if (acc->balance < amount) {
        printf("Not enough balance to repay loan.\n");
        free(acc);
        return;
    }
    if (amount > acc->loan) amount = acc->loan;
    acc->loan -= amount;
    acc->balance -= amount;
    updateAccount(*acc);
    addTransaction(accNumber, "loan_repay", amount, -1);
    printf("Loan repayment successful. Remaining loan: %.2f\n", acc->loan);
    free(acc);
}

// --------------------------- USER DASHBOARD ---------------------------

void userMenu(int accNumber) {
    int choice;
    double amt;
    int target;
    while (1) {
        printf("\n--- User Menu (Acc %d) ---\n", accNumber);
        printf("1. Deposit\n2. Withdraw\n3. Transfer\n4. Check Balance\n5. View Transactions\n6. Apply Loan\n7. Repay Loan\n8. Logout\nChoice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: printf("Enter amount: "); scanf("%lf", &amt); deposit(accNumber, amt); break;
            case 2: printf("Enter amount: "); scanf("%lf", &amt); withdraw(accNumber, amt); break;
            case 3: printf("Enter target acc no: "); scanf("%d", &target); printf("Amount: "); scanf("%lf", &amt); transfer(accNumber, target, amt); break;
            case 4: checkBalance(accNumber); break;
            case 5: viewTransactions(accNumber); break;
            case 6: printf("Loan amount: "); scanf("%lf", &amt); applyLoan(accNumber, amt); break;
            case 7: printf("Repay amount: "); scanf("%lf", &amt); repayLoan(accNumber, amt); break;
            case 8: return;
            default: printf("Invalid choice.\n");
        }
    }
}

// --------------------------- MAIN ---------------------------

int main() {
    int choice, accNumber;
    char password[MAX_PASS];

    while (1) {
        printf("\n--- BANK MANAGEMENT SYSTEM ---\n");
        printf("1. Create Account\n2. Login\n3. Exit\nChoice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount(); break;
            case 2:
                printf("Enter account number: ");
                scanf("%d", &accNumber);
                printf("Enter password: ");
                scanf("%s", password);
                if (login(accNumber, password)) {
                    printf("Login successful.\n");
                    userMenu(accNumber);
                } else {
                    printf("Invalid login.\n");
                }
                break;
            case 3: exit(0);
            default: printf("Invalid choice.\n");
        }
    }
}
