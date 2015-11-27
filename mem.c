/*******************************************
******UFR IMAG CSE**************************
******M1 Groupe 1 2015-2016*****************
******LEFRERE COUTAUD **********************
******TP2 **********************************
********************************************/

#include <mem.h>
#include <unistd.h>
#include <errno.h>

#define FREE_BLOCK 1
#define ALLOCATED_BLOCK 0

#define TRUE (2==2)
#define FALSE (2==3)


struct fb{
	size_t size;
	struct fb* next;
};

struct ab{
	size_t size;
	struct ab* next;
};

struct fb* headFreeB;
struct ab* headAllocatedB;
char* memoryAdress;
mem_fit_function_t* strategy;

#include "memRoutines.h"
#include "memRoutines.c"

void mem_init(char* mem, size_t taille){
	memoryAdress = mem;
	headAllocatedB = NULL;
	headFreeB = (struct fb*)(memoryAdress + __BIGGEST_ALIGNMENT__ - sizeof(struct ab));
	headFreeB->size = taille-sizeof(struct ab);
	headFreeB->next = NULL;
	mem_fit(mem_fit_first);

	return;
}

void* mem_alloc(size_t size){
	struct fb** pointerToChosen;
	struct ab* pointerToNewAllocated;
	struct fb* copyOfTheHead;

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

	return (void*)pointerToNewAllocated + sizeof(struct ab);
}

void mem_free(void* adr){
    struct ab* blockToFree;
    struct ab* whichAd;
    struct ab* pointerToNewFree;

	blockToFree=(struct ab*) adr;
	whichAd=headAllocatedB;
	// Il n'y a aucun bloc alloué
	if (headAllocatedB == NULL){
		return;
	}
	pointerToNewFree=(struct ab*)( ((char*)blockToFree)-sizeof(struct ab) );
	// Le bloc a liberer est le premier de la liste
    if ( whichAd==pointerToNewFree ){
		headAllocatedB=headAllocatedB->next;
		insertFB((struct fb*) pointerToNewFree);
		return;
	}
	// Recherche du bloc dans la liste
    while (whichAd->next!=NULL && whichAd->next!=pointerToNewFree ){
		whichAd=whichAd->next;
   	}
	// Le bloc n'est pas dans la liste
    if (whichAd->next==NULL) {
		return;
	}
	// Le bloc est dans la liste
	else {
		whichAd->next = whichAd->next->next;
		insertFB((struct fb*) pointerToNewFree);
		return;
    }
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

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *b, size_t t, int free)){
	struct ab* ab = headAllocatedB;
	struct fb* fb = headFreeB;
	while (fb!=NULL){
	    print((char*)fb+sizeof(struct fb),mem_get_size((char*)fb+sizeof(struct fb)),FREE_BLOCK);
		fb=fb->next;
	}
	while (ab!=NULL){
	    print((char*)ab+sizeof(struct ab),mem_get_size((char*)ab+sizeof(struct ab)),ALLOCATED_BLOCK);
		ab=ab->next;
	}
}

void mem_fit(mem_fit_function_t* fun){
	strategy = fun;
}

/*
 *	Retourne l'adresse de la variable next dans le block précedant du block voulu
 */
struct fb** mem_fit_first(struct fb** fb, size_t t){
	struct fb** f = fb;
	int flag = FALSE;

	while (*f!=NULL && (*f)->size<t){
		f=&((*f)->next);
		flag = TRUE;
	}
	if (*f!=NULL && !flag ){
		return &headFreeB;
	}
	else {
		return f;
	}
}

struct fb** mem_fit_best(struct fb** fb, size_t t){
    struct fb** f = fb;
    struct fb** fBest = f;
	int isHead = FALSE;
	int counter = 0;
	int flag = FALSE;
	
    while(*f!=NULL){
		if( ((*f)->size>=t) && (((*f)->size<(*fBest)->size)||!flag)){
			if (counter == 0){
				isHead=TRUE;
			}
			else{
				isHead=FALSE;
			}
			fBest = f;
			flag=TRUE;
		}
		counter++;
 		f=&((*f)->next);
    }
	
	if (isHead){return &headFreeB;}
	if (flag){return fBest;}
	return f;
}
		
struct fb** mem_fit_worst(struct fb** fb, size_t t){
    return NULL;
}




