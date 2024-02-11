#include <stdio.h>  // Including standard input/output library
#include <stdlib.h> // Including standard library
#include <string.h> // Including string manipulation functions
#include <pthread.h> // Including pthread library for multithreading
#include <sys/socket.h> // Including socket-related functions
#include <unistd.h> // Including POSIX operating system API
#include <arpa/inet.h> // Including functions for manipulating IP addresses
#include <stdbool.h> // Including boolean type support

#define PORT 8888 // Defining server port number
#define MAX_CLIENTS 5 // Defining maximum number of clients
#define MAX_MSG_LEN 1024 // Defining maximum message length
#define MAX_USERS 10 // Defining maximum number of users

// Defining a structure for User details
struct User {
    char username[50];
    char name[50];
    char phoneNumber[20];
    char surname[32];
};

// Defining a structure for messages
struct Message {
    char sender[50];
    char receiver[50];
    char content[MAX_MSG_LEN];
};
 // Defining a structure for server data
struct ServerData {
    struct User userList[MAX_USERS];
    struct Message messages[MAX_USERS][MAX_USERS];
};

// Creating an instance of ServerData
struct ServerData globalServerData;
int clientSockets[MAX_CLIENTS]; // Array to store client socket descriptors


   // Function to check messages for a user
void checkMessages(int clientSocket, char* user, struct ServerData* serverData) {
    char messages[MAX_MSG_LEN * MAX_USERS] = {0};
    int messageFound = 0;

// Iterate through the list of users
    int i,j;
    for (i = 0; i < MAX_USERS; ++i) {
        if (strcmp(serverData->userList[i].username, user) == 0) {
            for (j = 0; j < MAX_USERS; ++j) {
             // Check if the current message is addressed to the requested user
                if (strcmp(serverData->messages[i][j].receiver, user) == 0) {
                    strcat(messages, "Sender: ");
                    strcat(messages, serverData->messages[i][j].sender);
                    strcat(messages, "\nContent: ");
                    strcat(messages, serverData->messages[i][j].content);
                    strcat(messages, "\n\n");
                        // Clear the message after retrieving it                 
                    serverData->messages[i][j] = (struct Message){0};
                    messageFound = 1;
                }
            }
        }
    }
// Check if messages were found for the user
    if (messageFound) {
    // Send the collected messages to the client
        send(clientSocket, messages, strlen(messages), 0);
    } else {
        // If no messages were found, send a message indicating so
        char errMsg[] = "No messages found!";
        send(clientSocket, errMsg, strlen(errMsg), 0);
    }
}

 // Function to send a message between users
void sendMessage(int clientSocket, char* sender, char* receiver, char* message, struct ServerData* serverData) {
    int senderIndex = -1, receiverIndex = -1;

 // Find the indices of sender and receiver in the user list
    int i;
    for (i = 0; i < MAX_USERS; ++i) {
        if (strcmp(serverData->userList[i].username, sender) == 0) {
            senderIndex = i;
        }
        if (strcmp(serverData->userList[i].username, receiver) == 0) {
            receiverIndex = i;
        }
    }

 // Check if both sender and receiver exist
    if (senderIndex != -1 && receiverIndex != -1) {
        struct Message newMsg;
   // Fill in the sender, receiver, and message content for the new message
        strcpy(newMsg.sender, sender);
        strcpy(newMsg.receiver, receiver);
        strcpy(newMsg.content, message);

        int messageSaved = 0;// Flag to track if the message was saved
        
           // Iterate through receiver's message box to find an empty slot to save the message
      for (i = 0; i < MAX_USERS; ++i) {
              // Check if the content length of the message box is 0 (empty slot)
          if (strlen(serverData->messages[receiverIndex][i].content) == 0) {
                serverData->messages[receiverIndex][i] = newMsg;
                printf("msg: \n");
                messageSaved = 1;
                break; // Break the loop after saving the message
            }
        }
            // Check if the message was saved
        if (messageSaved) {
            char ackMsg[] = "Message sent successfully!";
            send(clientSocket, ackMsg, strlen(ackMsg), 0);
            char notifyMsg[MAX_MSG_LEN];
            sprintf(notifyMsg, "New message from %s", sender);
            send(clientSockets[receiverIndex], notifyMsg, strlen(notifyMsg), 0);
            
            // Saving the message in the receiver's file
    char filenameReceiver[50];
    strcpy(filenameReceiver, receiver);
    strcat(filenameReceiver, ".txt");
    FILE *fileReceiver = fopen(filenameReceiver, "a");
    fprintf(fileReceiver, "Sender: %s\tReceiver: %s\tContent: %s\n", sender, receiver, message);
    fclose(fileReceiver);
    
    // Saving the message in the sender's file
    char filenameSender[50];
    strcpy(filenameSender, sender);
    strcat(filenameSender, ".txt");
    FILE *fileSender = fopen(filenameSender, "a");
    fprintf(fileSender, "Sender: %s\tReceiver: %s\tContent: %s\n", sender, receiver, message);
    fclose(fileSender);
            
        } else {
            char errMsg[] = "Receiver's message box full!";
            send(clientSocket, errMsg, strlen(errMsg), 0);
        }
    } else {
        char errMsg[] = "Sender or receiver not found!";
        send(clientSocket, errMsg, strlen(errMsg), 0);
    }
}

// Function to handle client interactions in a thread
void *handleClient(void *data) {
    int clientSocket = *((int *)data);// Extracting client socket descriptor
    int index = -1;
    int i;
  // Loop through available client sockets to find an empty slot to store the new client socket
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (clientSockets[i] == 0) {
            clientSockets[i] = clientSocket;
            index = i;
            break;
        }
    }
    struct ServerData *serverData = &globalServerData;
    int valread;
    // Allocate memory for clientAction on the heap
    int *clientAction = (int *)malloc(sizeof(int));
    *clientAction = 0;  
    char username[50] = {0};
    bool loginFlag = false;
    

  // Loop to continuously read client actions and handle accordingly
      while ((valread = read(clientSocket, clientAction, sizeof(int))) > 0) {
      
             // Handle action code 4: send List users except the logged-in user
       if (*clientAction == 4) {
               if(loginFlag){
            char msgBuffer[MAX_USERS * 50] = {0}; 
            for (i = 0; i < MAX_USERS; ++i) {
                if (strcmp(serverData->userList[i].username, username) != 0) {
                    strcat(msgBuffer, serverData->userList[i].username);
                    strcat(msgBuffer, "\n");
                }
            }
             // Send the list of users to the client
            send(clientSocket, msgBuffer, strlen(msgBuffer), 0);
        
               }else{
                  // Send a message to prompt the client to log in first
                   char loginMsg[] = "Please login first !";
                   send(clientSocket, loginMsg, strlen(loginMsg), 0);
               }
            }
              // Handle action code 1: Client login/register
        else if (*clientAction == 1) {
           // Read username, name, phone number, and surname sent by the client
           

            char name[50], phoneNumber[20], surname[50];
            read(clientSocket, username, sizeof(username));
            printf("username recieved is: %s\n",username);
            read(clientSocket, name, sizeof(name));
            printf("Name recieved is: %s\n",name);
            read(clientSocket, phoneNumber, sizeof(phoneNumber));
            printf("phone recieved is: %s\n",phoneNumber);
            read(clientSocket, surname, sizeof(surname));
            printf("surName recieved is: %s\n",surname);
            int userExists = 0;
                  
                   // Check if the user already exists and handle accordingly
      
            for (i = 0; i < MAX_USERS; ++i) {
                if (strcmp(serverData->userList[i].username, username) == 0) {
                    userExists = 1;
                    break;
                }
            }
            if (userExists==0) {
            //use file as storage to save user data 
                FILE *userFile;
                char filename[50];
                strcpy(filename, username);
                strcat(filename, ".txt");

                userFile = fopen(filename, "w");
                fprintf(userFile, "Unique ID: %s\nName: %s\nPhone Number: %s\nSurname: %s\n",
                        username, name, phoneNumber, surname);
                fclose(userFile);
                printf("after add data in a file\n");

                for (i = 0; i < MAX_USERS; ++i) {
                    if (strlen(serverData->userList[i].username) == 0) {
                        strcpy(serverData->userList[i].username, username);
                        strcpy(serverData->userList[i].name,name);
                        strcpy(serverData->userList[i].phoneNumber,phoneNumber);
                        strcpy(serverData->userList[i].surname,surname);
                        break;
                    }
                }
                char successMsg[] = "Login successful!";
                send(clientSocket, successMsg, strlen(successMsg), 0);
            } else {
                char failureMsg[] = "User already exists!";
                send(clientSocket, failureMsg, strlen(failureMsg), 0);
            }
            loginFlag = true;
            // Handle action code 2: Send a message
        } else if (*clientAction == 2) {
           // Check if the client is logged in to send messages
          if(loginFlag){
            char sender[50], receiver[50], message[MAX_MSG_LEN];
            read(clientSocket, sender, sizeof(sender));
            printf("username recieved is: %s\n",sender);
            read(clientSocket, receiver, sizeof(receiver));
            printf("ReceiverID recieved is: %s\n",receiver);
            read(clientSocket, message, sizeof(message));
            printf("Msg recieved is: %s\n",message);
            
            // Code to handle sending messages between users
            sendMessage(clientSocket, sender, receiver, message,serverData);
            }else{
                   char loginMsg[] = "Please login first to send msg!";
                   send(clientSocket, loginMsg, strlen(loginMsg), 0);
               }
          // Handle action code 3: Check messages for the client
        } else if (*clientAction == 3) {
            if(loginFlag){
            char user[50];
            read(clientSocket, user, sizeof(user));
            printf("username recieved is: %s\n",user);
            //Code to check and send messages for the client)
            checkMessages(clientSocket, user,serverData);
            }else{
                   char loginMsg[] = "Please login first to getting new msg!";
                   send(clientSocket, loginMsg, strlen(loginMsg), 0);
               }
               // Handle action code 5: Retrieve user data from file
        } else if (*clientAction == 5) {
            if (loginFlag) {
        // Fetch user data from file
        FILE *userFile;
        char filename[50];
        char user[50];
            read(clientSocket, user, sizeof(user));
            printf("username recieved is: %s\n",user);

        strcpy(filename, user);
        strcat(filename, ".txt");
        // Check if the client is logged in to retrieve messages
        userFile = fopen(filename, "r");
        if (userFile != NULL) {
            char buffer[MAX_MSG_LEN * MAX_USERS];
            size_t bytesRead = fread(buffer, sizeof(char), sizeof(buffer), userFile);
            fclose(userFile);

             // Code to retrieve and send user data from a file
            if (bytesRead > 0) {
                send(clientSocket, buffer, bytesRead, 0);
            } 
            else {
                char errMsg[] = "No data found for the user!";
                send(clientSocket, errMsg, strlen(errMsg), 0);
            }
        } else {
            char errMsg[] = "Error opening user file!";
            send(clientSocket, errMsg, strlen(errMsg), 0);
        }
    } else {
        char loginMsg[] = "Please login first!";
        send(clientSocket, loginMsg, strlen(loginMsg), 0);
    }
        }
         // Handle action code 0: Client termination
        else if(clientAction==0){
           printf("%s is terminate\n",username); 
           break; // Exit the loop
        }
    }

    free(clientAction); // Free allocated memory for clientAction
    close(clientSocket);// Close the client socket
    pthread_exit(NULL);// Exit the thread
}

    // Main function where the server setup and connection handling occur
int main() {
    int serverSocket, clientSocket, addrLen;// Socket and address length variables
    struct sockaddr_in serverAddr, clientAddr;// Server and client address structures
    pthread_t threadId;
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        clientSockets[i] = 0; // Initializing client socket array

    }


    // Creating a socket for the server
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  // Setting up server address information
    memset(&serverAddr, '0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);


    // Binding the server socket to the specified address and port
    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  // Listening for incoming client connections
    listen(serverSocket, MAX_CLIENTS);
    memset(&globalServerData, 0, sizeof(struct ServerData));
    printf("server is on listening port\n");
  // Accepting incoming client connections and creating threads to handle them
    while (1) {
        addrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&addrLen);

        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if (pthread_create(&threadId, NULL, handleClient, (void *)&clientSocket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        pthread_detach(threadId);
    }

      // Closing server socket when done
    close(serverSocket);
    return 0;
}

