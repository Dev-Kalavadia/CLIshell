// OS Phase 4 CPU Scheduling: Multithread Simulation
// Dev and Soumen

#include <unistd.h>     // / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>      // header for input and output from console : printf, perror
#include <stdlib.h>     // header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>     // header for string functions declarations: strlen()
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <poll.h>
#include <pthread.h>   // header for thread functions declarations: pthread_create, pthread_join, pthread_exit
#include <signal.h>    // header for signal related functions and macros declarations
#include <sys/time.h>  // For time and clock
#include <fcntl.h>     // header for definations of the flag values such as O_CREAT
#include <semaphore.h> // header for POSIX semaphore implementation related functions declarations: sem_wait()

// Including the header files
#include "print.h"
#include "read.h"
#include "splitter.h"
#include "checkpipe.h"
#include "singlecommand.h"
#include "pipe.h"
#include "client.h"

#define clear() printf("\033[H\033[J") // Clearing the screen
#define PORT 5555                      // client server port number
#define BUFFER_SIZE 4096               // We made the buffer big at first to allow much as ouput possible (for long strings and command)
#define NUM_CLIENTS 10
#define MAX_QUEUE_SIZE 100

#define PROGRAMNAME "@mp" // Progame name starting identifier 

sem_t *sem1, *runningBlock, *mutex; // Declaring semaphores
pthread_mutex_t lock; // Declaring Mutex

//Global Variables
int queueSize = 0;
int idCounter = 0;
int runningState = 0;
int seconds = 0;

// Forward Declaration
void printStats();

//Structure to hold each process detail
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
  int roundsCounter;
  // pthread threadID;
};

struct process queue[MAX_QUEUE_SIZE];

//Running a clock as soon as the program start to keep track of arrival time
void handle(int sig)
{ // Handles the signal alarm
  seconds++;
  alarm(1); // Sends alarm signal after one second
}

// void printQueue()
// {
//   printf("-----------------QUEUE----------------------- \n");
//   printf(" ____________________________________________\n");
//   printf("|Process Name| AT | BT |Process ID|Socket ID|\n");
//   printf("|____________|____|____|__________|_________|\n");

//   for (int i = 0; i < queueSize; i++)
//   {
//     printf("|%-12s|%-4d|%-4d|%-10d|%-9d|\n", queue[i].processName, queue[i].processAT, queue[i].processTime, queue[i].processID, queue[i].socket);
//     // avg_waiting_time += queue[i].processWT;
//   }

//   printf("|____________|____|____|__________|_________|\n");

//   printf("--------------END-QUEUE----------------------- \n");
// }

// Sorting the Queue by the program time (burse time) 
// Basically shortest time first implementation is done through this 
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
}

// Sorting the queue by priority (by default it is 0 for all process)
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

void removefromQueue(int ID)
{
  int index = 0;
  for (int j = 0; j < queueSize; j++)
  {
    if (queue[j].processID == ID)
    {
      index = j;
      for (int i = index; i < queueSize - 1; i++)
      {
        queue[i] = queue[i + 1]; // Removing from queue
      }
      // printf("Done removing from Queue \n");
      --queueSize; // Decreasing the size
      break; // braking out of the for loop
    }
  }

  // printStats();
}

// Adding each process to the queue
struct process addToQueue(char *command, int socket) // Command from client and it socket number
{
  struct process newProgram; // Creating a new struct
  char *ret; // Check our return string to compare
  ret = strstr(command, PROGRAMNAME); // Lookgin for @mp in command
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
    newProgram.roundsCounter = 0; 
  }

  newProgram.processAT = seconds; // Setting the arrival time to seconds (running clock)
  //Setting all time values to be 0 intially 
  newProgram.processTAT = 0;
  newProgram.processWT = 0;
  newProgram.processDone = 0;
  newProgram.processCompletedTime = 0;

  ++idCounter; // increasing the counter
  ++queueSize; // Increasing the queue size as well

  return newProgram; // returning the new process
}

// Exit interrupt handler (ctrl+c)
void serverExitHandler(int sig_num)
{
  char *exitString = "srvexit";
  printf("\n Exiting server.  \n");
  fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty continent function
  exit(0);
}

// If its a program it should run this 
void programRunner(struct process *process, int quantum) // Process passes as pointer
{
  runningState = 1; // its running
  for (int i = 0; i < quantum; i++) // for each counter 
  {
    sleep(1); // sleeping for 1 sec for simulation and visualization
    printf("%s running; Time left : %d\n", process->processName, process->processTime); // Printing info for the user to see on terminal 
    if (process->processTime > 0) // if there is time left (avoid negative time)
    {
      --process->processTime; // Decrease the time
    }
    if (process->processTime == 0) // if the time has reached 0
    {
      // Program is done running 
      process->processDone = 1; // Setting the flag to finished
      break; // breaking out of the for loop
    }
  }

  if (process->processTime != 0) // if it didnt finish
  {
    // struct process temp = process;
    int tempTime = process->processTime; // keeping track of the time 
    process->processAT = seconds; //updating arrival time since we are putting it back at the end of the queue

    process->processTime = tempTime; 
    queue[queueSize] = *process; // Putting the process back at the bottom of the queue
    ++queueSize;// Increasing the queue size
  }
  // char donestr[] = ("#done\n");
  // send(process->socket, donestr, strlen(donestr), 0);
  // If reached here it is done so remove from the queue
  removefromQueue(process->processID);  // removing by process ID 

  runningState = 0; // Settign running state to 0
}

// Command runner is it is command 
// Basically most of the core code from previous phases
void commandRunner(struct process *arg) // passing the process 
{
  runningState = 1; // Setting it to running 
  int socket = arg->socket; // Specify the socket to an int for recv and send to the specific socket
  int connectFlag = 0;                //
  int commandFlag = 0, pipeFlag = -1; // Flags to control the pipe and command input functions
  // char *command[BUFFER_SIZE] = {0};                                                        // command is the string input read from the user
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;

  char *command = arg->processName;
  pipeFlag = checkPipe(command); // Checks the number of pipes in the input and returns an Integer
  // char donestr[] = ("#done\n");
  // send(arg->socket, donestr, strlen(donestr), 0);

  if (pipeFlag == 0)
  { // If there are no pipes

    arguments = lineSplitter(command); // get the arguments array from the line spliter (parsing)
    commandExecute(arguments, socket); // execute the single command
    free(arguments);                   // free the arguments after using since the array was dynamically allocated
  }
  else if (pipeFlag > 0) // if there is a pipe
  {
    if (pipeFlag > 3)
    {
      printf(">Sorry, You have entered more than 3 pipes\n");
      return;
    }

    int pipeflag = 0;

    pipe_cmd = pipeSplitter(command, pipeflag); // use the pipe splitter to get the array of commands parsed by the pipe delimiter

    if (pipeFlag == 1)
    { // if there is one pipe
      // Basically we are getting the whole string split from the pipe and then splitting that further example "ls -l" -> ["ls","-l"]
      argument1 = lineSplitter(pipe_cmd[0]); // getting the first argument from the pipe slitter and splitting that further into single arguments
      argument2 = lineSplitter(pipe_cmd[1]);

      if (argument1[0] != NULL && argument2[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
      {
        singlepipeExecute(argument1, argument2, socket); // send the 2 arguments into the single pipe function
      }
      else
      {
        char pipeerror[] = (">Null Argument was detected. Please try again\n");
        send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
        return;
      }
      // printf("Executed single pipe \n");
      free(argument1); // free the arguments after using since they were dynamically allocated
      free(argument2);
    }

    if (pipeFlag == 2)
    { // if there is two pipe  and the input is not null
      argument1 = lineSplitter(pipe_cmd[0]);
      argument2 = lineSplitter(pipe_cmd[1]);
      argument3 = lineSplitter(pipe_cmd[2]);

      if (argument1[0] != NULL && argument2[0] != NULL && argument3[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL && pipe_cmd[2] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
      {
        // singlepipeExecute(argument1, argument2, socket); // send the 2 arguments into the single pipe function
        doublepipeExecute(argument1, argument2, argument3, socket); // send the 3 arguments into the double pipe function
      }
      else
      {
        char pipeerror[] = (">Null Argument was detected. Please try again\n");
        send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
        return;
      }

      free(argument1); // free the arguments after using since they were dynamically allocated
      free(argument2);
      free(argument3);
    }

    if (pipeFlag == 3)
    { // if there is three pipe and the input is not null
      argument1 = lineSplitter(pipe_cmd[0]);
      argument2 = lineSplitter(pipe_cmd[1]);
      argument3 = lineSplitter(pipe_cmd[2]);
      argument4 = lineSplitter(pipe_cmd[3]);
      if (argument1[0] != NULL && argument2[0] != NULL && argument3[0] != NULL && argument4[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL && pipe_cmd[2] != NULL && pipe_cmd[3] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
      {
        triplepipeExecute(argument1, argument2, argument3, argument4, socket); // send the 4 arguments into the triple pipe function
        return;
      }
      else
      {
        char pipeerror[] = (">Null Argument was detected. Please try again\n");
        send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
        return;
      }

      free(argument1); // free the arguments after using since they were dynamically allocated
      free(argument2);
      free(argument3);
      free(argument4);
    }
  }
  
  // Command has finsihed so remove from queue and update the flag
  removefromQueue(arg->processID);
  arg->processDone = 1;
  // Sleep for a second for simulation 
  sleep(1); 
  return; // Returning back to the caller
}

// RR code for all the scheduling 
// Not all our RR logic is handled here since we have it broken done in multiple places but all the fundamental features of RR has been implemented in this code
void RR()
{
  int time_elapsed = 0; // if time has passed to update completed time later
  int time_quantum = 4; // Declaring quantum to 5
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

void printStats()
{
  printf(" _______________________________________________________\n");
  printf("|Process Name| AT | BT |Process ID|Socket ID| Priority |\n");
  printf("|____________|____|____|__________|_________|__________|\n");

  for (int i = 0; i < queueSize; i++)
  {
    printf("|%-12s|%-4d|%-4d|%-10d|%-9d|%-10d|\n", queue[i].processName, queue[i].processAT, queue[i].processTime, queue[i].processID, queue[i].socket, queue[i].processPriority);
    // avg_waiting_time += queue[i].processWT;
  }

  printf("|____________|____|____|__________|_________|__________|\n");
}

// Scheduler which is running on a separate thread is always running RR
void *Scheduler(void *arg)
{
  while (1)
  {
    RR();
  }
  return 0;
}

// Function that handle dedicated communication with a client
void *HandleClient(void *new_socket)
{

  // pthread_detach(pthread_self()); // detach the thread as we don't need to synchronize/join with the other client threads, their execution/code flow does not depend on our termination/completion
  int socket = *(int *)new_socket;
  free(new_socket);
  printf("Handling new client in a thread using socket: %d\n", socket);
  printf("Listening to client..\n"); // while printing make sure to end your strings with \n or \0 to flush the stream, other wise if in anyother concurent process is reading from socket/pipe-end with standard input/output redirection, it will keep on waiting for stream to end.

  int i = 0; // to keep track of messages received
  // variable such as message buffers to receive and send messages
  char message[1024] = {0};  // to send message
  char message2[1024] = {0}; // to receive message
  int data_len;              // for flusing the received message

  int connectFlag = 0;                //
  int commandFlag = 0, pipeFlag = -1; // Flags to control the pipe and command input functions
  // char *command[BUFFER_SIZE] = {0};                                                        // command is the string input read from the user
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;                                                      // getting the pipe Command
  char buffer[BUFFER_SIZE];
  // clear(); // Clearing the screen

  pthread_detach(pthread_self()); // detach the thread as we don't need to synchronize/join with the other client threads, their execution/code flow does not depend on our termination/completion

  while (1)
  {
    // bzero(command, BUFFER_SIZE);
    char command[BUFFER_SIZE];
    bzero(command, sizeof(command));

    recv(socket, command, sizeof(command), 0); // receive message from client

    char *exitString = "exit";
    if (strcmp(command, exitString) == 0)
    { // comapring the command entered to the exit string
      printf(">Client on socket %d has disconnected.\n", socket);
      connectFlag = 0;
      close(socket);
      pthread_exit(NULL); // terminate the thread
    }
    // printf("Handling new client in a thread using socket: %d\n", socket);

    if (strcmp(command, "\0") == 0)
    {
      continue;
    }
    // After receiving the command it will be added to thr queue at the bottom 
    queue[queueSize] = addToQueue(command, socket);
    int ID = queue[queueSize].processID; // Setting a new ID for this added process

    sortQueue(); // Sorting the queue based on the time 
    sortQueueByPriority(); // Sorting the queue based on the priority  
    // printQueue();
    // pthread_exit(NULL);

    printf(">Server Received using socket %d: %s\n", socket, command); // print the received message
    printStats(); // to see the process added to the queue

    // while(1){
    //   // sleep(1);
    //   int found = 0;
    //   // printf("@@@@ %d", ID);
    //   for (int i = 0; i < queueSize; i++)
    //   {
    //     if (ID == queue[i].processID)
    //     {   
    //       found = 1;
    //       break;
    //     }
    //   }
    //   if (found == 1){
    //     char waitstr[] = ("#wait\n");
    //     send(socket, waitstr, strlen(waitstr), 0);
    //     // printf("still in q");
    //     continue;
    //   }
    //   else{
    //   // printf("done from q");
    //   break;
    //   }
    // }
  }
}

int main() // main function
{
  // sem_init(&mutex,0,1);
  mutex = sem_open("/semaphorenamed", O_CREAT, 0644, 1); // Declaring named semaphores
  runningBlock = sem_open("/semaphorenamed", O_CREAT, 0644, 1);


  int connectFlag = 0;
  int commandFlag = 0, pipeFlag = -1; // Flags to control the pipe and command input functions
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;                                                      // getting the pipe Command
  char buffer[BUFFER_SIZE];
  clear(); // Clearing the screen

  printf("\tWelcome to D&S CPU Scheduler: Multithread Simulation \n");
  printf("\t --------------------------------------------------------\n");
  signal(SIGINT, serverExitHandler);

  int sock1, sock2, valread;
  struct sockaddr_in address;    // structure for storing addres; local interface and port
  int addrlen = sizeof(address); // Getting the size off the address

  if (pthread_mutex_init(&lock, NULL) != 0) // Mutex init
  {
    printf("Mutex init has failed\n");
    return 1;
  }

  // Creating the file descripter for the socket to allow communication with the domain of Internet protocol and the socket stream for reliable connection
  if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) // checking if socket creation fail
  {
    perror(">Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Setting and defining the address for the socket to bind on
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(PORT);

  // Socket is being attached to the address on the specified address's  (local IP and port)
  if (bind(sock1, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror(">Binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(sock1, NUM_CLIENTS) < 0) // defining for number of clients in queue
  {
    perror("Listen Failed");
    exit(EXIT_FAILURE);
  }
  char *arg;
  pthread_t corethread_id; // Core thead to run the scheduler 
  int corethread = pthread_create(&corethread_id, NULL, Scheduler, arg);
  if (corethread) // if corethread is > 0 imply could not create new thread
  {
    printf("\n ERROR: return code from pthread_create is %d \n", corethread);
    exit(EXIT_FAILURE);
  }

  while (1) // to keep server alive forever
  {
    // Code for keeping track of time and clock
    setbuf(stdout, NULL);
    signal(SIGALRM, handle); // Assigns a handler for the alarm signal (in order for the time to keep running)
    alarm(1);

    printf(">Waiting for Client to connect\n"); // Server is listening and waiting for connection
    // accepting the client's connection with the creation of the new socket and that's establishes a connection between a client and server
    if ((sock2 = accept(sock1, (struct sockaddr *)&address,
                        (socklen_t *)&addrlen)) < 0)
    {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    // connectFlag = 1; // Setting connection flag to be true
    printf(">Successfully Connected: Listening to client.\n");

    pthread_t thread_id, t;                       // the thread ID which is used to identify new thread
    int *new_socket = (int *)malloc(sizeof(int)); // Dynamically allocated memory for the new socket
    if (new_socket == NULL)
    {
      fprintf(stderr, "Couldn't allocate memory for thread new socket argument.\n");
      free(new_socket); // Freeing the dynamically allocated socket number
      exit(EXIT_FAILURE);
    }
    *new_socket = sock2;

    printf("New Client has Connected!\n");

    int catch = 0;

    // Using semaphore to wait on each handle client 
    sem_wait(runningBlock);
    int rc = pthread_create(&thread_id, NULL, HandleClient, new_socket);
    sem_post(runningBlock);

    if (rc) // if rc is > 0 imply could not create new thread
    {
      printf("\n ERROR: return code from pthread_create is %d \n", rc);
      free(new_socket); // Freeing the dynamically allocated socket number
      exit(EXIT_FAILURE);
    }

  }
  pthread_mutex_destroy(&lock); // Freeing the mutex
  // Destroying the semaphores
  sem_close(runningBlock);
  sem_close(mutex);
  sem_close(sem1);

  close(sock1);
  pthread_exit(NULL); // terminate the main thread
}