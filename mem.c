/*******************************************
 ******UFR IMAG CSE**************************
 ******M1 Groupe 1 2015-2016*****************
 ******LEFRERE COUTAUD **********************
 ******TP2 **********************************
 ********************************************/




#include <mem.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "memStruct.h"
#include "memRoutines.h"
#include "memRoutines.c"
#include "memory_allocation_strategies.h"
#include "memory_allocation_strategies.c"

#define TRUE (2==2)
#define FALSE (2==3)



struct fb* headFreeB;
struct ab* headAllocatedB;
char* memoryAdress;

/*
 * Retourne l'adresse de la variable next dans le block précedant du block alloué
 */
mem_fit_function_t* strategy;

int memInitialSize;
int instant=0;
int dataFile=0;
int assertNoLeak=FALSE;
int traceMemory=FALSE;

char* envVarStrategy="MEM_ALLOC_STRATEGY";
char* availableStrategies[]={"first_fit","best_fit","worst_fit", NULL};

char* envVarTrace="MEM_TRACE";
char* envVarDebug="MEM_DEBUG";
char* available[]={"true"};

pthread_mutex_t lock;


void mem_init(char* mem, size_t taille){

  memoryAdress = mem;
  headAllocatedB = NULL;
  headFreeB = (struct fb*)(memoryAdress + __BIGGEST_ALIGNMENT__ - sizeof(struct ab));
  headFreeB->size = taille-sizeof(struct ab);
  headFreeB->next = NULL;

  if( readEnvArg(envVarDebug,available) == 0 ){
    assertNoLeak=TRUE;
  }

  if( readEnvArg(envVarTrace,available) == 0 ){
    traceMemory=TRUE;
  }



  switch ( readEnvArg(envVarStrategy,availableStrategies) ){
  case 0 :
    mem_fit(mem_fit_first);
    break;
  case 1:
    mem_fit(mem_fit_best);
    break;
  case 2:
    mem_fit(mem_fit_worst);
    break;
  default:
    mem_fit(mem_fit_first);
    break;
  }

  atexit(closeDataFile);
  memInitialSize=taille;
  dataFile=open("./evalAllocBestFit.data", O_WRONLY|O_CREAT|O_TRUNC, 0666);
  write(dataFile,"#Evaluation allocateur memoire\n", sizeof("#Evaluation allocateur memoire\n"));

  if(traceMemory)memorySnapshot();
  if(assertNoLeak)assert(sumMyMemory()==memInitialSize);
 

	
  return;
}

void* mem_alloc(size_t size){
  struct fb** pointerToChosen;
  struct ab* pointerToNewAllocated;
  struct fb* copyOfTheHead;


  pthread_mutex_lock(&lock);
  copyOfTheHead = headFreeB;
  pointerToChosen = (struct fb **)strategy(&copyOfTheHead,size); // On a l'adresse du pointeur qui indique la case a allouer
  if (*pointerToChosen == NULL ){ //Pas de place en mem
    errno=EAGAIN;
    return NULL;
  }
  pointerToNewAllocated = (struct ab*)*pointerToChosen; // On sauve l'adresse de l'espace memoire qu'on alloue		
  if (shouldSplitFreeBlock(*pointerToChosen,size)){ //on a la place pour split.
    splitFreeBlock(pointerToChosen,size);
  }
  extractFB(pointerToChosen);
  //on alloue le nouveau "allocatedblock"
  pointerToNewAllocated->next = headAllocatedB;
  //pointerToNewAllocated->size = (*pointerToChosen)->size; //Conserver la même valeur au meme emplacement memoire;
  headAllocatedB = pointerToNewAllocated;
  if(traceMemory)memorySnapshot();
 if(assertNoLeak)assert(sumMyMemory()==memInitialSize);
 	
  pthread_mutex_unlock(&lock);
 return (void*)pointerToNewAllocated + sizeof(struct ab);
}

void mem_free(void* adr){
  struct ab* blockToFree;
  struct ab* whichAd;
  struct ab* pointerToNewFree;

 pthread_mutex_lock(&lock);

  blockToFree=(struct ab*) adr;
  whichAd=headAllocatedB;
  // Il n'y a aucun bloc alloué
  if (headAllocatedB == NULL){
    //return;
  }else{
    pointerToNewFree=(struct ab*)( ((char*)blockToFree)-sizeof(struct ab) );
    // Le bloc a liberer est le premier de la liste
    if ( whichAd==pointerToNewFree ){
      headAllocatedB=headAllocatedB->next;
      insertFB((struct fb*) pointerToNewFree);
      //return;
    }else{
      // Recherche du bloc dans la liste
      while (whichAd->next!=NULL && whichAd->next!=pointerToNewFree ){
	whichAd=whichAd->next;
      }
      // Le bloc n'est pas dans la liste
      if (whichAd->next==NULL) {
	//return;
      }

      else {    // Le bloc est dans la liste
	whichAd->next = whichAd->next->next;
	insertFB((struct fb*) pointerToNewFree);
	//return;
      }
    }
  }
  memorySnapshot();
 pthread_mutex_unlock(&lock);
  return;
}

/*
 *	Si adr désigne un bloc mémoire, retourne sa taille.
 *	0 sinon
 */
size_t mem_get_size(void* adr){
  struct ab* ab = headAllocatedB;
  struct fb* fb = headFreeB;
	
  while(ab!=NULL && ((char*)ab+sizeof(struct ab))!=adr){
    ab=ab->next;
  }	
  if (ab!=NULL){
    struct ab* ab=(struct ab*)((char*)adr - sizeof(struct ab));
    return ab->size;
  }
	
  while(fb!=NULL && ((char*)fb+sizeof(struct fb))!=adr){
    fb=fb->next;
  }	
  if (fb!=NULL){
    struct fb* fb=(struct fb*)((char*)adr - sizeof(struct fb));
    return fb->size;
  }
  return 0;
}


void mem_fit(mem_fit_function_t* fun){
  strategy = fun;
}



