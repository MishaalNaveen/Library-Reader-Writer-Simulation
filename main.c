#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#define MAX_LEN 256
#define BOOK_FILE "books.txt"

int readCount = 0;
int waitingWriters = 0;
int running = 1;

pthread_mutex_t readMutex;
pthread_mutex_t writeMutex;
sem_t resource;
sem_t readerTry;

int activeReaders = 0; 

// Custom case-insensitive substring search
char* strcasestr_custom(const char* alpha1, const char* alpha2) {
    if (!alpha2) return (char*)alpha1;

    for (; *alpha1; ++alpha1) {
        const char *h = alpha1;
        const char *n = alpha2;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (*n == '\0') 
		return (char*)alpha1;
    }
    return NULL;
}

// Time formatting
void getTimeString(char* buffer) {
    time_t now = time(0);
    struct tm *lt = localtime(&now);
    sprintf(buffer, "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
}

// Book selection utility
int listBooksAndSelect(char* selectedLine) {
    FILE* f = fopen(BOOK_FILE, "r");
    if (!f) {
        printf("\n[!] No books available.\n");
        return 0;
    }

    char books[100][MAX_LEN];
    int count = 0;
    char line[MAX_LEN];

    printf("\n=====================================\n");
    printf("            Book List\n");
    printf("=====================================\n");
    while (fgets(line, sizeof(line), f)) {
        printf("%2d. %s", count + 1, line);
        strcpy(books[count], line);
        count++;
    }
    fclose(f);

    if (count == 0) return 0;

    int choice;
    do {
        printf("-------------------------------------\n");
        printf("Select a book to read (1-%d): ", count);
        scanf("%d", &choice);
    } while (choice < 1 || choice > count);

    strcpy(selectedLine, books[choice - 1]);
    return 1;
}

// Search books
void searchBook() {
    char keyword[MAX_LEN];
    printf("\n=====================================\n");
    printf("         Search Book by Title\n");
    printf("=====================================\n");
    printf("Enter keyword: ");
    getchar(); 
    fgets(keyword, MAX_LEN, stdin);
    keyword[strcspn(keyword, "\n")] = 0;

    FILE* f = fopen(BOOK_FILE, "r");
    if (!f) {
        printf("[!] Book database not found.\n");
        return;
    }

    char line[MAX_LEN];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char lineCopy[MAX_LEN];
        strcpy(lineCopy, line);
        char* title = strtok(lineCopy, "|");

        if (title && strcasestr_custom(title, keyword)) {
            char* author = strtok(NULL, "|");
            char* year = strtok(NULL, "|");
            char* genre = strtok(NULL, "|\n");

            printf("\n-------------------------------------\n");
            printf("Title : %s\n", title);
            printf("Author: %s\n", author);
            printf("Year  : %s\n", year);
            printf("Genre : %s\n", genre);
            found = 1;
        }
    }
    fclose(f);

    if (!found) {
        printf("\n[!] No book found with title containing \"%s\".\n", keyword);
    }
    printf("=====================================\n");
}

// Reader structure
typedef struct {
    int id;
    char selectedBook[MAX_LEN];
} ReaderInfo;

// Reader thread
void* reader(void* arg) {
    ReaderInfo* info = (ReaderInfo*)arg;
    char timestamp[20];
    char lineCopy[MAX_LEN];
    strcpy(lineCopy, info->selectedBook);

    sem_wait(&readerTry);
    pthread_mutex_lock(&readMutex);
    readCount++;
    activeReaders++;
    if (readCount == 1) 
	sem_wait(&resource);
	
    pthread_mutex_unlock(&readMutex);
    sem_post(&readerTry);

    getTimeString(timestamp);
    printf("\n[%s] Reader %d is READING a book...\n", timestamp, info->id);

    char* title = strtok(lineCopy, "|");
    char* author = strtok(NULL, "|");
    char* year = strtok(NULL, "|");
    char* genre = strtok(NULL, "|\n");

    printf("-------------------------------------\n");
    printf("Title : %s\n", title);
    printf("Author: %s\n", author);
    printf("Year  : %s\n", year);
    printf("Genre : %s\n", genre);
    printf("-------------------------------------\n");

    sleep(10); 

    pthread_mutex_lock(&readMutex);
    readCount--;
    activeReaders--;
    if (readCount == 0) sem_post(&resource);
    pthread_mutex_unlock(&readMutex);

    getTimeString(timestamp);
    printf("[%s] Reader %d has FINISHED reading.\n", timestamp, info->id);
    free(info);
    pthread_exit(0);
}

// Writer thread
void* writer(void* arg) {
    char title[MAX_LEN], author[MAX_LEN], year[MAX_LEN], genre[MAX_LEN];
    char timestamp[20];

    // Wait until all readers finish before the librarian starts updating
    while (1) {
        pthread_mutex_lock(&readMutex);
        int active = activeReaders;
        pthread_mutex_unlock(&readMutex);
        if (active == 0)
		 break;
        printf("[!] Waiting for all readers to finish...\n");
        sleep(1); 
    }

    pthread_mutex_lock(&writeMutex);
    waitingWriters++;
    if (waitingWriters == 1) 
	sem_wait(&readerTry);
    pthread_mutex_unlock(&writeMutex);

    sem_wait(&resource);
    getTimeString(timestamp);
    printf("\n[%s] Librarian is UPDATING records...\n", timestamp);
    printf("=====================================\n");

    getchar(); // flush newline
    printf("Enter Book Title : "); 
	fgets(title, MAX_LEN, stdin);
    printf("Enter Author     : "); 
	fgets(author, MAX_LEN, stdin);
    printf("Enter Year       : "); 
	fgets(year, MAX_LEN, stdin);
    printf("Enter Genre      : ");
	fgets(genre, MAX_LEN, stdin);

    title[strcspn(title, "\n")] = 0;
    author[strcspn(author, "\n")] = 0;
    year[strcspn(year, "\n")] = 0;
    genre[strcspn(genre, "\n")] = 0;

    FILE* f = fopen(BOOK_FILE, "a");
    if (f) {
        fprintf(f, "%s|%s|%s|%s\n", title, author, year, genre);
        fclose(f);
        printf("\n[?] Book added successfully!\n");
    } 
	else {
        printf("[!] Error writing to book file.\n");
    }

    getTimeString(timestamp);
    printf("[%s] Librarian has FINISHED updating records.\n", timestamp);
    sleep(1);

    sem_post(&resource);

    pthread_mutex_lock(&writeMutex);
    waitingWriters--;
    if (waitingWriters == 0) 
	sem_post(&readerTry);
    pthread_mutex_unlock(&writeMutex);

    pthread_exit(0);
}

// Main function
int main() {
    pthread_mutex_init(&readMutex, NULL);
    pthread_mutex_init(&writeMutex, NULL);
    sem_init(&resource, 0, 1);
    sem_init(&readerTry, 0, 1);

    int choice, readerId = 1;

    printf("=====================================\n");
    printf("     Welcome to the Library System   \n");
    printf("=====================================\n");

    while (running) {
        printf("\nMenu:\n");
        printf("1. Librarian (Add Book)\n");
        printf("2. Reader (View Book)\n");
        printf("3. Search Book by Title\n");
        printf("4. Exit\n");
        printf("-------------------------------------\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            // Wait until all readers finish
            while (1) {
                pthread_mutex_lock(&readMutex);
                int active = activeReaders;
                pthread_mutex_unlock(&readMutex);
                if (active == 0) 
				break;
                sleep(1); // Wait for active readers
            }
            pthread_t w;
            pthread_create(&w, NULL, writer, NULL);
            pthread_join(w, NULL);
        } 
		else if (choice == 2) {
            char selected[MAX_LEN];
            if (listBooksAndSelect(selected)) {
                pthread_t r;
                ReaderInfo* info = malloc(sizeof(ReaderInfo));
                info->id = readerId++;
                strcpy(info->selectedBook, selected);
                pthread_create(&r, NULL, reader, info);
                sleep(1); 
            }
        } 
		else if (choice == 3) {
            searchBook();
        } 
		else if (choice == 4) {
            printf("\nExiting system... Thank you!\n");
            running = 0;
            sleep(1);
        } 
		else {
            printf("[!] Invalid choice. Try again.\n");
        }
    }

    pthread_mutex_destroy(&readMutex);
    pthread_mutex_destroy(&writeMutex);
    sem_destroy(&resource);
    sem_destroy(&readerTry);

    printf("=====================================\n");
    printf("         System Shutdown\n");
    printf("=====================================\n");
    return 0;
}
