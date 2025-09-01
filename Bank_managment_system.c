#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME "bankdata.dat"
#define HISTORY "history.dat"

typedef struct {
    int id;
    char name[50];
    char password[20];
    double balance;
} Account;

typedef struct {
    int accId;
    char action[50];
    double amount;
    char date[30];
} Transaction;

void getDate(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

void saveAccount(Account acc) {
    FILE *fp = fopen(FILENAME, "ab");
    fwrite(&acc, sizeof(Account), 1, fp);
    fclose(fp);
}

void saveTransaction(int id, const char *action, double amt) {
    Transaction t;
    t.accId = id;
    strcpy(t.action, action);
    t.amount = amt;
    getDate(t.date);
    FILE *fp = fopen(HISTORY, "ab");
    fwrite(&t, sizeof(Transaction), 1, fp);
    fclose(fp);
}

Account *findAccount(int id) {
    FILE *fp = fopen(FILENAME, "rb");
    Account *acc = malloc(sizeof(Account));
    while (fread(acc, sizeof(Account), 1, fp)) {
        if (acc->id == id) {
            fclose(fp);
            return acc;
        }
    }
    fclose(fp);
    free(acc);
    return NULL;
}

void updateAccount(Account acc) {
    FILE *fp = fopen(FILENAME, "rb+");
    Account temp;
    while (fread(&temp, sizeof(Account), 1, fp)) {
        if (temp.id == acc.id) {
            fseek(fp, -sizeof(Account), SEEK_CUR);
            fwrite(&acc, sizeof(Account), 1, fp);
            fclose(fp);
            return;
        }
    }
    fclose(fp);
}

void createAccount() {
    Account acc;
    printf("\nEnter Account ID: ");
    scanf("%d", &acc.id);
    getchar();
    printf("Enter Name: ");
    fgets(acc.name, 50, stdin);
    acc.name[strcspn(acc.name, "\n")] = 0;
    printf("Set Password: ");
    scanf("%s", acc.password);
    acc.balance = 0.0;
    saveAccount(acc);
    printf("Account Created!\n");
}

void deposit() {
    int id;
    double amt;
    printf("Enter Account ID: ");
    scanf("%d", &id);
    Account *acc = findAccount(id);
    if (!acc) {
        printf("Account not found.\n");
        return;
    }
    printf("Enter Amount: ");
    scanf("%lf", &amt);
    acc->balance += amt;
    updateAccount(*acc);
    saveTransaction(id, "Deposit", amt);
    printf("Deposited Successfully. New Balance: %.2f\n", acc->balance);
    free(acc);
}

void withdraw() {
    int id;
    double amt;
    printf("Enter Account ID: ");
    scanf("%d", &id);
    Account *acc = findAccount(id);
    if (!acc) {
        printf("Account not found.\n");
        return;
    }
    printf("Enter Amount: ");
    scanf("%lf", &amt);
    if (amt > acc->balance) {
        printf("Insufficient Balance.\n");
    } else {
        acc->balance -= amt;
        updateAccount(*acc);
        saveTransaction(id, "Withdraw", amt);
        printf("Withdrawn Successfully. New Balance: %.2f\n", acc->balance);
    }
    free(acc);
}

void transfer() {
    int from, to;
    double amt;
    printf("From Account ID: ");
    scanf("%d", &from);
    printf("To Account ID: ");
    scanf("%d", &to);
    printf("Amount: ");
    scanf("%lf", &amt);
    Account *a1 = findAccount(from);
    Account *a2 = findAccount(to);
    if (!a1 || !a2) {
        printf("One of the accounts not found.\n");
    } else if (amt > a1->balance) {
        printf("Insufficient Balance.\n");
    } else {
        a1->balance -= amt;
        a2->balance += amt;
        updateAccount(*a1);
        updateAccount(*a2);
        saveTransaction(from, "Transfer Out", amt);
        saveTransaction(to, "Transfer In", amt);
        printf("Transfer Successful.\n");
    }
    if (a1) free(a1);
    if (a2) free(a2);
}

void balanceInquiry() {
    int id;
    printf("Enter Account ID: ");
    scanf("%d", &id);
    Account *acc = findAccount(id);
    if (!acc) {
        printf("Account not found.\n");
        return;
    }
    printf("Account: %d | Name: %s | Balance: %.2f\n", acc->id, acc->name, acc->balance);
    free(acc);
}

void transactionHistory() {
    int id;
    printf("Enter Account ID: ");
    scanf("%d", &id);
    FILE *fp = fopen(HISTORY, "rb");
    Transaction t;
    printf("History for Account %d:\n", id);
    while (fread(&t, sizeof(Transaction), 1, fp)) {
        if (t.accId == id) {
            printf("%s | %s | %.2f\n", t.date, t.action, t.amount);
        }
    }
    fclose(fp);
}

void deleteAccount() {
    int id;
    printf("Enter Account ID to delete: ");
    scanf("%d", &id);
    FILE *fp = fopen(FILENAME, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    Account acc;
    int found = 0;
    while (fread(&acc, sizeof(Account), 1, fp)) {
        if (acc.id == id) {
            found = 1;
            continue;
        }
        fwrite(&acc, sizeof(Account), 1, temp);
    }
    fclose(fp);
    fclose(temp);
    remove(FILENAME);
    rename("temp.dat", FILENAME);
    if (found)
        printf("Account Deleted.\n");
    else
        printf("Account not found.\n");
}

int login() {
    char user[20], pass[20];
    printf("\n--- Admin Login ---\n");
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);
    if (strcmp(user, "admin") == 0 && strcmp(pass, "1234") == 0) {
        return 1;
    }
    return 0;
}

int main() {
    if (!login()) {
        printf("Login Failed.\n");
        return 0;
    }
    int choice;
    while (1) {
        printf("\n--- Bank Management System ---\n");
        printf("1. Create Account\n2. Deposit\n3. Withdraw\n4. Transfer\n5. Balance Inquiry\n6. Transaction History\n7. Delete Account\n8. Exit\nChoice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: createAccount(); break;
            case 2: deposit(); break;
            case 3: withdraw(); break;
            case 4: transfer(); break;
            case 5: balanceInquiry(); break;
            case 6: transactionHistory(); break;
            case 7: deleteAccount(); break;
            case 8: exit(0);
            default: printf("Invalid Choice.\n");
        }
    }
    return 0;
}
