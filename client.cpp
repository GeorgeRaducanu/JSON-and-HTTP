// Copyright Raducanu George-Cristian 321CAb 2022-2023

// Credit Laboratorul 9 PCOM

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

// pun si nlohmann aici ca sa fct si in caz ca nu exista pe 
// calculatorul pe care ruleaza (daca nu are nlohmann instalat)
#include "json.hpp"

// acum asa cum zice in documentatie
// acest using pentru lejeritate si lizibilitate
// in cod
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    char *response;
    char *request;
    int sockfd;
    char my_cookie[400] = {0};

    int ok = 1;
    char command[400] = {0};

    char token_jwt[400] = {0};

    char *helper_url = (char *)"34.254.242.81";
    char *help_url_bonus = (char *)"34.254.242.81:8080";
    
    // am ales ok si nu while(1) cu break ca sa fie codul mai "corect"
    // din punct de vedere al executiei programului (fara jump)
    while (ok) {
        memset(command, 0, sizeof(command));
        fgets(command, 400, stdin);

        if (strncmp(command, "exit", 4) == 0) {
            printf("Exiting! Bye bye!\n");
            ok = 0;
        } else if (strncmp(command, "register", 8) == 0) {
            // aici register
            
            // se citesc in buffere datele cu care
            // userul doreste sa se inregistreze in sistem
            char userN[100] = {0};
            printf("username=");
            fgets(userN, 100, stdin);
            int lg_user = strlen(userN);
            userN[lg_user - 1] = '\0';

            char passW[100] = {0};
            printf("password=");
            fgets(passW, 100,stdin);
            int lg_pass = strlen(passW);
            passW[lg_pass - 1] = '\0';

            // pun in json datele
            json my_reg;
            my_reg["username"] = userN;
            my_reg["password"] = passW;

            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // calculez si trimit cererea de tip post
            request = compute_post_request(help_url_bonus,
             "/api/v1/tema/auth/register",
              "application/json",
               my_reg.dump().c_str(), NULL, 0, NULL);
            //printf("%s\n", request);

            send_to_server(sockfd, request);

            // primesc raspunsul
            response = receive_from_server(sockfd);
            // in fct de raspuns afisez altceva
            if (strstr(response, "Bad Request") != NULL) {
                printf("The username %s is already taken.\n", userN);
            } else {
                //printf("%s\n", response);
                printf("Registration completed!\n");
            }
            free(response);
            free(request);
            // inchid conexiunea si dau free
            close_connection(sockfd);

        } else if (strncmp(command, "login", 5) == 0) {
            // aici login

            // citesc in buffere datele de logare
            char userN[100] = {0};
            printf("username=");
            fgets(userN, 100, stdin);
            int lg_user = strlen(userN);
            userN[lg_user - 1] = '\0';

            char passW[100] = {0};
            printf("password=");
            fgets(passW, 100,stdin);
            int lg_pass = strlen(passW);
            passW[lg_pass - 1] = '\0';

            // le pun in json
            json my_reg;
            my_reg["username"] = userN;
            my_reg["password"] = passW;

            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // calculez si dupa trimit cererea de tip post
            request = compute_post_request(help_url_bonus,
             "/api/v1/tema/auth/login",
              "application/json",
               my_reg.dump().c_str(), NULL, 0, NULL);

            //printf("My login: %s\n", request);
            send_to_server(sockfd, request);

            // primesc raspunsul
            response = receive_from_server(sockfd);

            // in fct de continutul raspunsului se da mesaj corespunzator
            // clientului
            if (strstr(response, "Credentials are not good!") != NULL) {
                printf("The provided credidentials are wrong. Try again!\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            if (strstr(response, "No account with this username") != NULL) {
                printf("The provided account does not exist!\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            // trb sa gasesc cookie-ul
            memset(my_cookie, 0, sizeof(my_cookie));
            strcpy(my_cookie, strstr(response, "Set-Cookie: "));
            memcpy(my_cookie, my_cookie + 12, sizeof(my_cookie));
            strtok(my_cookie, ";");
            // inchid conexiunea
            close_connection(sockfd);
            printf("Login succesfully!\n");
            free(response);
            free(request);

        } else if (strncmp(command, "enter_library", 13) == 0) {
            // cerere de a intra in biblioteca
            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // aici container ul meu unde pun cookie ul meu
            // fac asta ca in fct aveam lista de cookie uri, aici am unul singur
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);
            // calculez si trimit cererea de tip get
            request = compute_get_request(help_url_bonus,
             "/api/v1/tema/library/access",
                NULL, cookie_container, 1, NULL );
            //printf("%s\n", request);
            free(*cookie_container);
            free(cookie_container);

            send_to_server(sockfd, request);
            // primesc raspunsul
            response = receive_from_server(sockfd);
            // in fct de raspuns notific user ul corespunzator
            if (strstr(response, "Unauthorized") != NULL) {
                printf("You are not logged in!\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            // retin token ul
            memset(token_jwt, 0, sizeof(token_jwt));
            strcpy(token_jwt, strstr(response, "{"));
            // sar peste cate caractere trebuie ca sa ajung la incputul
            // tokenului
            memcpy(token_jwt, token_jwt + 10, sizeof(token_jwt));
            strtok(token_jwt, "\"");
            // inchid si eliberez toata memoria si notific userul in caz de 
            // succes
            free(response);
            free(request);
            close_connection(sockfd);
            printf("Library accesed with succes.\n");
            //printf("Token JWT is: %s\n", token_jwt);

        } else if (strncmp(command, "get_books", 9) == 0) {
            // aici comanda get_books
            // aici o sa folosesc si tokenul
            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // aceeasi poveste cu cookie_container ca si mai sus
            // ca sa nu modific functiile din laborator
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);
            // calculez cererea si dupa o sa o trimit trimitand atat
            // cookie cat si token_jwt
            request = compute_get_request(help_url_bonus,
             "/api/v1/tema/library/books",
                NULL, cookie_container, 1, token_jwt );
            //printf("%s\n", request);
            free(*cookie_container);
            free(cookie_container);

            send_to_server(sockfd, request);
            // primesc raspunsul
            response = receive_from_server(sockfd);
            //printf("%s\n", response);
            // in functie de ce primesc in raspuns notific user ul
            if (strstr(response, "Error when decoding token") != NULL) {
                printf("User is not logged or does not have permissions to acces library!\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            // aici in  caz de succes afisez ce trebuie
            char *info = strstr(response, "[{");
            printf("Accesed succesfully!\n");
            printf("The content of the library:\n");
            if (info == NULL) {
                printf("No books\n");
            } else {
                printf("%s\n", info);
            }
            // inchid si eliberez conexiunea
            close_connection(sockfd);
            free(response);
            free(request);

        } else if (strncmp(command, "get_book", 8) == 0) {
            // comanda get_book
            // retin in buffer
            char id_b[100] = {0};
            printf("id=");
            fgets(id_b, 100, stdin);
            int lg_id_b = strlen(id_b);
            id_b[lg_id_b - 1] = '\0';
            // validez corectitudinea datelor
            if (atoi(id_b) <= 0) {
                printf("The data you entered does not respect the requirements!\n");
                printf("Please type another command or try again\n");
                continue;
            }
            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // povestea cu containerul pt cookie ul meu
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);
            char url_helper[400] = "/api/v1/tema/library/books/";
            // concatenez la url is ul cartii
            strcat(url_helper, id_b);
            // printf("Cerere catre %s/n", url_helper);
            // calculez si trimit la server cererea
            request = compute_get_request(help_url_bonus,
             url_helper,
                NULL, cookie_container, 1, token_jwt );
            
            //printf("%s\n",request);
            free(*cookie_container);
            free(cookie_container);

            send_to_server(sockfd, request);

            // primesc raspunsul
            response = receive_from_server(sockfd);

            // in functie de raspuns informez user ul corespunzator
            if (strstr(response, "Error when decoding tokenn") != NULL) {
                printf("The user is not logged in or has not accesed the library\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            if (strstr(response, "No book was found") != NULL) {
                printf("No book with this id was found.\n");
                close_connection(sockfd);
                free(response);
                free(request);
                continue;
            }
            printf("More informations about the book:\n");
            char *extra = strstr(response, "{\"");
            printf("%s\n", extra);
            close_connection(sockfd);
            free(response);
            free(request);
            
        } else if (strncmp(command, "add_book", 8) == 0) {
            // comanda de add book
            // in buffere stochez toate detaliile cartii
            char title[100] = {0};
            printf("title=");
            fgets(title, 100, stdin);
            int lg_title = strlen(title);
            title[lg_title - 1] = '\0';

            char author[100] = {0};
            printf("author=");
            fgets(author, 100, stdin);
            int lg_author = strlen(author);
            author[lg_author - 1] = '\0';

            char genre[100] = {0};
            printf("genre=");
            fgets(genre, 100, stdin);
            int lg_genre = strlen(genre);
            genre[lg_genre - 1] = '\0';

            char publ[100] = {0};
            printf("publisher=");
            fgets(publ, 100, stdin);
            int lg_publ = strlen(publ);
            publ[lg_publ - 1] = '\0';

            char pg_cnt[100] = {0};
            printf("page_count=");
            fgets(pg_cnt, 100, stdin);
            int lg_pg_cnt = strlen(pg_cnt);
            pg_cnt[lg_pg_cnt - 1] = '\0';

            // eventual o mica verificare nr de pag e nr pozitiv
            if (atoi(pg_cnt) <= 0) {
                printf("Data is not in the right format!\n");
                printf("HINT: The number of pages must be a positive integer!\n");
                continue;
            }
            // formez json ul
            json my_book;
            my_book["title"] = title;
            my_book["author"] = author;
            my_book["genre"] = genre;
            my_book["publisher"] = publ;
            my_book["page_count"] = atoi(pg_cnt);
            // stochez aici 
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);

            // calculez si trimit cererea avand grija si cu cookie si token jwt
            request = compute_post_request(helper_url,
             "/api/v1/tema/library/books",
              "application/json",
               my_book.dump().c_str(), cookie_container, 1, token_jwt);
            
            free(*cookie_container);
            free(cookie_container);
            //printf("%s\n", request);

            // deschid conexiunea
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            close_connection(sockfd);
            // in functie de raspunsul serverului voi notifica in mod corespunzator userul
            if (strstr(response, "Error when decoding tokenn") != NULL) {
                printf("Not logged in or in the library. Cannot add books\n");
                free(response);
                free(request);
                continue;
            }
            printf("Book added to collection\n");
            free(response);
            free(request);
            //printf("%s\n", response);
        } else if (strncmp(command, "delete_book", 11) == 0) {
            // aici delete de cartea respectiva
            // citire a id ului cartii ce se doreste a fi stearsa
            char id_d[100] = {0};
            printf("id=");
            fgets(id_d, 100, stdin);
            int lg_id_d = strlen(id_d);
            id_d[lg_id_d - 1] = '\0';

            // verificare a validitatii informatiei inroduse
            if (atoi(id_d) <= 0) {
                printf("The id entered is not a positive integer!\n");
                continue;
            }
            // deschidere conexiune
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);

            // cookie container
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);
            char url_helper[400] = "/api/v1/tema/library/books/";
            // concatenare id la url_helper
            strcat(url_helper, id_d);
            // calcul cerere si ulterior trimitere
            request = compute_delete_request(help_url_bonus,
             url_helper,
                NULL, cookie_container, 1, token_jwt );
            // printf("%s\n", request);
            free(*cookie_container);
            free(cookie_container);

            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            close_connection(sockfd);
            // in functie de raspuns notific userul in mod corespunzator
            if (strstr(response, "No book was deleted") != NULL) {
                printf("No book with the given id\n");
                free(response);
                free(request);
                continue;
            }

            if (strstr(response, "Error when decoding tokenn") != NULL) {
                printf("User not logged in or didn't acces the library\n");
                free(response);
                free(request);
                continue;
            }

            printf("Book deleted succesfully\n");

        } else if (strncmp(command, "logout", 6) == 0) {
            // comanda de logout
            sockfd = open_connection(helper_url, 8080, AF_INET, SOCK_STREAM, 0);
            char **cookie_container = (char **)malloc(sizeof(char *));
            *cookie_container = (char *)malloc(sizeof(char) * 400);
            strcpy(*cookie_container, my_cookie);
            char url_helper[400] = "/api/v1/tema/auth/logout";
            // calculare si trimitere comanda de get
            request = compute_get_request(help_url_bonus,
             url_helper,
                NULL, cookie_container, 1, token_jwt );
            //printf("%s\n", request);
            free(*cookie_container);
            free(cookie_container);

            send_to_server(sockfd, request);

            response = receive_from_server(sockfd);

            //printf("%s\n", response);
            close_connection(sockfd);
            // in orice caz trb sa zeroizez token ul jwt si cookieul
            memset(token_jwt, 0, sizeof(token_jwt));
            memset(my_cookie, 0, sizeof(my_cookie));
            if (strstr(response, "You are not logged in") != NULL) {
                printf("You are not logged in. Please login first\n");
                free(response);
                free(request);
                continue;
            }
            printf("Succesfully logged out\n");
            free(response);
            free(request);

        } else {
            // comanda invalida
            printf("Invalid command. Please enter again!\n");
        }
    }

    return 0;
}
