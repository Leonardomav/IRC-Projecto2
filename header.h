#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#define MAX_NAME 10
#define MAX_PASS 32
#define MAX_EMAIL 5
#define EMAIL_SIZE 256
#define MAX_CLIENTS 5
#define PRIM_SIZE 15

typedef struct Cliente{
	char name[MAX_NAME];
	char pass[MAX_PASS];
}Cliente;

void process_client(int client_fd);
int check_login(int client_fd, Cliente* cliente);
char * option();
void sig_handler(int signo);
void list_mess(int client_fd, char *nome);
void list_users(int client_fd);
void send_mess(int client_fd, int login);
void list_read(int client_fd, char *nome);
void remove_one(int client_fd, char *nome);
void remove_read(char *nome);
void remove_all(char *nome);
void change_passw(int client_fd, Cliente *cliente);
void oper(int client_fd, Cliente *cliente);
int process_request(int client_fd, Cliente *cliente, char primitiva[], int login);
void desencripta(char string[], char * new);
void encripta(char string[], char * new);