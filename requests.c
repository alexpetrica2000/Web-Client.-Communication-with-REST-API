#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

// REMINDER : Functiile sunt asemanatoare laboratorului 10 doar ca am adaugat
// campul type si tokenul JWT; Daca type este setat pe 1 se adauga tokenul JWT

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, int type, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *line_aux = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line, "HOST: %s", host);
    compute_message(message,line);

    if (type == 1 && JWT != NULL) {
    	sprintf(line, "Authorization: Bearer %s", JWT);
    	compute_message(message, line);
    }

    if (cookies != NULL && cookies_count != 0) {
        int i;
        strcpy(line, "Cookie: ");
        
        for (i = 0; i < cookies_count - 1; i++) {
                sprintf(line_aux, "%s;\n", cookies[i]);
                strcat(line,line_aux);
        }

        sprintf(line_aux, "%s", cookies[cookies_count - 1]);
        strcat(line, line_aux);
        compute_message(message, line);
    }
    
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, int type, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    char *line_aux = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "HOST: %s", host);
    compute_message(message,line);

    if (type == 1 && JWT != NULL) {
    	sprintf(line, "Authorization: Bearer %s", JWT);
    	compute_message(message, line);
    }

    int i;
    int nr_char = 0;
    memset(body_data_buffer, 0, LINELEN);

    for (i = 0; i < body_data_fields_count; i++) {
        nr_char += strlen(body_data[i]);
            strcat(body_data_buffer, body_data[i]);
            if (i != body_data_fields_count - 1)
                strcat(body_data_buffer,"&");

    }

    body_data_buffer[strlen(body_data_buffer)] = '\0';
    nr_char += body_data_fields_count - 1;
    sprintf(line,"Content-Length: %d", nr_char);
    compute_message(message,line);

    sprintf(line, "Content-Type: %s",content_type);
    compute_message(message, line);

     if (cookies != NULL) {
        int i;
        strcpy(line, "Cookie: ");
        
        for (i = 0; i < cookies_count - 1; i++) {
                sprintf(line_aux, "%s;\n", cookies[i]);
                strcat(line,line_aux);
        }

        sprintf(line_aux, "%s", cookies[cookies_count-1]);
        strcat(line, line_aux);
        compute_message(message, line);
    }

    compute_message(message,"");
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
 
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "HOST: %s", host);
    compute_message(message,line);

    if (JWT != NULL) {
	    sprintf(line, "Authorization: Bearer %s", JWT);
	    compute_message(message, line);
	}

    compute_message(message,"");
   
    free(line);
    return message;
}