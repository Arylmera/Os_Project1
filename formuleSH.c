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

/***********************************************************************************************************************
 *                               définitions
 **********************************************************************************************************************/


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define TURN 3
#define SECTION 3
#define CAR 20
#define KEY 666
#define STANDPOURCENT 25

/***********************************************************************************************************************
 *                               déclarations
 **********************************************************************************************************************/

typedef struct {
    int number;
    int stands;
    bool in_stands;
    bool out;
    int totalTime;

    int circuit[TURN][SECTION];
} f1;
int carListNumber[CAR] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
f1 carList[CAR];


void circuit_son(int shmid,int carPosition);
void circuit_father(int shmid);
/***********************************************************************************************************************
 *                               fonctions initalisation
 **********************************************************************************************************************/

/**
 * initalistation de la voiture passé en param
 * @param carNumber
 */
f1 init_car(int carNumber){
    f1 tmp;
    tmp.number = carNumber;
    tmp.stands = 0;
    tmp.in_stands = false;
    tmp.out = false;
    tmp.totalTime = 0;
    memset(tmp.circuit, 0, sizeof(tmp.circuit));
    return tmp;
}

/**
* generation de la liste des structur voitrure sur base de la liste des numero de voitures
* @return void
*/
void init_car_list(int *carListNumber){
    for(int i = 0; i < CAR; i++){
        carList[i].number = carListNumber[i];
        carList[i].stands = 0;
        carList[i].in_stands = false;
        carList[i].out = false;
        carList[i].totalTime = 0;
        memset(carList[i].circuit, 0, sizeof(carList[i].circuit));
    }
}

/**
 * initalisation de la shared memory
 */
void init_mem(shmid){
    f1 *mem = (f1 *) shmat(shmid, 0, 0);
    for(int i = 0; i < CAR; i++){
        mem[i] = init_car(carListNumber[i]);
    }
}

/***********************************************************************************************************************
 *                               fonctions supp
 **********************************************************************************************************************/

/**
 * renvois un nombre compris entre 40 et 45 secondes pour le
 * clacul du temps des tours de circuits
 * @return integer between 40 and 45
 */
int genSection(){
    int time = 45 - (rand() % 9);
    sleep(time/10);
    return time; // gestion du random et du temps perdu
}

/**
 * génération du temps d'arret au stand
 * @return integer between 2 and 5
 */
int genRandomStand(){
    return  20 - (rand() % 5);
}

/**
 * génération nombre entre 1 et 100
 * @return int between 1 and 100
 */
int genRandom(){
    return 100 - (rand() % 100);
}

/**
 * gestion des tours d essais des voitures
 * @param shmid id de la memoire partagée
 * @return -1 if error else 0
 */
int gen_circuit(int shmid){
    for (int car = 0; car < CAR; car++) {
        int pid = fork();
        if (pid < 0) {
            perror("error on creation of car");
            printf("\n");
            return -1;
        }
            /* Son */
        else if (pid == 0) {
            circuit_son(shmid,car);
        }
    }
    /* Parent */
    circuit_father(shmid);
    return 0;
}

/**
 * swap de 2 paramètre pour le bubbleSort
 * @param x
 * @param y
 */
void swap(f1 *x, f1 *y){
    f1 temp = *x;
    *x = *y;
    *y = temp;
}

/**
 * buubleSort
 * @param arr
 */
void bubbleSortCarList(){
    int size = (sizeof(carList)/sizeof(carList[0]));
    for (int i = 0; i < size-1; i++){
        for (int j = 0; j < size - i - 1; j++){
            if (carList[j].totalTime > carList[j+1].totalTime){
                swap(&carList[j], &carList[j+1]);
            }
        }
    }
}

/***********************************************************************************************************************
 *                               fonctions affichage et de save
 **********************************************************************************************************************/

/**
 * clear de la console
 */
void clrscr(){
    system("clear");
    printf ("\33c\e[3J");
}

/**
 * renvois du char P si la voiture est au stand N si non
 */
char atStand(bool stand){
    if (stand){
        return 'P';
    }
    return 'N';
}

/**
 * affichage du tableau de résultat de tout les tours pour toutes les voitures et sections
 */
void showRunTotal(){
    clrscr();
    bubbleSortCarList();
    for (int turn = 0; turn < TURN; turn++){
        for(int car = 0; car < CAR; car++){
            printf(BLU"Voiture %3d "RED"||"RESET" turn : %1d "RED"||"RESET" "GRN"S1"RESET" : %1d | "GRN"S2"RESET" : %2d | "GRN"S3"RESET" : %2d  "RED"||"RESET" "CYN"At stands"RESET" : %c | "CYN"Stands stop"RESET" : %2d "RED"||"BLU"Total Turn"RESET" : %3d \n",carList[car].number,
                   turn+1,
                   carList[car].circuit[turn][0],
                   carList[car].circuit[turn][1],
                   carList[car].circuit[turn][2],
                   atStand(carList[car].in_stands),
                   carList[car].stands,
                   carList[car].circuit[turn][0]+carList[car].circuit[turn][1]+carList[car].circuit[turn][2]);
        }
        printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    }
}

/**
 * affichage des temps d'arrivé totaux
 */
void showRun(){
    clrscr();
    bubbleSortCarList();
    for(int car = 0; car < CAR; car++){
        printf(BLU"Voiture "RESET" %2d "RED"||"CYN" Nombre arrets aux stands : "RESET" %1d "RED"||"BLU" Temps Total : "RESET" %4d \n",
                carList[car].number,
                carList[car].stands,
                carList[car].totalTime);
    }
}

/***********************************************************************************************************************
 *                               fonctions voitures
 **********************************************************************************************************************/

/**
 * fonction du code du fils (voiture)
 */
void circuit_son(int shmid,int carPosition){
    int carNumber = carListNumber[carPosition];
    f1 *output = (f1 *) shmat(shmid, 0, 0);
    printf("Départ de la voiture %d\n",carNumber);
    f1 *currentCar;
    srand(time()+getpid()); // génération du nouveau random pour chaque fils
    for(int i = 0; i < CAR; i++){
        if(output[i].number == carNumber){
            currentCar = &output[i];
            break;
        }
    }
    for(int i = 0; i < TURN; i++){ // pour chaque tour
        for(int j = 0; j < SECTION; j++){ // pour chaque section du tour
            int section_time = genSection();
            currentCar->circuit[i][j] = section_time;
            currentCar->totalTime += section_time;
        }
        if (genRandom() < STANDPOURCENT || (i == (TURN-1) && currentCar->stands == 0)){ // 50% de s'arreter ou si jamais arrêter pendant la course
            currentCar->in_stands = true;
            int time_in_stands = genRandomStand();
            currentCar->circuit[i][SECTION-1] += time_in_stands;
            currentCar->totalTime+= time_in_stands;
            sleep(time_in_stands/10);
            printf("arret de la voiture %d au stand , temps total de la section 3 : %d \n",currentCar->number,currentCar->circuit[i][SECTION-1]);
            currentCar->stands++;
            currentCar->in_stands = false;
        }
    }
    exit(EXIT_SUCCESS);
}

/**
 * fonction du père
 * @return
 */
void circuit_father(int shmid){
    int status = 0;
    pid_t wpid;
    // récupération des données de la SM
    f1 *input = (f1*) shmat(shmid,0,0);
    do{ // temps que un processus est en cours
        memcpy(carList, input, sizeof(carList));
        showRunTotal();
    }while ((wpid = wait(&status)) > 0);
}

/***********************************************************************************************************************
 *                               fonctions Main
 **********************************************************************************************************************/

/**
 *
 * @return
 */
int main(){
    // initalisation des voitures
    init_car_list(carListNumber);

    // allocation de la mem partagée
    int shmid = shmget(KEY, (20 * sizeof(f1)),0666 | IPC_CREAT); // 0775 || user = 7 | groupe = 7 | other = 5
    if (shmid == -1){
        perror("ERROR in creation of the Shared Memory");
        printf("\n");
        shmctl(shmid,IPC_RMID,NULL); // suppression de la memoire partagée
        return 1;
    }
    init_mem(shmid);

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