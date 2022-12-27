#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int signum){
  printf("Hello World!\n");
}

int main(){

  //declare a struct sigaction
  struct sigaction action, oldaction;

  //set the handler
  action.sa_handler = handler;

  //call sigaction with the action structure
  sigaction(SIGALRM, &action, &oldaction); // When the SIGALRM signl is called, replace the old action by action but 
                                          // also return the old action so I know what the previous action was.

  //schedule an alarm
  alarm(1); // After 1s send the SIGALRM

  //pause
  pause();

  //call sigaction with the action structure. Restore the ction
  sigaction(SIGALRM, &oldaction, NULL);

  //schedule an alarm
  alarm(1);

  pause();
}
