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

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char* auth_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    // char message[BUFLEN] = {0};
    // char line[LINELEN] = {0};
    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        line[strlen(line) - 1] = '\0';
        compute_message(message, line);
    }

    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params, char **cookies, int cookies_count, char* auth_token) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    // char message[BUFLEN] = {0};
    // char line[LINELEN] = {0};
    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        line[strlen(line) - 1] = '\0';
        compute_message(message, line);
    }

    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    
    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, const char *body_data,
                             char **cookies, int cookies_count, char* auth_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char *)calloc(LINELEN, sizeof(char));
    // char message[BUFLEN] = {0};
    // char line[LINELEN] = {0};
    // char body_data_buffer[LINELEN] = {0};
    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
      compute_message(message, line);

   sprintf(line, "Content-Type: %s", content_type);
      compute_message(message, line);


    if (body_data != NULL) {
        strcat(body_data_buffer, body_data);
    }
    
   

    sprintf(line, "Content-Length: %lu", strlen(body_data_buffer));
    compute_message(message, line);
    
    // sprintf(line, "");
    // compute_message(message, line);
    //sprintf(line, "");

    
    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; ++i) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }
        line[strlen(line) - 1] = '\0';
        compute_message(message, line);
    }
    
    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }
    

    // Step 5: add new line at end of header

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);

    compute_message(message, line);
    
    strcat(message, body_data_buffer);
    compute_message(message, line);

    
    free(line);
    free(body_data_buffer);
    return message;
}
