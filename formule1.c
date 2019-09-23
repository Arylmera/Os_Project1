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

int carList[] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
int mainBoard[sizeof(carList)][5]; // tableau global des voitures et des temps
int * lap;

/*
* structure des voitures
*/
struct car {
    int number = -1;
    bool stands = false;
    bool out = false;
    
    int essais[3];
    int qualif[3];
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
 * génération d'un tableau conprenand le temps des sections et du temps total
 * pour les voitures sous forme de pointeur
 * @return array int *
 */
int * getLapTime(){
    int lap[] = {getRandomTime(), getRandomTime(), getRandomTime(), 0};
    lap[3] = lap[0] + lap[1] + lap[2];

    return lap;
}

/**
 * fonction de génération d'un tour de chacune des voitures par secteurs et total
 * @return void
 */
void genOneTurn(){
    for (int i = 0; i < sizeof(carList); i++){ // pour la liste des voitures
        mainBoard[i][0] = carList[i];
        lap = getLapTime(); // génération des secteurs et tu total pour la voiture
        for (int j = 0; j < sizeof(lap); j++){ // remplissage du mainboard des lap pour les voitures
            mainBoard[i][j+1] = lap[j];
        }
    }
}

/**
 * affichage en console du tour qui vient d'etre effectué
 */
void showTurn(){
    for (int i = 0; i < sizeof(mainBoard[0]) ; i++) {
        printf("La voiture numéro %d ces temps de segments et tour sont : \n", mainBoard[i][0]);
        for (int j = 1; j <= sizeof(mainBoard[i][0]); j++){
            if(j == sizeof(mainBoard[i][0])){
                printf("Total : %d\n", mainBoard[i][j]);
            }
            else {
                printf("Secteur %d : %d\n",j,mainBoard[i][j]);
            }
        }
    }
}

/**
 * fonction principal du programme
 * @return 0 => quit
 */
int main(){
    srand(time(NULL)); // init unique du random

    genOneTurn(); // generation de un tour
    showTurn(); // affichage du tour

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