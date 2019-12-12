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
#include <string.h> //memset
#include <sys/wait.h> //wait
#include <sys/stat.h> // mkdir
#include <signal.h>
#include <sys/shm.h>
#include <ctype.h>
#include <semaphore.h> // semaphore
#include <sys/time.h>

/***********************************************************************************************************************
 *                               définitions
 **********************************************************************************************************************/

//#define _GNU_SOURCE

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"

#define TURN 3
#define SECTION 3
#define CAR 20
#define KEY 666
#define STANDPOURCENT 25
#define OUTPOURCENT 2
#define SLEEPDIVISER 10
#define PATH_SIZE 1024
#define MAXCHAR 256

/***********************************************************************************************************************
 *                               déclarations et variables globales
 **********************************************************************************************************************/

typedef struct {
    int number;
    int stands;
    bool in_stands;
    bool out;
    int totalTime;

    int circuit[TURN][SECTION];
    int currrent_section[SECTION];
} f1;

int carListNumber[CAR] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
f1 carList[CAR];
FILE * file;
FILE * logFile;
int shmid;
char path[(PATH_SIZE/2)+50];
char dir_path[(PATH_SIZE/2)+50];
char race_name[50];
int best_sect_time[SECTION];

sem_t* sem_parent;
sem_t* sem_fils;
int sem_car_pos = 0;
int car_finished = 0;

const char * part_separator = "$";
const char * data_separator = "!";

int essais = 0;
int qualif = 0;
int course = 0;

void circuit_son(int shmid,int carPosition);
void circuit_father(int shmid,char* entry);

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
    memset(tmp.currrent_section, 0, sizeof(tmp.currrent_section));
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
        memset(carList[i].currrent_section, 0, sizeof(carList[i].currrent_section));
    }
}

/**
 * initalisation de la shared memory
 */
void init_mem(int shmid){
    memset(best_sect_time,0,sizeof(best_sect_time));
    f1 *mem = (f1 *) shmat(shmid, 0, 0);
    for(int i = 0; i < CAR; i++){
        mem[i] = init_car(carListNumber[i]);
    }
}

/**
 * remise a zero des valeurs propre a la course pour chaque voiture
 */
void resetTimeCar(){
    f1 *mem = (f1 *) shmat(shmid, 0, 0);
    for(int i = 0; i < CAR; i++){
        mem[i].stands = 0;
        mem[i].in_stands = false;
        mem[i].totalTime = 0;
        memset(mem[i].circuit, 0, sizeof(mem[i].circuit));
        memset(mem[i].currrent_section, 0, sizeof(mem[i].currrent_section));
    }
    memset(best_sect_time,0,sizeof(best_sect_time));
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
    sleep(time/SLEEPDIVISER);
    return time; // gestion du random et du temps perdu
}

/**
 * génération du temps d'arret au stand
 * @return integer between 2 and 5
 */
int genRandomStand(){
    int time =  20 - (rand() % 5);
    sleep(time/SLEEPDIVISER);
    return time;
}

/**
 * génération nombre entre 1 et 100
 * @return int between 1 and 100
 */
int genRandom(){
    return 100 - (rand() % 100);
}

/**
 * singal process killed
 */
void processKilled(){
    printf("end\n");
}

/**
 * gestion des tours d essais des voitures
 * @param shmid id de la memoire partagée
 *        entry est le type de la course
 * @return -1 if error else 0
 */
int gen_circuit(int shmid,char* entry){
    // mise en place des signaux
    signal(SIGINT, processKilled);
    //mise en place du sémaphore
    sem_parent = sem_open("semP", O_CREAT | O_EXCL, 0644, 0);
    sem_fils = sem_open("semN", O_CREAT | O_EXCL, 0644, 1);

    car_finished = 0;
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
    circuit_father(shmid,entry);
    // suppression du semaphore
    sem_unlink ("semP");
    sem_unlink ("semN");

    sem_close(sem_parent);
    sem_close(sem_fils);
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
    // tri des autres voitures
    for (int i = 0; i < size-1; i++){
        for (int j = 0; j < size - i - 1; j++){
            if (carList[j].totalTime > carList[j+1].totalTime){
                swap(&carList[j], &carList[j+1]);
            }
        }
    }
    // gestion des voitures out en fin de liste
    for(int i = 0; i < size; i++){
        if(carList[i].out) {
            for (int j = size; j > i; j--) {
                if(!carList[j].out){
                    swap(&carList[i],&carList[j]);
                }
            }
        }
    }
}

/**
 * mise a jour du path du dossier de course
 * @param dir_name
 */
void getDir(char* dir_name){
    strcpy(dir_path,path);
    strcat(dir_path,"-");
    strcat(dir_path,dir_name);
    strcat(dir_path,"/");
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
 * réécriture sur ce qui est déjà en console
 */
void softClr(){
    printf ("\033[;H");
}

/**
 * renvois du char P si la voiture est au stand N si non
 *
 * @return R = Running / O = Out / P = Pit
 */
char status(bool stand,bool out){
    if (stand){
        return 'P';
    }
    else if(out){
        return 'O';
    }
    return 'R';
}

/**
 * affichage du tableau de résultat de tout les tours pour toutes les voitures et sections
 * @param banner 1 si bannière 0 si sans
 */
void showRunTotal(int banner){
    if (banner){
        printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
        printf(RED "                                       Tableau des Récapitulatif                                               \n" RESET);
        printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    }
    for (int turn = 0; turn < TURN; turn++){
        for(int car = 0; car < CAR; car++){
            printf(BLU"Voiture %3d "RED"||"RESET" turn : %1d "RED"||"RESET" "GRN"S1"RESET" : %2d | "GRN"S2"RESET" : %2d | "GRN"S3"RESET" : %2d  "RED"||"RESET" "CYN"Status "RESET" : %c | "CYN"Stands stop"RESET" : %2d "RED"||"BLU"Total Turn"RESET" : %3d \n",carList[car].number,
                   turn+1,
                   carList[car].circuit[turn][0],
                   carList[car].circuit[turn][1],
                   carList[car].circuit[turn][2],
                   status(carList[car].in_stands,carList[car].out),
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
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                            Tableau des Résultats                                              \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    for(int car = 0; car < CAR; car++){
        printf(BLU"Voiture "RESET" %2d "RED"||"CYN" Nombre arrets aux stands : "RESET" %2d "RED"||"CYN"Out :"RESET" %c || "BLU" Temps Total : "RESET" %4d \n",
               carList[car].number,
               carList[car].stands,
               status(carList[car].in_stands,carList[car].out),
               carList[car].totalTime);
    }
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * affichage des meilleurs temps par secteurs par voitures
 * @param entry type de la course
 */
void showCurrentSect(char* entry){
    printf(CYN "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(CYN "                                                 %s                                                \n" RESET,entry);
    printf(CYN "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                                 Tableau des Secteurs                                                 \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    for(int car = 0; car < CAR; car++){
        printf(CYN "---------------------------------------------------------------------------------------------------------------\n" RESET);
        printf(BLU"Voiture"RESET" %3d "CYN"||"GRN" S1 "RESET": %3d "CYN"|"GRN" S2 "RESET": %3d "CYN"|"GRN" S3 "RESET": %3d "CYN"|--|"RED" Status "RESET": %2c "CYN"|--|"BLU" Total "RESET": %4d\n",
               carList[car].number,
               carList[car].currrent_section[0],
               carList[car].currrent_section[1],
               carList[car].currrent_section[2],
               status(carList[car].in_stands,carList[car].out),
               carList[car].totalTime);
    }
    printf(CYN "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * affichage des meilleurs secteurs entre les voitures
 */
void showBestSect(){
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                           Best Time For Secteurs                                               \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED"Best Secteurs Globaux "CYN"|-|"GRN" S1 "RESET": %3d "CYN"||"GRN" S2 "RESET": %3d "CYN"||"GRN" S3 "RESET": %3d "CYN"|-|\n"RESET,
           best_sect_time[0],
           best_sect_time[1],
           best_sect_time[2]);
}

/**
 * message de bien venus et demande des paramètres de la course
 */
void showWelcome(){
    printf("\n");
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                            Welcome to the Race                                                \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * helper pour les status des courses
 * @param type type de la course Essais/qualif/run
 * @return string
 */
char* existingRunHelper(int type,int value){
    static char exist_run_helper[75] = {0};
    memset(exist_run_helper,0,sizeof(exist_run_helper));
    if(type == 1){ // essais
        if (value == 0){
            strcat(exist_run_helper,RED);
            strcat(exist_run_helper,"TODO TODO TODO");
            strcat(exist_run_helper,RESET);
        }
        else if (value == 1){
            strcat(exist_run_helper,GRN);
            strcat(exist_run_helper,"DONE ");
            strcat(exist_run_helper,RESET);
            strcat(exist_run_helper,RED);
            strcat(exist_run_helper,"TODO TODO");
            strcat(exist_run_helper,RESET);
        }
        else if (value == 2){
            strcat(exist_run_helper,GRN);
            strcat(exist_run_helper,"DONE DONE ");
            strcat(exist_run_helper,RESET);
            strcat(exist_run_helper,RED);
            strcat(exist_run_helper,"TODO");
            strcat(exist_run_helper,RESET);

        }
        else{
            strcat(exist_run_helper,GRN);
            strcat(exist_run_helper,"DONE DONE DONE");
            strcat(exist_run_helper,RESET);

        }
    }
    else { // qualif and run
        if(value == 1){
            strcat(exist_run_helper,GRN);
            strcat(exist_run_helper,"DONE");
            strcat(exist_run_helper,RESET);
        }
        else {
            strcat(exist_run_helper,RED);
            strcat(exist_run_helper,"TODO");
            strcat(exist_run_helper,RESET);
        }
    }
    return exist_run_helper;
}

/**
 * printf les nom des courses déja trouvée dans le log
 */
void printExistingRun(){
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    strcat(log_path,"-log.txt");
    logFile = fopen(log_path, "rb");
    if (logFile == NULL) {
        printf("No past run found or log files missing.\n");
        return;   // error de log, sortie
    }
    printf("\nCurrent Race Data found \n");

    char buffer[MAXCHAR];
    char * data_save_ptr;
    char * type_save_ptr;

    printf("lecture depuis le fichier log à l'adresse : %s\n",log_path);
    printf(YEL"----------------------------------------------------------------\n"RESET);
    while(fgets(buffer, MAXCHAR, logFile) != NULL){
        // name
        char *buffer_run = strtok_r(buffer, part_separator, &data_save_ptr);
        char *tmp_name = buffer_run;
        // status
        buffer_run = strtok_r(NULL,part_separator, &data_save_ptr);
        long tmp;
        char *buffer_type = strtok_r(buffer_run, data_separator, &type_save_ptr); // essais
        tmp = strtol(buffer_type, NULL, 10);
        int tmp_essais = (int) tmp;
        buffer_type = strtok_r(NULL, data_separator, &type_save_ptr); // qualif
        tmp = strtol(buffer_type, NULL, 10);
        int tmp_qualif = (int) tmp;
        buffer_type = strtok_r(NULL, data_separator, &type_save_ptr); // run
        tmp = strtol(buffer_type, NULL, 10);
        int tmp_run = (int) tmp;
        // affichage course
        printf(CYN"\t %10s "RESET" |",tmp_name);
        printf("| essais : %s \t",existingRunHelper(1,tmp_essais));
        printf("| qualif : %s \t",existingRunHelper(2,tmp_qualif));
        printf("| course : %s \t",existingRunHelper(3,tmp_run));
        printf("\n");
    }
    printf(YEL"----------------------------------------------------------------\n"RESET);
    fclose(logFile);
}

/**
 * affichage des questions et intéractions pour la demande de nouvelle course de début de partie
 * @return true si nouvelle partie false si non
 */
bool continueTheRace(){
    printExistingRun();
    printf(CYN"Do you plan to continue a race ?\n"RESET);
    printf("enter :"GRN" Y to continue \n"RESET);
    printf("enter : "RED" N to start a new one \n"RESET);
    char new_race;
    scanf(" %c",&new_race);
    if (new_race == 'Y' || new_race == 'y'){
        return true;
    }
    return false;
}

/**
 * affichage de la demande d'utilisation de la carListNumber par défault
 * @return true si default carListNumber
 */
bool useDefaultCarList(){
    printf("do you want to use a predifined list of 20 numbers of cars ? \n");
    printf("enter :"GRN" Y to use the default  \n"RESET);
    printf("enter :"RED" N to enter yourself the numbers of the 20 cars \n"RESET);
    char predif_value;
    scanf(" %c",&predif_value);
    if (predif_value == 'Y' || predif_value == 'y') {
        return true;
    }
    return false;
}

/**
 * check des best temps aux secteurs
 */
void checkBestSect(){
    for (int turn = 0; turn < TURN; turn++) {
        for (int sec = 0; sec < SECTION; sec++) {
            for (int car = 0; car < CAR; car++) {
                if ((best_sect_time[sec] > carList[car].circuit[turn][sec] || best_sect_time[sec] == 0) && carList[car].circuit[turn][sec] != 0) {
                    best_sect_time[sec] = carList[car].circuit[turn][sec];
                }
            }
        }
    }
}

/***********************************************************************************************************************
 *                               fonctions Gestion des fichiers
 **********************************************************************************************************************/

/**
 * output des données dans le fichier prévus pour la lecture depuis l'utilisateur
 */
void outputData(){
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"                                            Tableau des Résultats                                              \n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    for(int car = 0; car < CAR; car++){
        fprintf(file,"Voiture %2d || Nombre arrêts aux stands : %2d || Out : %c || Temps Total : %4d \n",
                carList[car].number,
                carList[car].stands,
                status(carList[car].in_stands,carList[car].out),
                carList[car].totalTime);
    }
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n" );
    fprintf(file,"                                                 Tableau des Secteurs                                          \n" );
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    for(int car = 0; car < CAR; car++){
        fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
        fprintf(file,"Voiture %3d || S1 : %3d | S2 : %3d | S3 : %3d |--| Total : %4d\n",
                carList[car].number,
                carList[car].currrent_section[0],
                carList[car].currrent_section[1],
                carList[car].currrent_section[2],
                carList[car].totalTime);
    }
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"                                          Best Time For Secteurs                                               \n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"Best Secteurs Globaux |-| S1 : %3d || S2 : %3d || S3 : %3d |-|\n",
            best_sect_time[0],
            best_sect_time[1],
            best_sect_time[2]);
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"                                       Tableau des Récapitulatif                                               \n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    for (int turn = 0; turn < TURN; turn++){
        for(int car = 0; car < CAR; car++){
            fprintf(file,"Voiture %3d || turn : %1d || S1 : %2d | S2 : %2d | S3 : %2d  || Status : %c | Stands stop : %2d ||Total Turn : %3d \n",
                    carList[car].number,
                    turn+1,
                    carList[car].circuit[turn][0],
                    carList[car].circuit[turn][1],
                    carList[car].circuit[turn][2],
                    status(carList[car].in_stands,carList[car].out),
                    carList[car].stands,
                    carList[car].circuit[turn][0]+carList[car].circuit[turn][1]+carList[car].circuit[turn][2]);
        }
        fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    }
}

/**
 * écriture dans le fichier sur base du nom des résultats globaux
 * @param result_name nom du fichier de résulat sur base du nom de la course
 */
void outputFile(char* result_name){
    char result_file_path[PATH_SIZE];
    getDir(race_name);
    printf(RED"current path : %s \n"RESET,path);
    printf(RED"current dir : %s \n"RESET,dir_path);
    strcpy(result_file_path,dir_path);
    strcat(result_file_path,race_name);
    strcat(result_file_path,"-");
    strcat(result_file_path,result_name);
    strcat(result_file_path,".txt");
    //ouverture du fichier
    printf("ecriture des résultats à : "GRN"%s \n"RESET,result_file_path);
    file = fopen(result_file_path,"w");
    outputData();
    //fermeture du fichier
    fclose(file);
}

/**
 * demande le numéro de voitures a utiliser dans la carListNumber
 */
void getCarNumber(){
    printf("enter the "CYN"number of the cars"RESET" that will participate in the race : "RED"(20 cars needed)\n"RESET);
    bool used = false;
    for (int i = 0; i < CAR; i++) {
        int number;
        do {
            char number_buffer[10];
            used = false;
            bool is_integer = false;
            do {
                printf("Car %d : ", i + 1);
                scanf("%s", &number_buffer[0]);
                int length = strlen(number_buffer);
                for(int i = 0; i < length; i++) {
                    if(!isdigit(number_buffer[i])){
                        printf("Please enter a number between 0 and 99\n");
                        break;
                    }
                    else if (i == length - 1){
                        is_integer = true;
                        number = atoi(number_buffer);
                    }
                }
            }while(!is_integer);
            for (int j = i; j >= 0; j--) {
                if (number == carListNumber[j]) {
                    printf("\nThis car is already set, please enter an other number of car not already used \n");
                    used = true;
                    break;
                }
            }
        }while(used);
        carListNumber[i] = number;
    }
}

/**
* demande du choix de la partie de course a lancer
* @return
*/
int choiceTypeOfRun(){
    char type_string[5];
    int type =0;
    printf(CYN"Quelle partie de la course voulez-vous lancer ?\n"RESET);
    printf("\t 1 : Les essais\n");
    printf("\t 2 : Les Qualifs\n");
    printf("\t 3 : La  Course\n");
    scanf("%s",type_string);
    type = (int) atoi(type_string);
    return type;
}

/**
 * met les voitures out sur base du nombre de voitures données a partir de la fin
 * @param num nombre de voiture a mettre out
 */
void setOut(int shmid,int num){
    f1 *mem = (f1 *) shmat(shmid, 0, 0);
    for(int car = CAR; car > (CAR-(num-1)); car-- ){
        mem[car].out = true;
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
    srand(time(NULL)+getpid()); // génération du nouveau random pour chaque fils
    for(int i = 0; i < CAR; i++){ // récupération de l'entrée en shmem
        if(output[i].number == carNumber){
            currentCar = &output[i];
            break;
        }
    }
    for(int i = 0; i < TURN ; i++){ // pour chaque tour
        for(int j = 0; j < SECTION; j++) { // pour chaque section du tour
            //printf("section critique du fils %d\n",carNumber);
            if (currentCar->out){ // sila voiture est out
                currentCar->circuit[i][j] = 0;
            }
            else { // si la voiture est encore en course
                int section_time = genSection();
                // écriture dans la mem
                currentCar->circuit[i][j] = section_time;
                currentCar->currrent_section[j] = section_time;
                currentCar->totalTime += section_time;
                // test random out de la voiture
                if (genRandom() < OUTPOURCENT) {
                    currentCar->out = true;
                }
                // test pour stand
                if ((genRandom() < STANDPOURCENT || ((i == (TURN - 1)) && currentCar->stands == 0)) && j == (SECTION-1)) { // x% de s'arreter ou si jamais arrêter pendant la course
                    currentCar->in_stands = true;
                    int time_in_stands = genRandomStand();
                    currentCar->circuit[i][SECTION - 1] += time_in_stands;
                    currentCar->totalTime += time_in_stands;
                    currentCar->stands++;
                    currentCar->in_stands = false;
                }
                //fin
            }
            // gestion des semaphores
            //printf(RED"fils %d bloqué\n"RESET,carPosition);
            sem_wait(sem_fils);
            //printf(GRN"fils %d entée \n"RESET,carPosition);
            sem_car_pos = carPosition;
            //printf(YEL"libération du père || carPosion = %d\n"RESET,carPosition);
            sem_post(sem_parent);
        }
    }
    car_finished++;
    if (car_finished == CAR){
        kill(getppid(), SIGKILL);
    }
    exit(EXIT_SUCCESS);
}

/**
 * fonction du père
 * @param shmid id de la memoire partagée
 * @param entry type de la course
 */
void circuit_father(int shmid,char* entry){
    int status = 0;
    pid_t wpid;
    clrscr();
    // récupération des données de la SM
    clrscr();
    f1 *input = (f1*) shmat(shmid,0,0);
    do{ // temps que un processus est en cours
        // sc du pere
        //printf(YEL"section critique du pere -- Voiture %d\n"RESET,sem_car_pos);
        f1 *car_imput = &input[sem_car_pos];
        f1 *car_list_pos = &carList[sem_car_pos];
        memcpy(carList, input, sizeof(carList));
        bubbleSortCarList();
        softClr();
        showCurrentSect(entry);
        checkBestSect();
        showBestSect();
        // semaphore
        sem_post(sem_fils);
        //printf(GRN"libération du fils %d depuis le pere\n"RESET,sem_car_pos);
        sem_wait(sem_parent);
    }while(1);     //(wpid = wait(&status)) > 0);
}

/***********************************************************************************************************************
 *                               fonctions gestion des logs
 **********************************************************************************************************************/

/**
 * generation du fichier de log de la course ou écriture si existant
 */
void genLog(){
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    strcat(log_path,"-log.txt");

    logFile = fopen(log_path, "at");
    if (!logFile) logFile = fopen("logfile.txt", "wt");
    if (!logFile) {
        printf("can not open logfile.txt for writing.\n");
        return;   // error de log, sortie
    }
    // data to put in the log file
    // race name
    fprintf(logFile,"%s",race_name);
    fprintf(logFile,"$");
    // race state
    fprintf(logFile,"%d!",essais);
    fprintf(logFile,"%d!",qualif);
    fprintf(logFile,"%d!",course);
    fprintf(logFile,"$");
    // list of car number
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d!",carList[car].number);

    }
    fprintf(logFile,"$");
    /*
    // total time of cars
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d!",carList[car].totalTime);
    }
    fprintf(logFile,"$");
     */
    // car status
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%c!",status(carList[car].in_stands,carList[car].out));
    }
    fprintf(logFile,"\n");
    // fermeture du log
    fclose(logFile);
}

/**
 * lecture depuis le fichier de log
 */
void recupLog(){
    FILE * logFile_tmp;
    char log_path_tmp[PATH_SIZE];
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    memcpy(log_path_tmp, log_path, PATH_SIZE);
    strcat(log_path,"-log.txt");
    strcat(log_path_tmp,"-log_tmp.txt");

    logFile = fopen(log_path, "rb");
    logFile_tmp = fopen(log_path_tmp, "wt");
    if (!logFile) {
        printf("can not open logfile.txt for writing or doens't exist.\n");
        return;   // error de log, sortie
    }
    // lecture du fichier log
    char buffer[MAXCHAR];
    bool found = false;

    char * part_save_ptr;
    char * data_save_ptr;

    while ((fgets(buffer, MAXCHAR, logFile) != NULL)) {
        // récupération de la 1er partie etant le nom
        char buffer_line[MAXCHAR];
        memset(buffer_line,0,sizeof(buffer_line));
        memcpy(buffer_line,buffer,sizeof(buffer));
        char *buffer_part = strtok_r(buffer, part_separator, &part_save_ptr);
        if (strcmp(buffer_part, race_name) == 0) { // si c est la bonne course
            found = true;
            // race state
            buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
            long tmp;
            // essais
            char *buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
            tmp = strtol(buffer_data, NULL, 10);
            essais = (int) tmp;
            // qualid
            buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
            tmp = strtol(buffer_data, NULL, 10);
            qualif = (int) tmp;
            // course
            buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
            tmp = strtol(buffer_data, NULL, 10);
            course = (int) tmp;
            // récupération de la partie numéro de voiture
            buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
            buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
            printf("récupération des numéro de voiture \n");
            for(int car = 0; car < CAR; car++){
                long tmp;
                tmp = strtol(buffer_data, NULL, 10);
                carListNumber[car] = (int) tmp;
                printf("car number %d \n", carListNumber[car]);
                buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
            }
            printf("récupération effectuée \n");
            printf("initialisation de la liste des voiture sur base des numéro chargé \n");
            init_car_list(carListNumber);
            printf("initalisation effectuée\n");
            /*
            // récupération de la partie des temps totaux
            buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
            buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
            printf("récupération du temps total de partie\n");
            for(int car = 0; car < CAR; car++){
                tmp = strtol(buffer_data, NULL, 10);
                carList[car].totalTime = (int) tmp;
                printf("car total time %d = %d \n", carList[car].number, carList[car].totalTime);
                buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
            }
            printf("temps totaux loaded\n");
             */
            // récupération des status de voiture
            buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
            buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
            printf("récupération du status des voitures\n");
            for(int car = 0; car < CAR; car++){
                char value = buffer_data[0];
                if (value == 'O') {
                    carList[car].out = true;
                    printf("car %d, is out \n", carList[car].number);
                }
                buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
            }
        }
        else { // si ce n'est pa la bonne ligne
            fputs(buffer_line, logFile_tmp);
        }
    }
    if (!found) {
        printf("No run under this name found\n");
        printf("please enter the name of the run you whant to load : ");
        scanf("%s",race_name);
        printf("\n");
    }

    // fermeture du log
    fclose(logFile);
    fclose(logFile_tmp);
    // switch des 2 files pour supprimer la ligne non voulue
    remove(log_path);
    rename(log_path_tmp, log_path);
    //clear de la console
    clrscr();
}

/**
 * chargement de la course depuis le fichier log ou création de la course si nouvelle
 */
void raceLoading(){
    printf(RED"current path : %s \n"RESET,path);
    if(continueTheRace()){
        printf(GRN"what's the name of your race ? \n"RESET);
        // récup nom de la course
        printf("name of the race : ");
        scanf("%s",race_name);
        printf("\n");
        // récupération des données de la course depuis le fichier de sauvgarde
        getDir(race_name);
        recupLog();

    }
    else{
        clrscr();
        showWelcome();
        printf(GRN"let's start a new one then\n"RESET);
        // demande des paramètres de course
        printf("what is the name of your race ? \n"RED"max 50 characters and no space\n"RESET);
        printf("name of the race : ");
        scanf("%s",race_name);
        printf("\n");
        // création du directory de sauvgarde de la course
        getDir(race_name);
        mkdir(dir_path,0777);
        // initalisation des voitures
        if(!useDefaultCarList()){
            getCarNumber();
        }
        init_car_list(carListNumber);
    }
}

/***********************************************************************************************************************
 *                               fonctions de type de course
 **********************************************************************************************************************/

/**
 * Gestion des essais d'un course
 */
void lunchEssais(){
    char essais_name[12];
    sprintf(essais_name, "Essais-%d", (essais + 1));
    if(essais >= 3){ // si tout les essais ont déja ete fait
        printf("You already have done all the essais for this run \n");
    }
    else {
        gen_circuit(shmid, essais_name); // génération de la course
        bubbleSortCarList(); // tri des voitures sur base de leur temps totaux
        clrscr(); // clear de la console
        showRun(); // affichage des stats globales des voitures
        showCurrentSect(essais_name); // affichages des meilleurs sections par voiture
        showBestSect(); // affichage des meilleurs temps par secteurs
        showRunTotal(1); // affichage récapitulatif avec bannière

        essais += 1;
    }
    // génération du fichier de résultats
    char* result_name = essais_name;
    outputFile(essais_name);
    genLog();
}

/**
 * Gestion des qualif d'une course
 */
void lunchQualif(){
    if(essais < 3){
        printf("please run all the essais before.\n");
    }
    else if (qualif == 1){ // si qualif deja fait
        printf("You already have done the qualifications for this run \n");
    }
    else {
        // gestion des qualif
        // Q1
        gen_circuit(shmid, "Qualif -- Q1");
        bubbleSortCarList();
        setOut(shmid, 5); // 5 dernières OUT
        // Q2
        resetTimeCar();
        gen_circuit(shmid, "Qualif -- Q2");
        bubbleSortCarList();
        setOut(shmid, 10); // 10 dernières OUT
        // Q3
        resetTimeCar();
        gen_circuit(shmid, "Qualif -- Q3");
        bubbleSortCarList();
        // affichage final
        bubbleSortCarList();
        clrscr(); // clear de la console
        showRun(); // affichage des stats globales des voitures
        showCurrentSect("Qualif"); // affichages des meilleurs sections par voiture
        showBestSect(); // affichage des meilleurs temps par secteurs
        showRunTotal(1); // affichage récapitulatif avec bannière

        qualif += 1;
    }
    // génération du fichier de résultats
    char* result_name = "Qualif";
    outputFile(result_name);
    genLog();
}

/**
 * Gestion de la crouse en elle même
 */
void lunchRun(){
    if(essais < 3 || qualif != 1){
        printf("Please run all the essais and qualifications before.\n");
        return;
    }
    else if (course == 1){ // si course déja fait
        printf("You already have done the run \n");
        return;
    }
    else {
        // gestion de la course
        gen_circuit(shmid, "Course"); // génération de la course
        bubbleSortCarList(); // tri des voitures sur base de leur temps totaux
        clrscr(); // clear de la console
        showRun(); // affichage des stats globales des voitures
        showCurrentSect("Course"); // affichages des meilleurs sections par voiture
        showBestSect(); // affichage des meilleurs temps par secteurs
        showRunTotal(1); // affichage récapitulatif avec bannière

        course += 1;
    }
    // génération du fichier de résultats
    char* result_name = "Course";
    outputFile(result_name);
    genLog();
}

/***********************************************************************************************************************
 *                               fonctions Main
 **********************************************************************************************************************/

/**
 *  function main
 * @return
 */
int main(int argc, char *argv[]) {
    // récupération des données de la course depuis un fichier
    clrscr();
    strcpy(path,realpath(argv[0],0));
    showWelcome();
    raceLoading();
    // allocation de la mem partagée
    shmid = shmget(KEY, (CAR * sizeof(f1)), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror(RED"ERROR in creation of the Shared Memory"RESET);
        printf("\n");
        shmctl(shmid, IPC_RMID, NULL); // suppression de la memoire partagée
        return 1;
    }
    init_mem(shmid);

    // gestion du circuit
    int choice_type = choiceTypeOfRun();
    if (choice_type == 1) {
        lunchEssais();
    }
    else if (choice_type == 2){
        lunchQualif();
    }
    else if (choice_type == 3){
        lunchRun();
    }

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