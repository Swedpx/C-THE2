#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "the2.h"

/*
 *  returns the order of the letter in the alphabet
 *  if given char is not an ascii letter, returns 0
 */
int letter_order(char c) {
    if (c < 91 && c > 64) {
        return c - 64;
    } else if (c < 123 && c > 96) {
        return c - 96;
    }
    return 0;
}
/*
 * Prints the connections of given user.
 */
void print_connections(User *user) {
    Connection *connection;
    int i;
    for (i = 0; i < BUCKET_SIZE; i++) {
        printf("Bucket %d: ", i);
        if (user->connections[i] != NULL) {
            connection = user->connections[i];
            do {
                printf("[user1: %s, user2: %s, connection_time: %ld] -> ", connection->user1->name,
                       connection->user2->name, connection->connection_time);

            } while ((connection = connection->next_connection) != NULL);
        }
        printf("NULL\n");


    }
}

/*
 * Returns the hash code of the user.
 */
unsigned long hash_code(User *user) {
    unsigned long hash = 0;
    int i = 0;

    while ((*user).name[i] != 0) {
        hash += letter_order((*user).name[i]) * pow(HASH_KEY, i);
        i++;
    }

    return hash;
}

/*
 * Returns the User with given id from environment.
 */
User *get_user(Environment environment, int id) {
    while(*environment.users){
        if( (*environment.users)->id == id )
            return *environment.users;
        (environment.users)++;
    }
    return NULL;
}
void insert(Connection* conn_to_be_added, Connection** connections, long connection_time, int index){
    Connection* curr, *prev;
    curr = connections[index]; prev = NULL;
    while(curr && connection_time > curr->connection_time){
        prev = curr;
        curr = curr->next_connection;
    }
    conn_to_be_added->next_connection = curr;
    if (curr)
        curr->prev_connection = conn_to_be_added;
    conn_to_be_added->prev_connection = prev;
    if (prev)
        prev->next_connection = conn_to_be_added;
    else
        connections[index] = conn_to_be_added;
    return;
}

/*Connects two user and registers the related connection objects to both users' connection hash tables.*/

void connect_users(Environment environment, int id1, int id2, long connection_time) {
    Connection* check;
    Connection* conn1 = (Connection *) malloc(sizeof(Connection));
    Connection* conn2 = (Connection *) malloc(sizeof(Connection));
    int i;
    User* user1 = get_user(environment, id1), *user2 = get_user(environment, id2);
    
    /* EGER CONNECTION MEVCUTSA ? */
    
    if (conn1 && conn2 && user1 && user2){
        conn1->user1 = user1; conn1->user2 = user2;
        conn2->user1 = user2; conn2->user2 = user1;
        conn1->connection_time = connection_time; conn2->connection_time = connection_time;
        conn1->next_connection = NULL; conn1->prev_connection = NULL;
        conn2->next_connection = NULL; conn2->prev_connection = NULL;

        for (i=0; i<10; i++){
            check = user1->connections[i];
            while(check){
                if (check->user1->id == id1 && check->user2->id == id2){
                    free(conn1);
                    free(conn2);
                    return;
                }
                check = check->next_connection;
            }
            check = user2->connections[i];
            while(check){
                if (check->user1->id == id1 && check->user2->id == id2){
                    free(conn1);
                    free(conn2);
                    return;
                }
                check = check->next_connection;
            }
        }
        /* insertion*/
        insert(conn1, user1->connections, connection_time, hash_code(user2)%10);
        insert(conn2, user2->connections, connection_time, hash_code(user1)%10);
    }
}

/*
 * Rstrip the given string.
 */

char *rstrip(char *string) {
  char *end;

  while(isspace((unsigned char)*string)){
    string++;
  } 

  if(*string == 0) {  
    return string;
  }

  end = string + strlen(string) - 1;
  while(end > string && isspace((unsigned char)*end)){ 
    end--;
  }

  end[1] = '\0';

  return string;
}


/*
 * Creates a new environment with the information in the given file that contains users
 * with connections and returns it.
 */
Environment *init_environment(char *user_file_name) {
    Environment *environment;
    User *user;
    FILE *fp;
    char *line = NULL;
    char *iter;
    char *sep = " ";
    size_t len = 0;
    size_t read;
    int id, id1, id2;
    long timestamp;
    char *name;
    User* user1, *user2;

    int u_count = 0;

    environment = malloc(sizeof(Environment));
    environment->users = malloc(sizeof(User*));
    environment->users[0] = NULL;

    fp = fopen(user_file_name, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        line = rstrip(line);
        if (strcmp(line, "#Users#") == 0) {
            continue;
        } else if (strcmp(line, "#Connections#") == 0) {
            break;
        }
        user = malloc(sizeof(User));
        iter = strtok(line, sep);
        id = atoi(iter);
        iter = strtok(NULL, sep);
        name = malloc(sizeof(char)*strlen(iter));
        strcpy(name, iter);
        user->id=id;
        user->name=name;
        environment->users = realloc(environment->users, sizeof(User*)*(u_count+2));
        environment->users[u_count] = user;
        environment->users[u_count+1] = NULL;
        u_count++;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        line = rstrip(line);
        iter = strtok(line, sep);
        id1 = atoi(iter);
        iter = strtok(NULL, sep);
        id2 = atoi(iter);
        iter = strtok(NULL, sep);
        timestamp = atol(iter);
        connect_users(*environment, id1, id2, timestamp);

    }

    return environment;

}

/*
 * Returns the connection between users with id1 and id2. The connection
 * is the one stored in user1's hash table. If there is no such a connection returns
 * NULL.
 */
Connection *get_connection(Environment environment, int id1, int id2) {
    int i;
    Connection* check = NULL;
    for(i=0; i<10; i++){
        check = (get_user(environment, id1))->connections[i];
        
        while(check){
            if (check->user1->id == id1 && check->user2->id == id2)
                return check;
            check = check->next_connection;
        }
    }
}

/*
 * Remove connection from the given user. Additionally, finds the other user and removes related connection
 * from her/his table also.
 */
void remove_connection(Environment environment, int id1, int id2) {
    Connection **connections, *prev=NULL, *curr;
    User *user1 = get_user(environment, id1), *user2 = get_user(environment, id2);
    
    connections = user1->connections;
    curr = connections[hash_code(user2)%10];
    
    while (curr){
        if(curr->user1->id == id1 && curr->user2->id == id2){
            
            if (prev)
                prev->next_connection = curr->next_connection;
            
            else
                connections[hash_code(user2)%10] = curr->next_connection;
            if(curr->next_connection)
                (curr->next_connection)->prev_connection = prev;
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next_connection;
    }
    connections = user2->connections;
    curr = connections[hash_code(user1)%10];
    prev = NULL;
    while (curr){
        if(curr->user1->id == id2 && curr->user2->id == id1){
            
            if (prev)
                prev->next_connection = curr->next_connection;
            
            else
                connections[hash_code(user1)%10] = curr->next_connection;
            if(curr->next_connection)
                (curr->next_connection)->prev_connection = prev;
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next_connection;
    }
}

/*
 * Returns dynamic User array which contains common connections. The last element
 * of the array is NULL, which is mandatory for detecting the end.
 */
User **get_common_connections(Environment environment, User *user1, User *user2) {
    User** Array = (User**) malloc(sizeof(User*));
    Connection* check1, *check2;
    int i, j, size = 2, index=0;
    for (i=0; i<10; i++){
        check1 = user1->connections[i];
        if(!check1) continue;
        while(check1){
            
            for(j=0; j<10; j++){
                check2 = user2->connections[j];
                if(!check2) continue;
                while(check2){
                    if(check1->user2 == check2->user2){
                        Array = (User**) realloc(Array, sizeof(User*)*(size++));
                        Array[index++] = check1->user2;
                    }
                    check2 = check2->next_connection;
                }
            }
            
            check1 = check1->next_connection;
        }
    }
    Array[index] = NULL;
    return Array;
}


