#include "header.h"

int fd;
Cliente* cliente;

int main(int argc, char *argv[]) {
	signal(SIGINT, sig_handler);
	char endServer[100], pass[MAX_PASS],username[MAX_NAME], stringaux[EMAIL_SIZE], userslist[MAX_CLIENTS*MAX_NAME], *primitiva, c;
	int aux, login, i;
	struct sockaddr_in addr;
	struct hostent *hostPtr;

	cliente = (Cliente*) malloc(sizeof(Cliente));

	if (argc != 5) {
		printf("cliente <host> <port> <nome> <pass>\n");
		exit(-1);
	}

	if (strlen(argv[3]) > 9){
		printf("ERRO! Username demasiado grande!\n");
		exit(-1);
	}

	strcpy(endServer, argv[1]);

	if ((hostPtr = gethostbyname(endServer)) == 0){
		printf("Nao consegui obter endereço");
		exit(-1);
	}

	bzero((void *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;	// dominio usado pela socket 
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;

	addr.sin_port = htons((short) atoi(argv[2]));	

	printf("Host address gotten!\n" );

	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		printf("Erro socket");
		exit(-1);
	}

	printf("Socket created!\n" );

	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0){
		printf("Erro Connect");
		exit(-1);
	}

	printf("Pedido para conecção enviado!\n" );

	//SEND INFO -------------------------------
	strcpy(cliente->name, argv[3]);
	strcpy(cliente->pass, argv[4]);
	cliente->pass[strlen(cliente->pass)]='\0';

	write(fd, cliente, sizeof(Cliente));

	//SEND INFO -------------------------------

	//LOGIN-------------------------------------------------

	read(fd, &login, sizeof(int));
	if(login == -1){
		printf("Credenciais Invalidas!\n");
		close(fd);
		exit(0);
	}
	else printf("Login Autorizado!\n");

	//LOGIN--------------------------------------------------

	primitiva = (char*) malloc(PRIM_SIZE*sizeof(char));

	while(1){
		do{
			primitiva = option();
		}while(strcmp(primitiva, "INVALID")==0);

		if(strcmp(primitiva, "QUIT")==0){
			write(fd, primitiva, 1+strlen(primitiva));
			break;
		}

		else if((strcmp(primitiva, "LIST_MESS")==0) || (strcmp(primitiva, "LIST_READ")==0)) {
			write(fd, primitiva, 1 + strlen(primitiva));
			printf("\nLista de Emails:\n");
			aux=0;
			while(1){
				read(fd, stringaux, EMAIL_SIZE);
				if(strcmp(stringaux, "end")!=0){
					printf("-> %s\n", stringaux);
					aux=1;
				}
				else if(aux==0){
					printf("\nNão foram encontrados Emails!\n");
					break;
				}
				else break;
			}

		}

		else if(strcmp(primitiva, "LIST_USERS")==0){
			write(fd, primitiva, 1 + strlen(primitiva));
			aux=0;
			while(1){
				read(fd, username, MAX_NAME);

				if(strcmp(username, "end")!=0){
					printf("Nome-> %s\n", username);
					aux=1;
				}

				else if(aux==0){
					printf("\nNão foram encontrados utilizadores!\n");
					break;
				}

				else break;
			}
		}

		else if(strcmp(primitiva, "SEND_MESS")==0){
			write(fd, primitiva, 1 + strlen(primitiva));
			read(fd, &aux, sizeof(int));

			if(aux!=1) printf("ERRO! Cliente não é um operador!\n");

			else if(aux==1){
				printf("Destinatários (usa espaço para separar os vario destinatários): ");
				while((c = getchar())!= '\n' && c != EOF);
				scanf("%[^\n]", userslist);
				while((c = getchar())!= '\n' && c != EOF);
				write(fd, userslist, sizeof(userslist));

				printf("Email: ");
				scanf("%[^\n]", stringaux);
				write(fd, stringaux, sizeof(stringaux));
				printf("Email email enviado aos destinatários possíveis!\n");
			}
		}

		else if(strcmp(primitiva, "REMOVE_MESS")==0){

			do{
				printf("Deseja apagar:\n1 - Uma mensagem\n2 - As mensagens lidas\n3 - Todas a mensagens\nOpçao: ");
				scanf("%d", &aux);
			}while(aux!=1 && aux!=2 && aux!=3);

			write(fd, primitiva, 1 + strlen(primitiva));
			write(fd, &aux, sizeof(int));

			if(aux==1){
				aux=0, i=0;
				while(1){
					read(fd, stringaux, EMAIL_SIZE);
					if(strcmp(stringaux, "end")!=0){
						printf("%d ->%s\n", ++i, stringaux);
						aux=1;
					}

					else if(aux==0){
						printf("\nNão foram encontrados Emails!\n");
						break;
					}

					else break;
				}

				printf("Option: ");
				scanf("%d", &aux);
				write(fd, &aux, sizeof(int));
				printf("Mensagem removida");
			}	

			else{
				printf("Mensagens removidas");
			}
		}

		else if(strcmp(primitiva, "CHANGE_PASSW")==0){
			write(fd, primitiva, 1 + strlen(primitiva));
			do{
				printf("Nova Password: ");
				scanf("%s", pass);
			}while(strlen(pass)>MAX_PASS);
			write(fd, pass, MAX_PASS);
			printf("Password alterada!\n");	
		}

		else if(strcmp(primitiva, "OPER")==0){
			write(fd, primitiva, 1 + strlen(primitiva));
			read(fd, &aux, sizeof(int));
			if(aux==1) printf("Cliente já era operador!\n");
			else if(aux==0) printf("cliente é agora operador!\n");
		}

		else{
			write(fd, primitiva, 1 + strlen(primitiva));
			printf("Mensagem Enviada!\n" );
		}
	}

	printf("Fim da conecção!\n");
	close(fd);
	free(cliente);
	exit(0);
}

char * option(){
	int option;
	printf("\n-------------------------\n1 - LIST_MESS\n2 - LIST_USERS\n3 - SEND_MESS\n4 - LIST_READ\n5 - REMOVE_MESS\n6 - CHANGE_PASSW\n7 - OPER\n8 - QUIT\n-------------------------\n\nOPTION: ");
    scanf("%d", &option);
    switch(option){

   		case 1:
      		return "LIST_MESS";
	
		case 2:
  			return "LIST_USERS";

    	case 3:
      		return "SEND_MESS";

    	case 4:
    		return "LIST_READ";

   		case 5:
    		return "REMOVE_MESS";

		case 6:
    		return "CHANGE_PASSW";

   		case 7:
   			return "OPER";

 		case 8:
	   		return "QUIT";
      
   		default:
   			printf("INVALID OPTION\n");
   			return "INVALID";
   	}
}

void sig_handler(int signo){
	printf("\rEnd of connection\n");
	write(fd, "QUIT", 1 + strlen("QUIT"));
	close(fd);
	free(cliente);
	exit(0);
}
