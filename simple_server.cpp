/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include "http_server.hh"
#include <signal.h>

void error(char *msg) {
  perror(msg);
  exit(1);
}


using namespace std;

pthread_t thread_pool[10];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

void * threadpool_func(void *arg);
void * handle_connection(void* p_client_socket);

void handleSignal(int dummy){
	exit(0);
}

int main(int argc, char *argv[]) {

	signal(SIGINT, handleSignal);
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    for(int i=0; i<10; i++) {
		pthread_create(&thread_pool[i], NULL, threadpool_func, NULL);
	}
	
  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */
  cout<<"+++++++++++ Listening +++++++++++++++++"<< endl;
  listen(sockfd, 10000);
  clilen = sizeof(cli_addr);
 
  /* accept a new request, create a newsockfd */
while(1){
  	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  	
  	if (newsockfd < 0)
   	error("ERROR on accept");

  	int *pclient = (int*)malloc(sizeof(int));
  	*pclient = newsockfd;
  	
  	pthread_mutex_lock(&mutex);
	enqueue(pclient);
	pthread_cond_signal(&condition_var);
	pthread_mutex_unlock(&mutex);
		
  	//free(pclient);
  }
  return 0;

}

void * threadpool_func(void *arg){

	while(true) {
		int *pclient;
		pthread_mutex_lock(&mutex);		
		if((pclient = dequeue()) == NULL) {
			pthread_cond_wait((pthread_cond_t*)&pclient, &mutex);
			pclient = dequeue();
		}
		pthread_mutex_unlock(&mutex);
		
		if(pclient != NULL) {
			handle_connection(pclient);			
		}
		//delete pclient;
	}
}

void * handle_connection(void* p_client_socket){
	int newsockfd = *((int*)p_client_socket);
	free(p_client_socket);	
	
	char buffer[1024];
	bzero(buffer, 1024);
	
 	int n = read(newsockfd, buffer, 255);

 	printf("Here is the message: %s", buffer);
  	HTTP_Response *abc = handle_request(buffer);
 
   	int size = abc->get_string().length();
  
  	
  	char response[1024]; 
  	bzero(response, 1024);	

  	strcpy(response, abc->get_string().c_str()); 		
   	write(newsockfd, response , strlen(response));
	
	close(newsockfd);
	delete abc;
    return NULL;
    
}
