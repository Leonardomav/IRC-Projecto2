#include "header.h"

int fd;

int main(int argc, char *argv[]) {
	int port;

	if(argc!=2){
		printf("./server -p<port>\n");
		exit(-1);
	}

	sscanf(argv[1], "-p %d", &port);
	if(port < 1025|| port > 65000){
		printf("Invalid port!!!\nPort must be betwent 1025 and 65000\n");
		exit(-1);
	}

	signal(SIGINT,sig_handler);

	int client_fd;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;	

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("in socket");
		return 1;
	}

	if(bind(fd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
		perror("in bind");
		return 1;
	}

	if(listen(fd, MAX_CLIENTS) == -1){
		perror("in listen");
		return 1;
	}	

	printf("Waiting for a connection!\n");
	while(1){
		client_addr_size = sizeof(client_addr);
		if((client_fd = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size)) != -1){
			printf("Accepted connection\n");
		}
		else{
			perror("Connection rejected\n");
			return 1;
		}

		if (fork() == 0){
			process_client(client_fd);
			printf("End of connection\n");
			close(client_fd);
			exit(0);
		}	
	}
	
	return 0;
}

void process_client(int client_fd){
	char primitiva[PRIM_SIZE];
	int login;
	Cliente* cliente;
	cliente = (Cliente*) malloc(sizeof(Cliente));
	
	login = check_login(client_fd, cliente);
	write(client_fd, &login, sizeof(int));
	if(login == -1){
		printf("Invalid login!\n");
		free(cliente);
		return;
	}

	else{
		printf("User ->%s\nPass ->%s\n", cliente->name, cliente->pass);
	}
	
	do{
		read(client_fd , primitiva, sizeof(primitiva)-1);
		printf("Request: %s\n",primitiva);
		login=process_request(client_fd, cliente, primitiva, login);
	}while(strcmp(primitiva,"QUIT") != 0);

	free(cliente);
	return;
}

int check_login(int client_fd, Cliente* cliente){
	char name_buff[MAX_NAME], pass_buff[MAX_PASS], *desencripta_nome, *desencripta_pass;
	int oper;
	FILE *fp;

	read(client_fd, cliente, sizeof(Cliente));

	if((fp = fopen("client.aut.txt","r")) == NULL){
		perror("Error 404: File not found");
		return -1;
	}

	desencripta_nome = (char*) malloc(MAX_NAME*sizeof(char));
	desencripta_pass = (char*) malloc(MAX_PASS*sizeof(char));

	while(fscanf(fp,"%s %s %d", name_buff, pass_buff, &oper)!=EOF){
		desencripta(name_buff, desencripta_nome);
		desencripta(pass_buff, desencripta_pass);
		if((strcmp(desencripta_nome, cliente->name)==0) && (strcmp(desencripta_pass, cliente->pass)==0)){
			return oper;
		}
	}

	free(desencripta_nome);
	free(desencripta_pass);
	return -1;
}

int process_request(int client_fd, Cliente *cliente, char primitiva[], int login){
	int aux=0;

	if ((strcmp(primitiva,"LIST_MESS")) == 0){
		printf("Sending unread emails...\n");
		list_mess(client_fd, cliente->name);
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"LIST_USERS")) == 0){
		printf("Sending operators...\n");
		list_users(client_fd);
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"SEND_MESS")) == 0){
		printf("Sending a message...\n");
		printf("login: %d\n", login);
		send_mess(client_fd, login);
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"LIST_READ")) == 0){
		printf("Printing read emails...\n");
		list_read(client_fd, cliente->name);
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"REMOVE_MESS")) == 0){
		read(client_fd, &aux, sizeof(int));

		if(aux==1){
			printf("Removing one message...\n");
			remove_one(client_fd, cliente->name);
		}
		else if(aux==2){
			printf("Removing read messages...\n");
			remove_read(cliente->name);
		}
		else if(aux==3){
			printf("Removing all messages...\n");
			remove_all(cliente->name);
		}
		else{
			printf("Invalid option!\n");
			fflush(stdout);
			return login;
		}
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"CHANGE_PASSW")) == 0){
		printf("Changing password...\n");
		change_passw(client_fd, cliente);
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"OPER")) == 0){
		write(client_fd, &login, sizeof(int));
		if(login==1){
			printf("Cliente is already an operator!\n");
			return login;
		}
		printf("Changing permissions...\n");
		oper(client_fd, cliente);
		login=1;
		printf("Done...\n");
	}
	else if ((strcmp(primitiva,"QUIT")) == 0){
		printf("Client quiting...\n");
	}
	else{
		printf("Request not recognized\n");
	}
	
	return login;

}

void list_mess(int client_fd, char *nome){
	FILE *fp;	
	char filename[MAX_NAME+4], flag_read, email[EMAIL_SIZE], read[EMAIL_SIZE+1];
	int flag=0;
	fpos_t pos;

	sprintf(filename, "%s.txt", nome);
	if((fp = fopen(filename,"r+")) == NULL){
		fp = fopen(filename,"w");
		fclose(fp);
		fp = fopen(filename,"r+");
	}

	while(flag==0){
		fgetpos(fp, &pos);
		fscanf(fp,"%c %[^\n]\n", &flag_read, email);
		flag=feof(fp);
		if(flag_read=='+'){
			write(client_fd, email, sizeof(email));
			fsetpos(fp, &pos);
			sprintf(read, "- %s\n", email);
			fputs(read, fp);
		}
	}
	strcpy(email,"end");
	write(client_fd, email, sizeof(email));

	fclose(fp);
}

void list_users(int client_fd){
	char name_buff[MAX_NAME], pass_buff[MAX_PASS], *desencripta_nome;
	int oper;
	FILE *fp;

	if((fp = fopen("client.aut.txt","r")) == NULL){
		perror("Error 404: File not found");
		return;
	}

	desencripta_nome = (char*) malloc(MAX_NAME*sizeof(char));

	while(fscanf(fp,"%s %s %d", name_buff, pass_buff, &oper)!=EOF){
		if(oper==1){
			desencripta(name_buff, desencripta_nome);
			strcpy(name_buff,desencripta_nome);
			write(client_fd, name_buff, sizeof(name_buff));
		}
	}
	strcpy(name_buff,"end");
	write(client_fd, name_buff, sizeof(name_buff));
	free(desencripta_nome);
}

void send_mess(int client_fd, int login){
	FILE *fp1,*fp2; 
	char receiver[MAX_NAME], receivers[MAX_CLIENTS*MAX_NAME], name_buff[MAX_NAME], lixo[MAX_PASS+1], email[EMAIL_SIZE], filename[MAX_NAME+4], final[EMAIL_SIZE+1], *username, *desencripta_nome;

	write(client_fd, &login, sizeof(int));
	if(login!=1){
		printf("Client is not an operator!\n");
		return;
	}

	read(client_fd, receivers, sizeof(receivers));
	if((fp1 = fopen("client.aut.txt","r")) == NULL){
		perror("Error 404: File not found");
		return;
	}

	desencripta_nome = (char*) malloc(MAX_NAME*sizeof(char));

	read(client_fd, email, sizeof(email));
	sprintf(final, "+ %s\n", email);

	username=strtok(receivers, " ");
	while(username!=NULL){
		while(fscanf(fp1,"%s %[\n]\n", name_buff, lixo)!=EOF){
			desencripta(name_buff, desencripta_nome);
			strcpy(name_buff,desencripta_nome);
			if(strcmp(username, name_buff)==0){
				strcpy(receiver, username);
				sprintf(filename, "%s.txt", receiver);
				if((fp2 = fopen(filename,"a")) != NULL){
					fputs(final, fp2);
					fclose(fp2);
					break;
				}
			}
		}
		username=strtok(NULL, " ");
	}


	fclose(fp1);
	free(desencripta_nome);
}

void list_read(int client_fd, char *nome){
	FILE *fp;	
	char filename[MAX_NAME+4], flag_read, email[EMAIL_SIZE];

	sprintf(filename, "%s.txt", nome);
	if((fp = fopen(filename,"r+")) == NULL){
		fp = fopen(filename,"w");
		fclose(fp);
		fp = fopen(filename,"r+");
	}

	while(fscanf(fp,"%c %[^\n]\n", &flag_read, email)!=EOF){
		if(flag_read=='-'){
			write(client_fd, email, sizeof(email));
		}
	}
	strcpy(email,"end");
	write(client_fd, email, sizeof(email));

	fclose(fp);
}

void remove_one(int client_fd, char *nome){
	FILE *fp1, *fp2;
	char filename[MAX_NAME+4], flag_read, email[EMAIL_SIZE], full[EMAIL_SIZE+1];
	int count=0, option;

	sprintf(filename, "%s.txt", nome);
	if((fp1 = fopen(filename,"r+")) == NULL){
		fp1= fopen(filename,"w");
		fclose(fp1);
		fp1 = fopen(filename,"r+");
	}

	while(fscanf(fp1,"%c %[^\n]\n", &flag_read, email)!=EOF){
		write(client_fd, email, sizeof(email));
	}
	strcpy(email,"end");
	write(client_fd, email, sizeof(email));

	if((fp2 = fopen("replica.txt","w")) == NULL){
		perror("Error");
		fclose(fp1);
		return;
	}

	read(client_fd, &option, sizeof(int));

	rewind(fp1);
	while(fgets(full, sizeof(full), fp1)){
		count++;
		if (count!=option){
			fputs(full, fp2);
		}
	}

	remove(filename);
	rename("replica.txt", filename);

	fclose(fp1);
	fclose(fp2);
}

void remove_read(char *nome){
	FILE *fp1, *fp2;
	char filename[MAX_NAME+4], flag_read, email[EMAIL_SIZE], full[EMAIL_SIZE+1];

	sprintf(filename, "%s.txt", nome);
	if((fp1 = fopen(filename,"r+")) == NULL){
		fp1 = fopen(filename,"w");
		fclose(fp1);
		fp1 = fopen(filename,"r+");
	}

	if((fp2 = fopen("replica.txt","w")) == NULL){
		perror("Error");
		fclose(fp1);
		return;
	}

	while(fscanf(fp1,"%c %[^\n]\n", &flag_read, email)!=EOF){
		if (flag_read=='+'){
			sprintf(full, "+ %s\n", email);
			fputs(full, fp2);
		}
	}

	remove(filename);
	rename("replica.txt", filename);

	fclose(fp1);
	fclose(fp2);
}

void remove_all(char *nome){
	FILE *fp1, *fp2;
	char filename[MAX_NAME+4];

	sprintf(filename, "%s.txt", nome);
	if((fp1 = fopen(filename,"r+")) == NULL){
		fp1 = fopen(filename,"w");
		fclose(fp1);
		fp1 = fopen(filename,"r+");
	}

	if((fp2 = fopen("replica.txt","w")) == NULL){
		perror("Error");
		fclose(fp1);
		return;
	}

	remove(filename);
	rename("replica.txt", filename);

	fclose(fp1);
	fclose(fp2);
}

void change_passw(int client_fd, Cliente *cliente){
	FILE *fp1, *fp2;
	char newpass[MAX_PASS], name_buff[MAX_NAME], pass_buff[MAX_PASS], *encripta_pass, *desencripta_nome, *desencripta_pass;
	int oper;

	read(client_fd, newpass, sizeof(newpass));

	if((fp1 = fopen("client.aut.txt","r")) == NULL){
		perror("Error 404: File not found");
		return;
	}

	if((fp2 = fopen("replica.txt","w")) == NULL){
		perror("Error");
		fclose(fp1);
		return;
	}

	desencripta_nome = (char*) malloc(MAX_NAME*sizeof(char));
	desencripta_pass = (char*) malloc(MAX_PASS*sizeof(char));
	encripta_pass = (char*) malloc(MAX_PASS*sizeof(char));

	while(fscanf(fp1,"%s %s %d", name_buff, pass_buff, &oper)!=EOF){
		desencripta(name_buff, desencripta_nome);
		desencripta(pass_buff, desencripta_pass );
		if((strcmp(cliente->name, desencripta_nome)==0)&&(strcmp(cliente->pass, desencripta_pass)==0)){
			encripta(newpass, encripta_pass );
			strcpy(newpass, encripta_pass);
			fprintf(fp2, "%s %s %d\n", name_buff, newpass, oper);
		}
		else fprintf(fp2, "%s %s %d\n", name_buff, pass_buff, oper);
	}

	remove("client.aut.txt");
	rename("replica.txt", "client.aut.txt");

	fclose(fp1);
	fclose(fp2);
	free(encripta_pass);
	free(desencripta_nome);
	free(desencripta_pass);

}

void oper(int client_fd, Cliente *cliente){
	FILE *fp1, *fp2;
	char name_buff[MAX_NAME], pass_buff[MAX_PASS], *desencripta_nome, *desencripta_pass;
	int oper;

	if((fp1 = fopen("client.aut.txt","r")) == NULL){
		perror("Error 404: File not found");
		return;
	}

	if((fp2 = fopen("replica.txt","w")) == NULL){
		perror("Error");
		fclose(fp1);
		return;
	}

	desencripta_nome = (char*) malloc(MAX_NAME*sizeof(char));
	desencripta_pass = (char*) malloc(MAX_PASS*sizeof(char));

	while(fscanf(fp1,"%s %s %d", name_buff, pass_buff, &oper)!=EOF){
		desencripta(name_buff, desencripta_nome);
		desencripta(pass_buff, desencripta_pass);
		if((strcmp(cliente->name, desencripta_nome)==0)&&(strcmp(cliente->pass, desencripta_pass)==0)){
			fprintf(fp2, "%s %s 1\n", name_buff, pass_buff);
		}
		else fprintf(fp2, "%s %s %d\n", name_buff, pass_buff, oper);
	}

	remove("client.aut.txt");
	rename("replica.txt", "client.aut.txt");

	fclose(fp1);
	fclose(fp2);
	free(desencripta_nome);
	free(desencripta_pass);

}

void desencripta(char string[], char * new){
	int i;
	for(i=0; i<strlen(string); i++){
		new[i] = string[i]-3;
	}
}

void encripta(char string[], char * new){
	int i;
	for(i=0; i<strlen(string); i++){
		new[i] = string[i]+3;
	}
}

void sig_handler(int signo){
	printf("\rClosing\n");
	close(fd);
	kill(0,SIGKILL);
}