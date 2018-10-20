#include <stdio.h>
#include <conio.h>
#include <winsock.h>

//SERVER

#define LOCALHOST "127.0.0.1"
#define IPV4 AF_INET

enum PACKET{
    CHAT_MESSAGE
};

SOCKET connections[100];
int connection_counter = 0;

bool send_int(unsigned int ID, const int value){
    int check = send(connections[ID], (char*)&value, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

#define send_size send_int

bool get_int(unsigned int ID, int& value){
    int check = recv(connections[ID], (char*)&value, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

#define get_size get_int

bool send_packet_type(unsigned int ID,const PACKET packet_type){
    int check = send(connections[ID], (char*)&packet_type, sizeof(packet_type), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

bool get_packet_type(unsigned int ID, PACKET& packet_type){
    int check = recv(connections[ID], (char*)&packet_type, sizeof(packet_type), 0);

    if(check == SOCKET_ERROR){
        return false;
    }

    return true;
}

bool send_data(unsigned int ID, const char* data){
    unsigned int data_size = strlen(data);

    if( !send_packet_type(ID, CHAT_MESSAGE)){ // sent the type of packet which will be sent.
        return false;
    }

    if( !send_size(ID, data_size)){ // sends the size of the buffer
        return false;
    }

    if( send(connections[ID], data, data_size, 0) == SOCKET_ERROR){ // SEND THE MESSAGE TO THE CONNECTED SOCKET
        return false;
    }

    return true;
}

#define send_string send_data

bool get_data(unsigned int ID, char*& data){
    int data_size;

    if( !get_size(ID, data_size)){
        return false;
    }

    data = (char*)malloc((sizeof(char) * data_size) + 1);
    data[data_size] = '\0';

    if( recv(connections[ID], data, data_size, 0) == SOCKET_ERROR){
        return false;
    }

    return true;
}

#define get_string get_data

bool process_packet(int ID,const PACKET& packet_type){

    char* message;

    switch(packet_type){

    case CHAT_MESSAGE :
        if( !get_string(ID, message) ){
            return false;
        }

        for(int i = 0; i < connection_counter; i++){
            if(i == ID){
                continue;
            }

            printf("SENDING MESSAGE.\n");

            if( !send_string(i, message) ){
                return false;
            }

            printf("MESSAGE SENT.\n");
        }
        break;


    default :

        printf("unrecognised packet type : %d.\n", packet_type);
        break;
    }

    printf("RETURNING TRUE.\n");
    return true;
}

void client_handler_thread(int ID){

    PACKET packet_type;
    while(true){
        if( !get_packet_type(ID, packet_type)){
            break;
        }

        if(!process_packet(ID, packet_type)){
            printf("failed to process packet from client with id : %d\n", ID);
            break;
        }

    }
   closesocket(connections[ID]);
}

int main(){

    printf("hello world.\n");

    //WINSOCK START UP

    WSADATA wsa_data;
    WORD dllversion = MAKEWORD(2,1);

    if(WSAStartup(dllversion, &wsa_data)){ //RETURNS 0 ON SUCCESS NON_ZERO VALUE OTHERWISE
        MessageBox(NULL, "failed to start up WSAS", "error!!!",MB_ICONERROR | MB_OK );
        exit(1);
    }

    SOCKADDR_IN socket_addr = {0}; //ADDRESS TO BIND TO CONNECTION SOCKET
    socket_addr.sin_addr.S_un.S_addr = inet_addr(LOCALHOST); //ADRESS = LOCALHOST
    socket_addr.sin_port = htons(1111); //PORT = 1111
    socket_addr.sin_family = IPV4; //IPv4

    int addr_size = sizeof(socket_addr);//NEED SIZE OF ADDRESS FOR CONNECTION FUNCTION

    /// SERVER STUFF

    SOCKET soc_listen = socket(IPV4, SOCK_STREAM, 0); // CREATES SOCKET TO LISTEN FOR NEW CONNECTIONS (NOT YET IN LISTENING STATE THOUGH)
    bind(soc_listen, (SOCKADDR*)&socket_addr, addr_size); // BINDS ADRESS TO SOCKET
    listen(soc_listen, SOMAXCONN); // PLACES SOCKET IN A LISTENING STATE. SOMAXCONN - Socket Outstandin Max Connections


    SOCKET new_connection = {0}; // THIS STORES THE SOCKET OF THE CLIENT

    for(int i = 0; i < 100; i++){
        new_connection = accept(soc_listen, (SOCKADDR*)&socket_addr, &addr_size); // ACCEPTS THE CONNECTION

        if(new_connection){
            connections[i] = new_connection;
            printf("CLIENT CONNECTED!!\n");


            const char* message = "This is a message from the server."; // a char buffer to be sent

            // SENDING THE MESSAGE
            send_string(i, message);

            // MOVING ON TO OTHER CLIENTS
            connection_counter++;

            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client_handler_thread, (LPVOID)i, 0, 0);

            printf("%d\n", i);
        }else{
            printf("failed to connect to client.\n");
        }

    }
    return 0;
}
//SERVER
