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
#include <sys/types.h>
#include <sys/shm.h>
#include <ctype.h>
#include <semaphore.h>

/***********************************************************************************************************************
 *                               définitions
 **********************************************************************************************************************/

#define _GNU_SOURCE

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
#define OUTPOURCENT 2
#define SLEEPDIVISER 15
#define PATH_SIZE 1024
#define MAXCHAR 1024

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
int buffer_array[CAR];
int best_sect_time[SECTION];

sem_t semaphore; // déclatration du sémaphore

const char * part_separator = "$";
const char * data_separator = "!";

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
void init_mem(shmid){
    memset(best_sect_time,0,sizeof(best_sect_time));
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
 * return le path symbolique de l'emplacement d'exécution du programme
 * @return char[1024]
 */
void getPath(){
    u_int32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        //printf("Files will be saved to : %s\n", path);
    }
    else{
        printf("buffer too small; need size %u to define the path of save\n", size);
    }
}

/**
 * mise a jour du path du dossier de course
 * @param dir_name
 */
void getDir(char* dir_name){
    getPath();
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
 * renvois du char P si la voiture est au stand N si non
 *
 * @return R = Running / O = Out / P = Pit
 *
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
        printf(BLU"Voiture "RESET" %2d "RED"||"CYN" Nombre arrets aux stands : "RESET" %1d "RED"||"CYN"Out :"RESET" %c || "BLU" Temps Total : "RESET" %4d \n",
               carList[car].number,
               carList[car].stands,
               status(carList[car].in_stands,carList[car].out),
               carList[car].totalTime);
    }
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * affichage des meilleurs temps par secteurs par voitures
 */
void showCurrentSect(){
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
 * message de bien venus et demande des paramètres de la course
 */
void showWelcome(){
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                            Welcome to the Race                                                \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * printf les nom des courses déja trouvée dans le log
 */
void printExistingRun(){
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    strcat(log_path,"-log.txt");
    logFile = fopen(log_path, "rb");
    if (!logFile) {
        printf("can not open logfile.txt for writing or doens't exist.\n");
        return;   // error de log, sortie
    }

    printf("\nCurrent Race Data found \n");
    printf(YEL"----------------------------------------------------------------\n"RESET);
    char buffer[MAXCHAR];
    while(fgets(buffer, MAXCHAR, logFile) != NULL){
        char *buffer_name = strtok(buffer,part_separator);
        printf(CYN"\t%s \n"RESET,buffer_name);
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
    printf(CYN"Do you plan to run a new race ?\n"RESET);
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

/***********************************************************************************************************************
 *                               fonctions Gestion des fichiers
 **********************************************************************************************************************/
void outputData(){
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"                                            Tableau des Résultats                                              \n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    for(int car = 0; car < CAR; car++){
        fprintf(file,"Voiture %2d || Nombre arrêts aux stands : %1d || Out : %c || Temps Total : %4d \n",
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
 * @param result_name
 */
void outputFile(char* result_name){
    char result_file_path[PATH_SIZE];
    getPath();
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
 * génération d'un tableau de int sur base d'un string par dilimiteur !
 * @param line string délimité par des ! entre les nombres
 * @return array[CAR] de int
 */
int* genArrayByString(char* line){
    int j = 0;
    for(int i = 0; i < CAR; i++){
        char tmp[10];
        int x = 0;
        while ((line[j] != '!') && j < strlen(line)){
            tmp[x] = line[j];
            x++;
            j++;
        }
        j++;
        buffer_array[i] = (int) tmp;
    }
    return buffer_array;
}

/**
 * demande du choix de la partie de course a lancer
 * @return
 */
int choiceTypeOfRun(){
    char type_string[5];
    int type =0;
    printf("Quelle partie de la course voulez-vous lancer ?\n");
    printf("\t 1 : Les essais\n");
    printf("\t 2 : Les Qualifs\n");
    printf("\t 3 : La  Course\n");
    scanf("%s",type_string);
    type = (int) atoi(type_string);
    return type;
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
    for(int i = 0; i < TURN ; i++){ // pour chaque tour
        for(int j = 0; j < SECTION; j++) { // pour chaque section du tour
            sem_wait(&semaphore);
            if (currentCar->out){
                currentCar->circuit[i][j] = 0;
            }
            else {
                int section_time = genSection();
                currentCar->circuit[i][j] = section_time;
                currentCar->currrent_section[j] = section_time;
                currentCar->totalTime += section_time;
                if (genRandom() < OUTPOURCENT) {
                    currentCar->out = true;
                    //printf("voiture %d a eu un problmème et est OUT", currentCar->number);
                }

                if (genRandom() < STANDPOURCENT || (i == (TURN - 1) && currentCar->stands == 0)) { // 50% de s'arreter ou si jamais arrêter pendant la course
                    currentCar->in_stands = true;
                    int time_in_stands = genRandomStand();
                    currentCar->circuit[i][SECTION - 1] += time_in_stands;
                    currentCar->totalTime += time_in_stands;
                    //printf("arret de la voiture %d au stand , temps total de la section 3 : %d \n", currentCar->number,currentCar->circuit[i][SECTION - 1]);
                    currentCar->stands++;
                    currentCar->in_stands = false;
                }
                sem_post(&semaphore);
            }
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
        sem_wait(&semaphore);
        memcpy(carList, input, sizeof(carList));
        bubbleSortCarList();
        clrscr();
        //showRunTotal(0);
        showCurrentSect();
        sem_post(&semaphore);
    }while ((wpid = wait(&status)) > 0);
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
    fprintf(logFile,"%s",race_name);
    fprintf(logFile,"$");
    // list of car number
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d!",carList[car].number);

    }
    fprintf(logFile,"$");
    // total time of cars
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d!",carList[car].totalTime);
    }
    fprintf(logFile,"$");
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
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    strcat(log_path,"-log.txt");

    logFile = fopen(log_path, "rb");
    if (!logFile) {
        printf("can not open logfile.txt for writing or doens't exist.\n");
        return;   // error de log, sortie
    }
    // lecture du fichier log
    char buffer[MAXCHAR];
    bool found = false;
    
    char * part_save_ptr;
    char * data_save_ptr;

    do {
        while ((fgets(buffer, MAXCHAR, logFile) != NULL) && !found) {
            // récupération de la 1er partie etant le nom
            char *buffer_part = strtok_r(buffer, part_separator, &part_save_ptr);
            if (strcmp(buffer_part, race_name) == 0) { // si c est la bonne course
                found = true;
                // récupération de la partie numéro de voiture
                buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
                char *buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
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
                // récupération de la partie des temps totaux
                buffer_part = strtok_r(NULL, part_separator, &part_save_ptr);
                buffer_data = strtok_r(buffer_part, data_separator, &data_save_ptr);
                printf("récupération du temps total de partie\n");
                for(int car = 0; car < CAR; car++){
                    long tmp;
                    tmp = strtol(buffer_data, NULL, 10);
                    carList[car].totalTime = (int) tmp;
                    printf("car total time %d = %d \n", carList[car].number, carList[car].totalTime);
                    buffer_data = strtok_r(NULL, data_separator, &data_save_ptr);
                }
                printf("temps totaux loaded\n");
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
        }
        if (!found) {
            printf("No run under this name found\n");
            printf("please enter the name of the run you whant to load : ");
            scanf("%s",race_name);
            printf("\n");
        }
    }while(!found);

    // fermeture du log
    fclose(logFile);
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
    gen_circuit(shmid); // génération de la course
    bubbleSortCarList(); // tri des voitures sur base de leur temps totaux
    clrscr(); // clear de la console
    showRun(); // affichage des stats globales des voitures
    showCurrentSect(); // affichages des meilleurs sections par voiture
    showRunTotal(1); // affichage récapitulatif avec bannière

    // génération du fichier de résultats
    char* result_name = "Essais";
    outputFile(result_name);
    genLog();
}

/**
 * Gestion des qualif d'une course
 */
void lunchQualif(){
    // gestion des qualif

    // génération du fichier de résultats
    char* result_name = "Qualif";
    outputFile(result_name);
    genLog();
}

/**
 * Gestion de la crouse en elle même
 */
void lunchRun(){
    // gestion de la course

    // génération du fichier de résultats
    char* result_name = "Course";
    outputFile(result_name);
    genLog();
}

/***********************************************************************************************************************
 *                               fonctions Main
 **********************************************************************************************************************/

/**
 *
 * @return
 */
int main(int argc, char *argv[]) {
    // récupération des données de la course depuis un fichier
    clrscr();
    showWelcome();
    getPath();
    raceLoading();
    //mise en place du sémaphore
    sem_init(&semaphore, 0, 1);
    // allocation de la mem partagée
    shmid = shmget(KEY, (CAR * sizeof(f1)), 0666 | IPC_CREAT); // 0775 || user = 7 | groupe = 7 | other = 5
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
    //lunchEssais();

    // suppression du semaphore
    sem_destroy(&semaphore);
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