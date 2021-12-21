/*******************************************
* Group Name  : Firewall

* Member1 Name: Wang, Chen
* Member1 SIS ID: 831712302
* Member1 Login ID: daynight

* Member2 Name: Schwanke, Seth
* Member2 SIS ID: 831441493
* Member2 Login ID: sethgene
********************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define MAXDATASIZE 100
#define BACKLOG 10
#define MAXPENDING 5
using namespace std;

void sigchld_handler(int s){	
	int saved_errno = errno;
	while(waitpid(-1,NULL,WNOHANG) > 0);
	errno = saved_errno;
} 

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void chat_client(string IP, const char *charPort){
    
	string SADDRESS = IP;
	int PORT = atoi(charPort);
	/*int serverSocket;

	struct sockaddr_in clientAddress;
	*/
	struct Packet{
		u_int16_t version;
		u_int16_t length;
		char message[140];
	};
	
		int clientSocket;
		struct sockaddr_in serverAddress;

		if((clientSocket=socket(PF_INET,SOCK_STREAM,0))<0){
			cout << "Error: Client socket could not be created." << endl;
			exit(1);
		}
		
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = htonl(inet_network(SADDRESS.c_str())); 
		serverAddress.sin_port = htons(PORT);

   		if((connect(clientSocket,(struct sockaddr *)&serverAddress,sizeof(sockaddr_in))< 0)){
			cout << "Error: Failed to connect with server." << endl;
			exit(1);
		}
		cout << "Connecting to server... Connected!" << endl;
		cout << "Connected to a friend! You send first." << endl;
		
		while(1){
			string output="";
			cout << "You: ";
			getline(cin,output);
			while( output.length()>140 ){
				cout << "Error: Input too long." << endl;
				cout << "You: ";
				getline(cin,output);
				
			}
			
			Packet packet;
			packet.version=htons(457);
			packet.length=htons(output.length());
			strcpy(packet.message,output.c_str());
			send(clientSocket,(void*)&packet,sizeof(Packet),0);
			cout << "Awaiting server response\n";
			recv(clientSocket,(void*)&packet,sizeof(Packet),0);
			printf("Friend: %s\n",packet.message);
			
		}
		

}

void start_server(void){
	int serverSocket;
	int clientSocket;

	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;

	struct Packet{
		u_int16_t version;
		u_int16_t length;
		char message[140];
	};

	if((serverSocket=socket(PF_INET,SOCK_STREAM,0))<0){
			cout << "Error: Server socket could not be created." << endl;
			exit(1);
		}
		
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddress.sin_port = htons(0);
		
		if(bind(serverSocket,(struct sockaddr*)&serverAddress,sizeof(serverAddress))<0){
			cout << "Error: Server failed to bind." << endl;
			exit(1);
		}
		
		if(listen(serverSocket, MAXPENDING)<0){
			cout << "Error: Server failed to start listen." << endl;
			exit(1);
		}
		
		
		string SADDRESS;
		char buff[80];
   		gethostname(buff, sizeof(buff));
    		struct hostent *IP = gethostbyname(buff);
    		for (int i = 0; IP->h_addr_list[i] != 0; ++i) {
      			struct in_addr addr;
      			memcpy(&addr, IP->h_addr_list[i], sizeof(struct in_addr));
			SADDRESS = inet_ntoa(addr);
    		}
		socklen_t length = sizeof(serverAddress);
   		getsockname(serverSocket,(struct sockaddr*)&serverAddress,&length);
		cout << "Waiting for a connection on " << SADDRESS << " port " << ntohs(serverAddress.sin_port) << endl;
		

		socklen_t clientLength = sizeof(clientAddress);
		if((clientSocket=accept(serverSocket,(struct sockaddr*)&clientAddress,&clientLength))<0){
			cout << "Error: Server failed to accept connection." << endl;
			exit(1);
		}
		
		cout << "Found a friend! You receive first." << endl;
		while(1){
		
			Packet packet;
			recv(clientSocket,(void*)&packet,sizeof(Packet),0);
			printf("Friend: %s\n",packet.message);
			
			string output="";
			cout << "You: ";
			getline(cin,output);
			while( output.length()>140 ){
				cout << "Error: Input too long." << endl;
				cout << "You: ";
				getline(cin,output);
			}

			packet.version=htons(457);
			packet.length=htons(output.length());
			strcpy(packet.message,output.c_str());
			send(clientSocket,(void*)&packet,sizeof(Packet),0);
						
		}
	
}


int main(int argc, char* argv[]){
	
	string IP = "";
	string port= "";	
	bool server = false;
	//arg check
	if((argc != 5) && (argc != 1) && (argc !=2)){
	cerr << "Incorrect number of arguments! Please use ./chat -h for more information.\n";
	cerr << "Number of arguments: " << argc-1 << "\n"; 
	exit(0);
	}
	
	//server creation
	if(argc == 1){
		server = true; 
		start_server();
		
	}

	//help argument or incorrect input
	if(argc == 2){
	string test_flag = string(argv[1]);
		if(test_flag == "-h"){
			cout << "To start server type './chat' with no arguments\n";
			cout << "To connect to a chat server type: './chat -p PORT# -s IP_ADDRESS' or: './chat -s IP_ADDRESS -p PORT#' \n";
			exit(0);
		}

		else{
		cerr << "Incorrect flag for number of arguments. Please use './chat -h' for more information\n"; 
		exit(0);
		}	
	}
	
	if(server != true){
	string firstArg = string(argv[1]);
		if(firstArg == "-p"){
			port = string(argv[2]);
			IP = string(argv[4]);
		} 
		else{
			IP = string(argv[2]);
			port = string(argv[4]);
		}
		
		//const char * charIP = IP.c_str();
		const char * charPort = port.c_str();
		chat_client(IP, charPort);

		/*cout << "Port: " << port << "\n";
		cout << "IP: " << IP << "\n";*/
	}
	
  return 0;
}
