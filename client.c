#include <stdio.h>		//printf
#include <string.h>		//strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>		//write

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

#define NB_CARS 3
#define NB_ROUNDS 2

/****PROTOTYPE****/

void initialise_generateur_aleatoire();
int gettid();
int randomizer(int, int);

/****************/

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char message[1000], server_reply[2000];

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("connection failed. Error");
		return 1;
	}

	puts("Connected\n");

	/*   APP   */

	int nbr_rounds = NB_ROUNDS;
	int i, duration, time_spent = 0;
	initialise_generateur_aleatoire();
	printf("Running car #%d\n", gettid());

	int light1 = 1;
	int light2 = 1;

	for (int i = 0; i < nbr_rounds; i++)
	{

		/* section OA */
		duration = randomizer(1, 3);
		time_spent += duration;
		sleep(duration);
		/* section AB */
		duration = randomizer(2, 7);
		time_spent += duration;
		sleep(duration);
		/* section BO */
		duration = randomizer(1, 3);
		time_spent += duration;
		sleep(duration);

		char message[100] = "1";

		if (send(sock, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}

		//Receive a reply from the server
		if (recv(sock, server_reply, 2000, 0) < 0)
		{
			puts("recv failed");
		}

		/* section OC */
		duration = randomizer(1, 3);
		time_spent += duration;
		sleep(duration);
		/* section CD */
		duration = randomizer(3, 5);
		time_spent += duration;
		sleep(duration);
		/* section DO */
		duration = randomizer(1, 3);
		time_spent += duration;
		sleep(duration);

		strcpy(message, "2");

		if (send(sock, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
		}

		//Receive a reply from the server
		if (recv(sock, server_reply, 2000, 0) < 0)
		{
			puts("recv failed");
		}

		printf("Car #%d, Round  #%d, time to reach the center:%d sec\n", gettid(), i + 1, time_spent);
	}

	printf("Car #%d\n has reached the finish line with total time of %d", gettid(),time_spent);

	/*         */
	close(sock);
	return 0;
}

int gettid()
{
	return syscall(SYS_gettid);
}
/*** initalise le generateur aleatoire.*/
void initialise_generateur_aleatoire()
{
	srandom(time(NULL) + getpid());
}

/*** Retourne une valeur aleatoire entre
* min (inclus) et max (inclus).*/
int randomizer(int min, int max)
{
	int val;
	float f = random() * (max - min);
	f = (f / RAND_MAX) + min;
	return (f + 0.5);
}
