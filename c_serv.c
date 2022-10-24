#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

//CHAT server will run on the port 2000
//CHAT server will send auth + db server on port 2001
//CHAT server will receive from auth + db server from port 2002

int encrypt(char array[15000]);
int decrypt(char array[15000]);

char key[] = "hellousman";

struct user
{
	struct sockaddr_in useraddr;
	socklen_t user_len;
	char prim_key[4];

}USERS[255];

int main(int argc, char* argv[])
{
	/*Declarations for parsing*/
	char* REST;
	int rst_counter;
	/*Ends*/

	/*Miscellaneous Declarations*/
	int i, j, k;
	/*Ends*/
	
	/*Declarations to manipulate users*/
	int num_users = 0;
	/*Ends*/
	
	/*socket declarations to communicate with clients*/
	int clientfd;
	struct sockaddr_in clientaddr;
	socklen_t client_len;
	
	struct sockaddr_in dummy_clientaddr;
	socklen_t dummy_client_len = sizeof(dummy_clientaddr);
	/*Ends*/
	
	/*socket declarations for Sending & Receiving from Authentication + db server*/
	int servfd;	
	struct sockaddr_in sendaddr;
	struct sockaddr_in receiveaddr;
	
	char R_FINAL[15000];
	char S_FINAL[15000];
	/*Ends*/
	
	
	/*declarations for packet format*/
	char MACHINE[2];
	char TYPE[2];
	char CLASS[2];
	char VALID[2];
	char ADD_DEL[2];
	char PRIM_KEY[4];
	char TEMP_NICK[50];	
	
	char MESSAGE[14800];
	/*Ends*/
	
	
	/*Creating the Receiving interface with Auth + db server*/	
	if( ( servfd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 )
	{
		printf("Receiving Socket error");
		exit(0);
	}
	
	bzero( &receiveaddr, sizeof(receiveaddr) );	
	receiveaddr.sin_port = htons(2002);
	receiveaddr.sin_family = AF_INET;	
	receiveaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	
	if( ( bind(servfd, (struct sockaddr*) &receiveaddr, sizeof(receiveaddr)) ) < 0 )
	{
		printf("bind error");
		exit(0);
	}
	/*Ends*/
	
	/*Creating the Sending interface with Auth + db server*/	
	bzero( &sendaddr, sizeof(sendaddr) );	
	sendaddr.sin_port = htons(2001);
	sendaddr.sin_family = AF_INET;
	
	//IP address of Auth + db server
	if( ( inet_pton(AF_INET, "127.0.0.1", &sendaddr.sin_addr) ) < 0 )
	{
		printf("Illegal IP");
		exit(0);
	}
	
	if( ( connect(servfd, (struct sockaddr*) &sendaddr, sizeof(sendaddr)) ) < 0 )
	{
		printf("Connection failed");
		exit(0);
	}
	/*Ends*/
	
	/*Creating the communication interface with clients*/
	if( ( clientfd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 )
	{
		printf("Server Socket error");
		exit(0);
	}
	
	bzero( &clientaddr, sizeof(clientaddr) );	
	clientaddr.sin_port = htons(2000);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	
	if( ( bind(clientfd, (struct sockaddr*) &clientaddr, sizeof(clientaddr)) ) < 0 )
	{
		printf("Server Socket bind error");
		exit(0);
	}
	/*Ends*/
	
	while(1)
	{
		rst_counter = 0;

		recvfrom(clientfd, R_FINAL, 15000, 0, (struct sockaddr*)&dummy_clientaddr, &dummy_client_len);

		decrypt(R_FINAL);
		
		printf("%s\n", R_FINAL);
		
		sscanf(R_FINAL, "%s %s", TYPE, CLASS);
		rst_counter += (strlen(TYPE) + 1);
		rst_counter += (strlen(CLASS) + 1);
		REST = (R_FINAL + rst_counter);
		strcpy(MESSAGE, REST);
		
		if( strcmp(TYPE, "1") == 0 )	//Control message
		{
			if( strcmp(CLASS, "1") == 0 )	//Initialization request from client
			{
				strcpy(MACHINE, "1");
				strcpy(TYPE, "1");
				
				strcpy(S_FINAL, MACHINE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				
				encrypt(S_FINAL);

				write(servfd, S_FINAL, 15000);	//send the request to Auth + db Server
				
				read(servfd, R_FINAL, 15000);

				decrypt(R_FINAL);
				
				rst_counter = 0;

				sscanf(R_FINAL, "%s %s %s %s", MACHINE, TYPE, PRIM_KEY, VALID);
				rst_counter += (strlen(MACHINE) + 1);
				rst_counter += (strlen(TYPE) + 1);
				rst_counter += (strlen(PRIM_KEY) + 1);
				rst_counter += (strlen(VALID) + 1);
				REST = (R_FINAL + rst_counter);
				strcpy(MESSAGE, REST);
				
				if( strcmp(MACHINE, "2") == 0 )
				{
					if(strcmp(TYPE, "1") == 0)
					{
						if( (strcmp(PRIM_KEY, "400") == 0) || (strcmp(VALID, "1") == 0) )//invalid username or pswd
						{
							strcpy(TYPE, "1");
							strcpy(CLASS, "1");																
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, CLASS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, VALID);
							
							encrypt(S_FINAL);
								sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &dummy_clientaddr, dummy_client_len);
						}
						else	//valid user							
						{
							strcpy(USERS[num_users].prim_key, PRIM_KEY);
							
							bzero( &(USERS[num_users].useraddr), sizeof(USERS[num_users].useraddr) );	
							USERS[num_users].useraddr.sin_port = dummy_clientaddr.sin_port;
							USERS[num_users].useraddr.sin_family = dummy_clientaddr.sin_family;
							USERS[num_users].useraddr.sin_addr.s_addr = dummy_clientaddr.sin_addr.s_addr;
							
							USERS[num_users].user_len = dummy_client_len;
							
							num_users++;
							
							strcpy(TYPE, "1");
							strcpy(CLASS, "1");
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, CLASS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, VALID);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, MESSAGE);
							
							encrypt(S_FINAL);
							sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &dummy_clientaddr, dummy_client_len);
								
							for(i = 0; i < (num_users - 1); i++)
							{
								
								strcpy(MACHINE, "1");
								strcpy(TYPE, "3");
								strcpy(PRIM_KEY, USERS[i].prim_key);
								
								strcpy(S_FINAL, MACHINE);
								strcat(S_FINAL, " ");
								strcat(S_FINAL, TYPE);
								strcat(S_FINAL, " ");
								strcat(S_FINAL, PRIM_KEY);
								
								encrypt(S_FINAL);
								write(servfd, S_FINAL, 15000);	//send the request to Auth + db Server
								
								read(servfd, R_FINAL, 15000);

								decrypt(R_FINAL);
									
								rst_counter = 0;
								sscanf(R_FINAL, "%s %s %s", MACHINE, TYPE, PRIM_KEY);
								rst_counter += (strlen(MACHINE) + 1);
								rst_counter += (strlen(TYPE) + 1);
								rst_counter += (strlen(PRIM_KEY) + 1);
								REST = (R_FINAL + rst_counter);
								
								strcpy(MESSAGE, REST);
										
								strcpy(TYPE, "1");
								strcpy(CLASS, "6");
								
								strcpy(S_FINAL, TYPE);
								strcat(S_FINAL, " ");
								strcat(S_FINAL, CLASS);
								strcat(S_FINAL, " ");
								strcat(S_FINAL, MESSAGE);
								encrypt(S_FINAL);

								sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
							}
								
						}
					}
						
				}				
			}//initializations ends
			
			else if( strcmp(CLASS, "8") == 0 )	//delete session request to a friend
			{
				rst_counter = 0;
				sscanf(MESSAGE, "%s", PRIM_KEY);
				rst_counter += (strlen(PRIM_KEY) +1);
				REST = (MESSAGE + rst_counter);				
				strcpy(MESSAGE, REST);
				
				strcpy(TYPE, "1");
				strcpy(CLASS, "8");
				
				strcpy(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, CLASS);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				
				for(i = 0; i < num_users; i++)
				{
					printf("%s %s\n", USERS[i].prim_key, PRIM_KEY);
					
					if( strcmp(PRIM_KEY, USERS[i].prim_key) == 0 )
					{	
						encrypt(S_FINAL);

						sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
						break;
					}
				}
			}
			
			else if( strcmp(CLASS, "9") == 0)	//offline request
			{
				for(i = 0; i < num_users; i++)
				{
					//finding the user from which the request has come
					if( (dummy_clientaddr.sin_addr.s_addr == USERS[i].useraddr.sin_addr.s_addr) &&
						(dummy_clientaddr.sin_port == USERS[i].useraddr.sin_port) )
					{
						strcpy(MACHINE, "1");
						strcpy(TYPE, "2");
						strcpy(MESSAGE, USERS[i].prim_key);
						
						strcpy(S_FINAL, MACHINE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, MESSAGE);
						
						printf("%s\n", S_FINAL);

						encrypt(S_FINAL);
						
						write(servfd, S_FINAL, 15000);
						
						for(j = i; j < (num_users - 1); j++)
						{
							strcpy(USERS[j].prim_key, USERS[j+1].prim_key);
							bzero( &(USERS[j].useraddr), sizeof(USERS[j].useraddr) );	
							USERS[j].useraddr.sin_port = USERS[j+1].useraddr.sin_port;
							USERS[j].useraddr.sin_family = USERS[j+1].useraddr.sin_family;
							USERS[j].useraddr.sin_addr.s_addr = USERS[j+1].useraddr.sin_addr.s_addr;
							USERS[j].user_len = USERS[j+1].user_len;
						}
						
						num_users--;

						for(k = 0; k < num_users; k++)
						{						
							strcpy(MACHINE, "1");
							strcpy(TYPE, "3");
							strcpy(PRIM_KEY, USERS[k].prim_key);
							
							strcpy(S_FINAL, MACHINE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, PRIM_KEY);
							
							encrypt(S_FINAL);
							write(servfd, S_FINAL, 15000);	//send the request to Auth + db Server
							
							read(servfd, R_FINAL, 15000);
							decrypt(R_FINAL);
							
							rst_counter = 0;
							sscanf(R_FINAL, "%s %s %s", MACHINE, TYPE, PRIM_KEY);
							rst_counter += (strlen(MACHINE) + 1);
							rst_counter += (strlen(TYPE) + 1);
							rst_counter += (strlen(PRIM_KEY) + 1);
							REST = (R_FINAL + rst_counter);
							strcpy(MESSAGE, REST);
								
							strcpy(TYPE, "1");
							strcpy(CLASS, "6");
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, CLASS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, MESSAGE);
							
							encrypt(S_FINAL);
							sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[k].useraddr, USERS[k].user_len);	
						}
					break;
					}				
				}
			}
			
			else if( strcmp(CLASS, "7") == 0)	//session creation request for friend
			{
				rst_counter = 0;
				sscanf(MESSAGE, "%s", PRIM_KEY);
				rst_counter += (strlen(PRIM_KEY) + 1);
				REST = (MESSAGE + rst_counter);

				printf("%s\n", MESSAGE);
				
				strcpy(MESSAGE, REST);
				
				printf("%s\n", MESSAGE);
				
				strcpy(TYPE, "1");
				strcpy(CLASS, "7");
				
				strcpy(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, CLASS);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				
				for(i = 0; i < num_users; i++)
				{
					printf("%s %s\n", USERS[i].prim_key, PRIM_KEY);
					
					if( strcmp(USERS[i].prim_key, PRIM_KEY) == 0 )
					{
						encrypt(S_FINAL);
						
						printf("%s\n", S_FINAL);

						sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
						break;
					}
				}
			}			
			
			else if( strcmp(CLASS, "0") == 0 )	//add in session request for a friend
			{
				rst_counter = 0;
				sscanf(MESSAGE, "%s", PRIM_KEY);
				rst_counter = (strlen(PRIM_KEY) + 1);
				REST = (MESSAGE + rst_counter);

				strcpy(MESSAGE, REST);
					
				strcpy(TYPE, "1");
				strcpy(CLASS, "0");
				
				strcpy(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, CLASS);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				
				for(i = 0; i < num_users; i++)
				{
					if( strcmp(USERS[i].prim_key, PRIM_KEY) == 0 )
					{
						encrypt(S_FINAL);

						sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
						break;
					}					
				}
			}

			else if( strcmp(CLASS, "2") == 0)	//change nick request
			{
				for(i = 0; i < num_users; i++)
				{
					//finding the user from which the request has come
					if( (dummy_clientaddr.sin_addr.s_addr == USERS[i].useraddr.sin_addr.s_addr) &&
						(dummy_clientaddr.sin_port == USERS[i].useraddr.sin_port) )
					{
						strcpy(MACHINE, "1");
						strcpy(TYPE, "4");
						strcpy(TEMP_NICK, MESSAGE);

						strcpy(MESSAGE, USERS[i].prim_key);
						strcat(MESSAGE, " ");
						strcat(MESSAGE, TEMP_NICK);
						
						strcpy(S_FINAL, MACHINE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, MESSAGE);
						
						encrypt(S_FINAL);

						write(servfd, S_FINAL, 15000);
						
						for(i = 0; i < num_users; i++)		//now after changing the nick server has to send the 
															//updated buddy list of each user to him
						{
							
							strcpy(MACHINE, "1");
							strcpy(TYPE, "3");
							strcpy(PRIM_KEY, USERS[i].prim_key);
							
							strcpy(S_FINAL, MACHINE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, PRIM_KEY);
							
							encrypt(S_FINAL);

							write(servfd, S_FINAL, 15000);	//send the request to Auth + db Server
							
							read(servfd, R_FINAL, 15000);

							decrypt(R_FINAL);
							
							rst_counter = 0;
							sscanf(R_FINAL, "%s %s %s", MACHINE, TYPE, PRIM_KEY);
							rst_counter += (strlen(MACHINE) + 1);
							rst_counter += (strlen(TYPE) + 1);
							rst_counter += (strlen(PRIM_KEY) + 1);
							REST = (R_FINAL + rst_counter);
							
							strcpy(MESSAGE, REST);
								
							strcpy(TYPE, "1");
							strcpy(CLASS, "6");
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, CLASS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, MESSAGE);
							
							encrypt(S_FINAL);
								
							sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
						}
						
					}
					
				}
			}
				
			else if( strcmp(CLASS, "6") == 0 )
			{
				for(i = 0; i < num_users; i++)
				{
					if( (dummy_clientaddr.sin_addr.s_addr == USERS[i].useraddr.sin_addr.s_addr) &&
						(dummy_clientaddr.sin_port == USERS[i].useraddr.sin_port) )
					{
						strcpy(MACHINE, "1");
						strcpy(TYPE, "3");
						strcpy(PRIM_KEY, USERS[i].prim_key);
						
						strcpy(S_FINAL, MACHINE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, PRIM_KEY);
						
						encrypt(S_FINAL);
						
						write(servfd, S_FINAL, 15000);	//send the request to Auth + db Server
							
						read(servfd, R_FINAL, 15000);

						decrypt(R_FINAL);
							
						rst_counter = 0;
						sscanf(R_FINAL, "%s %s %s", MACHINE, TYPE, PRIM_KEY);
						rst_counter += (strlen(MACHINE) + 1);
						rst_counter += (strlen(TYPE) + 1);
						rst_counter += (strlen(PRIM_KEY) + 1);
						REST = (R_FINAL + rst_counter);
						
						strcpy(MESSAGE, REST);
							
						strcpy(TYPE, "1");
						strcpy(CLASS, "6");
						
						strcpy(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, CLASS);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, MESSAGE);
						
						encrypt(S_FINAL);
							
						sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);					
						break;
					}					
				}				
			}
		}

		else if( strcmp(TYPE, "2") == 0 )
		{
			for(i = 0; i < num_users; i++)
			{
				printf("%s %s\n", CLASS, USERS[i].prim_key);
				
				if( strcmp(CLASS, USERS[i].prim_key) == 0 )
				{
					strcpy(TYPE, "2");
					
					strcpy(S_FINAL, TYPE);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, MESSAGE);
					
					printf("%s\n", MESSAGE);

					encrypt(S_FINAL);

					sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
				}
			}
		}
		
		else if( strcmp(TYPE, "3") == 0 )
		{
			for(i = 0; i < num_users; i++)
			{
				printf("%s %s\n", CLASS, USERS[i].prim_key);
				
				if( strcmp(CLASS, USERS[i].prim_key) == 0 )
				{
					strcpy(TYPE, "3");
					
					strcpy(S_FINAL, TYPE);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, MESSAGE);
					
					printf("%s\n", MESSAGE);

					encrypt(S_FINAL);

					sendto(clientfd, S_FINAL, 15000, 0, (struct sockaddr*) &USERS[i].useraddr, USERS[i].user_len);
				}
			}
		}	
	}	
	return 0;
}

int encrypt(char array[15000])
{
	int i, j, counter = 0;

	for(i = 0; i < 1500; i++)	
		for(j = 0; j < 10; j++)
		{
			array[counter] = (array[counter] ^ key[j]);
			counter++;
		}

		return 1;
}

int decrypt(char array[15000])
{
	int i, j, counter = 0;

	for(i = 0; i < 1500; i++)	
		for(j = 0; j < 10; j++)
		{
			array[counter] = (array[counter] ^ key[j]);
			counter++;
		}

		return 1;
}
