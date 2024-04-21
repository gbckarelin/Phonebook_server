#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include "lib/json/single_include/nlohmann/json.hpp"
#include <thread>
using json = nlohmann::json;
const int PORT = 8080;
const int BACKLOG = 10;

void logMessage(std::ofstream &logfile, const std::string &message)
{
    // std::cout << message << std::endl;
    logfile << message << std::endl;
}

int createSocket(std::ofstream &logfile)
{
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        logMessage(logfile, "Socket creation failed");
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return serverSocket;
}

void setSocketOptions(int serverSocket, std::ofstream &logfile)
{
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        logMessage(logfile, "Setsockopt failed");
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
}

void bindSocket(int serverSocket, std::ofstream &logfile)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        logMessage(logfile, "Bind failed");
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
}

void listenForConnections(int serverSocket, std::ofstream &logfile)
{
    if (listen(serverSocket, BACKLOG) < 0)
    {
        logMessage(logfile, "Listen failed");
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server srart on port " << PORT << std::endl;
}

std::string getUsername(const std::string &json_data, std::ofstream &logfile)
{
    try
    {
        json username = json::parse(json_data);
        if (username.find("username") != username.end())
        {
            return username["username"];
        }
        else
        {
            logMessage(logfile, "Cant find username");
            return "";
        }
    }
    catch (const std::exception &e)
    {
        logMessage(logfile, "Error parsing JSON data: " + std::string(e.what()));
        return "";
    }
}

std::string getJsonAction(const std::string &json_data, std::ofstream &logfile)
{
    try
    {
        json action = json::parse(json_data);
        if (action.find("action") != action.end())
        {
            return action["action"];
        }
        else
        {
            logMessage(logfile, "Key 'action' not found in JSON data");
            return "";
        }
    }
    catch (const std::exception &e)
    {
        logMessage(logfile, "Error parsing JSON data: " + std::string(e.what()));
        return "";
    }
}

void addContact(const std::string &username, const std::string &json_data, std::ofstream &logfile)
{
    json contact = json::parse(json_data);
    std::string filename = username + ".json";
    json data;
    std::ifstream file(filename);
    if (!file.good())
    {
        std::ofstream outfile(filename, std::ios::out);
        outfile << "[" << std::endl;
        outfile << "{" << std::endl;
        outfile << "\"nickname\": ";
        outfile << contact["nickname"].dump(4) << "," << std::endl;
        outfile << "\"phone_number\": ";
        outfile << contact["phone_number"].dump(4) << "," << std::endl;
        outfile << "\"info\": ";
        outfile << contact["info"].dump(4) << std::endl;
        outfile << "}" << std::endl;
        outfile << "]";
        outfile.close();
    }
    else
    {
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        if (!data.empty())
        {
            data.pop_back();
            data.erase(0, 1);
        }
        file.close();
        std::ofstream outfile(filename, std::ios::trunc);
        outfile << "[" << std::endl;
        outfile << data << std::endl;
        if (data.size() == 0)
        {
            outfile << "{" << std::endl;
        }
        else
        {
            outfile << ",{" << std::endl;
        }
        outfile << "\"nickname\": ";
        outfile << contact["nickname"].dump(4) << "," << std::endl;
        outfile << "\"phone_number\": ";
        outfile << contact["phone_number"].dump(4) << "," << std::endl;
        outfile << "\"info\": ";
        outfile << contact["info"].dump(4) << std::endl;
        outfile << "}" << std::endl;
        outfile << "]";
        outfile.close();
    }
    logMessage(logfile, "Contact added successfully to " + filename);
}

void deleteContact(const std::string &username, const std::string &json_data, std::ofstream &logfile)
{
    json contact = json::parse(json_data);

    std::string filename = username + ".json";
    try
    {

        if (contact.find("delete_choice") != contact.end())
        {
            if (contact["delete_choice"] == "P")
            {
                std::string phone = contact["phone_number"];

                std::ifstream file(filename);
                if (!file.is_open())
                {
                    logMessage(logfile, "Failed to open " + filename + " for reading");
                    return;
                }

                json data;
                file >> data;
                file.close();

                bool deleted = false;
                for (auto it = data.begin(); it != data.end();)
                {
                    if ((*it)["phone_number"] == phone)
                    {
                        it = data.erase(it);
                        deleted = true;
                    }
                    else
                    {
                        ++it;
                    }
                }

                if (!deleted)
                {
                    logMessage(logfile, "Failed to find contact with phone number: " + phone);
                    return;
                }

                std::ofstream file2(filename, std::ios::trunc);
                if (!file2.is_open())
                {
                    logMessage(logfile, "Failed to open " + filename + " for writing");
                    return;
                }

                file2 << data.dump(4);
                file2.close();
                data = "";
                logMessage(logfile, "Contact " + phone + " was deleted!");
            }
            else
            {
                std::string nickname = contact["nickname"];
                std::ifstream file(filename);
                if (!file.is_open())
                {
                    logMessage(logfile, "Failed to open " + filename + " for reading");
                    return;
                }

                json data;
                file >> data;
                file.close();

                bool deleted = false;
                for (auto it = data.begin(); it != data.end();)
                {
                    if ((*it)["nickname"] == nickname)
                    {
                        it = data.erase(it);
                        deleted = true;
                    }
                    else
                    {
                        ++it;
                    }
                }

                if (!deleted)
                {
                    logMessage(logfile, "Failed to find contact with nickname: " + nickname);
                    return;
                }

                std::ofstream file2(filename, std::ios::trunc);
                if (!file2.is_open())
                {
                    logMessage(logfile, "Failed to open " + filename + " for writing");
                    return;
                }

                file2 << data.dump(4);
                file2.close();
                data = "";
                logMessage(logfile, "Contact " + nickname + " was deleted!");
            }
        }
        else
        {
            logMessage(logfile, "Key 'action' not found in JSON data");
        }
    }
    catch (const std::exception &e)
    {
        logMessage(logfile, "Error parsing JSON data: " + std::string(e.what()));
    }
}

std::string findContact(const std::string &username, const std::string &json_data, std::ofstream &logfile)
{
    json contact = json::parse(json_data);
    std::string search = contact["search_term"];
    std::string filename = username + ".json";
    std::ifstream file(filename);
    if (!file.is_open())
    {
        logMessage(logfile, "Failed to open " + filename + " for reading");
    }
    json find;
    file >> find;
    json found_contacts = json::array();
    bool found = false;
    for (const auto &contacts : find)
    {
        if (contacts["nickname"] == search ||
            contacts["phone_number"] == search ||
            contacts["info"]["first_name"] == search ||
            contacts["info"]["last_name"] == search ||
            contacts["info"]["middle_name"] == search ||
            contacts["info"]["note"] == search)
        {
            found_contacts.push_back(contacts);
            found = true;
        }
    }
    return found ? found_contacts.dump() : "";
}

std::string listingContacts(const std::string &username, const std::string &json_data, std::ofstream &logfile)
{
    std::string filename = username + ".json";
    std::ifstream file(filename);
    if (!file.is_open())
    {
        logMessage(logfile, "Failed to open " + filename + " for reading");
    }
    json find;
    file >> find;
    file.close();
    if (find.size() == 0)
    {
        return "";
    }
    else
    {
        json contacts = json::array();
        contacts.push_back(find);
        return contacts.dump();
    }
}

void processMessage(const char *message, int clientSocket, std::ofstream &logfile)
{
    std::string json_data(message);
    std::string username_client = getUsername(json_data, logfile);
    if (username_client.empty())
    {
        logMessage(logfile, "Failed to get username");
    }
    std::string json_action = getJsonAction(json_data, logfile);
    if (json_action.empty())
    {
        logMessage(logfile, "Failed to get action from JSON data");
    }
    if (json_action == "add_contact")
    {
        addContact(username_client, json_data, logfile);
        const char *response = "Contact Added!";
        send(clientSocket, response, strlen(response), 0);
    }
    else if (json_action == "delete_contact")
    {
        deleteContact(username_client, json_data, logfile);
        const char *response = "Contact Deleted!";
        send(clientSocket, response, strlen(response), 0);
    }
    else if (json_action == "find_contact")
    {
        std::string found_contacts_json = findContact(username_client, json_data, logfile);
        if (!found_contacts_json.empty())
        {
            const char *response = found_contacts_json.c_str();
            send(clientSocket, response, strlen(response), 0);
            logMessage(logfile, "Found contacts sent to client");
        }
        else
        {
            const char *response = "Contacts not found";
            send(clientSocket, response, strlen(response), 0);
            logMessage(logfile, "No contacts found");
        }
    }
    else if (json_action == "listing_contacts")
    {
        std::string data = listingContacts(username_client, json_data, logfile);
        if (!data.empty())
        {
            const char *response = data.c_str();
            send(clientSocket, response, strlen(response), 0);
            logMessage(logfile, "Found contacts sent to client");
        }
        else
        {
            const char *response = "Phone book is clear";
            send(clientSocket, response, strlen(response), 0);
            logMessage(logfile, "Phone book is clear");
        }
    }
}

void handleClient(int clientSocket, std::ofstream &logfile)
{
    char buffer[1024] = {0};
    ssize_t valread = read(clientSocket, buffer, 1024);
    logMessage(logfile, "Received message: " + std::string(buffer));

    if (valread > 0)
    {
        processMessage(buffer, clientSocket, logfile);
    }
    else
    {
        logMessage(logfile, "Failed to find JSON data in request");
    }

    logMessage(logfile, "Buffer Clear\n");
    memset(buffer, 0, sizeof(buffer));
    close(clientSocket);
}

void acceptConnections(int serverSocket, std::ofstream &logfile)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    while (true)
    {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen)) < 0)
        {
            perror("Accept failed");
            logMessage(logfile, "Accept failed");
            exit(EXIT_FAILURE);
        }
        std::thread clientThread(handleClient, clientSocket, std::ref(logfile));
        clientThread.detach();
    }
}

int main()
{
    std::ofstream logfile("server.log", std::ios::app);
    if (!logfile.is_open())
    {
        std::cerr << "Failed to open log file" << std::endl;
        return EXIT_FAILURE;
    }

    int serverSocket = createSocket(logfile);
    setSocketOptions(serverSocket, logfile);
    bindSocket(serverSocket, logfile);
    listenForConnections(serverSocket, logfile);
    acceptConnections(serverSocket, logfile);

    logfile.close();
    return 0;
}
