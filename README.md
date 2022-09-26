# CLIshell
OS Final Phase 4 Project

Operating Systems 
Assignment - Phase 4: Remote Multitasking CLI Shell with Scheduling Capabilities 
[Here is the link to the project Documentation](https://github.com/Dev-Kalavadia/CLIshell/blob/main/Phase%204%20Documentation.pdf)

# Remote shell 

## Phase 1:-
###Description: 
For this assignment, we programmed our own local shell / CLI using C. It stimulates the services of the operating system using system calls. We used features such as process creation, interprocess communication, etc. to execute the same. 15 commands that have been listed below can be run, including pipes (upto three pipes). 
How to Compile: 
We have used makefile to organize the project’s code compilation. To run a makefile code, run the following commands in the terminal. 
First, run $make clean 
Then run $make to compile the code 
Lastly, run the executable. Run using $./shell 
List of commands:- 
1. ls: Command to display the files in a directory 
2. touch: Command to create a file 
3. mkdir: Command to create a folder 
4. pwd: Command to print the current directory 
5. rm: Command to remove objects 
6. cp: Command to copy 
7. rmdir: Command to remove a directory 
8. grep: Command to search for a string of characters in a file 
9. wc: Command to find the word count 
10. find: Command that locates an object 
11. mv: Command that moves an object 
12. cat: Command that outputs the contents of a file 
13. ps: Command that displays the running processes 
14. df: Command to display the display the available disk space 
15. whoami: Command to print the name of the current user
Pipe Commands: Used to combine two or more commands. The output of one command is the input of the next command. 
1. ls | grep c 
2. ls -l | grep d | wc -c 
3. cat xyz.txt | grep r | tee abc.txt | wc -l 
Test Cases can be found under phase 2 below.
 
### Challenges :- 
Along the way, we faced a few challenges we were able to overcome. 
1. The first challenge we faced was parsing the entered command(s). We overcame this by using strtok() which broke the string into a series of strings using our given delimiters. We were able to modify this in our pre-built functions to reuse them for extracting commands when pipes were used. 
2. Figuring out the right nested forks for triple pipes proved to be a challenging task as well. We were able to overcome this by doing some research as well as using trial and error till we were able to have an error free case.







## Phase 2:-

### Description:

For this assignment, we programmed our own Client-Server remote shell using socket communication in C. It stimulates the services of the Client server network using sockets, ports and local IP addresses to create a remote connection between the client and server. In this phase, we were taking the input from the user client and sending the request to the server. For each input from the client which could either be a comment from our listed menu or a pipe command with up to three pipes. Then the server will process the command and send the output or result back to the client which will be printed on the clients terminal screen

### How to Compile and Run:

We have used makefile to organize the project’s code compilation. To run a makefile code, run the following commands in the terminal. 

First, run $make clean (this is only if you want to run the code again and clear all the .o files)
Then run $make to compile the code

Then, the server should be started first using $./server
Finally start the client using $./client

### List of commands that the server:-

ls: Command to display the files in a directory
touch: Command to create a file 
mkdir: Command to create a folder
pwd: Command to print the current directory
rm: Command to remove objects
cp: Command to copy 
rmdir: Command to remove a directory
grep: Command to search for a string of characters in a file
wc: Command to find the word count
find: Command that locates an object
mv: Command that moves an object
cat: Command that outputs the contents of a file
ps: Command that displays the running processes
df: Command to display the display the available disk space
whoami: Command to print the name of the current user

Pipe Commands: Used to combine two or more commands. The output of one command is the input of the next command.  
```
ls | wc
ls -l | grep d | wc -c
cat xyz.txt | grep r | tee abc.txt | wc -l
```
### File Contents:

client.c : Contains the main function for running the client side application. 
server.c : Contains the main function for running the server side application.
checkpipe.c : Checks the number of pipes in the entered command
pipes.c : Performs execution of commands with pipes. There are separate functions for one, two and three pipes in this file. 
pipesplitter.c : performs parsing and tokenizing of all entered commands. Also performs the task of splitting pipes commands into separate commands. 
print.c : Prints the user prompt and current directory during each execution. 
singlecommand.c : Runs execution of the commands which do not use pipes. 



### Functions and Functionalities:

Pipe Checking:
 Pipe checking is done by the file checkpipe.c which checks if an entered string has pipes. If pipes are present, each section of the pipe is parsed and tokenized separately and sent to the appropriate pipe execution function (single pipe, double pipe, etc…). 
```
int checkPipe(char* str1){
 int count = 0;
   // int pipeFlag = 0;
   for(int i=0;i<=(strlen(str1));i++){
       if(str1[i]=='|'){
         ++count;
       }
   }
 return count;
}
```


### Parsing and Tokenizing: 
We handle parsing and tokenizing of commands in the splitter.c file. Both lineSplitter() and pipeSplitter() functions work in a similar manner. Token sizes are dynamically allocated and the tokens are separated using predefined delimiters for each function. The tokens are returned as an array which is further used by other functions. 

```
char **pipeSplitter(char *line, int pipeflag){

  int bufferSize = BUFFER_SIZE, indexPosition = 0;
  char **tokens = malloc(bufferSize * sizeof(char*)); // Array of tokens dynamically allocated
  char *token; // Each token (string between pipes)

  if (!tokens) { // If tokens memory was allocated 
    fprintf(stderr, "Token memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, PIPE_DELIM); // Strtok breaks string line into a series of tokens using the token delimiters provided as PIPE_DELIM Macro
  while (token != NULL) {
    tokens[indexPosition] = token; // place the token into the array of tokens
    indexPosition++;  // Increment the index position for the token to be saved in the array
    if (indexPosition >= bufferSize) {
      bufferSize += BUFFER_SIZE;
      tokens = realloc(tokens, bufferSize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "Token memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, PIPE_DELIM);  // Strtok NULL tells the function to continue tokenizing the string that was passed first
  }
  tokens[indexPosition] = NULL;
  return tokens; // return the array of tokens (arguments pipe separated )
}
```



### Piped Commands Execution:
Commands which pipes are directed to the functions in the pipes.c file. The pipes make use of the concept of process communication to execute multiple commands using multiple child processes. The number of commands determines the number of pipes used. For instance, a single pipe command requires two separate commands to be run, where the output of the first command becomes the input of the second command. This is done using two child processes (forks), which communicate using a file descriptor. The standard output of the first child is redirected to the second child using the dup2() function. The final output is then sent back to the output stream. Double and Triple pipe functions work similarly, except that the number of file descriptors, child processes, output redirections, etc… increase proportionately with the number of pipes. 
```
dup2(socket, STDOUT_FILENO); // reading redirected output of ls through pipe 1
dup2(socket, STDERR_FILENO);
```

### Sockets for Server to Client Communication:
This function is the core of our program. It demonstrated communication between two nodes (here the Server and Client) over a network. Various functions like listen(), bind(), accept() and connect() are used to enable a successful connection between the server and client. Messages are sent back and forth between the two nodes using send() and recv(). 
```
int sock1, sock2, valread;
struct sockaddr_in address; // structure for storing address
int addrlen = sizeof(address); // Getting the size off the address
 if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
 {
   perror("Socket creation failed");
   exit(EXIT_FAILURE);
 }
 // Setting and defining the address for the socket to bind on
 address.sin_family = AF_INET;
 address.sin_addr.s_addr = htonl(INADDR_ANY);
 address.sin_port = htons(PORT);
 
 if (bind(sock1, (struct sockaddr *)&address, sizeof(address)) < 0)
 {
   perror("Binding failed");
   exit(EXIT_FAILURE);
 }
 
 while (1) // to keep server alive forever
 {
   if (listen(sock1, 10) < 0) // defining for number of clients in queue (not necessary here since its single client connection)
   {
     perror("Listen Failed");
     exit(EXIT_FAILURE);
   }
   if (connectFlag == 0)
   {
     printf("Waiting for Client to connect\n"); // Server is listening and 
     if ((sock2 = accept(sock1, (struct sockaddr *)&address,
                         (socklen_t *)&addrlen)) < 0)
     {
       perror("accept");
       exit(EXIT_FAILURE);
     }
```

### Client Disconnection and Reconnection
The server waits for the client to connect and accepts commands as long as it is connected to the. When the client leaves using ‘exit’, the connection is disrupted and the server now goes back to waiting for a client to connect. This functions similar to how an actual server and client system would work. This feature is implemented by using a flag which checks which changes to true if the client sends exit. This makes the client exit and sends the server to waiting for a client. 

### Challenges :-

We ran into a lot of bugs for this Phase. Most were due to the complex working of the c programming language. A few of them are detailed below. 

The first issue after we connected successfully was the lagging of buffer outputs between the client and server. For instance, when we entered ls, the first command worked as expected and for the next command onwards the buffer was lagging because everything from the server was being output into the client. 
We realized this issue was created by an incorrect placement of the dup2() function which. Placing it in the parent process was creating a problem. We moved it to the child area and fixed the issue. 

Another challenge we faced was the buffer size problem. The commands output would queue up into the buffer but would not print correctly and commands would be sent in chunks when printing on the client.  In order to fix this we started by clearing the buffer and the input  before sending and receiving from server and client. Additionally, to fix the problem of large output getting sent as chucks, we added a recv() at the start with a MSG_DONTWAIT argument which didn’t block the client from running. 

We faced a bug where the client kept quitting after entering piped commands. Upon careful debugging we realized that the file descriptors and sockets were not closed properly and in the right area. With a little trial and error we figured this part out successfully. 




## PHASE 3:-

### File Contents:

client.c : Contains the main function for running the client side application.
server.c : Contains the main function for running the server side application.
checkpipe.c : Checks the number of pipes in the entered command
pipes.c : Performs execution of commands with pipes. There are separate functions for one, two and three pipes in this file.
pipesplitter.c : performs parsing and tokenizing of all entered commands. Also performs the task of splitting pipes commands into separate commands.
print.c : Prints the user prompt and current directory during each execution. singlecommand.c : Runs execution of the commands which do not use pipes.

### How to Compile and Run:

We have used makefile to organize the project’s code compilation. To run a makefile code, run the following commands in the terminal. 

First, run $make clean (this is only if you want to run the code again and clear all the .o files)
Then run $make to compile the code

Then, the server should be started first using $./server
Finally start clients using $./client




### Functions and Functionalities:

#### Multithreading for multiple Clients
 ```
 while (1) // to keep server alive forever
 {
   printf(">Waiting for Client to connect\n"); // Server is listening and waiting for connection
   if ((sock2 = accept(sock1, (struct sockaddr *)&address,
                       (socklen_t *)&addrlen)) < 0)
   {
     perror("accept");
     exit(EXIT_FAILURE);
   }
   // connectFlag = 1; // Setting connection flag to be true
   printf(">Successfully Connected: Listening to client.\n");
   int rc;                                       // return value from pthread_create to check if new thread is created successfully                          */
   pthread_t thread_id;                          // thread's ID (just an integer, typedef unsigned long int) to identify new thread
   int *new_socket = (int *)malloc(sizeof(int)); // for passing safely the integer socket to the thread
   if (new_socket == NULL)
   {
     fprintf(stderr, "Couldn't allocate memory for thread new socket argument.\n");
     free(new_socket);
     exit(EXIT_FAILURE);
   }
   *new_socket = sock2;
   printf("New Client has Connected!\n");
   // create a new thread that will handle the communication with the newly accepted client
   rc = pthread_create(&thread_id, NULL, HandleClient, new_socket);
   if (rc) // if rc is > 0 imply could not create new thread
   {
     printf("\n ERROR: return code from pthread_create is %d \n", rc);
     free(new_socket);
     exit(EXIT_FAILURE);
    
   }
 }
```



#### Exiting Client using “exit” and ^C:
```
void clientExitHandler(int sig_num)
{
 char *exitString = "exit";
 // printf("\nExiting shell successfully \n");
 send(sock, exitString, strlen(exitString) + 1, 0); // sending exit message to server
 close(sock);                           // close the socket/end the conection
 printf("\n Exiting client.  \n");
 fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
 exit(0);
}

int main() // main function
{
 printf("\t\t Welcome to D&S Remote Shell: Multi Server\n");
 printf("\t --------------------------------------------------------\n");
 signal(SIGINT, serverExitHandler);
}
```


### Challenges :-
The challenges faced in this phase were easier to analyze and debug.  

The first error we came across after merging our Phase 2 code with a multi-client code was while exiting our client. It was causing the server to spam a null character. We fixed this by closing the sockets in the right places and in the right order. The incorrect termination of the threads might have also contributed to the error which we handled correctly. 

Another thing we struggled with was to get the thread to terminate and have the closing of the socket at the end so that the particular client will successfully quit and disconnect. Once we had this working we realized that the client could reconnect to the server and was assigned the same thread number. 

The exiting of the client using two different methods, ^c and “exit” gave us different results, which was an error. We figured this problem was caused by using the clientExitHandler() function that worked differently from our exit, and debugged it appropriately. 

#PHASE 4:-

### File Contents:

client.c : Contains the main function for running the client side application.
server.c : Contains the main function for running the server side application.
checkpipe.c : Checks the number of pipes in the entered command
pipes.c : Performs execution of commands with pipes. There are separate functions for one, two and three pipes in this file.
pipesplitter.c : performs parsing and tokenizing of all entered commands. Also performs the task of splitting pipes commands into separate commands.
print.c : Prints the user prompt and current directory during each execution. singlecommand.c : Runs execution of the commands which do not use pipes.

###How to Compile and Run:

We have used makefile to organize the project’s code compilation. To run a makefile code, run the following commands in the terminal. 

First, run $make clean (this is only if you want to run the code again and clear all the .o files)
Then run $make to compile the code

Then, the server should be started first using $./server
Finally start clients using $./client



### Functions and Functionalities:

#### Struct for each Process
Structure to hold each process detail
```
struct process
{
 int processTime; // Burst time
 int processID; // Process ID
 char processName[1024]; // Name (either command or @mp)
 int processPriority; // Priority of the process
 int processFlag;          // Flag to see to keep track if its a command or process
 int processAT;            // Arrival time
 int processTAT;           // Turn around time
 int processWT;            // Waiting time
 int processResT;          // Response time
 int processCompletedTime; // Completion time
 int processDone;          // Done Flag time
 int socket;               // Process socket it is running on
 // pthread threadID;
};

struct process queue[MAX_QUEUE_SIZE];
```

### Shortest Job First
```
void sortQueue()
{
 int index, i, j;
 for (i = 0; i < queueSize; ++i)
 {
   for (j = i + 1; j < queueSize; ++j)
   {
     if (queue[i].processTime > queue[j].processTime) // Checking the time to see which one is greater
     {
       struct process a = queue[i]; // Temp struct
       queue[i] = queue[j]; // Swaping
       queue[j] = a;
     }
     else if (queue[i].processTime == queue[j].processTime) // If arrival time is same
     { // Then RR should run the one which hasnt got any completed time or hasnt got a chance to go
       if (queue[i].processCompletedTime > queue[j].processCompletedTime)
       {
         struct process a = queue[i]; // Swapping to sort the queue for this case
         queue[i] = queue[j];
         queue[j] = a;
       }
     }
   }
 }
 ```


### Priority in the Queue
Sorting the queue by priority (by default it is 0 for all process)
```
void sortQueueByPriority()
{
 int index, i, j;
 for (i = 0; i < queueSize; ++i)
 {
   for (j = i + 1; j < queueSize; ++j)
   {
     if (queue[i].processPriority < queue[j].processPriority) // Checking if there is a higher priority
     {
       struct process a = queue[i];
       queue[i] = queue[j]; // Swapping
       queue[j] = a;
     }
   }
 }
}
```


### Adding to the queue 

This function basically adds each command to the queue by taking in the common and socket as arguments and create a new structure for that process to be added to the queue. By default it has a priority of zero and the name and first time specified by the user which is parsed accordingly and assigned to the respective variables in the structure. Here is a commented code to see our implementation for this function.

Adding each process to the queue
```
struct process addToQueue(char *command, int socket) // Command from client and the socket #
{
 struct process newProgram; // Creating a new struct
 char *ret; // Check our return string to compare
 ret = strstr(command, PROGRAMNAME); // Looking for @mp in command
 newProgram.socket = socket; // Setting the socket
 newProgram.processPriority = 0; // Priority by default is 0
 if (ret) // if command entered is a program
 {
   char **arg;
   int i = 0;
   arg = lineSplitter(command); // Split the arguments
   strcpy(newProgram.processName, arg[0]); // Set the process name
   newProgram.processTime = atoi(arg[1]); // Set the process time

   if (arg[2]) // If priority is provided set it
   {
     newProgram.processPriority = atoi(arg[2]);
   }
   newProgram.processFlag = 1; // Set the flag to indicate its a process not command
   newProgram.processID = idCounter; // set the counter for ID
 }
 else // if command entered is not a process (command)
 {
   strcpy(newProgram.processName, command); // Set the command name
   newProgram.processTime = 1; // Time is 1sec by default
   newProgram.processFlag = 0; // Command flag
   newProgram.processID = idCounter;
 }

 newProgram.processAT = seconds; // Setting the arrival time to seconds (running clock)
 //Setting all time values to be 0 initially
 newProgram.processTAT = 0;
 newProgram.processWT = 0;
 newProgram.processDone = 0;
 newProgram.processCompletedTime = 0;

 ++idCounter; // increasing the counter
 ++queueSize; // Increasing the queue size as well

 return newProgram; // returning the new process
}
```



### Scheduler 
The schedule function runs in a wild look because the schedule will always be running on its own core thread. Function will call our scheduling algorithm which is round robin.
// Scheduler which is running on a separate thread is always running RR
```
void *Scheduler(void *arg)
{
 while (1)
 {
   RR();
 }
 return 0;
}

 pthread_t corethread_id; // Core thead to run the scheduler
 int corethread = pthread_create(&corethread_id, NULL, Scheduler, arg);
 if (corethread) // if corethread is > 0 imply could not create new thread
 {
   printf("\n ERROR: return code from pthread_create is %d \n", corethread);
   exit(EXIT_FAILURE);
 }

```



### Real time clock for Arrival time.
This functions allowed us to implement a real time stopwatch for arrival time which was used for each process as soon as it was received by the server and add it to the queue. We used a signal which with our alarm for one second and incremented a global seconds variable.
```
void handle(int sig)
{ // Handles the signal alarm
 seconds++;
 alarm(1); // Sends alarm signal after one second
}

   // Code for keeping track of time and clock
   setbuf(stdout, NULL);
   signal(SIGALRM, handle); // Assigns a handler for the alarm signal (in order for the time to keep running)
   alarm(1);
```

### Program Runing Simulation 
Program runner function allowed us to simulate a program which would run in the OS. For this the user specifies the time and priority if needed which is updated in the structure. The running structure is paused into the function along with the quantum which is used in the follow-up to run for that specified quantum seconds which is simulated by using sleep for one second and decrementing the process time.

Program runner also takes care of the case where the process is not finished it updates the time it has run for quantum and puts it back in the queue at the bottom thus preparing for a round robin to take place.
```
void programRunner(struct process *process, int quantum) // Process passes as pointer
{
 runningState = 1; // its running
 for (int i = 0; i < quantum; i++) // for each counter
 {
   sleep(1); // sleeping for 1 sec for simulation and visualization
   printf("%s running; Time left : %d\n", process->processName, process->processTime);
   if (process->processTime > 0) // if there is time left (avoid negative time)
   {
     --process->processTime; // Decrease the time
   }
   if (process->processTime == 0) // if the time has reached 0
   {
     process->processDone = 1; // Setting the flag to finished
     break; // breaking out of the for loop
   }
 }

 if (process->processTime != 0) // if it didn't finish
 {
   int tempTime = process->processTime; // keeping track of the time
   process->processAT = seconds; //updating arrival time since we are putting it back at the end of the queue
   process->processTime = tempTime;
   queue[queueSize] = *process; // Putting the process back at the bottom of the queue
   ++queueSize;// Increasing the queue size
 }
 removefromQueue(process->processID);  // removing by process ID
 runningState = 0; // Setting running state to 0
}
```


Similarly, we used command runner to implement the the command case where do use the word specify a Linux terminal command and that function was built upon all the code from our previous phases.

### Exiting Client using “exit” and ^C:
```
void clientExitHandler(int sig_num)
{
 char *exitString = "exit";
 // printf("\nExiting shell successfully \n");
 send(sock, exitString, strlen(exitString) + 1, 0); // sending exit message to server
 close(sock);                           // close the socket/end the connection
 printf("\n Exiting client.  \n");
 fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convenient function
 exit(0);
}


int main() // main function
{

 signal(SIGINT, serverExitHandler);
}
```

## Algorithms Used

The hybrid algorithm we used for this assignment was a combination of shortest job first and round robin algorithm. The round robin is based on a changing time quantum depending on the rounds you have run and we also implemented priority which the user can specify and the process will be given higher priority to run first in the queue when it gets assigned. 

Round Robin 
So that on robin function starts with the predefined time quantum which we said to five seconds then we get into checking if the key was not empty since our schedule is always running we are ensuring that round robin is always called on a separate thread so by checking in the queue is not empty we initiate the round robin functionality. By running a for loop for the queue size we are able to interate through the entire queue get access to each process at a time.

For the actual execution we are using destruct process flag which indicates if it's a program or command specified by the user and that's calls the respective runner code. We then pass the the process structure at that index along with a time quantum which is incremented accordingly.

In order to ensure that a process turns on a different quantum after finishing its initial around we are using a rounds counter variable inside the structure and incrementing it every single time it runs per quantum. This allows us to increment the quantum for that particular process every single time it runs each round.

Importantly we are locking each critical section where a program or command is being ran with a lock Mutex (global) which is acquired and released wherever our critical section (function) is being called. 
```
       pthread_mutex_lock(&lock); // acquiring the lock
       printf("Semaphore acquired programRunner\n");
       // fflush(stdout);
       queue[i].roundsCounter++;
       programRunner(&queue[i], time_quantum+queue[i].roundsCounter);
       printf("Semaphore Released programRunner\n");
       pthread_mutex_unlock(&lock); // releasing the lock
```


Lastly we update the time elapsed variable by adding the respective time that he has run and updating the process completed time accordingly.

     queue[i].processCompletedTime = time_quantum+queue[i].roundsCounter;



## RR Code
```
void RR()
{
 int time_elapsed = 0; // if time has passed to update completed time later
 int time_quantum = 5; // Declaring quantum to 5
 if (queueSize != 0) // If its not empty
 {
   for (int i = 0; i < queueSize; i++) // for each process in the queue
   {
    
     for (int j = i + 1; j < queueSize; j++) // loop for the next process
     {

       if (queue[i].processTime == queue[j].processTime) // if the time is the same for 2 processes
       {
         if (queue[i].processCompletedTime == 0) // if one of them have 0 completed time, meaning they haven't gotten a chance to run at all
         {
           // printf("\nDoing earlier job with same BT :%s\n", queue[i].processName);

           if (queue[i].processFlag == 1) // running the process
           {
             pthread_mutex_lock(&lock); // acquiring the lock
             printf("Semaphore acquired programRunner\n");
             runningState = 1;
             programRunner(&queue[i], time_quantum+queue[i].roundsCounter); // passing the process to the runner along with the quantum
             queue[i].roundsCounter++;
             printf("Semaphore Released programRunner\n");
             pthread_mutex_unlock(&lock); // releasing the lock
           }
         }
       }
     }
     if (queue[i].roundsCounter > 0) // this means it has already run ones and is here for the second time.
         {

           if (queue[i].processFlag == 1) // running the process
           {
             pthread_mutex_lock(&lock); // acquiring the lock
             printf("Semaphore acquired programRunner\n");
             runningState = 1;
             queue[i].roundsCounter++;
             programRunner(&queue[i], time_quantum+queue[i].roundsCounter); // passing the process to the runner along with the quantum
             printf("Semaphore Released programRunner\n");
             pthread_mutex_unlock(&lock); // releasing the lock
           }
         }

     // Running the round robin on the ith process since the queue has already been sorted
     if (queue[i].processFlag == 1) // if its a program it goes to programRunner
     {
       // sem_wait(&);
       pthread_mutex_lock(&lock); // acquiring the lock
       printf("Semaphore acquired programRunner\n");
       // fflush(stdout);
       queue[i].roundsCounter++;
       programRunner(&queue[i], time_quantum+queue[i].roundsCounter);
       printf("Semaphore Released programRunner\n");
       pthread_mutex_unlock(&lock); // releasing the lock
     }
     else
     { // else its a command it goes to commandRunner
       // sem_wait(&);
       pthread_mutex_lock(&lock); // acquiring the lock
       printf("Semaphore acquired commandRunner\n");
       // fflush(stdout);
       queue[i].roundsCounter++;
       commandRunner(&queue[i]);
       printf("Semaphore Released commandRunner\n");
       pthread_mutex_unlock(&lock); // releasing the lock
       // sem_post(&);
     }
     // printStats(); // printing the stats of the queue

     time_elapsed += queue[i].processTime;
     // Updating the completed time
     queue[i].processCompletedTime = time_quantum+queue[i].roundsCounter;
   }
 }
}
```



### Challenges:
The challenges faced in this phase were easier to analyze and debug.  

The biggest challenge he faced in this phase was with the use of semaphores and how most of the functions for it wouldn't work on macOS and many of its functions have been deprecated. 
We used named semaphores instead and primarily used mutex to lock our critical section. We were also unable to implement shared mutex across different c files. It kept giving us this error that we couldn't resolve;
clang: error: linker command failed with exit code 1 (use -v to see invocation)

A crucial error we faced initially was being unable to compute different threads parallely. Our program flow was not returning to the main. Although we fixed the error by sending data to the client through the sockets, we changed the implementation and no longer needed to go back to the main thread.

Another thing we struggled with was getting the thread to terminate and have the closing of the socket at the end so that the particular client will successfully quit and disconnect. Once we had this working we realized that the client could reconnect to the server and was assigned the same thread number. 

The exiting of the client using two different methods, ^c and “exit” gave us different results, which was an error. We figured this problem was caused by using the clientExitHandler() function that worked differently from our exit, and debugged it appropriately.
