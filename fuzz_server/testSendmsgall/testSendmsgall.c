#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "../server.h"

int main(){
  clnt_cnt = MAX_USER;
  clnt_socks[0] = open("./testoutput1", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[1] = open("./testoutput2", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[2] = open("./testoutput3", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[3] = open("./testoutput4", O_WRONLY|O_CREAT|O_TRUNC, 0644);

  char event_buf[128];
  fgets(event_buf,128,stdin);
  send_msg_all((void*)event_buf, strlen(event_buf));

  for(int i=0;i<4;i++){
    close(clnt_socks[i]);
  }
}