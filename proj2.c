/***************************************/
/*                                     */
/*      vytvoril: xgurec00             */
/*      zacatek projektu: 22.4.2019    */
/*      posedni upravy: 28.4.2019      */
/*                                     */
/***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>

int* MoloHacker = NULL;
int* MoloSurfer = NULL;
int* rowNumber = NULL;
int* boardedNumber = NULL;
int* boardedHacker = NULL;
int* boardedSurfer = NULL;
int* surfer = NULL;
int* hacker = NULL;
int* onlyHackers = NULL;
int* onlySurfers = NULL;
int* mixed = NULL;

sem_t* molo = NULL;
sem_t* print = NULL;
sem_t* voyage = NULL;
sem_t* capitan = NULL;
sem_t* boarding = NULL;

//struktura hodnot
typedef struct values{
    int P;
    int H;
    int S;
    int R;
    int W;
    int C;
} values_t;

//funkce pro nalodeni a plavbu
void board(int x, int i, values_t* vals, FILE* f){
    char* type = (x == 0 ? "HACK" : "SERF");
    
    if (x==0){
        (*hacker)++;
    }
    else if (x==1){
        (*surfer)++;
    }

    //cestujici na molu se pokousi projit pres podminky
    for(;;){
        sem_wait(boarding);
        if (*hacker >= 2 && *surfer >= 2 && *boardedNumber != 4 && *mixed == 0 && ((*boardedHacker < 2 && x==0) || (*boardedSurfer < 2 && x==1))){
            //zablokovani vsech ostatnich moznosti plavby
            (*onlyHackers) = 1;
            (*onlySurfers) = 1;

            if (x == 0){
                (*boardedHacker)++;
            }
            else if (x == 1){
                (*boardedSurfer)++;
            }

            //kazdy nalodeny ucastnik zvysi pocet o 1 pro prehled nalodenych pasazeru
            (*boardedNumber)++;

            //overeni zda se proces nestane kapitanem plavby
            if (*boardedNumber < 4){
                sem_post(boarding);
                sem_wait(voyage);
            }


            //pokud prave ted nastupuje posledni clen plavby, stava se kapitanem a ridi plavbu
            else if (*boardedNumber == 4){
                sem_wait(print);
                (*MoloHacker)-=2;
                (*MoloSurfer)-=2;
                fprintf(f, "%d \t : %s %d \t : boards  \t\t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
                (*rowNumber)++;
                sem_post(print);
                if (vals->R > 0){
                    usleep(random() % vals->R * 1000);
                }
                sem_post(voyage);
                sem_post(voyage);
                sem_post(voyage);
                sem_wait(capitan);
                (*boardedSurfer) = 0;
                (*boardedHacker) = 0;
                (*onlyHackers) = 0;
                (*onlySurfers) = 0;
            }

            break;
        }

        else if (*hacker >= 4 && x == 0 && *boardedNumber != 4 && *onlyHackers == 0){
            //zablokovani vsech ostatnich moznosti plavby
            (*onlySurfers) = 1;
            (*mixed) = 1;

            //kazdy nalodeny ucastnik zvysi pocet o 1 pro prehled nalodenych pasazeru
            (*boardedNumber)++;

            //overeni zda se proces nestane kapitanem plavbys
            if (*boardedNumber < 4){
                sem_post(boarding);
                sem_wait(voyage);
            }

            //pokud prave ted nastupuje posledni clen plavby, stava se kapitanem a ridi plavbu
            else if (*boardedNumber == 4){
                sem_wait(print);
                (*MoloHacker)-=4;
                fprintf(f, "%d \t : %s %d \t : boards  \t\t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
                (*rowNumber)++;
                sem_post(print);
                if (vals->R > 0){
                    usleep(random() % vals->R * 1000);
                }
                sem_post(voyage);
                sem_post(voyage);
                sem_post(voyage);
                sem_wait(capitan);
                (*onlySurfers) = 0;
                (*mixed) = 0;
            }

            break;
        }
        
        else if (*surfer >= 4 && x == 1 && *boardedNumber != 4 && *onlySurfers == 0){
            //zablokovani vsech ostatnich moznosti plavby
            (*onlyHackers) = 1;
            (*mixed) = 1;

            //kazdy nalodeny ucastnik zvysi pocet o 1 pro prehled nalodenych pasazeru
            (*boardedNumber)++;

            //overeni zda se proces nestane kapitanem plavby
            if (*boardedNumber < 4){
                sem_post(boarding);
                sem_wait(voyage);
            }
            
            //pokud prave ted nastupuje posledni clen plavby, stava se kapitanem a ridi plavbu
            else if (*boardedNumber == 4){
                sem_wait(print);
                (*MoloSurfer)-=4;
                fprintf(f, "%d \t : %s %d \t : boards  \t\t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
                (*rowNumber)++;
                sem_post(print);
                if (vals->R > 0){
                    usleep(random() % vals->R * 1000);
                }
                //kapitan propousti ucastniky plavby
                sem_post(voyage);
                sem_post(voyage);
                sem_post(voyage);
                //kapitan ceka nez se vsichni vylodi
                sem_wait(capitan);
                (*onlyHackers) = 0;
                (*mixed) = 0;
            }
            
            break;
        }
        sem_post(boarding);
    }
    //vylodeni ucastniku
    sem_wait(print);
    if (x==0){
        (*hacker)--;
    }
    else if (x==1){
        (*surfer)--;
    }
    sem_post(print);
}

//funkce pro simulaci mola
int pier(int x, int i, values_t* vals, FILE* f){
    char* type = (x == 0 ? "HACK" : "SERF");
    
    if (vals->C > *MoloHacker+*MoloSurfer){

        if (x == 0){
            (*MoloHacker)++;
            fprintf(f, "%d \t : %s %d \t : waits  \t\t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
            (*rowNumber)++;
        }

        else if (x == 1){
            (*MoloSurfer)++;
            fprintf(f, "%d \t : %s %d \t : waits \t\t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
            (*rowNumber)++;
        }
        return 1;
    }
        
    else{
        return 0;
    }
    
}

//funkce pro generování (child) procesů
void makeProcces(int x, int i, values_t* vals, FILE* f){
    char* type = (x == 0 ? "HACK" : "SERF");
    srand(time(NULL));

    //vytvoreni haceru/surferu podle zadaneho casoveho limitu
    if (x == 0 && vals->H != 0){
        usleep(random() % vals->H * 1000);
    }

    else if (x == 1 && vals->S != 0){
        usleep(random() % vals->S * 1000);
    }

    sem_wait(print);
    fprintf(f, "%d \t : %s %d \t : starts\n",*rowNumber, type, i);
    (*rowNumber)++;
    sem_post(print);

    //proces se pokousi dostat na molo
    int onMolo = 0;
    while (onMolo != 1){
        sem_wait(print);
        onMolo = pier(x, i, vals, f);
        sem_post(print);
        if (onMolo == 1){
            sem_wait(molo);
            board(x, i, vals, f);
            sem_post(molo);
            break;
        }
    //pokud se proces nedostane na molo, ceka 
        else {
            sem_wait(print);
            fprintf(f, "%d \t : %s %d \t : leaves queue \t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
            (*rowNumber)++;
            sem_post(print);
            
            usleep(random() % vals->W * 1000);

            sem_wait(print);
            fprintf(f, "%d \t : %s %d \t : is back\n", *rowNumber, type, i);
            (*rowNumber)++;
            sem_post(print);
        }
    }


    sem_wait(print);
    //vylodeni ucastniku z lodi 
    (*boardedNumber)--;
    
    
    if (*boardedNumber >= 1){
        fprintf(f, "%d \t : %s %d \t : member exits \t : %d \t : %d\n",*rowNumber, type, i, *MoloHacker, *MoloSurfer);
        (*rowNumber)++;
        
        //predposledni ucastnik plavby propousti kapitana z lodi
        if (*boardedNumber == 1){
            sem_post(capitan);
        }
    }

    //kapitan opusti lod a umozni nastup cekajicim na molu 
    else if (*boardedNumber == 0){
        fprintf(f, "%d \t : %s %d \t : captain exits  \t : %d \t : %d\n", *rowNumber, type, i, *MoloHacker, *MoloSurfer);
        (*rowNumber)++;
        sem_post(boarding);
    }
    sem_post(print);
    
    exit(0);
}

//funkce pro generování Hackerů/Surferů
void genPerson(int x, values_t* vals, FILE* f){
        for (int i=0 ; i < vals->P ; i++){
            pid_t id = fork();
            if (id == 0){
                makeProcces(x, i+1, vals, f);
            }
        }
        for (int i = 0; i < vals->P; i++){
            wait(NULL);
        }
    exit(0);
}

//funkce pro overeni zda jsou hodnoty argumentu sparvne
int conditionals(values_t* vals){
    if (((vals->P % 2) > 0) || (vals->P < 2))
        return 1;
    else if (vals->H > 2000 || vals->H < 0)
        return 1;
    else if (vals->S > 2000 || vals->S < 0)
        return 1;
    else if (vals->R > 2000 || vals->R < 0)
        return 1;
    else if (vals->W > 2000 || vals->W < 20)
        return 1;
    else if (vals->C < 5)
        return 1; 
    else 
        return 0;
}

int main(int argc, char *argv[]){

    //otevreni/vytvoreni souboru proj2.out pro zapis
    FILE *f = fopen ("proj2.out", "w+");
    //overeni zda se soubor spravne otevrel
    if (f == NULL){
        fprintf(stderr, "Nepodarilo se otevrit soubor proj2.out pro zapis\n");
        return EXIT_FAILURE;
    }

    //pro spravny zapis do souboru
    setbuf(f, NULL);
    
    //overeni spravneho poctu argumentu
    if (argc != 7){
        fprintf(stderr, "SPATNY POCET ARGUMENTU\n./proj2 P H S R W C\nP = Pocet osob\nH = Doba po které je generován nový proces hackers\nS = Doba po které je generován nový proces surfers\nR = Doba plavby\nW = Doba cekani pokud je molo plne\nC = Kapacita mola\npriklad: ./proj2 2 2 2 200 200 5\n");
        return EXIT_FAILURE;
    }

    //vytvoreni testovaciho pole
    char* test = "";
    
    //nacteni a overeni spravnosti argumentu
    values_t* vals = malloc(sizeof(values_t));
    vals->P = strtol(argv[1], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    vals->H = strtol(argv[2], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    vals->S = strtol(argv[3], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    vals->R = strtol(argv[4], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    vals->W = strtol(argv[5], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    vals->C = strtol(argv[6], &test, 10);
    if (strlen(test) != 0){
        fprintf(stderr, "Do argumentu se nesmi zadavat pismena\n");
        return EXIT_FAILURE;
    }
    
    //overeni zda jsou hodnoty argumentu sparvne
    int err = conditionals(vals);
    if (err == 1){
        fprintf(stderr, "CHYBA V ZADANI HODNOT ARGUMENTU\nP >= 2 && (P %% 2) == 0\nH >= 0 && H <= 2000\nS >= 0 && S <= 2000\nR >= 0 && R <= 2000\nW >= 20 && W <= 2000\nC >= 5\n");
        return EXIT_FAILURE;
    }

    //vytvoreni semaforu + overeni zda se semafor vytvoril
    molo = sem_open("xgurec00_molo", O_CREAT | O_EXCL, 0666, vals->C);
    if (molo == SEM_FAILED){
        fprintf(stderr, "nepodarilo se vytvorit semafor mol\nsoubor xgurec00_molo nejspise jiz existuje\n");
        return EXIT_FAILURE;
    }

    print = sem_open("xgurec00_print", O_CREAT | O_EXCL, 0666, 1);
    if (print == SEM_FAILED){
        fprintf(stderr, "nepodarilo se vytvorit semafor print\nsoubor xgurec00_print nejspise jiz existuje\n");
        return EXIT_FAILURE;
    }

    voyage = sem_open("xgurec00_voyage", O_CREAT | O_EXCL, 0666, 0);
    if (voyage == SEM_FAILED){
        fprintf(stderr, "nepodarilo se vytvorit semafor print\nsoubor xgurec00_voyage nejspise jiz existuje\n");
        return EXIT_FAILURE;
    }

    capitan = sem_open("xgurec00_capitan", O_CREAT | O_EXCL, 0666, 0);
    if (capitan == SEM_FAILED){
        fprintf(stderr, "nepodarilo se vytvorit semafor print\nsoubor xgurec00_capitan nejspise jiz existuje\n");
        return EXIT_FAILURE;
    }

    boarding = sem_open("xgurec00_boarding", O_CREAT | O_EXCL, 0666, 1);
    if (boarding == SEM_FAILED){
        fprintf(stderr, "nepodarilo se vytvorit semafor print\nsoubor xgurec00_boarding nejspise jiz existuje\n");
        return EXIT_FAILURE;
    }

    //intiger ve sdilene pameti
    rowNumber =  mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    (*rowNumber)++;
    MoloHacker = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    MoloSurfer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    boardedNumber = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    boardedHacker = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    boardedSurfer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    surfer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    hacker = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    onlyHackers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    onlySurfers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    mixed = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);


    //fork
    pid_t a = fork();
    if (a == 0){
        genPerson(0, vals, f);
    }
    
    //fork
    pid_t b = fork();
    if (b == 0){
        genPerson(1, vals, f);
    }
    
    wait(NULL);
    wait(NULL);

    //ukonceni a smazani semaforu
    sem_close(molo);
    sem_unlink("xgurec00_molo");
    sem_close(print);
    sem_unlink("xgurec00_print");
    sem_close(voyage);
    sem_unlink("xgurec00_voyage");
    sem_close(capitan);
    sem_unlink("xgurec00_capitan");
    sem_close(boarding);
    sem_unlink("xgurec00_boarding");

    //ostraneni intigeru ve sdilene pameti
    munmap(MoloHacker, sizeof(int));
    munmap(MoloSurfer, sizeof(int));
    munmap(rowNumber, sizeof(int));
    munmap(boardedNumber, sizeof(int));
    munmap(boardedHacker, sizeof(int));
    munmap(boardedSurfer, sizeof(int));
    munmap(surfer, sizeof(int));
    munmap(hacker, sizeof(int));
    munmap(onlyHackers, sizeof(int));
    munmap(onlySurfers, sizeof(int));
    munmap(mixed, sizeof(int));

    //uzavreni a overeni zda se soubor spravne uzavrel
    if (fclose(f) == EOF)
    {
        fprintf(stderr, "Soubor proj2.out se nepodarilo uzavrit");
        return EXIT_FAILURE;
    }

    //uvolneni pameti datove struktury
    free(vals);
    
    return EXIT_SUCCESS;
}
