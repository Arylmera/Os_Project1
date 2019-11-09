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
#include <sys/stat.h> // mkdir
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
#define OUTPOURCENT 2
#define SLEEPDIVISER 80
#define PATH_SIZE 1024

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
} f1;

int carListNumber[CAR] = {7, 99, 5, 16, 8, 20, 4, 55, 10, 26, 44, 77, 11, 18, 23, 33, 3, 27, 63, 88};
f1 carList[CAR];
FILE * file;
FILE * logFile;
int shmid;
char path[(PATH_SIZE/2)+50];
char dir_path[(PATH_SIZE/2)+50];
char race_name[50];

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
    uint32_t size = sizeof(path);
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
 * affichage du meilleur segment de chacune des voitures
 */
void showBestSect(){
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                            Tableau des Secteurs                                               \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    for (int sec = 0; sec < SECTION; sec++) {
        for (int car = 0; car < CAR; car++) {
            int best_time = carList[car].circuit[0][sec];
            for (int turn = 1; turn < TURN; turn++) {
                if (best_time > carList[car].circuit[turn][sec]){
                    best_time = carList[car].circuit[turn][sec];
                }
            }
            printf(""BLU"Section"RESET" %3d "RED"||"CYN" Voiture"RESET" %2d "RED"||"BLU" Best time :"RESET" %2d \n",sec+1,carList[car].number,best_time);
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
 * message de bien venus et demande des paramètres de la course
 */
void showWelcome(){
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
    printf(RED "                                            Welcome to the Race                                                \n" RESET);
    printf(RED "---------------------------------------------------------------------------------------------------------------\n" RESET);
}

/**
 * affichage des questions et intéractions pour la demande de nouvelle course de début de partie
 * @return true si nouvelle partie false si non
 */
bool continueTheRace(){
    printf(CYN"Do you plan to run a new race ?\n"RESET);
    printf("enter :"GRN" Y to continue \n"RESET);
    printf("enter : "RED" N to start a new one \n"RESET);
    char new_race;
    scanf("%c",&new_race);
    if (new_race == 'Y'){
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
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    fprintf(file,"                                            Tableau des Secteurs                                               \n");
    fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    for (int sec = 0; sec < SECTION; sec++) {
        for (int car = 0; car < CAR; car++) {
            int best_time = carList[car].circuit[0][sec];
            for (int turn = 1; turn < TURN; turn++) {
                if (best_time > carList[car].circuit[turn][sec]){
                    best_time = carList[car].circuit[turn][sec];
                }
            }
            fprintf(file,"Section %3d || Voiture %2d || Best time : %2d \n",sec+1,carList[car].number,best_time);
        }
        fprintf(file,"---------------------------------------------------------------------------------------------------------------\n");
    }
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
            if (currentCar->out){
                currentCar->circuit[i][j] = 0;
            }
            else {
                int section_time = genSection();
                currentCar->circuit[i][j] = section_time;
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
        memcpy(carList, input, sizeof(carList));
        bubbleSortCarList();
        clrscr();
        showRunTotal(0);
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
    fprintf(logFile,"-- Other Race --\n");
    fprintf(logFile,"%s \n",race_name);
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d-",carList[car].number);
    }
    fprintf(logFile,"\n");
    for(int car = 0; car < CAR; car ++){
        fprintf(logFile,"%d-",carList[car].totalTime);
    }
    fprintf(logFile,"\n");

    fclose(logFile);
}

/**
 * lecture depuis le fichier de log
 */
void recupLog(){
    char log_path[PATH_SIZE];
    strcpy(log_path,path);
    strcat(log_path,"-log.txt");

    logFile = fopen(log_path, "at");
    if (!logFile) {
        printf("can not open logfile.txt for writing or doens't exist.\n");
        return;   // error de log, sortie
    }

    // récupération des données de course


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
        getDir(race_name);
        mkdir(dir_path,0777);
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
    showBestSect(); // affichages des meilleurs sections par voiture
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
}

/**
 * Gestion de la crouse en elle même
 */
void lunchRun(){
    // gestion de la course
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

    // initalisation des voitures
    init_car_list(carListNumber);
    // allocation de la mem partagée
    shmid = shmget(KEY, (20 * sizeof(f1)), 0666 | IPC_CREAT); // 0775 || user = 7 | groupe = 7 | other = 5
    if (shmid == -1) {
        perror("ERROR in creation of the Shared Memory");
        printf("\n");
        shmctl(shmid, IPC_RMID, NULL); // suppression de la memoire partagée
        return 1;
    }
    init_mem(shmid);

    // gestion du circuit
    lunchEssais();

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