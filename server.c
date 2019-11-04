#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>	//write
#include <pthread.h>   //for threading , link with lpthread

#define NB_CARS 3
#define NB_ROUNDS 2

pthread_t pthread_id[NB_CARS];
pthread_t pthread_light;
pthread_mutex_t lock;

int terminee = 0;
int light1 = 0;
int light2 = 1;

void *connection_handler(void *);
void *toggleState();

int main(int argc, char *argv[])
{
	int socket_desc, client_sock, c, *new_sock;
	struct sockaddr_in server, client;
	int nbrConnection = 0;

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	pthread_create(&pthread_light, NULL, toggleState, NULL);

	//Bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
	{
		nbrConnection++;
		puts("New Connection accepted");
		printf("Number of connected clients %d \n", nbrConnection);
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;

		if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		pthread_join(pthread_light, NULL);
		return 1;
	}
	pthread_join(pthread_light, NULL);
	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int *)socket_desc;
	int read_size;
	char *message, client_message[2000];

	//Send some messages to the client

	//Receive a message from client
	while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
	{
		puts(client_message);
		//Send the message back to client

		if ((*client_message) == '1')
		{
			puts("Request to pass from direction 1\n");
			pthread_mutex_lock(&lock);

			if (light1 == 1)
			{
				strcpy(client_message, "Yes");
				puts("Green light");
			}

			pthread_mutex_unlock(&lock);
		}
		if ((*client_message) == '2')
		{
			puts("Request to pass from direction 2\n");
			pthread_mutex_lock(&lock);

			if (light2 == 1)
			{
				strcpy(client_message, "Yes");
				puts("Green light");
			}

			pthread_mutex_unlock(&lock);
		}

		pthread_mutex_unlock(&lock);

		write(sock, client_message, strlen(client_message));
	}

	if (read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}

	//Free the socket pointer
	free(socket_desc);

	return 0;
}

void *toggleState()
{
	int *ptr1 = &light1;
	int *ptr2 = &light2;

	while (terminee == 0)
	{
		pthread_mutex_lock(&lock);
		*(ptr1) = !(light1);
		*(ptr2) = !(light2);
		pthread_mutex_unlock(&lock);
		sleep(2);
		printf("%d %d\n", light1, light2);
	}
}
