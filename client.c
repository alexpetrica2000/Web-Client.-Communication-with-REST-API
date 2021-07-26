#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// functie ca itereaza pe raspunsul primit de la server pentru a afla codul
char getCode(char *string) 
{
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == ' ')
			return string[i + 1];
	}
	return -1;
}

// functie ce afla cookieul din raspunsul de la server
void getCookie(char *string, char **cookies, int *cookies_size) 
{
	char *cookie = strstr(string, "Set-Cookie: ");
	cookie = cookie + strlen("Set-Cookie") + 2;
	char *dup = strstr(cookie, "\r\n");
	cookie[strlen(cookie) - strlen(dup) + 2] = '\0';

	cookies[*cookies_size] = strdup(cookie);
	*cookies_size = *cookies_size + 1;

}

// functie ce afla JWT tokenul din raspunsul de la server
void getJWT(char *string, char **JWT) 
{
	char *p = strstr(string, "\r\n\r\n{");
	*JWT = strdup(p + 4);
	JSON_Value *rep = json_parse_string(*JWT);
	JSON_Object *obj = json_object(rep);
	const char *name = json_object_get_name(obj,0);
	const char *val = json_object_get_string(obj,name);
	*JWT = strdup(val);
}

int main(int argc, char *argv[])
{	
	// buffere de stocare citire, body token JWT
	// cookies si dimensiune cookies
    char *message;
    char *response;
    int sockfd;
    char buffer[2000];
    char s1[2000];
    char s2[2000], s3[2000], s4[2000], s5[2000];
    char **body;
    char *JWT = NULL;
    char **cookies = malloc(100 * sizeof(char*));
    int cookie_size;   

    // in cadrul while fiecare comanda respecta un pattern
    // dupa care se efectueaza comanda ceruta
    while(1) {

    fgets(buffer, 2000, stdin);

    if (strcmp("register\n", buffer) == 0) { // caz register

    	// citire date
    	printf("username=");
    	fgets(s1, 2000, stdin);
    	s1[strlen(s1) - 1] = '\0';
    	printf("password=");
    	fgets(s2, 2000, stdin);
    	s2[strlen(s2) - 1] = '\0';

    	// parsare in format json
    	JSON_Value *root_value = json_value_init_object();
    	JSON_Object *root_object = json_value_get_object(root_value);
    	char *serialized_string = NULL;
    	json_object_set_string(root_object, "username", s1);
    	json_object_set_string(root_object, "password", s2);
    	serialized_string = json_serialize_to_string_pretty(root_value);
    	
    	// alcatuire body mesaj
    	body = (char**) malloc(1 * sizeof(char*));
    	body[0] = strdup(serialized_string);
    	
    	// trimitere tip POST / GET / DELETE
    	sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    	message = compute_post_request("34.118.48.238", " /api/v1/tema/auth/register", "application/json", body, 1, NULL, 0, 0, NULL);

    	send_to_server(sockfd, message);
    	response = receive_from_server(sockfd);
    	char code = getCode(response);

    	// daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	strcpy(response, "register successful");
	    	puts(response);
	    }

    	json_free_serialized_string(serialized_string);
    	json_value_free(root_value);
    	close_connection(sockfd);
	}

	if (strcmp("login\n", buffer) == 0) {

		// daca cookie size diferit de 0 inseamna ca am primit 
		// cookieul de login, sunt deja logat
		if (cookie_size != 0) {
			puts("Already logged in");
			continue;
		}

		// citire date
    	printf("username=");
    	fgets(s1, 2000, stdin);
    	s1[strlen(s1) - 1] = '\0';
    	printf("password=");
    	fgets(s2, 2000, stdin);
    	s2[strlen(s2) - 1] = '\0';

    	// parsare JSON
    	JSON_Value *root_value = json_value_init_object();
    	JSON_Object *root_object = json_value_get_object(root_value);
    	char *serialized_string = NULL;
    	json_object_set_string(root_object, "username", s1);
    	json_object_set_string(root_object, "password", s2);
    	serialized_string = json_serialize_to_string_pretty(root_value);
    	
    	// alcatuire body mesaj
    	body = (char**) malloc(1 * sizeof(char*));
    	body[0] = strdup(serialized_string);
    	
    	// trimitere tip POST
    	sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    	message = compute_post_request("34.118.48.238", " /api/v1/tema/auth/login", "application/json", body, 1, NULL, 0, 0, NULL);
    	
    	send_to_server(sockfd, message);
    	response = receive_from_server(sockfd);
    	char code = getCode(response);
		
		// daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	// in cazul login primesc cookie de auntetificare
	    	// il retin in cookies
	    	getCookie(response, cookies, &cookie_size);
	    	strcpy(response, "login successful");
	    	puts(response);
	    }

    	json_free_serialized_string(serialized_string);
    	json_value_free(root_value);
    	close_connection(sockfd);
	}

	if (strcmp("enter_library\n", buffer) == 0) { 

		// trimit mesaj de tip GET
	    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
	    message = compute_get_request("34.118.48.238", "/api/v1/tema/library/access", NULL, cookies, cookie_size, 0, NULL);
	    
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    char code = getCode(response);

	    // daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	// am primit tokenul JWT, il salvez in JWT
	    	getJWT(response, &JWT);
	    	strcpy(response, "enter_library successful");
	    	puts(response);
	    }
	    close_connection(sockfd);
	}

	if (strcmp("get_books\n", buffer) == 0) {
	    
	    // daca vreau sa accesez lista de cari
	    // trimit GET in care am setat cookiul de autentificare
	    // si JWTul care demonstreaza ca sunt in library
	    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
	    message = compute_get_request("34.118.48.238", "/api/v1/tema/library/books", NULL, cookies, cookie_size, 1, JWT);
	  
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    char code = getCode(response);

	    // daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response,"\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	char *p = strstr(response,"\r\n\r\n[");
	    	puts(p + 4);
	    }
	    close_connection(sockfd);
	}

	if (strcmp("get_book\n", buffer) == 0) {

		// citire date
		printf("id=");
		fgets(s1, 2000, stdin);
		s1[strlen(s1) - 1] = '\0';
		sprintf(s2,"/api/v1/tema/library/books/%s", s1); // formatare ur;

	    // trimit get in care am setat atat cookiul care demonstreaza autentificarea
	    // cat si jwtul care demonstreaza accesul in biblioteca
	    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
	    message = compute_get_request("34.118.48.238", s2, NULL, cookies, cookie_size, 1, JWT);
	  
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    char code = getCode(response);

	    // daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	// afisez date carte
	    	char *p = strstr(response, "\r\n\r\n[");
	    	puts(p + 4);
	    }
	    close_connection(sockfd);
	}
	if (strcmp("delete_book\n", buffer) == 0) {

		// citire date
		printf("id=");
		fgets(s1, 2000, stdin);
		s1[strlen(s1) - 1] = '\0';
		sprintf(s2, "/api/v1/tema/library/books/%s", s1); // formatare url
	    
	    // trimitere tip DELETE cu JWT setat
	    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
	    message = compute_delete_request("34.118.48.238", s2,JWT);
	  
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    char code = getCode(response);

	    // daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	strcpy(response, "delete_book successful");
	    	puts(response);
	    }
	    close_connection(sockfd);
	}

	if (strcmp("add_book\n", buffer) == 0) {

		// citire date
    	printf("title=");
    	fgets(s1, 2000, stdin);
    	s1[strlen(s1) - 1] = '\0';
    	printf("author=");
    	fgets(s2, 2000, stdin);
    	s2[strlen(s2) - 1] = '\0';
    	printf("genre=");
    	fgets(s3, 2000, stdin);
    	s3[strlen(s3) - 1] = '\0';
    	printf("publisher=");
    	fgets(s4, 2000, stdin);
    	s4[strlen(s4) - 1] = '\0';
    	printf("page_count=");
    	fgets(s5, 2000, stdin);
    	s5[strlen(s5) - 1] = '\0';

    	// parsare json
    	JSON_Value *root_value = json_value_init_object();
    	JSON_Object *root_object = json_value_get_object(root_value);
    	char *serialized_string = NULL;
    	json_object_set_string(root_object, "title", s1);
    	json_object_set_string(root_object, "author", s2);
    	json_object_set_string(root_object, "genre", s3);
    	json_object_set_number(root_object, "page_count", (double)atoi(s5));
		json_object_set_string(root_object, "publisher", s4);
    	serialized_string = json_serialize_to_string_pretty(root_value);
    	
    	// alcatuire body
    	body = (char**) malloc(1 * sizeof(char*));
    	body[0] = strdup(serialized_string);
    	
    	// trimitre tip POST cu JWT setat
    	sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    	message = compute_post_request("34.118.48.238", " /api/v1/tema/library/books", "application/json", body, 1, NULL, 0, 1, JWT);
 
    	send_to_server(sockfd, message);
    	response = receive_from_server(sockfd);
    	char code = getCode(response);

    	// daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	strcpy(response, "add_book successful");
	    	puts(response);
	    }

    	json_free_serialized_string(serialized_string);
    	json_value_free(root_value);
    	close_connection(sockfd);
	}

	if (strcmp("logout\n",buffer) == 0) {

		// trimitere tip GET cu cookies setate ( demonstrez ca ma deloghez fiind logat )
    	sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    	message = compute_get_request("34.118.48.238", " /api/v1/tema/auth/logout", NULL, cookies, cookie_size, 0, NULL);

    	send_to_server(sockfd, message);
    	response = receive_from_server(sockfd);
    	char code = getCode(response);

    	// daca am primit eroare codul este 4, printez eroarea
    	// altfel am mesaj de succes sau print informatii (depinde de comanda)
    	if (code == '4') {
	    	char *ret = strstr(response, "\r\n\r\n");
	    	char *p = strtok(ret, "\r\n");
	    	puts(p);
	    } else {
	    	strcpy(response, "logout successful");
	    	puts(response);
	    }
	    // daca ma deloghez resetez cookiurile si JWT
	    cookie_size = 0;
	    JWT = NULL;
    	close_connection(sockfd);
	}

	if (strcmp("exit\n", buffer) == 0)
		break;
	}

    return 0;
}


