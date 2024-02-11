#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>

#define PORT 8888
#define MAX_CONTACTS 10
#define MAX_MSG_LEN 1024
#define MAX_USERS 10

struct Contact {
    char contactID[50];
};
// Function to display menu options
void printMenu() {
    printf("\nMenu:\n");
    printf("1. Login\n");
    printf("2. Send a message\n");
    printf("3. Read messages\n");
    printf("4. Check User List \n");
    printf("5. Add/Remove Contact in contact List\n");
    printf("6. View Contact List \n");
    printf("7. Fetch User History/Info\n");
    printf("0. Terminate\n");
    printf("Enter your choice: ");
}

// Function to handle notifications from the server
void handleNotifications(int sock) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    struct timeval timeout;
    timeout.tv_sec = 3; // Set seconds
    timeout.tv_usec = 0; // Set microseconds
    printf("Notification checking ...\n");

    int ready = select(sock + 1, &fds, NULL, NULL, &timeout);
    if (ready > 0) {
        if (FD_ISSET(sock, &fds)) {
            char notify[100];
            int valread = read(sock, notify, sizeof(notify));
            if (valread > 0) {
                printf("Notification: %s\n", notify);
            }
        }
    } else if (ready == 0) {
        // Timeout occurred (no notification received within 3 seconds)
        printf("No notification received within the timeout.\n");
    } else {
        // Error in select operation
        perror("Select error");
    }
}

// Function to display the user list
bool displayUserlist(int sock) {
    char buffer[1024] = {0};
    int valread;

    // Request user list from the server
    int choice = 4;
    write(sock, &choice, sizeof(int));

    // Read and display user list
    valread = read(sock, buffer, sizeof(buffer));
    printf("User List:\n%s\n", buffer);
    if(strcmp(buffer,"Please login frist to send msg!")==0 || strcmp(buffer,"Please login frist !")==0){
        return false;
    }
    return true;
}


// Function to display the contact list
bool displayConatactList(struct Contact contacts[], int contactCount){
if (contactCount == 0) {
        printf("Contact list is empty.\n");
        return false;
    }
 // Displaying the contact list if it's not empty
    printf("Contact List:\n");
    int i;
    for (i = 0; i < contactCount; ++i) {
        printf("%d. %s\n", i + 1, contacts[i].contactID);
    }
    return true;
}

// Function to check if a contact exists
bool isContactExists(const char* contactID, struct Contact contacts[], int contactCount) {
    int i;
    for (i = 0; i < contactCount; ++i) {
        if (strcmp(contactID, contacts[i].contactID) == 0) {
            return true;
        }
    }
    return false;
}
int main(int argc, char *argv[]) {
    char username[50]={0};
    if (argc != 2) {
        printf("Usage: %s <username>\n", argv[0]);
   //     return -1;
    } else {
        strcpy(username, argv[1]);
        printf(" %s <--username\n", username);
    }
    
    struct Contact contacts[MAX_CONTACTS];
    int contactCount = 0;

    int sock = 0, valread;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    int choice = 0,terminateFlag=0;
    while (1) {
        
        printMenu();
        scanf("%d", &choice);
          
        int firstInputFlag =0;
         switch (choice) {
            case 1:
                write(sock, &choice, sizeof(int));
                if (username == NULL|| strlen(username) == 0) {
                    printf("Enter your username: ");
                    char tempUsername[50];
                    fgets(tempUsername, sizeof(tempUsername), stdin);
                    fgets(tempUsername, sizeof(tempUsername), stdin);
                    tempUsername[strcspn(tempUsername, "\n")] = 0;
                    strcpy(username, tempUsername);
                    firstInputFlag= 1;
                    
                }
                
                write(sock, username, strlen(username)+1); 
                        
                char name[50]={0}, phoneNumber[20]={0}, surname[50]={0};
                if(!firstInputFlag){
                    fgets(name, sizeof(name), stdin);
                }
                printf("Enter your name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                write(sock, name, sizeof(name));
                
                printf("Enter your phone number: ");
                fgets(phoneNumber, sizeof(phoneNumber), stdin);
                phoneNumber[strcspn(phoneNumber, "\n")] = 0;
                write(sock, phoneNumber, sizeof(phoneNumber));
                
                printf("Enter your surname: ");
                fgets(surname, sizeof(surname), stdin);
                surname[strcspn(surname, "\n")] = 0;
                write(sock, surname, sizeof(surname));

                valread = read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;
            
            case 2:
            
            if(!displayConatactList(contacts,contactCount)){
               printf("Please Add the user in Contact List\n");
               break;
            }
            char ID[50];
            fgets(ID, sizeof(ID), stdin);
            printf("Enter the ReceiverID or Username from the contact list: ");
            fgets(ID, sizeof(ID), stdin);
            ID[strcspn(ID, "\n")] = 0;
        
                if (isContactExists(ID,contacts,contactCount)) {
        int select = 2;
        char receiverID[50]={0},msg[1024] = {0};
        write(sock, &select, sizeof(int));

        write(sock, username, strlen(username) + 1);
        
        
        printf("Enter the Same ReceiverID or Username here: ");
        fgets(receiverID, sizeof(receiverID), stdin);
        receiverID[strcspn(receiverID, "\n")] = 0;
        write(sock, receiverID, sizeof(receiverID));

        printf("Enter the message: ");
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = 0;

        write(sock, msg, sizeof(msg));

        valread = read(sock, buffer, sizeof(buffer));
        printf("%s\n", buffer);
        
    }else{
       printf("Please Add the user in Contact List \n");
    }
    handleNotifications(sock);
    break;
                
                
            case 3:
                // Action 3: Reading messages
                int select =3;
                
                write(sock, &select, sizeof(int));
                write(sock, username, strlen(username)+1);
                printf("Req for Reading messages...\n");
                valread = read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;
            case 4:
                
                displayUserlist(sock);
                handleNotifications(sock);
                break;    
            case 5:
                // Add/Remove Contact
                if(!displayUserlist(sock)){
                   break;
                }
                char contactID[50];
    		fgets(contactID, sizeof(contactID), stdin);
                printf("Enter ID to add or remove:(ensure UserList will not be empty): ");
    		fgets(contactID, sizeof(contactID), stdin);
    		contactID[strcspn(contactID, "\n")] = 0;

                bool exists = false;	
    		

                // Check if the contact exists in the user list
    		char buffer[1024] = {0};
    		int choose = 4;
    		write(sock, &choose,sizeof(choose));
    
    		int valread = read(sock, buffer, sizeof(buffer));
    	

    if (valread > 0) {
        // Iterate through the user list in the buffer
        char* token = strtok(buffer, "\n");
        while (token != NULL) {
            if (strcmp(token, contactID) == 0) {
                exists = true;
                break;
            }
            token = strtok(NULL, "\n");
        }
    }

    if (!exists) {
        printf("Contact not exists in UserList.\n");
        break;
    }

    printf("Add (1) or Remove (2) contact? Enter choice: ");
    int addOrRemove;
    scanf("%d", &addOrRemove);

    if (addOrRemove == 1) {
        if (contactCount >= MAX_CONTACTS) {
            printf("Contact list is full.\n");
            break;
        }
        if(isContactExists(contactID,contacts,contactCount)){
       			printf("This is already exist in contact list\n");
       			break; 
    		}

        // Add contact to contact list
        strcpy(contacts[contactCount].contactID, contactID);
        contactCount++;
        printf("Contact added in contact list successfully.\n");
    } else if (addOrRemove == 2) {
    	
    	bool contactRemoved = false;
        int i;
        int j;
        for (i = 0; i < contactCount; ++i) {
            if (strcmp(contacts[i].contactID, contactID) == 0) {
                for (j = i; j < contactCount - 1; ++j) {
                    strcpy(contacts[j].contactID, contacts[j + 1].contactID);
                }
                contactCount--;
                contactRemoved = true;
                break;
            }
        }
        if (!contactRemoved) {
            printf("Contact doesn't exist in contact list.\n");
        } else {
            printf("Contact removed from contact list successfully.\n");
        }
    } else {
        printf("Invalid choice.\n");
    }
    break;
            case 6:
                displayConatactList(contacts,contactCount); 
                break;
            case 7:
            	int sel = 5;
            	write(sock, &sel, sizeof(int));
                write(sock, username, strlen(username)+1);

                char infoBuffer[MAX_MSG_LEN * MAX_USERS] = {0};
                int val = read(sock, infoBuffer, sizeof(infoBuffer));

                if (val > 0) {
                    printf("Your History/Info:\n%s\n", infoBuffer);
                } else {
                    printf("No data received from server!\n");
                }
                
                break;
        

            case 0:
            
               printf("Terminating...\n");
               terminateFlag=1;
                break;
            default:
               printf("Enter the valid number\n");
                break;
        }
        // Check if termination flag is set to break the loop
        if(terminateFlag){
            break;
        }
    }
     // Closing the socket and returning 0
    close(sock);
    return 0;
}

