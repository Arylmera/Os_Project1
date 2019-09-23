/*
                      ___..............._
             __.. ' _'.""""""\\""""""""- .`-._
 ______.-'         (_) |      \\           ` \\`-. _
/_       --------------'-------\\---....______\\__`.`  -..___
| T      _.----._           Xxx|x...           |          _.._`--. _
| |    .' ..--.. `.         XXX|XXXXXXXXXxx==  |       .'.---..`.     -._
\_j   /  /  __  \  \        XXX|XXXXXXXXXXX==  |      / /  __  \ \        `-.
 _|  |  |  /  \  |  |       XXX|""'            |     / |  /  \  | |          |
|__\_j  |  \__/  |  L__________|_______________|_____j |  \__/  | L__________J
     `'\ \      / ./__________________________________\ \      / /___________\
        `.`----'.'                                     `.`----'.'
          `""""'                                         `""""'
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int carListNumber[] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
int mainBoard[sizeof(carListNumber)]; // tableau global des voitures et des temps

/**
* structure des voitures
*/
struct car {
    int number;
    bool stands;
    bool out;
    
    int essais[4];
    int qualif[4];
};

struct car carList[sizeof(carListNumber)];

/**
* generation de la liste des structur voitrure sur base de la liste des numero de voitures
* @return void
*/
void initCar(int *carListNumber){
    for(int i = 0; i < sizeof(carListNumber); i++){
        carList[i].number = carListNumber[i];
        carList[i].stands = false;
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
 * generation du tour d'essais pour la voiture passé en param
 * @param current
 */
void genLapTime(struct car current){
    int lap[] = {getRandomTime(), getRandomTime(), getRandomTime(), 0};
    lap[3] = lap[0] + lap[1] + lap[2];

    current.essais = lap;
}

/**
 * génération des essais pour toutes les voitures
 */
void genEssais(){
    for(int i = 0; i < sizeof(carList); i++){
        genLapTime(carList[i]);
    }
}

/**
 * affichage en console du tour qui vient d'etre effectué
 */
void show(){
    for(int i = 0; i < sizeof(carList); i++){
        printf("Numéro de voiture : %d \nS1 : %d || S2 : %d || S3 : %d || Total : %d\n",carList->number,carList->essais[0],carList->essais[1],carList->essais[2],carList->essais[3]);
    }
}

/**
 * fonction principal du programme
 * @return 0 => quit
 */
int main(){
    srand(time(NULL)); // init unique du random

    genEssais(); // generation de un tour
    show(); // affichage du tour

    return 0; // fin du programme - logout de la console
}

/*
░░░░░░░░░░░░░░░░░░░░░▄▀░░▌
░░░░░░░░░░░░░░░░░░░▄▀▐░░░▌
░░░░░░░░░░░░░░░░▄▀▀▒▐▒░░░▌
░░░░░▄▀▀▄░░░▄▄▀▀▒▒▒▒▌▒▒░░▌
░░░░▐▒░░░▀▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒█
░░░░▌▒░░░░▒▀▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄
░░░░▐▒░░░░░▒▒▒▒▒▒▒▒▒▌▒▐▒▒▒▒▒▀▄
░░░░▌▀▄░░▒▒▒▒▒▒▒▒▐▒▒▒▌▒▌▒▄▄▒▒▐
░░░▌▌▒▒▀▒▒▒▒▒▒▒▒▒▒▐▒▒▒▒▒█▄█▌▒▒▌
░▄▀▒▐▒▒▒▒▒▒▒▒▒▒▒▄▀█▌▒▒▒▒▒▀▀▒▒▐░░░▄
▀▒▒▒▒▌▒▒▒▒▒▒▒▄▒▐███▌▄▒▒▒▒▒▒▒▄▀▀▀▀
▒▒▒▒▒▐▒▒▒▒▒▄▀▒▒▒▀▀▀▒▒▒▒▄█▀░░▒▌▀▀▄▄
▒▒▒▒▒▒█▒▄▄▀▒▒▒▒▒▒▒▒▒▒▒░░▐▒▀▄▀▄░░░░▀
▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▄▒▒▒▒▄▀▒▒▒▌░░▀▄
▒▒▒▒▒▒▒▒▀▄▒▒▒▒▒▒▒▒▀▀▀▀▒▒▒▄▀
 */