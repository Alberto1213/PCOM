#include <iostream>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "single_include/nlohmann/json.hpp"
#include "helpers.h"
#include "requests.h"
#include <string>
#include <unordered_set>

#define HOST "34.254.242.81"

using json = nlohmann::json;
using namespace std;

void reg(int sockfd)
{
    string username;
    cout << "username=";
    getline(cin, username);

    string password;
    cout << "password=";
    getline(cin, password);

    if (username.find(' ') != std::string::npos)
    {
        cout << "Invalid username and/or password!\n";
        return;
    }
    else
    {
        if (password.find(' ') != std::string::npos)
        {
            cout << "Invalid username and/or password!\n";
            return;
        }
    }

    json j;
    j["username"] = username;
    j["password"] = password;

    char **json_data;
    json_data = (char **)calloc(1, sizeof(char *));

    (*json_data) = (char *)calloc(j.dump().length() + 1, sizeof(char));
    strcpy((*json_data), j.dump().c_str());

    char *message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", json_data, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);

    free(message);
    free(json_data[0]);
    free(json_data);

    char *response;
    response = receive_from_server(sockfd);

    char *begin_with_code = strchr(response, ' ');
    if (!strncmp(begin_with_code + 1, "201", 3))
    {
        cout << "Account registered successfully!\n";
    }
    if (!strncmp(begin_with_code + 1, "400", 3))
    {
        char *json_response;
        json_response = basic_extract_json_response(response);

        json j2;
        j2 = json::parse(json_response);

        if (j2.contains("error"))
        {
            cout << j2.at("error");
            cout << endl;
        }
        else
        {
            cout << "Bad request!\n";
        }
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }
}

string login(int sockfd)
{
    string username;
    cout << "username=";
    getline(cin, username);

    string password;
    cout << "password=";
    getline(cin, password);

    if (username.find(' ') != std::string::npos)
    {
        cout << "Invalid username and/or password!\n";
        return "";
    }
    else
    {
        if (password.find(' ') != std::string::npos)
        {
            cout << "Invalid username and/or password!\n";
            return "";
        }
    }

    json j;
    j["username"] = username;
    j["password"] = password;

    char **json_data;
    json_data = (char **)calloc(1, sizeof(char *));

    (*json_data) = (char *)calloc(j.dump().length() + 1, sizeof(char));
    strcpy((*json_data), j.dump().c_str());

    char *message;
    message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", json_data, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);

    free(message);
    free(json_data[0]);
    free(json_data);

    char *response;
    response = receive_from_server(sockfd);

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        cout << "Logged in successfully!\n";
        char *cookie_start;
        cookie_start = strstr(response, "connect.sid");

        char *cookie_finish;
        cookie_finish = strchr(cookie_start, ';');

        string s;
        s.assign(cookie_start, cookie_finish - cookie_start);
        return s;
    }
    if (!strncmp(begin_with_code + 1, "400", 3))
    {
        char *json_response;
        json_response = basic_extract_json_response(response);

        json j2;
        j2 = json::parse(json_response);

        if (j2.contains("error"))
        {
            cout << j2.at("error");
            cout << endl;
        }

        else
        {
            cout << "Bad request!\n";
        }
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }

    return "";
}

json access(int sockfd, string sessionCookie)
{
    char **cookies;
    cookies = (char **)calloc(1, sizeof(char *));

    (*cookies) = (char *)calloc(sessionCookie.length() + 1, sizeof(char));
    strcpy((*cookies), sessionCookie.c_str());

    char *message;
    message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(message);
    free(cookies[0]);
    free(cookies);

    char *response;
    response = receive_from_server(sockfd);

    json j;

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        cout << "Access granted!\n";

        char *json_response;
        json_response = basic_extract_json_response(response);
        j = json::parse(json_response);
        return j;
    }
    if (!strncmp(begin_with_code + 1, "401", 3))
    {
        char *json_response;
        json_response = basic_extract_json_response(response);

        json j2;
        j2 = json::parse(json_response);
        cout << j2.at("error");
        cout << endl;
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }

    return j;
}

void get_books(int sockfd, json token_json)
{
    if (token_json.contains("token") == false)
    {
        cout << "No acces to the library!\n";
        return;
    }

    string token_value;
    token_value = token_json.at("token");

    char *auth;
    auth = (char *)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    char *message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0, auth);

    send_to_server(sockfd, message);

    free(auth);
    free(message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "[]"))
    {
        cout << "Lista este goala!\n";
        return;
    }
    json resp_json;
    resp_json = json::parse(strchr(response, '['));

    cout << resp_json.dump(4);
    cout << endl;
}

void get_book(int sockfd, json token_json)
{
    if (token_json.contains("token") == false)
    {
        cout << "No acces to the library!\n";
        return;
    }

    string id;
    cout << "id=";
    getline(cin, id);

    if (!id.length())
    {
        cout << "Invalid ID!\n";
        return;
    }
    size_t i = 0;
    while (i < id.length())
    {
        if (id.at(i) < '0' || id.at(i) > '9')
        {
            cout << "Invalid ID!\n";
            return;
        }
        i++;
    }

    string url;
    url = "/api/v1/tema/library/books/";
    url = url + id;

    string token_value;
    token_value = token_json.at("token");

    char *auth;
    auth = (char *)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    char *message = compute_get_request(HOST, url.c_str(), NULL, NULL, 0, auth);
    send_to_server(sockfd, message);

    free(auth);
    free(message);

    char *response = receive_from_server(sockfd);

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        if (strchr(response, '{'))
        {
            json resp_json;
            resp_json = json::parse(strchr(response, '{'));
            cout << resp_json.dump(4);
            cout << endl;
        }
        else
            cout << "1";
    }
    if (!strncmp(begin_with_code + 1, "404", 3))
    {
        cout << "No book with this ID!\n";
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }
}

void add_book(int sockfd, json token_json)
{
    if (token_json.contains("token") == false)
    {
        cout << "No acces to the library!!\n";
        return;
    }

    string title;
    cout << "title=";
    getline(cin, title);

    string author;
    cout << "author=";
    getline(cin, author);

    string genre;
    cout << "genre=";
    getline(cin, genre);

    string publisher;
    cout << "publisher=";
    getline(cin, publisher);

    string page_count;
    cout << "page_count=";
    getline(cin, page_count);

    if (title.length() == 0 || author.length() == 0 || genre.length() == 0 || publisher.length() == 0 || page_count.length() == 0)
    {
        cout << "Invalid book informations!\n";
        return;
    }

    size_t i = 0;
    while (i < page_count.length())
    {
        if (page_count.at(i) < '0' || page_count.at(i) > '9')
        {
            cout << "Invalid number of pages!\n";
            return;
        }
        i++;
    }

    json j;
    j["title"] = title;
    j["author"] = author;
    j["genre"] = genre;
    j["publisher"] = publisher;
    j["page_count"] = page_count;

    string token_value;
    token_value = token_json.at("token");

    char *auth;
    auth = (char *)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    char **json_data;
    json_data = (char **)calloc(1, sizeof(char *));

    (*json_data) = (char *)calloc(j.dump().length() + 1, sizeof(char));
    strcpy((*json_data), j.dump().c_str());

    char *message;
    message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", json_data, 1, NULL, 0, auth);
    send_to_server(sockfd, message);

    free(json_data[0]);
    free(json_data);
    free(message);
    free(auth);

    char *response;
    response = receive_from_server(sockfd);

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        cout << "Book succesfully added!\n";
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }
}

void delete_book(int sockfd, json token_json)
{
    if (token_json.contains("token") == false)
    {
        cout << "No acces to the library!!\n";
        return;
    }

    string id;
    cout << "id=";
    getline(cin, id);

    if (!id.length())
    {
        cout << "ID invalid!\n";
        return;
    }

    size_t i = 0;
    while (i < id.length())
    {
        if (id.at(i) < '0' || id.at(i) > '9')
        {
            cout << "Invalid ID!\n";
            return;
        }
        i++;
    }

    string url;
    url = "/api/v1/tema/library/books/";
    url = url + id;

    string token_value;
    token_value = token_json.at("token");

    char *auth;
    auth = (char *)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    char *message = compute_delete_request(HOST, url.c_str(), NULL, NULL, 0, auth);
    send_to_server(sockfd, message);

    free(auth);
    free(message);

    char *response = receive_from_server(sockfd);

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        cout << "Book succesfully deleted!\n";
    }

    if (!strncmp(begin_with_code + 1, "404", 3))
    {
        cout << "No book with this ID!\n";
    }

    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }
}

int logout(int sockfd, string &sessionCookie)
{
    char **cookies;
    cookies = (char **)calloc(1, sizeof(char *));

    (*cookies) = (char *)calloc(sessionCookie.length() + 1, sizeof(char));
    strcpy((*cookies), sessionCookie.c_str());

    char *message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(message);
    free(cookies[0]);
    free(cookies);

    char *response = receive_from_server(sockfd);

    char *begin_with_code;
    begin_with_code = strchr(response, ' ');

    if (!strncmp(begin_with_code + 1, "200", 3))
    {
        cout << "You've been logged out!\n";
        sessionCookie.clear();
        return 1;
    }
    if (!strncmp(begin_with_code + 1, "400", 3))
    {
        cout << "You are not logged in!\n";
    }
    if (!strncmp(begin_with_code + 1, "429", 3))
    {
        cout << "Too many requests! Try again later!\n";
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;

    string firstCommand;
    string sessionCookie;

    unordered_set<string> available;
    available.insert("register");
    available.insert("login");
    available.insert("enter_library");
    available.insert("get_books");
    available.insert("get_book");
    available.insert("add_book");
    available.insert("delete_book");
    available.insert("logout");
    available.insert("exit");

    json token_json;

    for (;;)
    {
        if (sockfd <= 0)
        {
            cout << "Connection failed\n";
            return 0;
        }

        getline(cin, firstCommand);
        sockfd = open_connection(HOST, 8080, AF_INET, SOCK_STREAM, 0);

        if (!available.count(firstCommand))
        {
            cout << "Invalid command\n";
            continue;
        }

        if (!firstCommand.compare("exit"))
        {
            close(sockfd);
            break;
        }

        if (!firstCommand.compare("register"))
        {
            if (!sessionCookie.empty())
            {
                cout << "User already connected!\n";
                continue;
            }
            reg(sockfd);
        }

        if (!firstCommand.compare("login"))
        {
            if (!sessionCookie.empty())
            {
                cout << "User already connected!\n";
                continue;
            }
            sessionCookie = login(sockfd);
        }

        if (!firstCommand.compare("enter_library"))
        {
            token_json = access(sockfd, sessionCookie);
        }

        if (!firstCommand.compare("get_books"))
        {
            get_books(sockfd, token_json);
        }

        if (!firstCommand.compare("add_book"))
        {
            add_book(sockfd, token_json);
        }

        if (!firstCommand.compare("get_book"))
        {
            get_book(sockfd, token_json);
        }

        if (!firstCommand.compare("delete_book"))
        {
            delete_book(sockfd, token_json);
        }

        if (!firstCommand.compare("logout"))
        {
            if (logout(sockfd, sessionCookie) == 1)
            {
                token_json.clear();
            }
        }
        close(sockfd);
    }

    return 0;
}