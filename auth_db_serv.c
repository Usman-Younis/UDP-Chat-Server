/*Header Directives*/
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
//Receiving port is 2001
//Sending port is 2002

int encrypt(char array[15000]);
int decrypt(char array[15000]);

char key[] = "hellousman";

struct buddylist
{
	char b_user_name[50];	//user name of buddy
	char b_nick[50];	//current nick of buddy
	char b_prim_key[4];	//primary key of the buddy
	char b_status[2];		//status of buddy (1 = online, 0 = offline)
	char b_unblocked[2];	//(0 = blocked, 1 = unblocked)
	
};

struct user
{
	char user_name[50];
	char password[50];
	char prim_key[4];
	int buddylist_len;
	struct buddylist buddies[254];
}users[255];

int main(int argc, char* argv[])
{
	/*user number 1*/
	strcpy( users[0].user_name, "usman" );
	strcpy( users[0].password, "mani" );
	strcpy( users[0].prim_key, "0" );
	users[0].buddylist_len = 2;
	
	strcpy( users[0].buddies[0].b_user_name, "khurram" );
	strcpy( users[0].buddies[0].b_nick, "khurram" );	
	strcpy( users[0].buddies[0].b_prim_key, "1" );
	strcpy( users[0].buddies[0].b_status, "0" );
	strcpy( users[0].buddies[0].b_unblocked, "1" );
	
	strcpy( users[0].buddies[1].b_user_name, "maryam" );
	strcpy( users[0].buddies[1].b_nick, "maryam" );	
	strcpy( users[0].buddies[1].b_prim_key, "2" );
	strcpy( users[0].buddies[1].b_status, "0" );
	strcpy( users[0].buddies[1].b_unblocked, "1" );
	/*Ends*/
	
	/*user number 2*/
	strcpy( users[1].user_name, "khurram" );
	strcpy( users[1].password, "khurram" );
	strcpy( users[1].prim_key, "1" );
	users[1].buddylist_len = 2;
	
	strcpy( users[1].buddies[0].b_user_name, "usman" );
	strcpy( users[1].buddies[0].b_nick, "mani" );	
	strcpy( users[1].buddies[0].b_prim_key, "0" );
	strcpy( users[1].buddies[0].b_status, "0" );
	strcpy( users[1].buddies[0].b_unblocked, "1" );
	
	strcpy( users[1].buddies[1].b_user_name, "maryam" );
	strcpy( users[1].buddies[1].b_nick, "maryam" );	
	strcpy( users[1].buddies[1].b_prim_key, "2" );
	strcpy( users[1].buddies[1].b_status, "0" );
	strcpy( users[1].buddies[1].b_unblocked, "1" );
	/*ends*/
	
	/*user number 3*/
	strcpy( users[2].user_name, "maryam" );
	strcpy( users[2].password, "maryam" );
	strcpy( users[2].prim_key, "2" );
	users[2].buddylist_len = 2;
	
	strcpy( users[2].buddies[0].b_user_name, "usman" );
	strcpy( users[2].buddies[0].b_nick, "mani" );	
	strcpy( users[2].buddies[0].b_prim_key, "0" );
	strcpy( users[2].buddies[0].b_status, "0" );
	strcpy( users[2].buddies[0].b_unblocked, "1" );
	
	strcpy( users[2].buddies[1].b_user_name, "khurram" );
	strcpy( users[2].buddies[1].b_nick, "khurram" );	
	strcpy( users[2].buddies[1].b_prim_key, "1" );
	strcpy( users[2].buddies[1].b_status, "0" );
	strcpy( users[2].buddies[1].b_unblocked, "1" );
	/*Ends*/
	
	/*Declarations for parsing*/
	char* REST;
	int rst_counter;
	/*Ends*/

	/*Miscellaneous Declarations*/
	int i, j, k, l, x;
	char u_name[50];
	char u_pswd[50];
	/*Ends*/
	
	/*socket declarations for Sending & Receiving from Chat server*/
	int authfd, val;	
	struct sockaddr_in sendaddr;
	struct sockaddr_in receiveaddr;
	
	char R_FINAL[15000];
	char S_FINAL[15000];
	/*Ends*/
	
	
	/*declarations for packet format*/
	char MACHINE[2];
	char TYPE[2];
	char VALID[2];
	char PRIM_KEY[4];
	char NUM_BUDDIES[4];
	char TEMP_NICK[50];
	
	char MESSAGE[14800];
	/*Ends*/
	
	/*Creating the Server*/	
	if( ( authfd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 )
	{
		printf("Receiving Socket error");
		exit(0);
	}
	
	bzero( &receiveaddr, sizeof(receiveaddr) );	
	receiveaddr.sin_port = htons(2001);
	receiveaddr.sin_family = AF_INET;
	receiveaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	val = bind(authfd, (struct sockaddr*) &receiveaddr, sizeof(receiveaddr));
	if( val < 0 )
	{
		printf("bind error");
		exit(0);
	}
	/*Ends*/
	
	/*Creating the Client*/	
	bzero( &sendaddr, sizeof(sendaddr) );	
	sendaddr.sin_port = htons(2002);
	sendaddr.sin_family = AF_INET;
	
	//IP address of chat server
	if( inet_pton(AF_INET, "127.0.0.1", &sendaddr.sin_addr) < 0 )
	{
		printf("Illegal IP");
		exit(0);
	}
	
	if( connect(authfd, (struct sockaddr*) &sendaddr, sizeof(sendaddr)) < 0 )
	{
		printf("Connection failed");
		exit(0);
	}
	/*Ends*/
	
	while(1)
	{	
		rst_counter = 0;

		read(authfd, R_FINAL, 15000);
		
		
		printf("%s\n", R_FINAL);
		
		decrypt(R_FINAL);
		x = htons(2001);
		
		sscanf(R_FINAL, "%s %s", MACHINE, TYPE);
		rst_counter += (strlen(MACHINE) + 1);
		rst_counter += (strlen(TYPE) + 1);
		REST = (R_FINAL + rst_counter);
		strcpy(MESSAGE, REST);

			
		if( strcmp(MACHINE, "1") == 0 )
		{
			if( strcmp(TYPE, "1") == 0 )	//INITIALIZATION
			{
				sscanf(MESSAGE, "%s %s", u_name, u_pswd);
				
				strcpy(MACHINE, "2");
				strcpy(TYPE, "1");
				
				for(i = 0; i < 255; i++)
				{
					if(strcmp(u_name, users[i].user_name) == 0)
					{
						strcpy(PRIM_KEY, users[i].prim_key);
						
						if(strcmp(u_pswd, users[i].password) == 0)
						{
							strcpy(VALID, "0");	//VALID
							snprintf(NUM_BUDDIES, 4, "%d", users[i].buddylist_len);
							
							strcpy(MESSAGE, NUM_BUDDIES);
							
							for(j = 0; j < users[i].buddylist_len; j++)
							{
								strcat(MESSAGE, " ");
								strcat(MESSAGE, users[i].buddies[j].b_nick);	//nick
								
								strcat(MESSAGE, " ");
								strcat(MESSAGE, users[i].buddies[j].b_prim_key);//primkey
								
								strcat(MESSAGE, " ");
								strcat(MESSAGE, users[i].buddies[j].b_status);	//status									
							}
							
							//now if the person has got the valid status
							//then it should appera to be online in all the
							//lists of his buddies
							
							for(k = 0; k < 255; k++)
							{
								for(l = 0; l < users[k].buddylist_len; l++)
								{
									if( strcmp(users[k].buddies[l].b_prim_key, users[i].prim_key) == 0 )
										strcpy(users[k].buddies[l].b_status, "1");
								}
							}
							
							break;
						}
						else
						{
							strcpy(VALID, "1");	//INVALID
						}
					}
					else
					{
						strcpy(PRIM_KEY, "400");	//if the user name is incorrect							
						strcpy(VALID, "1");			//INVALID							
					}
				}
				
				strcpy(S_FINAL, MACHINE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, PRIM_KEY);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, VALID);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
					
				encrypt(S_FINAL);
				write(authfd, S_FINAL, 15000);
			}
			
			
			else if( strcmp(TYPE, "2") == 0 )	//request for offline
			{
				for(k = 0; k < 255; k++)
				{
					for(l = 0; l < users[k].buddylist_len; l++)
					{
						printf("%s %s\n", users[k].buddies[l].b_prim_key, MESSAGE);
						
						if( strcmp(users[k].buddies[l].b_prim_key, MESSAGE) == 0 )
							strcpy(users[k].buddies[l].b_status, "0");
					}
				}
				
			}
			
			else if( strcmp(TYPE, "3") == 0 )	//request for buddy list
			{
				strcpy(MACHINE, "2");
				strcpy(TYPE, "2");
				strcpy(PRIM_KEY, MESSAGE);
				
				for(k = 0; k < 255; k++)
				{
					if( strcmp(PRIM_KEY, users[k].prim_key) == 0)
					{
						snprintf(NUM_BUDDIES, 4, "%d", users[k].buddylist_len);
						strcpy(MESSAGE, NUM_BUDDIES);
						
						for(i = 0; i < users[k].buddylist_len; i++)
						{
							strcat(MESSAGE, " ");
							strcat(MESSAGE, users[k].buddies[i].b_nick);	//nick
							
							strcat(MESSAGE, " ");
							strcat(MESSAGE, users[k].buddies[i].b_prim_key);//primkey
							
							strcat(MESSAGE, " ");
							strcat(MESSAGE, users[k].buddies[i].b_status);	//status							
						}
						break;
					}
				}
				
				strcpy(S_FINAL, MACHINE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, TYPE);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, PRIM_KEY);
				strcat(S_FINAL, " ");
				strcat(S_FINAL, MESSAGE);
				encrypt(S_FINAL);
				write(authfd, S_FINAL, 15000);
			}
		
			else if( strcmp(TYPE, "4") == 0 )		//request for change in nick
			{				
				sscanf(MESSAGE, "%s %s", PRIM_KEY, TEMP_NICK);
				for(i = 0; i < 255; i++)
				{
					for(j = 0; j < users[i].buddylist_len; j++)
					{
						if( strcmp(users[i].buddies[j].b_prim_key, PRIM_KEY) == 0)
						{
							strcpy(users[i].buddies[j].b_nick, TEMP_NICK);							
						}
					}						
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


