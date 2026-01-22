#include "proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// Max concurrent client server will handle;
#define MAX_CLIENTS 10

typedef struct cache_element cache_element;

// create the linkedList data type for LRU cache;
struct cache_element
{
    char *data;
    int len;
    char *url;
    time_t lru_time_track;
    cache_element *next;
};

// to find the data in cache
cache_element *find(char *url);

// to add the data in cache
int add_cache_element(char *data, int size, char *url);

// to delete the data from the cache
void remove_cache_element();

int port_number = 3000;     // Default Port
int proxy_socketId;         // socket descriptor of proxy server
pthread_t tid[MAX_CLIENTS]; // array to store the thread ids of clients
sem_t semaphore;            // if client requests exceeds the max_clients this seamaphore puts the waiting threads to sleep and wakes them when traffic on queue decreases
pthread_mutex_t lock;       // to lock the common data(cache) will write

cache_element *head; // head of the cache lindeklist
int cache_size;      // size of teh cache

int main(int agrc, char* argv[]){
    int client_scoketId, client_len; // client_socketId == to store the client socket id

    struct sockaddr_in server_addr, client_addr;  //gerneric structure that store IP address+Port;
    sem_inti(&semaphore, 0, MAX_CLIENTS); // maximum semaphores 
    pthread_mutex_init(&lock, NULL);

    // check if we had the valid argument to start the program
    if(argv == 2){
        port_number = atoi(argv[1]);
    }else{
        printf("Too few arguments\n");
    }

    printf("Starting Proxy server at port: %d\n", port_number);
    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0); // create the proxy soxket
    if(proxy_socketId<0){
        perror("Failed to create a socket\n");
        exit(1);
    }
    int reuse = 1;
    //Allows reusing the same port immediately after restart.
    if(setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse))<0){
        perror("setSockOpt failed\n");
    }

    bzero((char *)&server_addr, sizeof(server_addr)); // just remove the garbage value

    server_addr.sin_family = AF_INET; // tell the os that this is an IPv4 
    server_addr.sin_port = htons(port_number); //Sets the port number your server listens on
    server_addr.sin_addr.s_addr= INADDR_ANY; //accepts connections on ALL network interfaces

   // Associate this socket (proxy_socketId) with this IP address + port (server_addr)
    if(bind(proxy_socketId, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        perror("Port is not available\n");
        exit(1);
    }
    printf("Binding on port %d\n", port_number);

    int listen_status = listen(proxy_socketId, MAX_CLIENTS); // start listening on the socket with max queue size
    if(listen_status<0){
        perror("Error in listening\n");
        exit(1);
    }

    int i=0;
    int Connected_socketId[MAX_CLIENTS];

    while(1){
        bzero((char *)&client_addr, sizeof(client_addr));
        client_len = sizeof(client_addr);
        client_scoketId = accept(proxy_socketId, (struct sockaddr*)&client_addr, (socklen_t*)client_len);
        if(client_scoketId<0){
            printf("Not able to connect");
            exit(1);
        }else{
            Connected_socketId[i]=client_scoketId;
        }
        struct sockaddr_in * client_pt =  (struct sockaddr_in *)& client_addr;
        struct in_addr ip_addr = client_pt->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_addr, str, INET6_ADDRSTRLEN);
        printf("Client is connected with port number %d and ip address is %s\n, n", ntohs(client_addr.sin_port), str);
        pthread_create(&tid[i],NULL,thread_fn, (void*)&Connected_socketId[i]); // Creating a thread for each client accepted
		i++; 
    }
    close(proxy_socketId);
    return 0;
    
}