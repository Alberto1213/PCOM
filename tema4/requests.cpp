#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, char *query_params, char **cookies, int cookies_count, char *auth)
{
    char *line;
    char *message;
    line = (char *)calloc(1000, sizeof(char));
    message = (char *)calloc(4096, sizeof(char));

    if (!query_params)
    {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    if (query_params)
    {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    }

    compute_message(message, line);

    memset(line, 0, 1000);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth)
    {
        memset(line, 0, 1000);
        strcat(line, "Authorization: Bearer ");
        strcat(line, auth);
    }
    compute_message(message, line);

    if (cookies)
    {
        memset(line, 0, 1000);
        strcat(line, "Cookie: ");

        int i = 0;
        while (i < cookies_count - 1)
        {
            strcat(line, *(cookies + i));
            strcat(line, ";");
            i++;
        }

        strcat(line, *(cookies + cookies_count - 1));
        compute_message(message, line);
    }

    compute_message(message, "");

    return message;
    free(line);
}

char *compute_post_request(const char *host, const char *url, const char *content_type, char **body_data, int body_data_fields_count, char **cookies, int cookies_count, char *auth)
{
    char *body_data_buffer;
    char *message;
    char *line;
    message = (char *)calloc(4096, sizeof(char));
    line = (char *)calloc(1000, sizeof(char));
    body_data_buffer = (char *)calloc(1000, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    memset(line, 0, 1000);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth)
    {
        memset(line, 0, 1000);
        strcat(line, "Authorization: Bearer ");
        strcat(line, auth);
    }
    compute_message(message, line);

    int body_data_size = 0;
    int i = 0;
    while (i < body_data_fields_count - 1)
    {
        strcat(body_data_buffer, *(body_data + i));
        strcat(body_data_buffer, "&");
        body_data_size = body_data_size + strlen(*(body_data + i)) + 1;
        i++;
    }

    strcat(body_data_buffer, *(body_data + body_data_fields_count - 1));
    body_data_size = body_data_size + strlen(*(body_data + body_data_fields_count - 1));

    sprintf(line, "Content-Type: %s\r\nContent-Length: %d", content_type, body_data_size);
    compute_message(message, line);

    if (cookies)
    {
        memset(line, 0, 1000);
        strcat(line, "Cookie: ");

        int i = 0;
        while (i < cookies_count - 1)
        {
            strcat(line, *(cookies + i));
            strcat(line, ";");
            i++;
        }

        strcat(line, *(cookies + cookies_count - 1));
        compute_message(message, line);
    }

    compute_message(message, "");

    memset(line, 0, 1000);
    strcat(message, body_data_buffer);

    return message;
    free(line);
}

char *compute_delete_request(const char *host, const char *url, char *query_params, char **cookies, int cookies_count, char *auth)
{
    char *line;
    char *message;
    line = (char *)calloc(1000, sizeof(char));
    message = (char *)calloc(4096, sizeof(char));

    if (query_params)
    {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    }
    if (!query_params)
    {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    memset(line, 0, 1000);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth)
    {
        memset(line, 0, 1000);
        strcat(line, "Authorization: Bearer ");
        strcat(line, auth);
    }
    compute_message(message, line);

    if (cookies)
    {
        memset(line, 0, 1000);
        strcat(line, "Cookie: ");

        int i = 0;
        while (i < cookies_count - 1)
        {
            strcat(line, *(cookies + i));
            strcat(line, ";");
            i++;
        }

        strcat(line, *(cookies + cookies_count - 1));
        compute_message(message, line);
    }

    compute_message(message, "");

    return message;
    free(line);
}
