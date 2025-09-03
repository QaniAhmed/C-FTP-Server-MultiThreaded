#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
/* Definations */
#define DEFAULT_BUFLEN 1024
#define PORT 9090
typedef struct {
    int client;
    char* dataBase;
} Arguments;
void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }


int login(FILE *password,int client,char*userInput,int bytesReceived);
/*--------------------------------------------------------------------*/
/*--- commands   functions                                       ---*/
/*--------------------------------------------------------------------*/
int commands(int client,char*UserINput,char* database,int bytes_read) ;
int UserCommand(char*Database,int client,char *UserINput,int bytes_read);
void ListCommand(char *directory,int client);
void GetCommand(char *FileName,int client);
void PutCommand(char *FileName,int client);
void DelCommand(char *FileName,int client);
void QuitCommand(int client);
/*--------------------------------------------------------------------*/
/*--- commands   functions                                       ---*/
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/
/*--------------------------------------------------------------------*/
void* Child(void* arg){ 
Arguments* data = (Arguments*)arg;
int client = data->client;
char* Database = data->dataBase;	 
char welcomMsg[100]="welcome in abdulqani ahmed FTP server \nwrite <help> to see all commands\n";
char line[DEFAULT_BUFLEN];
int bytes_read, result, welcomeMsgStaus;

//send welcome message to the client 
	welcomeMsgStaus=send(client,welcomMsg,sizeof(welcomMsg),0);
	if(welcomeMsgStaus<0)
	printf("send failed\n");
    do
    {
	bytes_read = recv(client, line, sizeof(line), 0);
	
	if (bytes_read > 0) 
	{
		int CommandCheker= commands(client,line,Database,sizeof(line));
		if(CommandCheker!=1)
		{
			if ((bytes_read=send(client, line, bytes_read, 0)) < 0 ) 
				{
		                printf("Send failed\n");
		                break;
		        }	
		}       
	} 
	else if (bytes_read == 0) 
	{
	        printf("Connection closed by client\n");
	        break;
	} 
	else 
	{
	        printf("Connection error / Connection closed by client\n");
	        break;   
	}
    } while (bytes_read > 0);
    close(client);
    return arg;
}

/*--------------------------------------------------------------------*/
/*--- main - setup server and await connections (no need to clean  ---*/
/*--- up after terminated children.                                ---*/
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[]){ 
    char *DataBase = NULL;
    int sd,opt,optval;
    struct sockaddr_in addr;
    unsigned short port=0;

  
    while ((opt = getopt(argc, argv, "p:u:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                DataBase = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -p <port> -u <users database>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

 if (port == 0) {
        fprintf(stderr, "Error: Port not specified. Use -p <port>\n");
        exit(EXIT_FAILURE);
    }

    if (DataBase == NULL) {
        fprintf(stderr, "Error: Database not specified. Use -u <database>\n");
        exit(EXIT_FAILURE);
    }


    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port > 0 )
                addr.sin_port = htons(port);
    else
                addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("System ready on port %d\n",ntohs(addr.sin_port));

    while (1)
    {
	// Create a structure to hold the client ID and file path
  
    
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
         // Create and populate the Arguments structure
    Arguments args;
    args.client = client;          // Set client ID
    args.dataBase = DataBase;      // Set the database file path
    
        if ( pthread_create(&child, NULL, Child, (void*)&args) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }
    return 0;
}
int commands(int client,char*UserINput,char* database,int bytes_read){
	char commmand[10];
	char directory[100]="/home/kali/Desktop/target";
	char pathToRead[100];
	int static loginSuccess;
	char loginFirstMsg[100]="\nplease login first before :";
    
    //first word
    sscanf(UserINput, "%s", commmand);
    

    
    	if(strcmp(commmand,"USER")==0)
    		{
    			loginSuccess= UserCommand(database,client,UserINput,bytes_read);
    			return 1;
			}
			
		if(loginSuccess==1)
		{
					if(strcmp(commmand,"LIST")==0)
		    		{
		    			ListCommand(directory,client);
		    			
		    			return 1;
					}
					if(strcmp(commmand,"GET")==0)
		    		{
		    			sscanf(UserINput, "%*s %s", pathToRead);
		    			GetCommand(pathToRead,client);
		    			return 1;
					}
					if(strcmp(commmand,"DEL")==0)
		    		{
		    			sscanf(UserINput, "%*s %s", pathToRead);
		    			DelCommand(pathToRead,client);
		    			return 1;
					}
					if(strcmp(commmand,"PUT")==0)
		    		{
		    			sscanf(UserINput, "%*s %s", pathToRead);
		    			PutCommand(pathToRead,client);
		    			return 1;
					}
			
		}
			if( (strcmp(commmand,"LIST")==0 || strcmp(commmand,"GET")==0 ||strcmp(commmand,"DEL")==0 ||strcmp(commmand,"PUT")==0 )&& loginSuccess==0 ) 
			{
			send(client,loginFirstMsg,sizeof(loginFirstMsg),0);
			}
		
		
			 if(strcmp(commmand,"QUIT")==0 )
    		{
    			
    			QuitCommand(client);
    			return 1;
			}
			
			else if(strcmp(commmand,"help")==0)
    		{
    			char message[] = 
        "========================================\n"
        "           FTP SERVER COMMANDS          \n"
        "========================================\n"
        "USER <username:password>\n"
        "  - Authenticate with username and password.\n"
        "----------------------------------------\n"
        "LIST\n"
        "  - List all files with their sizes.\n"
        "----------------------------------------\n"
        "GET <filename>\n"
        "  - Download a file from the server.\n"
        "----------------------------------------\n"
        "PUT <filename>\n"
        "  - Upload a file to the server.\n"
        "----------------------------------------\n"
        "DEL <filename>\n"
        "  - Delete a file from the server.\n"
        "----------------------------------------\n"
        "QUIT\n"
        "  - Disconnect from the server.\n"
        "========================================\n";
    
    send(client, message, strlen(message), 0);	
    			return 1;
			} 
			
			
	
	
	
}
int login(FILE *password,int client,char *userInput,int bytesReceived){
		
	char loginFormat[200];
	int i;
	userInput[bytesReceived] = '\0';
	userInput[strcspn(userInput, "\n")] = '\0';
  for ( i= 0; i < strlen(userInput); i++) 
  	{
        loginFormat[i] = (userInput[i] != ' ') ? userInput[i] : ':';
    }
    loginFormat[i] = '\0';
				char myString[100];
			while(fgets(myString, 100, password)) {
    myString[strcspn(myString, "\n")] = 0;
	if(strcmp(loginFormat,myString)==0) return 1;	
	
}
return 0;	
}
int UserCommand(char*Database,int client,char *UserINput,int bytes_read){
	int result;
	static int loginCounter = 0;
	static int count=0;
	char correctLogin[100]="200 User test granted to access.\n",
	errorMsg[100]="400 User not found. Please try with another user. \n",
	NotAllwoed[100]="\nYou have exceeded the maximum number of login attempts.\n", 
	userAndPass[100];
	sscanf(UserINput, "USER %[^\n]", userAndPass);

			FILE *password = fopen(Database, "r");

			result= login(password,client,userAndPass,bytes_read);
		
			 if (result == 1) 
			 	{
					/*Debug for server*/printf("\nsuccess login for :%s \n", userAndPass);
					            send(client, correctLogin, sizeof(correctLogin), 0); 
					        	return 1;  
    			}
			else {
						loginCounter+=1;
						if(loginCounter>=3)
						{
							send(client,NotAllwoed,sizeof(NotAllwoed),0);
							close(client);
						}
						send(client,errorMsg,sizeof(errorMsg),0);
				}		
	return 0 ;  		
}
void ListCommand(char *directory,int client){
	char finish[10]=".";
	char newLine[10]="\n";
    DIR *currentDir;
    
    struct dirent *Next;

    currentDir = opendir(directory);
    if (currentDir == NULL) {
        /*Debug for server*/ perror("Unable to open directory");
        return;
    }

    while ((Next = readdir(currentDir)) != NULL) {
    	
    	char item[100]="\033[0;32m";
        if (strcmp(Next->d_name, ".") != 0 && strcmp(Next->d_name, "..") != 0) {
            printf("\033[0;32m%s\033[0m\t", Next->d_name);
            strcat(item,Next->d_name);
            strcat(item, newLine);
            send(client, item, strlen(item) + 1, 0);
        }
        
    }
    /*Debug for server*/ printf("file listed seuccessfully");
    char WhiteColor[] = "\033[37m";
send(client, WhiteColor, strlen(WhiteColor) + 1, 0);


    strcat(finish,newLine);
    send(client, finish, strlen(finish) + 1, 0);
    closedir(currentDir);
    
}
void PutCommand(char *FileName,int client){
	char myString[1024],
	error[100]="\n400 File cannot save on server side.\n",
	path[100]="/home/kali/Desktop/target/",
	MSGSuucess[100]="200",
	success[100]="Byte test file retrieved by server and was saved.\n";
	int bytes,TotalBytes=0;
	
	strcat(path,FileName);
/*Debug for the server*/ printf("\nfile that user need to GET is:%s\n",path);	
	FILE *f;
	f=fopen(path,"w");
	if(f==NULL)
	{
		printf("\nError to open file \n");
		send(client, error, strlen(error), 0);
	}
	else{
		
		while (1)
		{
			bytes=	recv(client, myString, sizeof(myString)-1, 0);
			myString[bytes]='\0';
			
			if(bytes<=0)
			{
				break;
			}  
			  /*Debug for the server*/ printf("\nbytes recived: %d, Content: %s\n", bytes, myString);
			  if (bytes > 0)/*Debug for the server*/printf("Last byte: %c position :%d\n", myString[bytes - 1],bytes - 1  );
			 if (bytes==2 && myString[0]=='.' && myString[1]=='\n')	break;
        	
        	/*Debug for the server*/printf("added %d bytes to file Done\n",bytes);
        	fwrite(myString, 1, bytes, f);
			TotalBytes += bytes-1;
			/*Debug for the server*/printf("current total file :%d\n",TotalBytes);
			memset(myString, 0, sizeof(myString));  				
		}
		
		fclose(f);
		char response[300];
	    snprintf(response, sizeof(response), "%d %d Byte %s retrieved by server and was saved.\n", 200, TotalBytes+4, FileName);
	    send(client, response, strlen(response), 0);
	}
}
void GetCommand(char *FileName,int client){
	
	char myString[1024],error[100]="\n404 File",path[100]="/home/kali/Desktop/target/";
	char terminate[10]="\r\n.\r\n";
	strcat(path,FileName);
	/*Debug for the server */ printf("\nfile that user need to GET is:%s\n",path);
	
	FILE *f;
	f=fopen(path,"r");
	if(f==NULL)
	{
		printf("\ncan't open file\n");
		strcat(error,FileName);
		strcat(error," not found.\n");
		
		send(client, error, strlen(error), 0);
	}

	else{
		while(fgets(myString, 1024, f)) {
	send(client, myString, strlen(myString), 0);	
	}
	fclose(f);
	send(client, terminate, strlen(terminate), 0);
	}
}
void DelCommand(char *FileName,int client){
char myString[1024],
error[100]="\n404 File nofile is not on the server.\n",
deletedFile[1024]="/home/kali/Desktop/target/",
terminate[10]="\r\n.\r\n",
successDel[30]="200 File ";
strcat(successDel,FileName);
strcat(successDel," deleted.\n");


	strcat(deletedFile,FileName);
	/*Debug for the server*/ printf("\nfile that user need to delete is:%s\n",deletedFile);
	
	if(remove(deletedFile)==0)
	{
		send(client, successDel, strlen(successDel), 0);
		/*Debug for the server*/ printf("\nfile:%s is deleted successfully\n",deletedFile);
	}
	else{
		send(client, error, strlen(error), 0);
		/*Debug for the server*/ printf("\nERROR: file:%s is NOT deleted\n",deletedFile);	
	}	
}
void QuitCommand(int client){	
	char ByeMsg[30]="\nGoodbye ^_^ \n";
		if (send(client, ByeMsg, strlen(ByeMsg), 0) == strlen(ByeMsg)) {
		    printf("\nQUIT Msg : sent successfully.\n");
		} else {
		    printf("\nQUIT msg : send failed\n");
		}
		close(client);
}

