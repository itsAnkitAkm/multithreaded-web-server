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
    int client_scoketId, client_len;

    struct sockaddr server_addr, client_addr;
    sem_inti(&semaphore, 0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);
    if(argv == 2){
        port_number = atoi(argv[1]);
    }else{
        printf("Too few arguments\n");
    }

    printf("Starting Proxy server at port: %d\n", port_number);
    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);
    if(proxy_socketId<0){
        perror("Failed to create a socket\n");
        exit(1);
    }
    int reuse = 1;
    if(setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse))<0){
        perror("setSockOpt failed\n");
    }

    //31:55
    return 0;
}