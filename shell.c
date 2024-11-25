#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX 1000
#include <signal.h>
#include <sys/time.h>
#include <time.h>

char** gin; //declaring global variable to access in different functions 
int gcount = 0;   
int globalpid = 0,globalhcount = 0; //time
struct timeval globalstart, globalend;
double * globalstarttime; 
double * globalruntime;
int* globalprocessid;
time_t* globaltime[MAX];
time_t globalcurrenttime;

void show_history(){     //history
    for(int i=0;i<gcount;i++){
        printf("%d. %s \n", i+1, gin[i]);
    }
}

// Function to read user input
char* read_user_input() {
    char* buffer = (char*)malloc(MAX * sizeof(char));
    // Read user input dynamically

    if (buffer==NULL){             //check for memory allocation
        printf("OS_A2@custom_shell:~$ Memory allocation failed!\n");
        exit(1);
    }

    if (fgets(buffer, MAX, stdin)==NULL){   //check for reading user input
        printf("OS_A2@custom_shell:~$ Could not read input\n");
        exit(1);
    }
    int length = strlen(buffer);

    buffer[strcspn(buffer, "\n")] = '\0';
    return buffer;
}

int parse(char * buff, char** arr, char* delimiter){
    char* tok = strtok(buff,delimiter);
    int arr_size = 0;
    while (tok != NULL ){           
        arr[arr_size++] = tok;
        tok = strtok(NULL, delimiter);
    }
    arr[arr_size] = NULL;
    return arr_size;
    //last is null but not added to returned size
}


int create_process_and_run(char* command) {
    char** arr = (char**)malloc(MAX * sizeof(char*));
    if (arr == NULL) {
        printf("@custom_shell:~$ Memory allocation failed!\n");
        exit(1);
    }

    gin[gcount++] = strdup(command);

    int arg_size = parse(command, arr, "|");
    if (arg_size == 1) {
        // If there's only one command, no need for pipes
        int status = fork();
        if (status == 0) {
            char** args = (char**)malloc(MAX * sizeof(char*));
            parse(arr[0], args, " ");
            execvp(args[0], args);
            perror("Error: execvp");
            exit(1);
        } else if (status < 0) {
            printf("Something bad happened.\n");
            exit(1);
        } else {
            globalcurrenttime=time(NULL);
            globaltime[globalhcount]=strdup(ctime(&globalcurrenttime));
            gettimeofday(&globalstart, NULL);     //time
            globalprocessid[globalhcount]=wait(NULL);  //pid
            gettimeofday(&globalend, NULL);
            globalruntime[globalhcount]= (globalend.tv_sec - globalstart.tv_sec) + (globalend.tv_usec - globalstart.tv_usec) / 1000000.0; //time
            globalhcount+=1;
        }
    } else {
        int pipes[arg_size][2];  // Create an array of pipes

        for (int i = 0; i < arg_size; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error: pipeline.");
                exit(1);
            }

            int stat = fork();
            if (stat == 0) {
                // Child process
                if (i > 0) {
                    close(pipes[i - 1][1]);  // write end of the previous pipe closed
                    dup2(pipes[i - 1][0], STDIN_FILENO);  // stdin duplicated to read
                    close(pipes[i - 1][0]);
                }
                if (i < arg_size - 1) {
                    close(pipes[i][0]);  // read end of the current pipe closed
                    dup2(pipes[i][1], STDOUT_FILENO);  // stdout duplicated to write
                    close(pipes[i][1]);
                }

                char** args = (char**)malloc(MAX * sizeof(char*));
                parse(arr[i], args, " ");
                execvp(args[0], args);                                 //execvp takes care of freeing allocated memory here
                perror("Error: execvp");
                free(args);
                exit(1);
            } else if (stat > 0) {
                // Parent process
                
                if (i > 0) {
                    close(pipes[i - 1][0]);  // read end of the previous pipe closed
                    close(pipes[i - 1][1]);  // write end of the previous pipe closed
                }

                // last child process 
                if (i == arg_size - 1) {
                    globalcurrenttime=time(NULL);
                    globaltime[globalhcount]=strdup(ctime(&globalcurrenttime));
                    gettimeofday(&globalstart, NULL);
                    close(pipes[i][0]);  // Close the read end of the last pipe
                    close(pipes[i][1]);  // Close the write end of the last pipe
                    globalprocessid[globalhcount]=wait(NULL);
                    gettimeofday(&globalend, NULL);
                    globalruntime[globalhcount]=(globalend.tv_sec - globalstart.tv_sec) + (globalend.tv_usec - globalstart.tv_usec) / 1000000.0;
                    globalhcount+=1;
                    
                }
                else{
                    wait(NULL);
                }
                

            } else {
                printf("Something bad happened.\n");
                exit(1);
            }
        }
    }

    return 0;
}

void process_info() {
    printf("%-50s%-15s%-15s%-30s\n", "Command Name", "PID", "Run Time", "Start Time");
    for (int i = 0; i < globalhcount; i++) {
        char* new = strchr(globaltime[i], '\n');
        if (new != NULL) {
            *new = '\0';
        }
        printf("%-50s%-15d%-15f%-30s\n", gin[i], globalprocessid[i], globalruntime[i], globaltime[i]);
    }
}

int launch (char *command) {
    int status;
    status = create_process_and_run(command);
    return status;
}

void shell_loop() {
    int status;
    do {
        printf("@custom_shell:~$ ");

        int flag = 0;
        char* command = read_user_input();


        for (int i=0;i<strlen(command);i++){        //check for backslash and quotes
            if (command[i]== '\\' || command[i]=='"'){
                printf("OS_A2@custom_shell:~$ Invalid Input, contains backslashes or quotes!\n");
                status = 0;
                flag = 1;
            }
        }
        if ((strcmp(command, "history")) != 0 && flag == 0){
            status = launch(command);
        }
        // making history also using fork
        else if ((strcmp(command, "history") == 0) && flag == 0){    //history
            int history_pid = fork();
            if (history_pid == 0){
                show_history(); //history
                exit(0);
            }
            else if (history_pid>0){
                globalcurrenttime=time(NULL);
                globaltime[globalhcount]=strdup(ctime(&globalcurrenttime));
                gettimeofday(&globalstart, NULL);
                gin[gcount]="history";  //history
                gcount+=1;    //history
                globalprocessid[globalhcount]=wait(NULL);
                gettimeofday(&globalend, NULL);
                globalruntime[globalhcount]=(globalend.tv_sec - globalstart.tv_sec) + (globalend.tv_usec - globalstart.tv_usec) / 1000000.0;
                globalhcount+=1;
            }
            else{
                printf("Something bad happened.\n");
                exit(1);
            }
        }
        free(command); // free malloc
    } while (!status);
}


static void personal_handler(int signum){
    if (signum == SIGINT){
        printf(" \nExit log.\n");
        process_info();
        exit(1);
    }
}

int main() {
    globalstarttime=(double*)malloc(MAX*sizeof(double));
    globalruntime=(double*)malloc(MAX*sizeof(double));
    globalprocessid=(int*)malloc(MAX*sizeof(int));
    gin=(char**)malloc(MAX*sizeof(char*));
    globalcurrenttime=time(NULL);

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = personal_handler;
    sigaction(SIGINT, &sig, NULL);
    shell_loop();
    return 0;
}