#include <stdio.h>
#include <conio.h>
#include <vector>
#include <winsock.h>

//CLIENT

#define LOCALHOST "127.0.0.1"
#define IPV4 AF_INET

SOCKET connection;

enum PACKET{
    CHAT_MESSAGE = 0
};

#define WAIT while(!kbhit()){}

bool send_int(int value){
    int check = send(connection, (char*)&value, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

#define send_size send_int

bool get_int(int& var){
    int check = recv(connection, (char*)&var, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

#define get_size get_int

bool send_packet_type(PACKET packet_type){
    int check = send(connection, (char*)&packet_type, sizeof(packet_type), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

bool get_packet_type(PACKET& packet_type){
    int check = recv(connection, (char*)&packet_type, sizeof(packet_type), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

bool send_data(char* data){
    unsigned int data_size = strlen(data);

    if( !send_packet_type(CHAT_MESSAGE) ){
        return false;
    }

    if( !send_size(data_size) ){
        return false;
    }

    if( send(connection, data, data_size, 0) == SOCKET_ERROR ){
        return false;
    }

    return true;
}
#define send_string send_data

bool get_data(char*& data){
    int data_size;

    if( !get_size(data_size) ){
        return false;
    }

    free(data);
    data = (char*)malloc((sizeof(char) * data_size) + 1);

    if( recv(connection, data, data_size, 0) == SOCKET_ERROR ){
        printf("%d", WSAGetLastError());
        WAIT;
        return false;
    }

    data[data_size] = '\0';

    return true;
}

#define get_string get_data

bool process_packet(PACKET& packet_type){

    char* data = nullptr;

    switch(packet_type){

        case CHAT_MESSAGE :

            if( !get_string(data) ){
                return false;
            }
            break;

        default:

            printf("unrecognised packet type.\n");
            break;
        }

    printf("someone said: %s\n", data);

    return true;
}

void client_thread(){
    PACKET packet_type;

    while(true){

      if( !get_packet_type(packet_type) ){
        break;
      }

      if( !process_packet(packet_type) ){
        break;
      }
    }

    printf("connection to server lost.\n");
    closesocket(connection);
}

int main()
{

    printf("hello world.\n");
    printf("255 character limit for now.\n");

    //WINSOCK START UP

    WSADATA wsa_data;
    WORD dllversion = MAKEWORD(2,1);

    if(WSAStartup(dllversion, &wsa_data)){ //RETURNS 0 ON SUCCESS NON_ZERO VALUE OTHERWISE
        MessageBox(NULL, "failed to start up WSAS", "error!!!", MB_ICONERROR | MB_OK );
        exit(1);
    }

    SOCKADDR_IN socket_addr = {0}; //ADDRESS TO BIND TO CONNECTION SOCKET
    socket_addr.sin_addr.S_un.S_addr = inet_addr(LOCALHOST); //ADRESS = LOCALHOST (broadcasts locally only)
    socket_addr.sin_port = htons(1111); //PORT = 1111
    socket_addr.sin_family = IPV4; //IPv4

    int addr_size = sizeof(socket_addr);//NEED SIZE OF ADDRESS FOR CONNECTION FUNCTION

    connection = socket(IPV4, SOCK_STREAM, 0); // SOCKET FOR CONNECTION TO SERVER
    if( connect(connection, (SOCKADDR*)&socket_addr, addr_size)){ // RETURN 0 IF SUCCESSFUL NON_ZERO VALUE OTHERWISE
        printf("failed to connect to server.\n");
        exit(2);
    }

    printf("CONNECTED TO SERVER!!\n");

    PACKET packet_type;
    get_packet_type(packet_type);

    char* message = nullptr; // CHAR BUFFER TO HOLD INCOMING MESSAGE
    get_string(message); // RECEIVES/ACCEPTS INCOMING MESSAGE FROM SERVER

    printf("SERVER SAYS: %s\n", message);


    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client_thread, 0, 0, 0);

    packet_type = CHAT_MESSAGE;

    while(true){
        memset((void*)message, 0, sizeof(message));
        scanf("%255[^\n]s", message);
        fflush(stdin);

        if(!send_string(message)){

            printf("error sending message.\n");
            free(message);

            WAIT;
            break;
        }

    }
    return 0;
}
//CLIENT
