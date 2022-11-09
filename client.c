#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
/*Ends*/

//CHAT Server IP address is considered to be local loop
//CHAT Server port is 2000
//Nickname is supposed to be of 50 BYTES
//Session Number starts with 0 upto 9
//If user wants to delete/block a buddy, then close the sessions opened with him.

int encrypt(char array[15000]);
int decrypt(char array[15000]);

char key[] = "hellousman";

struct session
{
	char O_SESS_NUM[4];	//session number of the originator who has
				//initiated this session
	int AddedFriends;	//Count to Measure the number of friends added
				//in the session
	
	/*This array will maintain the Primary Keys of the Freinds connected*/
	char Friends[5][4];	//Max number of friends allowed in a session
				//is 5
	/*Ends*/
	
	/*This array will maintain the Nick Names of the Freinds connected*/
	char Nick_Names[5][50];
	/*Ends*/
	
} Sessions[10];			//Max Number of sessions that a client
//can create at a time is 10


int main(int argc, char* argv[])
{
	/*File*/
	FILE* input;
	int counter = 0;
	char c;
	/*Ends*/

	/*Declarations for parsing*/
	char* REST;
	int rst_counter;
	/*Ends*/
	
	/*Socket Declarations*/
	int sockfd;
	struct sockaddr_in servaddr;
	/*Ends*/
	
	/*Miscellaneous Declarations*/	
	fd_set rset;
	int maxfd1 = 0;
	int Connected = 0;
	int temp = 0;
	int i;
	int j;	
	char UserName[50];
	char Password[50];	
	/*Ends*/
	
	/*Declarartions for Packet Formats*/
	char TYPE[2];
	char PRIM_KEY[4];
	char CLASS[2];
	char ADD_DEL[2];
	char B_UNB[2];	//(0 = block, 1 = unblock)
	char NUM_B_SESS[4];
	char NEW_NICK[50] = "mani";
	char SESS_NUM[4];
	char TEMP_NICK[50];
	char NICK_RECV[50];
	char MESSAGE[14800];
	
	char S_FINAL[15000];	//for sending
	char R_FINAL[15000];	//for receiving	
	/*Ends*/
	
	/*Declarartions for Packet Formats Initializations*/
	int k;
	char VALID[2];
	char NUM_BUDDIES[4];
	char BUDDYLIST[14800];
	char BUDDY[254][50];	//nick
	char P_KEY[254][4];	//primary key
	char ONLINE[254][2];	//status {1 = online, 0 = offline}	
	int buddylist_len = 0;
	/*Ends*/
	
	/*Declarartions for Console Input*/
	char COMMAND[20];	//command
	char ARG_1[50];		//first argument
	char ARG_2[50];
	/*Ends*/	
	
	/*Definitions for handling Sessions*/
	int SessCount = 0;	//counter to determine the number of 
				//friends connected in this session	
	/*Ends*/
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_port = htons(2000);
	servaddr.sin_family = AF_INET;
	
	if( inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) < 0 )
	{
		printf("Illegal IP");
		exit(0);
	}
	
	if( (sockfd = socket(AF_INET, SOCK_DGRAM,0)) < 0 )
	{
		printf("Socket Error");
		exit(0);
	}	
	
	if( connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
	{
		printf("Connection failed");
		exit(0);
	}
	
	FD_ZERO(&rset);		//setting rset to all zeros
	
	while(1)
	{	
start:			//start of execution
	
	rst_counter = 0;
	
	FD_SET(fileno(stdin), &rset);	//setting fd for console input
	FD_SET(sockfd, &rset);	//setting fd for socket
	
	maxfd1 = fileno(stdin);		//checking out for the maximum of them
	if(sockfd >= maxfd1)
		maxfd1 = sockfd;
	maxfd1 += 1;
		
	if(Connected == 0)
	{
		printf("Enter Your Username & Password\n");
		
		printf("Username: ");
		scanf("%s", UserName);
		
		printf("Password: ");
		scanf("%s" ,Password);
		
		strcpy(TYPE, "1");
		strcpy(CLASS, "1");			
		strcpy(MESSAGE, UserName);
		strcat(MESSAGE, " ");
		strcat(MESSAGE, Password);
		
		strcpy(S_FINAL, TYPE);
		strcat(S_FINAL, " ");
		strcat(S_FINAL, CLASS);
		strcat(S_FINAL, " ");
		strcat(S_FINAL, MESSAGE);
		
		encrypt(S_FINAL);
		write(sockfd, S_FINAL, 15000);			
		read( sockfd, R_FINAL, 15000);
		decrypt(R_FINAL);			
			
		rst_counter = 0;
		sscanf(R_FINAL,"%s %s %s %s", TYPE, CLASS, VALID, NUM_BUDDIES);
		rst_counter += (strlen(TYPE) + 1);
		rst_counter += (strlen(CLASS) + 1);
		rst_counter += (strlen(VALID) + 1);
		rst_counter += (strlen(NUM_BUDDIES) + 1);
		REST = (R_FINAL + rst_counter);
		strcpy(BUDDYLIST, REST);
		
		if( strcmp(TYPE, "1") == 0 )
		{
			if( strcmp(CLASS, "1") == 0 )
			{
				if(strcmp(VALID, "0") == 0)
				{
					Connected = 1;
					printf("Your Buddies are:\n");
					buddylist_len = atoi(NUM_BUDDIES);
					for(k = 0; k < buddylist_len; k++)
					{
						rst_counter = 0;
						sscanf(BUDDYLIST,"%s %s %s", BUDDY[k], P_KEY[k], ONLINE[k]);
						rst_counter += (strlen(BUDDY[k]) + 1);
						rst_counter += (strlen(P_KEY[k]) + 1);
						rst_counter += (strlen(ONLINE[k]) + 1);
						REST = (BUDDYLIST + rst_counter);
						strcpy(BUDDYLIST, REST);
						
						
						printf("%s ", BUDDY[k]);
						if( strcmp(ONLINE[k], "1") == 0)
							printf("ONLINE\n");
						else
							printf("OFFLINE\n");	
					}
					continue;					
				}
				else
				{
					printf("Invalid User Name and/or Password\n");
					continue;
				}	
			}
			else
			{
				printf("Invalid Packet Recieved\n");
				continue;
			}
		}
	}
	
	select(maxfd1, &rset, NULL, NULL, NULL);
	
	if( FD_ISSET(sockfd, &rset) )	//RECEIVING SIDE
	{
		read(sockfd, R_FINAL, 15000);

		decrypt(R_FINAL);
		
		rst_counter = 0;
		sscanf(R_FINAL, "%s", TYPE);
		rst_counter += (strlen(TYPE) +1);
		REST = (R_FINAL + rst_counter);
		
		strcpy(MESSAGE, REST);
				
		if( strcmp(TYPE, "2") == 0 )		//message received
		{
			rst_counter = 0;
			sscanf(MESSAGE, "%s %s", SESS_NUM, NICK_RECV);
			rst_counter += (strlen(SESS_NUM) + 1);
			rst_counter += (strlen(NICK_RECV) + 1);
			REST = (MESSAGE + rst_counter);
			
			strcpy(MESSAGE, REST);
			
			for(i = 0; i < SessCount; i++)
			{
				if( strcmp(SESS_NUM, Sessions[i].O_SESS_NUM) == 0 )
				{
					printf("Session Number %d\n", i);
					for(j = 0; j < Sessions[i].AddedFriends; j++)
						printf("%s ", Sessions[i].Nick_Names[j]);
					
					printf("\n%s says: %s\n", NICK_RECV, MESSAGE);
					break;
				}
			}		
		}
	
		if( strcmp(TYPE, "3") == 0 )		//file received
		{
			counter = 0;

			rst_counter = 0;
			sscanf(MESSAGE, "%s %s", SESS_NUM, NICK_RECV);
			rst_counter += (strlen(SESS_NUM) + 1);
			rst_counter += (strlen(NICK_RECV) + 1);
			REST = (MESSAGE + rst_counter);
			
			strcpy(MESSAGE, REST);
	
			input = fopen("received.txt", "w");
			
			while(1)
			{
				c = MESSAGE[counter++];
				if(c == '$')
					break;
				fputc(c, input);					
			}
			
			fclose(input);					
		}
		
		else if( strcmp(TYPE, "1") == 0 )	//control packet received
		{
			
			rst_counter = 0;
			sscanf(MESSAGE, "%s", CLASS);
			rst_counter += (strlen(CLASS) +1);
			REST = (MESSAGE + rst_counter);
			
			strcpy(MESSAGE, REST);			
			
			
			if( strcmp(CLASS, "6") == 0 )	//buddylist received
			{
				rst_counter = 0;
				sscanf(MESSAGE, "%s", NUM_BUDDIES);
				rst_counter += (strlen(NUM_BUDDIES) +1);
				REST = (MESSAGE + rst_counter);
				
				strcpy(BUDDYLIST, REST);				
				buddylist_len = atoi(NUM_BUDDIES);
				
				printf("BUDDY LIST RECEIVED\n");				
				for(k = 0; k < buddylist_len; k++)
				{
					rst_counter = 0;
					sscanf(BUDDYLIST,"%s %s %s", BUDDY[k], P_KEY[k], ONLINE[k]);
					rst_counter += (strlen(BUDDY[k]) +1);
					rst_counter += (strlen(P_KEY[k]) +1);
					rst_counter += (strlen(ONLINE[k]) +1);
				
					printf("%s ", BUDDY[k]);
					
					if(strcmp(ONLINE[k], "1") == 0 )
						printf("ONLINE\n");
					else
						printf("OFFLINE\n");
								
					REST = (BUDDYLIST + rst_counter);
					
					strcpy(BUDDYLIST, REST);
				}					
			}
			
			else if( strcmp(CLASS, "8") == 0 )	//session deletion from friend
			{
				sscanf(MESSAGE, "%s %s", SESS_NUM, NICK_RECV);
				
				for(i = 0; i < SessCount; i++)
				{
					if( strcmp(SESS_NUM, Sessions[i].O_SESS_NUM) == 0 )
					{
						for(j = i; j < (SessCount-1); j++)
						{
							strcpy(Sessions[j].O_SESS_NUM, Sessions[j+1].O_SESS_NUM);
							Sessions[j].AddedFriends = Sessions[j+1].AddedFriends;
							for(k = 0; k < Sessions[j].AddedFriends; k++)
							{ 
								strcpy(Sessions[j].Nick_Names[k], Sessions[j+1].Nick_Names[k]);
								strcpy(Sessions[j].Friends[k], Sessions[j+1].Friends[k]);
							}
						}
						SessCount--;
						break;
					}
				}
			}
			
			else if( strcmp(CLASS, "7") == 0 )	//session creation request
			{
				sscanf(MESSAGE, "%s %s %s %s", SESS_NUM, NUM_B_SESS, NICK_RECV, TEMP_NICK);
				
				strcpy(Sessions[SessCount].O_SESS_NUM, SESS_NUM);
				for(i = 0; i < buddylist_len; i++)
				{
					if( strcmp(NICK_RECV, BUDDY[i]) == 0)
					{
						strcpy(PRIM_KEY, P_KEY[i]);
						break;
					}
				}
				strcpy(Sessions[SessCount].Friends[0], PRIM_KEY);
				strcpy(Sessions[SessCount].Nick_Names[0], NICK_RECV);
				Sessions[SessCount].AddedFriends = 1;
				SessCount++;
				
				printf("Session Received from: %s\n", NICK_RECV);
			}
			
			else if( strcmp(CLASS, "0") == 0)	//add in a session request
			{
				rst_counter = 0;
				sscanf(MESSAGE, "%s %s %s", SESS_NUM, NUM_B_SESS, NICK_RECV);
				rst_counter += (strlen(SESS_NUM) + 1);
				rst_counter += (strlen(NUM_B_SESS) + 1);
				rst_counter += (strlen(NICK_RECV) + 1);
				REST = (MESSAGE + rst_counter);
				
				strcpy(MESSAGE, REST);
				
				if(SessCount == 0)
				{
					strcpy(Sessions[SessCount].O_SESS_NUM, SESS_NUM);

					for(i = 0; i < buddylist_len; i++)
					{
						if( strcmp(NICK_RECV, BUDDY[i]) == 0)
						{
							strcpy(PRIM_KEY, P_KEY[i]);
							break;
						}
					}
					strcpy(Sessions[SessCount].Friends[0], PRIM_KEY);
					strcpy(Sessions[SessCount].Nick_Names[0], NICK_RECV);
					Sessions[SessCount].AddedFriends = 1;
					
					for(j = 0; j < (atoi(NUM_B_SESS) - 1); j++)
					{
						rst_counter = 0;
						sscanf(MESSAGE, "%s", NICK_RECV);
						rst_counter += (strlen(NICK_RECV) + 1);
						REST = (MESSAGE + rst_counter);
	
						strcpy(MESSAGE, REST);
						
						for(i = 0; i < buddylist_len; i++)
						{
							if( strcmp(NICK_RECV, BUDDY[i]) == 0)
							{
								strcpy(PRIM_KEY, P_KEY[i]);
								break;
							}								
						}
						strcpy(Sessions[SessCount].Friends[j + 1], PRIM_KEY);
						strcpy(Sessions[SessCount].Nick_Names[j + 1], NICK_RECV);
						Sessions[SessCount].AddedFriends++;					
					}
	
					SessCount++;
				
					printf("Session Received from: %s\n", NICK_RECV);					
				}
				
				else
				{
					for(i = 0; i < SessCount; i++)
					{
						if( strcmp(SESS_NUM, Sessions[i].O_SESS_NUM) == 0 )
						{
							temp = atoi(NUM_B_SESS);
							
							for(j = 0; j < temp; j++)
							{
								rst_counter = 0;
								sscanf(MESSAGE, "%s", TEMP_NICK);
								rst_counter += (strlen(TEMP_NICK) + 1);
								REST = (MESSAGE + rst_counter);
							
								strcpy(MESSAGE, REST);
							}
						
							temp = Sessions[i].AddedFriends;
						
							for(j = 0; j < buddylist_len; j++)
							{
								if( strcmp(BUDDY[j], TEMP_NICK) == 0 )
								{
									
									strcpy(Sessions[i].Friends[temp], P_KEY[j]);
									break;								
								}
							}
							
							strcpy(Sessions[i].Nick_Names[temp], TEMP_NICK);
							Sessions[i].AddedFriends++;
	
							printf("Session Received from: %s\n", NICK_RECV);
						}
					}
				}					
			}
		}		
	}
		
	else 
	{
		printf("%s", "Enter Command\n");
		/*User Input starts over here*/
		
		printf("COMMAND: ");
		scanf("%s", COMMAND);
		
		printf("ARG 1: ");
		scanf("%s", ARG_1);
		
		printf("ARG 2: ");
		scanf("%s", ARG_2);

		if( strcmp(COMMAND, "file") == 0 )
		{
			if(ARG_1 != NULL)
			{
				if(ARG_2 != NULL)
				{
					for(j = 0; j < Sessions[atoi(ARG_1)].AddedFriends; j++)
					{
						input = fopen(ARG_2, "r");						
						
						strcpy(TYPE, "3");
						strcpy(PRIM_KEY, Sessions[atoi(ARG_1)].Friends[j]);
						strcpy(SESS_NUM, Sessions[atoi(ARG_1)].O_SESS_NUM);
						strcpy(MESSAGE, NEW_NICK);
						strcat(MESSAGE, " ");
						
						counter = (strlen(MESSAGE) + 1);
					
						while(1)
						{
							c = fgetc(input);	
							if(c == EOF)
							{
								MESSAGE[counter++] = '$';					
								counter = 0;
								break;
							}
							MESSAGE[counter++] = c;						
						}
				
						fclose(input);
												
						strcpy(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, PRIM_KEY);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, SESS_NUM);
						strcat(S_FINAL, " ");	
						strcat(S_FINAL, MESSAGE);	
						
						encrypt(S_FINAL);
			
						write(sockfd, S_FINAL, 15000);				
					}					
				}								
			}				
		}
		
		else if( strcmp(COMMAND, "session") == 0)
		{
			printf(":::OPENED SESSIONS:::\n");
			
			for(i = 0; i < SessCount; i++)
			{			
				printf("\nSession No. %d\n", i);			
				for(j = 0; j < Sessions[i].AddedFriends; j++)
					printf("| %s", Sessions[i].Nick_Names[j]);
				printf("\n");
			}				
		}
		
		else if( strcmp(COMMAND, "offline") == 0 )
		{
			//when user wants to become offline
			//first of all he must notify all his friends
			//in all his sessions
			
			/*Deleting all the sessions*/
			for(i = 0; i < SessCount; i++)
			{
				for(k = 0; k < Sessions[i].AddedFriends; k++)
				{
					strcpy(TYPE, "1");
					strcpy(CLASS, "8");
					strcpy(PRIM_KEY, Sessions[i].Friends[k]);
					strcpy(SESS_NUM, Sessions[i].O_SESS_NUM);
					strcpy(MESSAGE, NEW_NICK);
					
					strcpy(S_FINAL, TYPE);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, CLASS);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, PRIM_KEY);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, SESS_NUM);
					strcat(S_FINAL, " ");
					strcat(S_FINAL, MESSAGE);
					
					encrypt(S_FINAL);

					write(sockfd, S_FINAL, 15000);
				}
				/*Ends*/
			}
			
			strcpy(TYPE, "1");
			strcpy(CLASS, "9");
			strcpy(MESSAGE, COMMAND);
			
			strcpy(S_FINAL, TYPE);
			strcat(S_FINAL, " ");
			strcat(S_FINAL, CLASS);
			strcat(S_FINAL, " ");
			strcat(S_FINAL, MESSAGE);
			
			encrypt(S_FINAL);

			write(sockfd, S_FINAL, 15000);
			
			exit(0);
		}
			
		else if( strcmp(COMMAND, "createsession") == 0 )
		{		
			for(i = 0; i < buddylist_len; i++)
			{
				if( strcmp( ARG_1, BUDDY[i]) == 0)
				{
					if( strcmp( ONLINE[i], "1") == 0)
					{
						snprintf(Sessions[SessCount].O_SESS_NUM, 4,"%d", SessCount);
						strcpy( Sessions[SessCount].Friends[0], P_KEY[i]);
						strcpy( Sessions[SessCount].Nick_Names[0], ARG_1);
						Sessions[SessCount].AddedFriends = 1;
						
						strcpy(TYPE, "1");
						strcpy(CLASS, "7");
						strcpy(PRIM_KEY, P_KEY[i]);
						strcpy(SESS_NUM, Sessions[SessCount].O_SESS_NUM);
						strcpy(NUM_B_SESS, "2");
						strcpy(MESSAGE, NEW_NICK);
						strcat(MESSAGE, " ");
						strcat(MESSAGE, ARG_1);
						
						strcpy(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, CLASS);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, PRIM_KEY);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, SESS_NUM);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, NUM_B_SESS);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, MESSAGE);
							encrypt(S_FINAL);
						
						write(sockfd, S_FINAL, 15000);
						
						printf("Session Created\n");
						SessCount++;
					}
					else 
					{
						printf("User is not online\n");
						//goto start;
					}
				}
			}
		}
			
			else if( strcmp(COMMAND, "deletesession") == 0)
			{
				if(ARG_1 != NULL)
				{			
					for(k = 0; k < Sessions[atoi(ARG_1)].AddedFriends; k++)
					{
						strcpy(TYPE, "1");
						strcpy(CLASS, "8");
						strcpy(PRIM_KEY, Sessions[atoi(ARG_1)].Friends[k]);
						strcpy(SESS_NUM, Sessions[atoi(ARG_1)].O_SESS_NUM);
						strcpy(MESSAGE, NEW_NICK);
						
						strcpy(S_FINAL, TYPE);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, CLASS);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, PRIM_KEY);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, SESS_NUM);
						strcat(S_FINAL, " ");
						strcat(S_FINAL, MESSAGE);
						
						encrypt(S_FINAL);

						write(sockfd, S_FINAL, 15000);					
					}
					
					for(i = atoi(ARG_1); i < (SessCount - 1); i++)
					{				
						strcpy( Sessions[i].O_SESS_NUM ,Sessions[i+1].O_SESS_NUM);
						Sessions[i].AddedFriends = Sessions[i+1].AddedFriends;
						
						for(k = 0; k < Sessions[i].AddedFriends; k++)
						{
							strcpy(Sessions[i].Friends[k], Sessions[i+1].Friends[k]);
							strcpy(Sessions[i].Nick_Names[k],Sessions[i+1].Nick_Names[k]);						
						}					
					}
					
					SessCount--;					
					printf("Session Deleted\n");
				}
				else
				{
					printf("Enter a Session Number to Delete");					
				}
			}
			
			else if( strcmp(COMMAND, "addinsession") == 0)				
			{
				if(ARG_1 != NULL)
				{
					if(ARG_2 != NULL)
					{
						for(i = 0; i < buddylist_len; i++)
						{
							if((strcmp(ARG_2, BUDDY[i]) == 0)&&(strcmp(ONLINE[i], "1") == 0))
							{
								temp = Sessions[atoi(ARG_1)].AddedFriends;
								strcpy(Sessions[atoi(ARG_1)].Friends[temp], P_KEY[i]);
								strcpy(Sessions[atoi(ARG_1)].Nick_Names[temp], BUDDY[i]);
								Sessions[atoi(ARG_1)].AddedFriends++;
								
								break;
							}						
						}	
						
						for(k = 0; k < Sessions[atoi(ARG_1)].AddedFriends; k++)
						{
							strcpy(TYPE, "1");
							strcpy(CLASS, "0");
							strcpy(PRIM_KEY, Sessions[atoi(ARG_1)].Friends[k]);			
							strcpy(SESS_NUM, Sessions[atoi(ARG_1)].O_SESS_NUM);
							
							snprintf(NUM_B_SESS, 4, "%d", Sessions[atoi(ARG_1)].AddedFriends);
							strcpy(MESSAGE, NEW_NICK);
							
							for(j = 0; j < Sessions[atoi(ARG_1)].AddedFriends; j++)
							{	
								strcat(MESSAGE, " ");
								strcat(MESSAGE, Sessions[atoi(ARG_1)].Nick_Names[j]);
							}
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, CLASS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, PRIM_KEY);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, SESS_NUM);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, NUM_B_SESS);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, MESSAGE);
							
							encrypt(S_FINAL);						

							write(sockfd, S_FINAL, 15000);						
						}//for loop ends				
						
					}
					else
					{
						printf("Enter a Buddy Nick Name to be added\n");					
					}//ARG_2 ends
					
				}//ARG_1 ends
				else
				{
					printf("Enter a Valid Session Number\n");					
				}			
			}
			
			else if( strcmp(COMMAND, "nick") == 0 )	//command to change nick
			{
				strcpy(TYPE, "1");
				strcpy(CLASS, "2");
				
				if(ARG_1 != NULL)
					strcpy(NEW_NICK, ARG_1);
				else
				{
					printf("Enter a Valid nick");
					goto start;
				}
				
				strcpy(MESSAGE, NEW_NICK);
				
				strcpy(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, CLASS);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				
				encrypt(S_FINAL);

				write(sockfd, S_FINAL, 15000);						
			}
						
			else if( strcmp(COMMAND, "buddylist") == 0)//command to request buddylist
			{
				strcpy(TYPE, "1");
				strcpy(CLASS, "6");
				
				strcpy(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, CLASS);
			
				encrypt(S_FINAL);
				
				write(sockfd, S_FINAL, 15000);			
			}
			
			else if( strcmp(COMMAND, "send") == 0)
			{
				if(ARG_1 != NULL)
				{
					if(ARG_2 != NULL)
					{
						printf("%s\n", ARG_2);////////////////////
							
						for(j = 0; j < Sessions[atoi(ARG_1)].AddedFriends; j++)
						{
							strcpy(TYPE, "2");
							strcpy(PRIM_KEY, Sessions[atoi(ARG_1)].Friends[j]);
							strcpy(SESS_NUM, Sessions[atoi(ARG_1)].O_SESS_NUM);
							strcpy(MESSAGE, NEW_NICK);
							strcat(MESSAGE, " ");
							strcat(MESSAGE, ARG_2);
							
							strcpy(S_FINAL, TYPE);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, PRIM_KEY);
							strcat(S_FINAL, " ");
							strcat(S_FINAL, SESS_NUM);
							strcat(S_FINAL, " ");	
							strcat(S_FINAL, MESSAGE);	
							
							encrypt(S_FINAL);

							write(sockfd, S_FINAL, 15000);				
						}					
					}
					else
					{
						printf("Enter a message");					
					}			
				}
				else
				{
					printf("Enter a Session Number to whom you want to send message");				
				}
			}
			else
			{
				printf("Invalid Command");							
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


