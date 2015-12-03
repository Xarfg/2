#include <math.h>
#include <string.h>
#include "memStruct.h"

#define FREE_BLOCK 1
#define ALLOCATED_BLOCK 0

extern struct fb* headFreeB;
extern struct ab* headAllocatedB;
extern char* memoryAdress;
extern mem_fit_function_t* strategy;
extern int instant;
extern int dataFile;
extern char** environ;
extern char* varEnvStrategy;
extern char* availableStrategies[];

int shouldSplitFreeBlock(struct fb* fb, int askedSize){
	int paddingForAlignement;
	int totalSizeOfNewBlock = askedSize ;
	
	paddingForAlignement = (__BIGGEST_ALIGNMENT__ - ((totalSizeOfNewBlock + sizeof(struct fb)) % __BIGGEST_ALIGNMENT__)) % __BIGGEST_ALIGNMENT__ ;
	totalSizeOfNewBlock += paddingForAlignement;
	if ( fb->size > totalSizeOfNewBlock +1  ) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


void addPadding( struct fb** fb, int askedSize){
	int paddingForAlignement;
	int totalSizeOfNewBlock;
		
	totalSizeOfNewBlock = askedSize + sizeof(struct ab);		
	paddingForAlignement = (__BIGGEST_ALIGNMENT__ - (totalSizeOfNewBlock % __BIGGEST_ALIGNMENT__)) % __BIGGEST_ALIGNMENT__ ;
	(*fb)->size = askedSize + paddingForAlignement;
	
	return;
}

void splitFreeBlock( struct fb** fb, int askedSize){
	int paddingForAlignement;
	int totalSizeOfNewBlock;
	struct fb* pointerToLeftPart = *fb;
	struct fb* pointerToRightPart;

	totalSizeOfNewBlock = askedSize + sizeof(struct ab);		
	paddingForAlignement = (__BIGGEST_ALIGNMENT__ - (totalSizeOfNewBlock % __BIGGEST_ALIGNMENT__)) % __BIGGEST_ALIGNMENT__ ;
	totalSizeOfNewBlock += paddingForAlignement;
	
	pointerToRightPart =(struct fb*) ((char*)pointerToLeftPart + totalSizeOfNewBlock); //adresse du nouveau block libre a inserer 
	pointerToRightPart->size = pointerToLeftPart->size + sizeof(struct fb) - (totalSizeOfNewBlock + sizeof(struct fb));
	pointerToRightPart->next = pointerToLeftPart->next ;
	
	pointerToLeftPart->size = askedSize + paddingForAlignement;
	pointerToLeftPart->next = pointerToRightPart;
	
	return;
} 

void extractFB( struct fb** fb){

	if ((*fb)->next!=NULL){
		(*fb)= (*fb)->next;
		}
	else{
		(*fb) =NULL;
	}

	return;
}

void insertFB(struct fb* toInsert){
	struct fb** pointerFromPreviousBlock = &headFreeB;
	struct fb* nextBlock = *pointerFromPreviousBlock;
	struct fb* previousBlock = headFreeB ;
	int flag = FALSE;

	while ( ((long long unsigned)toInsert > (long long unsigned)nextBlock) && (nextBlock!=NULL) ){
		if (flag){ previousBlock = previousBlock->next;}	
		pointerFromPreviousBlock = &(nextBlock->next);
		nextBlock = nextBlock->next;
		flag = TRUE;
		}
	
	toInsert->next = nextBlock;
	*pointerFromPreviousBlock = toInsert;
	if ((char*)toInsert + sizeof (struct fb) + toInsert->size ==  (char*)nextBlock){ //Fusion aval
		fusionFB(toInsert,nextBlock);
		}
	
	if (!flag)return;
		if ((char*)previousBlock + sizeof (struct fb) + previousBlock->size ==  (char*)toInsert){ //Fusion amont
			fusionFB(previousBlock,toInsert);
			}
	return;
}
	
void fusionFB(struct fb* leftBlock, struct fb* rightBlock){
	leftBlock->size = leftBlock->size + rightBlock->size + sizeof (struct fb);
	extractFB(&(leftBlock->next));
	return;	
}


int sumMyMemory(){
	int sum = 0;
	struct fb* f = headFreeB;
	struct ab* a = headAllocatedB;
	
	while (f!=NULL){
		sum += f->size;
		sum += sizeof(struct fb);
		f = f->next;
		}
	
	while (a!=NULL){
		sum += a->size;
		sum += sizeof(struct ab);
		a = a->next;
		}

	return sum;
}




#include <stdio.h>
void writeInt(int fd, long long unsigned val){
  int nbDigits(long long unsigned int x){
    if ( x == 0){
      return 1;
    }else{
      return (unsigned int)log10(val)+1;
    }
  }	
  int n = nbDigits(val);
  char string[n];
  unsigned int i = n;
  

  string[0]='0';
  while ( i > 0 ) {
    i--;
    string[i] = '0' + (val % 10);
    val = val / 10;
  }

  write(fd,&string,n);
  return;
}

extern int memInitialSize;


void memorySnapshot(){
  
  int userMemoryUsage = 0;
  int allocMemoryUsage = 0;
  long long unsigned lastUsedByte = (long long unsigned)memoryAdress;
  long long unsigned firstUsedByte = (long long unsigned)memoryAdress;
  long long unsigned firstBlockByte = (long long unsigned)memoryAdress;
  long long unsigned lastBlockByte = (long long unsigned)memoryAdress;
    
  struct ab* a = headAllocatedB;
  int fd = dataFile;

  while (a!=NULL){
    userMemoryUsage += a->size;
    firstBlockByte = (long long unsigned)a; 
    lastBlockByte = firstBlockByte + a->size + sizeof(struct ab);
    
    if ( firstBlockByte <= firstUsedByte ){
      firstUsedByte = firstUsedByte;
    }
    if ( lastUsedByte <= lastBlockByte ){
      lastUsedByte = lastBlockByte;
    }
    allocMemoryUsage = lastUsedByte - firstUsedByte;
    
    a = a->next;
  }

  writeInt(fd, instant);
  write(fd," ",1);
  writeInt(fd, userMemoryUsage);
  write(fd," ",1);
  writeInt(fd, allocMemoryUsage);
  write(fd,"\n",1);

  instant++;
  return ;

}


//lecture de "arguments" passés en variables d'environnement
// retourne l'indice dans le tableau available strategie de l'option
// -1 si aucune stratégie correspondante.

int readEnvArg(char* opt, char* values[]){
  char ** varEnviron;
  char* strat;
  int i, j;

  varEnviron = environ;
  i=0;
  while ( varEnviron[i]!=NULL ){
    if( 0==strncmp(varEnviron[i],opt,strlen(opt)) ){
      strat=varEnviron[i]+1+strlen(opt);
      j=0;
      while ( availableStrategies[j]!=NULL ){
	if( strcmp(values[j],strat) == 0 ){
	  return j;
	}
	j++;
      }
    }
    i++;
  }
  return -1;

}



void closeDataFile(){
  close(dataFile);
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
