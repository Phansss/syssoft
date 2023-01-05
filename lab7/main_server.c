/**
 * \author Pieter Hanssens
 */

#include "main_server.h"


#define PORT 5678       // Port to listen to
#define MAX_LEN_COMM 80 // Max length of shell commands

#define SHELL_INSTRUCTIONS "\n ********** LAB 7 MAIN SERVER **********\n   'start' --> start listening to PORT\n   'add'   --> add a sensor\n   'stop'  --> stop listening\n   'clear' --> clear the shell\n   'exit'  --> exit the server\n ***************************************\n"
#define SHELL "\n   server_shell>" // Default shell command line prompt
#define SHELL_LISTEN "\n   server_shell (listening)>"

#define COMM_LISTEN "start"     //
#define COMM_EXIT "exit"        //  
#define COMM_ADD_SENSOR "add"   // Available shell commands
#define COMM_STOP "stop"        //
#define COMM_CLEAR "clear"      //
#define COMM_PS "ps"

#define EXEC_ADD_SENSOR "./build/sensor_node" // Path to the sensor_node executable

#define EXIT_TIMEOUT 1 // time (in s) to wait before the shell exits after COMM_EXIT call

/**
 * Implements a sequential test server (only one connection at the same time)
 */

int main(void) {

    char prog_name[MAX_LEN_COMM+1] = "";
    char *format_spec;
    pid_t* child_pid_connmgr;
    (*child_pid_connmgr) = 0;


    ASPRINTF_ERROR( asprintf( &format_spec, "%%%ds", MAX_LEN_COMM) );
    system("clear");
    printf(SHELL_INSTRUCTIONS);
    printf(SHELL);
    while ( 1 ) {
        
        // input argument read
        if (scanf(format_spec, prog_name) == 1 ) { 


            // Exit the shell
            if (  strcmp( prog_name, COMM_EXIT ) == 0 ) {
                if ((*child_pid_connmgr) != 0) {
                  stop_connmgr((&child_pid_connmgr));
                }
                atexit( final_message );
                free(format_spec);
                exit(0);
            }

            // Start listening
            if (  strcmp(prog_name, COMM_LISTEN ) == 0 ) {
                child_pid_connmgr =  connmgr_start(PORT); // start listening
                printf(SHELL_LISTEN);
            }

            // Stop listening
            if (  strcmp(prog_name, COMM_STOP ) == 0 ) {
                
                stop_connmgr((*child_pid_connmgr));
                (*child_pid_connmgr) = 0;
                free(child_pid_connmgr);
                printf(SHELL);
            }

            // Clear the shell
            if (  strcmp(prog_name, COMM_CLEAR ) == 0 ) {
                system("clear");
                printf(SHELL_INSTRUCTIONS);
                printf(SHELL);
            }

            // Show running processes the shell
            if (  strcmp(prog_name, COMM_PS ) == 0 ) {
                pid_t child_pid;
                child_pid = fork();
                SYSCALL_ERROR( child_pid );

                if ( child_pid != 0)
                {
                  printf("\n      SHELL PS: listing active processes:\n\n");
                  sleep(1);
                  printf(SHELL);
                }
                else //child code
                { 
                  char* arg_list[] = { COMM_PS, "au", NULL };
                  execvp (COMM_PS, arg_list);  // run subprogram
                  // should never come here ...
                  perror("\nAn error occurred in execvp");
                  exit(EXIT_FAILURE);
                }

            }
            
            // Add a sensor
            if (strncmp(prog_name, COMM_ADD_SENSOR, strlen(COMM_ADD_SENSOR))== 0 ) {
                if ((*child_pid_connmgr) != 0) {
                  kill((*child_pid_connmgr), SIGSTOP); //pause the listening process

                  char sid [] = "000";
                  char sleep_time[] = "000";
                  char server_port[] = "00000";
                  char server_ip[] = "000.000.000.000";
                  printf("       enter sid: ");
                  scanf("%4s", sid);

                  printf("       enter sleep_time: ");
                  scanf("%4s", sleep_time);

                  printf("       enter ip: ");
                  scanf("%16s", server_ip);

                  printf("       enter server port: ");
                  scanf("%6s", server_port);

                  add_sensor_node(sid, sleep_time, server_ip, server_port);
                  kill((*child_pid_connmgr), SIGCONT); //continue the listening process
                } else {
                  printf("\n      SHELL ADD: cannot add a sensor while not listening!\n");
                  printf(SHELL);
                } 

            
            
            }  
        } 
    }
}


int fork_and_listen(int port) 
{
  pid_t child_pid;
  child_pid = fork();
  SYSCALL_ERROR( child_pid );
  
  if ( child_pid != 0)
  {
     printf("\n      SHELL LISTEN: start listening process (pid: %d) on port %d:\n", child_pid, PORT);
    return child_pid;  // return to main_server shell
  }
  else 
  { 
    //connmgr_listen(port); // start listening
    return 0;
  }
}


void final_message(void) 
{
  pid_t pid = getpid();
  printf("      SHELL FINAL: Exiting  server_shell (%d) in %d seconds...\n", pid, EXIT_TIMEOUT);
  printf("      SHELL FINAL: bye bye ...\n");
  sleep(EXIT_TIMEOUT);
  system("clear");
}

int add_sensor_node(char* sid, char* st, char* server_ip, char* port) {
  
      pid_t child_pid = 0;
      child_pid = fork();
      SYSCALL_ERROR( child_pid );
      // printf("Child Process executing sensor_node with argc[]: %s %s %s %s", sid, st, server_ip, port);

      if ( child_pid != 0)
      {
        return child_pid;  // return to parent code
      }
      else 
      {
        printf("Child Process executing sensor_node with argc[]: %s %s %s %s", sid, st, server_ip, port);
        
        char* arg_list[] = { EXEC_ADD_SENSOR, sid, st, server_ip, port, NULL };
        char* arg_list_none[] = { EXEC_ADD_SENSOR, NULL };
        if (strcmp(sid,"0") == 0) {
           execvp (EXEC_ADD_SENSOR, arg_list_none);  // run subprogram with no parameters
        }
        execvp (EXEC_ADD_SENSOR, arg_list);  // run subprogram
        // should never come here ...
        perror("\nAn error occurred in execvp\n");
        exit(EXIT_FAILURE);
      }

      return 0;
    }

int stop_connmgr(pid_t** conn_mgr_pid) {
    if ((**conn_mgr_pid) != 0) { // Check whether the shell is listening (! not parent code ! )
      int child_exit_status = 0;
      free_connmgr();
      kill(**conn_mgr_pid, SIGTERM); //SIGTERM used if you want to clean up. SIGKILL is for when te child does not want to listen.
      SYSCALL_ERROR(waitpid(conn_mgr_pid, &child_exit_status, 0)); // Wait for the listening process to die
      printf("\n      SHELL STOP: stop listening process (pid: %d).  (exit status: %d)\n", conn_mgr_pid, child_exit_status);
      listen_terminate_child();
      return 0;
    } else {
      printf("\n      SHELL STOP: not listening!\n");
      return 0;
    }
}





