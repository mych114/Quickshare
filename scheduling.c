//Myra Chan
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printToFile(FILE *fp, int terminated, int running, int *notWorking, int *cycle, int processNum, int runStatus, int readyStatus, int blockStatus, int *status);
void updateNewArrivals(int processNum, int cycle, int readyStatus, int *arrivals, int *ready, int *status);
void terminateProcess(int *terminated, int *turnaround, int cycle, int *arrivals, int *status, int *run, int outStatus);

typedef struct Process{
    int processID;
    int cpuTime;
    int ioTime;
    int totalCPU;
    int timeLeft;

} process;


int main(int argc, char *argv[]){
    //Store input information
    char *input = argv[1];
    int algoChoice = 0;
    sscanf(argv[2], "%d", &algoChoice);
    //Determine output name and file
    char begin[30];
    strcpy(begin, input);
    const char delim[2] = ".";
    char *token;
    token = strtok(begin, delim);
    char fileName[30];
    strcpy(fileName, token);
    strcat(fileName,"-");
    strcat(fileName, argv[2]);
    strcat(fileName, ".txt");
    FILE *input_file;
    FILE *output_file;
    output_file = fopen(fileName, "w");

    int pID;
    int cpuT;
    int ioT;
    int arrivalT;
    int processNum = 0;
    int arrivals[processNum];
    int turnaround[processNum];
    int notWorking = 0;
    int min = 2147483647;

    //Parsing the input file
    input_file = fopen(input, "r");
    if (input_file == 0){
        printf("Unable to open file\n");
        exit(1);
    }
    else{
        fscanf(input_file, "%d", &processNum);
    }
    process * processes = malloc(processNum * sizeof(process)); //create array to store processes data
    for (int i = 0; i < processNum; i++){
        fscanf(input_file, "%d %d %d %d", &pID, &cpuT, &ioT, &arrivalT);
        processes[pID].processID = pID;
        if (cpuT % 2 == 1){ //add 1 to CPU time if CPU time is odd for subsequently rounding up
            processes[pID].cpuTime = cpuT + 1;
            processes[pID].totalCPU = cpuT + 1 + ioT;
            processes[pID].timeLeft = cpuT + 1 + ioT;
        }
        else{
            processes[pID].cpuTime = cpuT;
            processes[pID].totalCPU = cpuT + ioT;
            processes[pID].timeLeft = cpuT + ioT;
        }
        processes[pID].ioTime = ioT;
        arrivals[pID] = arrivalT;
    }
    //to help determine the state of each process
    int outStatus = 0;
    int runStatus = 1;
    int readyStatus = 2;
    int blockStatus = 3;

    int cycle = 0;
    int terminated = 0;
    int running = -1;
    int status[processNum];
    int ready[processNum];
    int blocked[processNum];
    //initialize all values of these arrays to -1
    for (int i = 0; i < processNum; i++){
        status[i] = -1;
        ready[i] = -1;
        blocked[i] = -1;
    }
    if (algoChoice == 0){ //if user chose First Come First Serve algorithm
        while (terminated != processNum){//loop until all processes have been terminated
            //move new arrivals to ready
            updateNewArrivals(processNum, cycle, readyStatus, arrivals, ready, status);
            //updating the running process
            if (running != -1){ //if there is a running process
                processes[running].timeLeft--;
                if (processes[running].timeLeft != 0){//if this process is not finished
                    //use CPU time left to calculate what state the process should be in
                    if((processes[running].timeLeft <= processes[running].totalCPU - processes[running].cpuTime / 2) && (processes[running].timeLeft > processes[running].cpuTime / 2)){
                        //if in I/O time, move process from running to blocked
                        blocked[running] = cycle + processes[running].totalCPU - processes[running].cpuTime;//store cycle when process will be unblocked
                        status[running] = blockStatus;//update status of this process to blocked
                        running = -1; //no current running process
                    }
                }
                else{//if this process is finished
                    terminateProcess(&terminated, turnaround, cycle, arrivals, status, &running, outStatus);
                }
            }
            //update the blocked processes
            for (int i = 0; i < processNum; i++){
                if (blocked[i] == cycle){//if blocked process has finished I/O time, move out of blocked
                    blocked[i] = -1;
                    ready[i] = cycle; //move process to ready processes
                    status[i] = readyStatus;
                }
                else if (blocked[i] != -1){//if I/O time not finished, decrease remaining time
                    processes[i].timeLeft--;
                }
            }
            //if necessary, update a ready process to running
            if (running == -1){ //if no current running process
                min = 2147483647;
                for (int i = 0; i < processNum; i++){//find closest time ready process to change to running
                //by nature of array traversal, even processes happen to be ready at the same time the lower ID process will be stored
                    if (ready[i] > -1 && ready[i] < min){
                        min = ready[i];
                        running = i;
                    }
                }
                if (running != -1){ //if found a ready process to update to running, update the status
                        status[running] = runStatus;
                        ready[running] = -1;//move process to running
                }
            }
            printToFile(output_file, terminated, running, &notWorking, &cycle, processNum, runStatus, readyStatus, blockStatus, status);
            
        }
    }
    else if(algoChoice == 1){//if user chose Round Robin algorithm
        int quantum = 0;
        while (terminated != processNum){
            updateNewArrivals(processNum, cycle, readyStatus, arrivals, ready, status);
            //updating running process
            if (running != -1){
                processes[running].timeLeft--;
                quantum++;
                if (processes[running].timeLeft != 0){//if process hasn't finished
                    if((processes[running].timeLeft <= processes[running].totalCPU - processes[running].cpuTime / 2) && (processes[running].timeLeft > processes[running].cpuTime / 2)){
                        //moving process from running to blocked
                        blocked[running] = cycle + processes[running].totalCPU - processes[running].cpuTime;
                        status[running] = blockStatus;
                        running = -1;
                        quantum = 0;
                    }
                    else if (quantum == 2) {//move process into ready state if quantum reaches 2 (ie, two cycles have passed in running)
                        ready[running] = cycle;
                        status[running] = readyStatus;
                        running = -1;
                        quantum = 0;
                    }
                }
                else{//if this process is finished
                    terminateProcess(&terminated, turnaround, cycle, arrivals, status, &running, outStatus);
                    quantum = 0;
                }
            }
            //update blocked processes
            for (int i = 0; i < processNum; i++){
                if (blocked[i] == cycle){//if blocked process has finished I/O time, move out of blocked
                    blocked[i] = -1;
                    ready[i] = cycle;
                    status[i] = readyStatus;
                }
                else if (blocked[i] != -1){
                    processes[i].timeLeft--;
                }
            }
            //same steps as First Come First Served algorithm in modifying a ready process to a running process with quantum update
            if (running == -1){
                min = 2147483647;
                for (int i = 0; i < processNum; i++){
                    if (ready[i] > -1 && ready[i] < min){
                        min = ready[i];
                        running = i; 
                    }
                }
                if (running != -1){
                        status[running] = runStatus;
                        ready[running] = -1;
                        quantum = 0;
                }
            }
            printToFile(output_file, terminated, running, &notWorking, &cycle, processNum, runStatus, readyStatus, blockStatus, status);
        }
    }
    else if (algoChoice == 2){ //if user chooess Shortest Remaining Job First
        int shortestTime = -1;
        while (terminated != processNum){
            for (int i = 0; i < processNum; i++){
                if(arrivals[i] == cycle){
                    ready[i] = processes[i].timeLeft; //adjusted for CPU time consideration
                    status[i] = readyStatus;
                }
            }
            //updating running process
            if (running != -1){
                processes[running].timeLeft--;
                if (processes[running].timeLeft != 0){//if process is not finished, check to see if it is blocked
                    if((processes[running].timeLeft <= processes[running].totalCPU - processes[running].cpuTime / 2) && (processes[running].timeLeft > processes[running].cpuTime / 2)){
                        //moving running process to blocked
                        blocked[running] = cycle + processes[running].totalCPU - processes[running].cpuTime;
                        status[running] = blockStatus;
                        running = -1;
                    }
                }
                else{//this process is finished
                    terminateProcess(&terminated, turnaround, cycle, arrivals, status, &running, outStatus);
                }
            }
            //update blocked processes
            for (int i = 0; i < processNum; i++){
                if (blocked[i] == cycle){
                    blocked[i] = -1;
                    ready[i] = processes[i].timeLeft;
                    status[i] = readyStatus;
                }
                else if (blocked[i] != -1){
                    processes[i].timeLeft--;
                }
            }
            //modify ready proccesses if they have lower CPU times
            if (running == -1){
                min = 2147483647;
                for (int i = 0; i < processNum; i++){//move process from ready to running
                    if (ready[i] > -1 && ready[i] < min){
                        min = ready[i];
                        running = i;
                    }
                }
                if (running != -1){//if the min find is successful, run that processs found
                        status[running] = runStatus;
                        ready[running] = -1;//move process to running
                }
            }
            else{//if there's already a running process
                min = 2147483647;
                shortestTime = -1;
                for (int i = 0; i < processNum; i++){//check by smallest proccess from the ready proccesses
                    if (ready[i] > -1 && ready[i] < min){
                        min = ready[i];
                        shortestTime = i;
                    }
                }
                //if there is a process with a lower CPU time than the running process, update with the ready process
                if ((shortestTime < running && min == processes[running].timeLeft) || min < processes[running].timeLeft ){
                    status[running] = readyStatus;
                    ready[running] = processes[running].timeLeft;
                    status[shortestTime] = runStatus;
                    ready[shortestTime] = -1;
                    running = shortestTime;
                }
            }
            printToFile(output_file, terminated, running, &notWorking, &cycle, processNum, runStatus, readyStatus, blockStatus, status);
        }
    }
    //Statistics
    fprintf(output_file, "\nFinishing time: %d\n", cycle - 1);
    fprintf(output_file, "CPU utilization: %.2f\n", (float)(cycle - notWorking)/cycle);
    for (int i = 0; i < processNum; i++){
        fprintf(output_file, "Turnaround process %d: %d", i, turnaround[i] - 1);
        if(i != processNum - 1){
            fprintf(output_file, "\n");
        }
    }
    fclose(input_file);
    fclose(output_file);
    free(processes);
    return 0;
}

void printToFile(FILE *fp, int terminated, int running, int *notWorking, int *cycle, int processNum, int runStatus, int readyStatus, int blockStatus, int *status){
    if(terminated != processNum){
        if (running == -1){//count when the CPU is not being utilized
            (*notWorking)++;
        }
        fprintf(fp, "%d ", *cycle);
        for (int i = 0; i < processNum; i++){//print processes and their status
            if (status[i] == runStatus){
                fprintf(fp, "%d:running ", i);
            }
            else if (status[i] == readyStatus){
                fprintf(fp, "%d:ready ", i);
            }
            else if (status[i] == blockStatus){
                fprintf(fp, "%d:blocked ", i);
            }
        }
        fprintf(fp,"\n");
        (*cycle)++;
    }
    
}

void updateNewArrivals(int processNum, int cycle, int readyStatus, int *arrivals, int *ready, int *status){
    //update processes to ready if it is their arrival time
    for (int i = 0; i < processNum; i++){
        if(arrivals[i] == cycle){
            ready[i] = cycle;
            status[i] = readyStatus;
        }
    }
}

void terminateProcess(int *terminated, int *turnaround, int cycle, int *arrivals, int *status, int *run, int outStatus){
    int running = *run;
    (*terminated)++;//increase count of finished processes
    turnaround[running] = cycle - arrivals[running] + 1; //store turnaround time of the process
    status[running] = outStatus;
    *run = -1;
}
