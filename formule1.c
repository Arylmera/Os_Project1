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

/***********************************************************************************************************************
 *                             Variables - Structures - Includes
 **********************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TURN 3
#define CAR 20
/**
* structure des voitures
*/
struct car {
    int number;
    int stands;
    int standsNumber;
    bool out;

    int essais[TURN][4];
    int qualif[TURN][4];
};

/**
 * Variables Globales
 */
int carListNumber[] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
int mainBoard[CAR]; // tableau global des voitures et des temps
struct car carList[CAR];

/***********************************************************************************************************************
 *                              Début des fonctions du programme
 **********************************************************************************************************************/

/**
* generation de la liste des structur voitrure sur base de la liste des numero de voitures
* @return void
*/
void initCar(int *carListNumber){
    for(int i = 0; i < CAR; i++){
        carList[i].number = carListNumber[i];
        carList[i].stands = 0;
        carList[i].standsNumber = 0;
        carList[i].out = false;
    }
}

/**
 * renvois un nombre compris entre 40 et 45 secondes pour le
 * clacul du temps des tours de circuits
 * @return integer between 40 and 45
 */
int getRandomTime(){
    return 45 - (rand() % 9); // gestion du random et du temps perdu
}

/**
 * génération du temps d'arret au stand
 * @return integer between 2 and 5
 */
int genRandomStand(){
    return  5 - (rand() % 3);
}

/**
 * ajout du temps des arrets au stands pour les voitures
 * @param current
 */
void standStop(struct car *current){
    for(int i = 0; i < CAR; i++){
        for (int j = 0; j < TURN; j++) {
            int standNumber = 3 - (rand() % 3);
            current[i].standsNumber = standNumber;
            int standTotalTime = 0;
            for (int k = 0; k < standNumber; k++) {

                standTotalTime += genRandomStand();
            }
            current[i].essais[j][2] += standTotalTime;
            current[i].essais[j][3] += standTotalTime;
            current[i].stands += standTotalTime;
        }
    }
}

/**
 * generation du tour pour la voiture passé en param
 * @param current
 */
void genLapTime(int *currentLap){
    int lap[] = {getRandomTime(), getRandomTime(), getRandomTime(), 0};
    lap[3] = lap[0] + lap[1] + lap[2];
    for (int i = 0; i < 4; i++) {
        currentLap[i] = lap[i];
    }
}

/**
 * génération des essais pour toutes les voitures
 */
void genEssais(){
    for(int i = 0; i < CAR; i++){
        for(int j = 0; j < 3; j++) {
            genLapTime(carList[i].essais[j]);
        }
    }
}

/**
 * affichage en console du tour qui vient d'etre effectué
 */
void showEssais(){
    printf("***********************************************************************\n");
    printf("*                                                                     *\n");
    printf("*                         résultat des essais                         *\n");
    printf("*                                                                     *\n");
    printf("***********************************************************************\n");
    for(int i = 0; i < CAR; i++){
        printf("\n Numéro de voiture : %d \n",carList[i].number);
        for(int j = 0; j < TURN; j++){
            printf("TOUR %d || S1 : %d | S2 : %d | S3 : %d |--| Total : %d\n",
                    (j+1),
                    carList[i].essais[j][0],carList[i].essais[j][1],carList[i].essais[j][2],
                    carList[i].essais[j][3]);
        }
        printf("nombre d'arret au stand %d\n",carList[i].standsNumber);
        printf("temps passé au stand %d",carList[i].stands);
        printf("\n");
    }
}

/**
 * fonction principal du programme
 * @return 0 => quit
 */
int main(){
    srand(time(NULL)); // init unique du random
    initCar(carListNumber); // initalisation des l'array des struct car
    // gestion des essais
    genEssais(); // generation des 3 tours d essais
    standStop(carList); // arret au stand
    showEssais(); // affichage du tour
    // gestion des qualif

    // gestion de la course

    return 0; // fin du programme - logout de la console
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