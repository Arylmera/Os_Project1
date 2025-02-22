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
#include <unistd.h> //access
#include <string.h> //strcat
#include <errno.h> //error number
#include <sys/stat.h> //mkdir

#define TURN 3
#define CAR 20
//FILE *file;

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
 *                               fonctions
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
 * génération nombre entre 1 et 100
 * @return int between 1 and 100
 */
int genRandom(){
    return 100 - (rand() % 100);
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
            if (current[i].essais[j][0] != 0) { // si la voiture n'as pas abandonnée
                current[i].essais[j][2] += standTotalTime;
                current[i].essais[j][3] += standTotalTime;
                current[i].stands += standTotalTime;
            }
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
        bool leave = false;
        for(int j = 0; j < 3 && !leave; j++) {
            genLapTime(carList[i].essais[j]);
            if (genRandom() < 2 && j < 2){ // 2% de chances d'abandonner et un tour commancé est terminé
                leave = true;
                carList[i].out = true;
            }
        }
    }
}

/**
 * écriture du fichier d'essais
 */
void printEssais(){
    FILE * file = fopen("./essais.txt","w+"); // ouverture du fichier essais.txt
    if(!file){
        printf("erreur lors de l'écriture du ficher essais");
    }
    else {
        fprintf(file, "-----------------------------------------------------------------------\n");
        fprintf(file, "|                                                                     |\n");
        fprintf(file, "|                         résultat des essais                         |\n");
        fprintf(file, "|                                                                     |\n");
        fprintf(file, "-----------------------------------------------------------------------\n");
        for (int i = 0; i < CAR; i++) {
            fprintf(file, "\n Numéro de voiture : %d \n", carList[i].number);
            for (int j = 0; j < TURN; j++) {
                fprintf(file, "TOUR %d || S1 : %d | S2 : %d | S3 : %d |--| Total : %d\n",
                        (j + 1),
                        carList[i].essais[j][0], carList[i].essais[j][1], carList[i].essais[j][2],
                        carList[i].essais[j][3]);
            }
            fprintf(file, "nombre d'arrêts au stand %d\n", carList[i].standsNumber);
            fprintf(file, "temps passé au stand %d", carList[i].stands);
            if (carList[i].out){
                fprintf(file,"\n----- la voiture a du abandonner -----");
            }
            fprintf(file, "\n");
        }
        fclose(file); // fermeture du fichier des essais
    }
    printf("\nFichier essais.txt crée à l'emplacement ");
    char cpath[1024];
    getcwd(cpath, sizeof(cpath));
    printf("%s\n",cpath);
}

/**
 * affichage en console des essais
 */
void showEssais(){
    printf("-----------------------------------------------------------------------\n");
    printf("|                                                                     |\n");
    printf("|                         résultat des essais                         |\n");
    printf("|                                                                     |\n");
    printf("-----------------------------------------------------------------------\n");
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
        if (carList[i].out){
            printf("\n \033[1;31m la voiture a du abandonner \033[0m \n");
        }
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

    //
    // gestion des essais
    //
    genEssais(); // generation des 3 tours d essais
    standStop(carList); // arret au stand
    showEssais();
    // écriture des essais
    printEssais(); // affichage du tour

    //
    // gestion des qualif
    //

    //
    // gestion de la course
    //

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