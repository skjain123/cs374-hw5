/**
 * Name: Sunil Jain
 * Student ID: 934059526
 * Oregon State University
 * Operating Systems 1
 * Spring 2024
 * Professor Chaudhrn
 * 
 * Description: This is a program called line_processor, that uses threads to read in input, separate the input,
 *              processes the input, and outputs the input without having to wait for all the input to be inserted
 *              before doing anything.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 1000
#define NUM_BUFFERS 3
#define LINE_SIZE 80

// shared buffers between threads
char buffer_1[BUFFER_SIZE][BUFFER_SIZE];
char buffer_2[BUFFER_SIZE][BUFFER_SIZE];
char buffer_3[BUFFER_SIZE][BUFFER_SIZE];

// buffer counts and indexes
int count_1 = 0; 
int prod_idx_1 = 0;
int con_idx_1 = 0;

int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;

int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;

// mutexes and condition variables
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_2 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_3 = PTHREAD_COND_INITIALIZER;

void* input_thread (void* args) {

    char line[BUFFER_SIZE];

    while (1) {
        // get input from stdin
        fgets(line, BUFFER_SIZE, stdin);

        // look for termination string "STOP"
        if (strncmp(line, "STOP\n", 5) == 0) {
            break;
        }

        pthread_mutex_lock(&mutex_1); // lock
        strcpy(buffer_1[prod_idx_1], line); // copy to buffer
        prod_idx_1 = (prod_idx_1 + 1) % BUFFER_SIZE; // loop back
        count_1++; // increment the line count
        pthread_cond_signal(&cond_1);
        pthread_mutex_unlock(&mutex_1);

    }

    pthread_mutex_lock(&mutex_1); // lock
    strcpy(buffer_1[prod_idx_1], "STOP\n"); // copy termination char to buffer
    prod_idx_1 = (prod_idx_1 + 1) % BUFFER_SIZE; // loop back
    count_1++; // increase lines
    pthread_cond_signal(&cond_1);
    pthread_mutex_unlock(&mutex_1);

    return NULL;
}

void* line_separator_thread (void* args) {
    char line[BUFFER_SIZE];

    while (1) {
        pthread_mutex_lock(&mutex_1); // lock
        
        while (count_1 == 0) { // wait until the first thread is done then continue on this thread
            pthread_cond_wait(&cond_1, &mutex_1);
        }

        strcpy(line, buffer_1[con_idx_1]); // copy to buffer
        con_idx_1 = (con_idx_1 + 1) % BUFFER_SIZE; // loop back
        count_1--; // decrease line count
        pthread_mutex_unlock(&mutex_1);
        
        // check for specified termination string
        if (strncmp(line, "STOP\n", 5) == 0) {
            break;
        }

        // go through line until termination character and replace newlines with spaces
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '\n') {
                line[i] = ' ';
            }
        }

        pthread_mutex_lock(&mutex_2); // lock thread until the following is done to prevent reading while writing
        strcpy(buffer_2[prod_idx_2], line); // output this thread's info to thread 2
        prod_idx_2 = (prod_idx_2 + 1) % BUFFER_SIZE; // loop back
        count_2++; // add line count
        pthread_cond_signal(&cond_2); // unlock thread
        pthread_mutex_unlock(&mutex_2);

    }

    pthread_mutex_lock(&mutex_2); // lock thread 2
    strcpy(buffer_2[prod_idx_2], "STOP\n"); // add a stop termination string to the end of the line
    prod_idx_2 = (prod_idx_2 + 1) % BUFFER_SIZE; // loop back to the beginning
    count_2++; // increase line count
    pthread_cond_signal(&cond_2); // unlock the thread
    pthread_mutex_unlock(&mutex_2);

    return NULL;

}

void* plus_sign_thread (void *args) {
    char line[BUFFER_SIZE];
    
    while (1) {
        pthread_mutex_lock(&mutex_2); // lock thread 2

        while (count_2 == 0) { // wait until the thread has input from thread 1
            pthread_cond_wait(&cond_2, &mutex_2);
        }

        strcpy(line, buffer_2[con_idx_2]); // get the input
        con_idx_2 = (con_idx_2 + 1) % BUFFER_SIZE; // loop back
        count_2--; // decrease line count
        pthread_mutex_unlock(&mutex_2); // unlock thread

        if (strncmp(line, "STOP\n", 5) == 0) {
            break;
        }

        for (int i = 0; line[i] != '\0'; i++) {

            if (line[i] == '+' && line[i+1] == '+') { // if there is, check if they are both pluses and replace
                line[i] = '^';
                memmove(&line[i+1], &line[i+2], strlen(line) - i - 1);
            }
        }

        pthread_mutex_lock(&mutex_3); // lock output thread from reading to begin writing
        strcpy(buffer_3[prod_idx_3], line); // write to buffer 3 
        prod_idx_3 = (prod_idx_3 + 1) % BUFFER_SIZE; // loop back
        count_3++; // increase line count
        pthread_cond_signal(&cond_3); // unlock output thread
        pthread_mutex_unlock(&mutex_3);


    }

    pthread_mutex_lock(&mutex_3); // lock the output thread from reading to begin writing
    strcpy(buffer_3[prod_idx_3], "STOP\n"); // write-append the termintation string "STOP" 
    prod_idx_3 = (prod_idx_3 + 1) % BUFFER_SIZE; // loop back
    count_3++; // increase line count
    pthread_cond_signal(&cond_3); // unlock the output thread to begin reading
    pthread_mutex_unlock(&mutex_3);

    return NULL;
}

void* output_thread (void* args) {
    char line[BUFFER_SIZE];
    char output_line[LINE_SIZE + 1];
    int line_len = 0;

    while (1) {
        pthread_mutex_lock(&mutex_3); // lock reading to confirm we have input
        
        while (count_3 == 0) { // wait for input
            pthread_cond_wait(&cond_3, &mutex_3);
        }

        strcpy(line, buffer_3[con_idx_3]); // get the input
        con_idx_3 = (con_idx_3 + 1) % BUFFER_SIZE; // loop back if needed
        count_3--; // increase line count
        pthread_mutex_unlock(&mutex_3); // unlock to start outputting

        if (strncmp(line, "STOP\n", 5) == 0) { // if you find the stop termination string break
            break;
        }

        for (int i = 0; line[i] != '\0'; i++) { // go through the line until the termination character

            // add the character to the output line
            output_line[line_len++] = line[i];

            // if we exceed the line size, put a termination character there and print the line
            if (line_len == LINE_SIZE) {
                output_line[LINE_SIZE] = '\0';
                printf("%s\n", output_line);
                line_len = 0; // reset line to the beginning
            }
        }
    }

    return NULL;

}



int main ()
{
    // thread variables
    pthread_t input_t;
    pthread_t line_separator_t;
    pthread_t plus_sign_t;
    pthread_t output_t;

    // create threads and assign functionality
    pthread_create(&input_t, NULL, input_thread, NULL);
    pthread_create(&line_separator_t, NULL, line_separator_thread, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
    pthread_create(&output_t, NULL, output_thread, NULL);

    // wait for threads to finish
    pthread_join(input_t, NULL);
    pthread_join(line_separator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);

    return EXIT_SUCCESS;
}