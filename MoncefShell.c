#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/msg.h>



#define BUFFER_SIZE 1024
#define SEM_NAME "/my_semaphore"
#define MESSAGE_SIZE 1024
#define FIFO_PATH "/tmp/mon_fifo"


/*----------------------------Déclarations-------------------------------------*/

void creer(char **args, int arg_count); 
void lister(char **args, int arg_count);
void supprimer(char **args, int arg_count);
void renommer(char **args);
void *addition(void *arg);
void *soustraction(void *arg);
void *multiplication(void *arg);
void calculer(char **args, int arg_count);
void compter(char **args); 
void ouvrir(char **args, int arg_count); 
int envoyer_message(int file_id, long type, char *message_data); 
void recevoir_message(int cle, int type); 
void recevoirf(char **args, int arg_count); 
void envoyerf(char **args, int arg_count); 
void manuel(); 

/*-----------------------------La fonction MAIN()---------------------------------*/

struct message {
    long type;
    char data[MESSAGE_SIZE];
};


int main(){

    signal(SIGINT,SIG_IGN); 

    int cle = 1234;
    char input[256];
    char buffer[BUFFER_SIZE];
    char *args[100];
    int arg_count;
    mkfifo(FIFO_PATH, 0666);

    sem_t *my_semaphore = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (my_semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("MoncefSHELL >> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        
        arg_count = 0;
        args[arg_count] = strtok(buffer, " \n");
        while (args[arg_count] != NULL) {
            arg_count++;
            args[arg_count] = strtok(NULL, " \n");
        }

        if (arg_count == 0) {
            continue;
        }

        if (strcmp(args[0], "lister") == 0) {
            lister(args, arg_count);
        }
        else if (strcmp(args[0], "ouvrir") == 0) {
            ouvrir(args, arg_count);
        }   
        else if (strcmp(args[0], "manuel") == 0) {
            manuel(); 
        }      
        else if (strcmp(args[0], "creer") == 0) {
            creer(args, arg_count);
        }
        else if (strcmp(args[0], "supprimer") == 0) {
            supprimer(args, arg_count);
        } 
        else if (strcmp(args[0], "renommer") == 0) {
            renommer(args);
        } 
        else if (strcmp(args[0], "calculer") == 0) {
            calculer(args, arg_count); 
        } 
        else if(strcmp(args[0], "compter") == 0){
            compter(args); 
        }
        else if (strcmp(args[0], "envoyer") == 0) {
            char message[MESSAGE_SIZE];
            printf("Entrez le message à envoyer : ");
            scanf("%s", message);
            envoyer_message(msgget(cle, 0666 | IPC_CREAT), 1, message);
        } 
        else if (strcmp(args[0], "recevoir") == 0) {
            recevoir_message(cle, 1);
        }
        else if (strcmp(args[0], "envoyerf") == 0) {
        envoyerf(args, arg_count);
        } 
        else if (strcmp(args[0], "recevoirf") == 0) {
        recevoirf(args, arg_count);
        } 


        else if (strcmp(args[0], "quitter") == 0) {
            sem_close(my_semaphore);
            sem_unlink(SEM_NAME);
            exit(0);
        }       
        else {
            pid_t pid = fork();

            if (pid == 0) {
                execvp(args[0], args);
                printf("Erreur : la commande '%s' a échoué\n", args[0]);
                exit(1);
            } else if (pid < 0) {
                printf("Erreur : échec de la création d'un processus fils\n");
            } else {
                waitpid(pid, NULL, 0);
            }
        }
    }

    unlink(FIFO_PATH);
    return 0;
}


/*----------------------------------Fontions-----------------------------------*/

void creer(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'creer'\n");
        return;
    }

    pid_t pid = fork();

    if (pid == 0) {
        execlp("touch", "touch", args[1], NULL);
        printf("Erreur : la commande 'touch' a échoué\n");
        exit(1);
    } else if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
    } else {
        waitpid(pid, NULL, 0);
    }
}

void lister(char **args, int arg_count) {
    pid_t pid = fork();

    if (pid == 0) {
        execvp("ls", args);
        printf("Erreur : la commande 'ls' a échoué\n");
        exit(1);
    } else if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
    } else {
        waitpid(pid, NULL, 0);
    }
}

void supprimer(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'supprimer'\n");
        return;
    }

    pid_t pid = fork();

    if (pid == 0) {
        execlp("rm", "rm", args[1], NULL);
        printf("Erreur : la commande 'rm' a échoué\n");
        exit(1);
    } else if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
    } else {
        waitpid(pid, NULL, 0);
    }
}

void renommer(char **args) {
    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }

    if (arg_count != 3) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'renommer'\n");
        return;
    }

    pid_t pid = fork();

    if (pid == 0) {
        execlp("mv", "mv", args[1], args[2], NULL);
        printf("Erreur : la commande 'mv' a échoué\n");
        exit(1);
    } else if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
    } else {
        waitpid(pid, NULL, 0);
    }
}

void *addition(void *arg) {
    int *args = (int *) arg;
    int a = args[0];
    int b = args[1];
    int *result = (int *) malloc(sizeof(int));
    *result = a + b;
    pthread_exit(result);
}

void *soustraction(void *arg) {
    int *args = (int *) arg;
    int a = args[0];
    int b = args[1];
    int *result = (int *) malloc(sizeof(int));
    *result = a - b;
    pthread_exit(result);
}

void *multiplication(void *arg) {
    int *args = (int *) arg;
    int a = args[0];
    int b = args[1];
    int *result = (int *) malloc(sizeof(int));
    *result = a * b;
    pthread_exit(result);
}

void calculer(char **args, int arg_count) {
    if (arg_count != 4) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'calculer'\n");
        return;
    }

    char *operation = args[1];
    int a = atoi(args[2]);
    int b = atoi(args[3]);

    pthread_t thread;
    void *result;

    if (strcmp(operation, "+") == 0) {
        int arguments[2] = {a, b};
        pthread_create(&thread, NULL, addition, (void *) arguments);
    } else if (strcmp(operation, "-") == 0) {
        int arguments[2] = {a, b};
        pthread_create(&thread, NULL, soustraction, (void *) arguments);
    } else if (strcmp(operation, "*") == 0) {
        int arguments[2] = {a, b};
        pthread_create(&thread, NULL, multiplication, (void *) arguments);
    } else {
        printf("Erreur : opération non valide pour la commande 'calculer'\n");
        return;
    }

    pthread_join(thread, &result);

    printf("%d %s %d = %d\n", a, operation, b, *((int *) result));
    free(result);
}

void compter(char **args) {
    int fd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    if (pipe(fd) == -1) {
        printf("Erreur : échec de la création d'un tube anonyme\n");
        return;
    }

    pid = fork();
    if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
        return;
    }

    if (pid == 0) {
        // Code du fils
        close(fd[1]); // Fermer l'extrémité d'écriture du tube

        // Rediriger la sortie standard vers l'entrée du tube
        dup2(fd[0], STDIN_FILENO);

        // Exécuter la commande "wc -l" pour compter les lignes
        execlp("wc", "wc", "-l", NULL);

        // En cas d'erreur
        printf("Erreur : échec de l'exécution de la commande 'wc'\n");
        exit(1);
    } else {
        // Code du parent
        close(fd[0]); // Fermer l'extrémité de lecture du tube

        // Rediriger la sortie de la commande "lister" vers l'entrée du tube
        FILE *fp = popen("ls", "r");
        while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
            write(fd[1], buffer, strlen(buffer));
        }
        pclose(fp);

        close(fd[1]); // Fermer l'extrémité d'écriture du tube

        // Attendre la fin du processus fils
        wait(NULL);
    }
}

void ouvrir(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'ouvrir'\n");
        return;
    }

    sem_t *my_semaphore = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (my_semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_wait(my_semaphore);

    pid_t pid = fork();

    if (pid == 0) {
        execlp("xdg-open", "xdg-open", args[1], NULL);
        printf("Erreur : la commande 'xdg-open' a échoué\n");
        exit(1);
    } else if (pid < 0) {
        printf("Erreur : échec de la création d'un processus fils\n");
    } else {
        waitpid(pid, NULL, 0);
        sem_post(my_semaphore);
    }

    sem_close(my_semaphore);
}

int envoyer_message(int file_id, long type, char *message_data) {
    struct message msg;
    msg.type = type;
    strncpy(msg.data, message_data, MESSAGE_SIZE);

    int result = msgsnd(file_id, &msg, strlen(msg.data)+1, IPC_NOWAIT);
    if (result == -1) {
        perror("Erreur : impossible d'envoyer le message");
        return -1;
    }

    printf("Message envoyé avec succès : %s\n", message_data);
    return 0;
}

void recevoir_message(int cle, int type) {
    // Ouvrir la file de messages
    int file = msgget(cle, 0666 | IPC_CREAT);
    if (file == -1) {
        perror("Erreur lors de l'ouverture de la file de messages");
        exit(EXIT_FAILURE);
    }
    
    // Recevoir un message du type spécifié
    struct message msg;
    int res = msgrcv(file, &msg, MESSAGE_SIZE, type, 0);
    if (res == -1) {
        perror("Erreur lors de la réception du message");
        exit(EXIT_FAILURE);
    }

    // Afficher le message reçu
    printf("Message reçu : %s\n", msg.data);
    
    // Fermer la file de messages
    res = msgctl(file, IPC_RMID, NULL);
    if (res == -1) {
        perror("Erreur lors de la fermeture de la file de messages");
        exit(EXIT_FAILURE);
    }
}

void envoyerf(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'envoyerf'\n");
        return;
    }

    // Ouvrir le fichier FIFO en écriture
    FILE *fifo = fopen(args[1], "w");
    if (fifo == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier FIFO '%s'\n", args[1]);
        return;
    }

    char message[MESSAGE_SIZE];
    printf("Entrez le message à envoyer : ");
    scanf("%s", message);

    // Écrire le message dans le fichier FIFO
    fprintf(fifo, "%s", message);
    fclose(fifo);

    printf("Message envoyé avec succès : %s\n", message);
}

void recevoirf(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Erreur : nombre d'arguments incorrect pour la commande 'recevoirf'\n");
        return;
    }

    // Ouvrir le fichier FIFO en lecture
    FILE *fifo = fopen(args[1], "r");
    if (fifo == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier FIFO '%s'\n", args[1]);
        return;
    }

    char message[MESSAGE_SIZE];

    // Lire le message depuis le fichier FIFO
    fscanf(fifo, "%s", message);
    fclose(fifo);

    printf("Message reçu : %s\n", message);
}

void manuel() {
    printf("Liste des commandes :\n");
    printf("lister\t\tAffiche la liste des fichiers dans le répertoire courant.\n");
    printf("ouvrir\t\tOuvre un fichier spécifié.\n");
    printf("creer\t\tCrée un nouveau fichier.\n");
    printf("supprimer\tSupprime un fichier spécifié.\n");
    printf("renommer\tRenomme un fichier spécifié.\n");
    printf("calculer\tEffectue un calcul spécifié.\n");
    printf("compter\t\tCompte le nombre de mots dans un fichier spécifié.\n");
    printf("envoyer\t\tEnvoie un message spécifié via une file de messages.\n");
    printf("recevoir\tReçoit un message via une file de messages.\n");
    printf("envoyerf\tEnvoie un fichier spécifié via un tube FIFO.\n");
    printf("recevoirf\tReçoit un fichier spécifié via un tube FIFO.\n");
    printf("quitter\t\tQuitte le programme.\n");
}
