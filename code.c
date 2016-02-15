/////////////////////////////////////////////////
//          ECSE 427 - Assignment 1            // 
//          Author- HABIB I. AHMED             // 
//              ID - 260464679                 //
//                  code.c                     //
//Please read the description before "fg"      //
/////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct History{
    char *cmd[20];
    struct History *next;
}History;

typedef struct jobList{
    pid_t jpid;
    char *jobName[20];
    int jobNumber;
    struct jobList *nextJob;
}jobList;

int historyCount;
int jobID;

struct History *head = NULL;
struct jobList *top = NULL;

int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0,j;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
        exit(-1);
    }

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    args[i]=NULL;
    return i;
}

int freecmd(){
    //Memory has been freed where necessary
}

int main()
{
    char *args[20];
    int bg = 0, i, j, k, x, count, cmdNumber, paramsCounter;
    int childStatus, jobStatus;
    pid_t pid, bgpid, sid;
    historyCount = 0;
    jobID=0;

    while(1){
        int duplicateCheck = 0;
        int cnt = getcmd("\n>>  ", args, &bg);
        struct History *current;
        struct jobList *currentJob;
        if (cnt != 0){
            
            //prints history not including built in commands
            if(strncmp(args[0],"history",7) == 0 && args[1] == NULL){
                duplicateCheck = 1;
                if(historyCount>=10){
                    //loop through the last 10 items from the linked list
                    for(cmdNumber=0;cmdNumber < 10;cmdNumber++){
                        paramsCounter = 0;
                        printf("\n%d   ", cmdNumber+1);
                        while(current->cmd[paramsCounter] != NULL){
                            //display command at each node
                            printf("%s ", current->cmd[paramsCounter]);
                            paramsCounter++;
                            fflush(stdout);
                        }
                        current = current->next;
                    }
                }
                else{
                    //loop through all the commands in case number of commands entered is less than 10
                    for(cmdNumber=0;cmdNumber < historyCount;cmdNumber++){
                        paramsCounter = 0;
                        printf("\n%d   ", cmdNumber+1);
                        while(current->cmd[paramsCounter] != NULL){
                            //display command at each node
                            printf("%s ", current->cmd[paramsCounter]);
                            paramsCounter++;
                            fflush(stdout);
                        }
                        current = current->next;
                    }
                }
                current = head;
                continue;
            }

            //prints all jobs
            if(strncmp(args[0],"jobs",4) == 0 && args[1] == NULL){
                int jobNumber;
                duplicateCheck = 1;
                printf("\nJob Number      PID      Process");
                //loop through all the jobs
                for(jobNumber=0;jobNumber<jobID;jobNumber++){
                        paramsCounter = 0;
                        printf("\n%d             %d    ", jobNumber, currentJob->jpid);
                        while(currentJob->jobName[paramsCounter] != NULL){
                            //display all jobs
                            printf("%s ", currentJob->jobName[paramsCounter]);
                            paramsCounter++;
                            fflush(stdout);
                        }
                        currentJob = currentJob->nextJob;
                }
                currentJob = top;
                continue;
            }

            //runs background jobs if "fg [jobid]" is entered
            //I tried to implement the fg function but it wouldn't work
            //When functional, the fg command would bring a command from the background to the foreground
            //For example, if "$ sleep 4 &" is entered at the prompt, and it is the first job,
            //entering "fg 0" would execute the sleep command and wait for 4 seconds before the
            //prompt appears again.
            if(strncmp(args[0],"fg",2) == 0){
                if(args[1] != NULL){
                    while(currentJob != NULL){
                        int a;
                        // if(currentJob->jobNumber == (args[1][0] - '0')){
                        //     waitpid((currentJob->jpid), &jobStatus, 0);
                        //     break;
                        // }
                        char *b = args[1];
                        int temp = atoi(b);
                        if(temp == currentJob->jobNumber){
                            a = 0;
                            while(currentJob->jobName[a] != NULL){
                                printf("%s ", currentJob->jobName[a]);
                                fflush(stdout);
                                a++;
                            }   
                            printf("\n"); 
                            execvp(currentJob->jobName[0], currentJob->jobName);
                        }
                        else{
                            currentJob = currentJob->nextJob;
                        }
                    }
                    currentJob = top;
                }
            }

            //checks the current directory
            if(strncmp(args[0],"pwd",3) == 0 && args[1] ==  NULL){
                duplicateCheck = 1;
                char* newDir = (char *)malloc(sizeof(char)*512);
                printf("%s\n",getcwd(newDir,512));
                free(newDir);
                continue;
            }

            //Changes the directory
            if(strncmp(args[0],"cd",2) == 0){
                duplicateCheck = 1;
                printf("\n%s",args[1]);
            //  args[1] = removeQuotes(args[1]);
                chdir(args[1]);
                continue;
            }

            //Exits if entered at prompt
            if(strncmp(args[0],"exit",4) == 0){
                return 0;
            }

            pid = fork();
            if (pid == -1){
                perror("Fork");
                exit(1);
            }


            //Child process goes here
            else if (pid == 0 && duplicateCheck != 1){
                //Checks if process is to run in background
                if(bg ==1){
                    setsid();
                    chdir("/");

                    close(STDIN_FILENO);
                    close(STDOUT_FILENO);
                    close(STDERR_FILENO);
                    
                }
                // "r" and "r x" function
                if(bg == 0){
                    if(strncmp(args[0],"r",1) == 0){
                        //runs only "r"
                        if(cnt == 1){
                            x = 0;
                            while(current->cmd[x] != NULL){
                                printf("%s ", current->cmd[x]);
                                fflush(stdout);
                                x++;
                            }
                            printf("\n");
                            execvp(current->cmd[0],current->cmd);

                        }
                        //runs "r x"
                        else{
                            while(current!=NULL) {
                                char c = current->cmd[0][0];
                                if(c == args[1][0]) {
                                    x = 0;
                                    while(current->cmd[x] != NULL){
                                        printf("%s ", current->cmd[x]);
                                        fflush(stdout);
                                        x++;
                                    }   
                                    printf("\n");                             
                                    execvp(current->cmd[0], current->cmd);
                                }
                                current=current->next;
                            }
                        }
                    }

                    else if(execvp(args[0], args) == 0){
                        exit(EXIT_SUCCESS);
                    }
                    else{
                            printf("\nInvalid Command");
                            exit(EXIT_FAILURE);
                    }  
                }                 
            }
        
            //Parent process goes here
            else if (pid > 0){
                if(bg == 0){
                    //wait
                    waitpid(pid, &childStatus, 0);
                    if(childStatus != 256 && strncmp(args[0],"r",1) != 0){
                        current = (History*)malloc(sizeof(History*));
                        for(k = 0;k < cnt;k++){
                            current->cmd[k]=(char *)malloc(sizeof(char*));;
                            strcpy(current->cmd[k], args[k]);
                        }
                        historyCount++;
                        current->cmd[k]=NULL;
                        current->next = head;
                        head = current;
                    }
                }
                else
                {
                    //Get next command
                    printf("\nBackground not zero, job stored");
                    int jobCount;
                    bgpid = fork();

                    if(bgpid < 0){
                        printf("\nChild not created");
                        exit(EXIT_FAILURE);
                    }

                    else if(bgpid == 0){
                        sid = setsid();
                        
                        if(sid < 0){
                            printf("\nSession creation failed");
                            exit(EXIT_FAILURE);
                        }

                        close(STDIN_FILENO);
                        close(STDOUT_FILENO);
                        close(STDERR_FILENO);

                        execvp(*args, args);
                    }
                    currentJob = (jobList*)malloc(sizeof(jobList*));
                    for(jobCount = 0;jobCount < cnt;jobCount++){
                        currentJob->jobName[jobCount]=(char *)malloc(sizeof(char*));
                        strcpy(currentJob->jobName[jobCount], args[jobCount]);
                    }
                    jobID++;
                    currentJob->jpid = bgpid;
                    currentJob->jobNumber = jobID;
                    currentJob->jobName[jobCount]=NULL;
                    currentJob->nextJob = top;
                    top = currentJob;
                }
            }
        }
    }
}
