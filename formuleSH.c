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

typedef struct {
    int number;
    int stands;
    int standsNumber;
    bool out;
    int totalTime;

    int circuit[TURN][SECTION];
} f1;

int carListNumber[CAR] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
f1 carList[CAR];
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
    sleep(time/20);
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
            printf("Voiture %3d || turn : %3d ||S1 : %1d | S2 : %2d | S3 : %2d  || Total Turn : %3d \n",carList[car].number,
                   turn+1,
                   carList[car].circuit[turn][0],
                   carList[car].circuit[turn][1],
                   carList[car].circuit[turn][2],
                   carList[car].circuit[turn][0]+carList[car].circuit[turn][1]+carList[car].circuit[turn][2]);
        }
        printf("---------------------------------------------------------------------------------------------------------------\n");
    }
}

/**
* generation de la liste des structur voitrure sur base de la liste des numero de voitures
* @return void
*/
void init_car_list(int *carListNumber){
    for(int i = 0; i < CAR; i++){
        carList[i].number = carListNumber[i];
        carList[i].stands = 0;
        carList[i].out = false;
        carList[i].totalTime = 0;
        memset(carList[i].circuit, 0, sizeof(carList[i].circuit));
    }
}

/**
 * initalistation de la voiture passé en param
 * @param carNumber
 */
f1 init_car(int carNumber){
    f1 tmp;
    tmp.number = carNumber;
    tmp.stands = 0;
    tmp.out = false;
    tmp.totalTime = 0;
    memset(tmp.circuit, 0, sizeof(tmp.circuit));
    return tmp;
}

/**
 * gestion des tours d essais des voitures
 * @param shmid id de la memoire partagée
 * @return -1 if error else 0
 */
int gen_circuit(int shmid){
    for(int car = 0; car < CAR; car++) {
        int pid = fork();
        if (pid < 0) {
            perror("error on creation of car");
            printf("\n");
            return -1;
        }
            /* Son */
        else if (pid == 0) {
            srand(time()+getpid()); // génération du nouveau random pour chaque fils
            f1 *output = (f1 *) shmat(shmid, 0, 0);
            f1 currentCar = init_car(carListNumber[car]);

            for(int i = 0; i < TURN; i++){
                for(int j = 0; j < SECTION; j++){
                    currentCar.circuit[i][j] = genSection();
                    printf("car %d , %d \n",currentCar.number,currentCar.circuit[i][j]);
                    output = &currentCar;
                }
            }
            exit(EXIT_SUCCESS);
        }
    }
    /* Parent */
    int status = 0;
    pid_t wpid;
    // récupération des données de la SM
    f1 *input = (f1*) shmat(shmid,0,0);
    do{ // temps que un processus est en cours
        memcpy(&carList, &input, sizeof(input));
        showRun();
    }while ((wpid = wait(&status)) > 0);


    return 0; // si tout c'est bien passé
}

int main(){
    // initalisation des voitures
    init_car_list(carListNumber);
    // allocation de la mem partagée
    int shmid = shmget(KEY, (20 * sizeof(f1)),0775 | IPC_CREAT); // 0775 || user = 7 | groupe = 7 | other = 5
    if (shmid == -1){
        perror("ERROR in creation of the Shared Memory");
        printf("\n");
        shmctl(shmid,IPC_RMID,NULL); // suppression de la memoire partagée
        return 1;
    }
    // gestion du circuit
    gen_circuit(shmid);
    printf("tout les tours sont terminé \n");
    printf("affichage des Résultats : \n");
    showRun();
    // fin de la course
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