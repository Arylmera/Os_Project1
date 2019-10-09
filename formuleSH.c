/***********************************************************************************************************************
                              ___..............._
                     __.. ' _'.""""""\\""""""""- .`-._                                       ____         .
        ______.-'         (_) |      \\           ` \\`-. _                                 |            /|
        /_       --------------'-------\\---....______\\__`.`  -..___                       |____       / |
        | T      _.----._           Xxx|x...           |          _.._`--. _                |          /  |
        | |    .' ..--.. `.         XXX|XXXXXXXXXxx==  |       .'.---..`.     -._           |             |
        \_j   /  /  __  \  \        XXX|XXXXXXXXXXX==  |      / /  __  \ \        `-.       |             |
        _|  |  |  /  \  |  |       XXX|""'            |     / |  /  \  | |          |
        |__\_j  |  \__/  |  L__________|_______________|_____j |  \__/  | L__________J
            `'\ \      / ./__________________________________\ \      / /___________\
                `.`----'.'                                     `.`----'.'
                  `""""'                                         `""""'
 **********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool
#include <unistd.h> //fork
#include <string.h> //strcat
#include <sys/wait.h> //wait
#include <sys/types.h>
#include <sys/shm.h>

#define TURN 3
#define CAR 20
#define KEY 666
#define SECTION 3

struct carTemplate {
    int number;
    int stands;
    int standsNumber;
    bool out;
    int totalTime;

    int circuit[TURN][SECTION];
};

int carListNumber[CAR] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
struct carTemplate carList[CAR];
/***********************************************************************************************************************
 *                               fonctions
 **********************************************************************************************************************/

/**
 * renvois un nombre compris entre 40 et 45 secondes pour le
 * clacul du temps des tours de circuits
 * @return integer between 40 and 45
 */
int genSection(){
    int time = 45 - (rand() % 9);
    //sleep(time/20);
    return time; // gestion du random et du temps perdu
}

/**
 * génération du temps d'arret au stand
 * @return integer between 2 and 5
 */
int genRandomStand(){
    return  5 - (rand() % 3);
}

/**
 * génération nombre entre 1 et 100
 * @return int between 1 and 100
 */
int genRandom(){
    return 100 - (rand() % 100);
}

/**
* generation de la liste des structur voitrure sur base de la liste des numero de voitures
* @return void
*/
void initCar(int *carListNumber){
    for(int i = 0; i < CAR; i++){
        carList[i].number = carListNumber[i];
        carList[i].stands = 0;
        carList[i].out = false;
        carList[i].totalTime = 0;
        memset(carList[i].circuit, 0, sizeof(carList[i].circuit));
    }
}

/**
 * gestion des tours d essais des voitures
 * @param shmid id de la memoire partagée
 * @return -1 if error else 0
 */
int genSeg(int shmid,int turn,int seg){
    for(int car = 0; car < CAR; car++) {
        int pid = fork();
        if (pid < 0) {
            printf("error on creation of car %d \n", car);
            return -1;
        }
            /* Son */
        else if (pid == 0) {
            srand(time()+getpid()); // génération du nouveau random pour chaque fils
            int *output = (int *) shmat(shmid, 0, 0);
            int temps = genSection(); // generation du temps aléatoire
            output[car] = temps;
            exit(EXIT_SUCCESS);
        }
    }
    /* Parent */
    // attente de la fin des fils
    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0);
    // récupération des données de la SM
    int *input = (int*) shmat(shmid,0,0);
    for (int i = 0; i < CAR; i++){
        carList[i].circuit[turn][seg] = input[i]; // ajout du temps du segment
        carList[i].totalTime += input[i]; // ajout au temps total
    }

    printf("retour du segment \n");
    return 0; // si tout c'est bien passé
}

/**
 * clear de la console
 */
void clrscr(){
    system("clear");
}

/**
 * affichage du tableau de résultat de tout les tours pour toutes les voitures et sections
 */
void showRun(){
    clrscr();
    for (int turn = 0; turn < TURN; turn++){
        for(int car = 0; car < CAR; car++){
            printf("Voiture %3d || turn : %2d ||\tS1 : %d \t|\tS2 : %d \t|\tS3 : %d \t||\tTotal Turn : %d \n",carList[car].number,
                   turn+1,
                   carList[car].circuit[turn][0],
                   carList[car].circuit[turn][1],
                   carList[car].circuit[turn][2],
                   carList[car].circuit[turn][0]+carList[car].circuit[turn][1]+carList[car].circuit[turn][2]);
        }
        printf("---------------------------------------------------------------------------------------------------------------\n");
    }
}

int main(){
    // initalisation des voitures
    initCar(carListNumber);
    // allocation de la mem partagée
    int shmid = shmget(KEY, (sizeof(int))*20,IPC_CREAT|0775); // 0775 || user = 7 | groupe = 7 | other = 5
    if (shmid == -1){
        printf("ERROR in creation of the Shared Memory \n");
        return 1;
    }
    // generation des segments
    /*
    for(int turn = 0; turn < TURN; ++turn){
        for(int seg = 0; seg < SECTION; ++seg){
            printf("\nSection %d du tour %d \n\n",seg,turn);
            genSeg(shmid,turn,seg); // generation du segment n du tour m
        }
        printf("Fin du tour %d \n",turn);
    }
     */
    genSeg(shmid,0,0);
    genSeg(shmid,0,1);
    genSeg(shmid,0,2);
    genSeg(shmid,1,0);
    genSeg(shmid,1,1);
    genSeg(shmid,1,2);
    genSeg(shmid,2,0);
    genSeg(shmid,2,1);
    genSeg(shmid,2,2);

    printf("tout les tours sont terminé \n");

    printf("affichage des Résultats : \n");
    showRun();

    printf("fin des tours \n");
    shmctl(shmid,IPC_RMID,NULL); // suppression de la memoire partagée
    return 0; // fin du programme
}

/***********************************************************************************************************************
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▄▀░░▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▄▀▐░░░▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▄▀▀▒▐▒░░░▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░▄▀▀▄░░░▄▄▀▀▒▒▒▒▌▒▒░░▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░▐▒░░░▀▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░▌▒░░░░▒▀▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░▐▒░░░░░▒▒▒▒▒▒▒▒▒▌▒▐▒▒▒▒▒▀▄░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░▌▀▄░░▒▒▒▒▒▒▒▒▐▒▒▒▌▒▌▒▄▄▒▒▐░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░▌▌▒▒▀▒▒▒▒▒▒▒▒▒▒▐▒▒▒▒▒█▄█▌▒▒▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░▄▀▒▐▒▒▒▒▒▒▒▒▒▒▒▄▀█▌▒▒▒▒▒▀▀▒▒▐░░░▄░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░▀▒▒▒▒▌▒▒▒▒▒▒▒▄▒▐███▌▄▒▒▒▒▒▒▒▄▀▀▀▀░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░▒▒▒▒▒▐▒▒▒▒▒▄▀▒▒▒▀▀▀▒▒▒▒▄█▀░░▒▌▀▀▄▄░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░▒▒▒▒▒▒█▒▄▄▀▒▒▒▒▒▒▒▒▒▒▒░░▐▒▀▄▀▄░░░░▀░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▄▒▒▒▒▄▀▒▒▒▌░░▀▄░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░▒▒▒▒▒▒▒▒▀▄▒▒▒▒▒▒▒▒▀▀▀▀▒▒▒▄▀░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
***********************************************************************************************************************/