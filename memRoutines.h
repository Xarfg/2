/*
 * Vrai si la taille du bloc choisi est suffisante pour la couper en deux bloc de mémoire libre
 */
int shouldSplitFreeBlock(struct fb* fb, int askedSize);

/*
 *	Precondition: Le bloc à couper doit être de taille suffisante
 *	Arguments : 
 *		fb: pointeur vers le bloc à couper 
 *		askedSize: taille du bloc demandé par l'utilisateur
 *	Effets de bord : 
 *		La seconde partie du bloc est une nouvelle zone libre insérée dans la liste des freeblocs
*/
void splitFreeBlock( struct fb** fb, int askedSize);

/*
 *	Précondition: fb pointe sur un champ next désignant le bloc a extraire
 *	Postcondition: Le bloc désigné n'est plus dans la liste des blocs libres
 */
void extractFB( struct fb** fb);

/*
 *	Précondition: ab pointe sur un champ next désignant le bloc a extraire
 *	Postcondition: Le bloc désigné n'est plus dans la liste des blocs alloués
 */
void extractAB( struct ab** fb);

/*
 *	Précondition : Le bloc de gauche et le bloc de droite sont justaposés et libres.
 *	Postcondition : Les 2 blocs ont étaient fusionnées en un un unique bloc libre;
 */
void fusionFB(struct fb* leftBlock, struct fb* rightBlock);

/*
 *	Précondition : toInsert pointe un bloc libre
 *	Postcondition : Le bloc a été inséré dans la liste. Maintient la liste ordonnée par ordre croissant des adresses. Les blocs libres justaposés sont fusionnés.
 */
void insertFB(struct fb* toInsert);

/*
 * Augmente la taille du bloc pour l'aligner
 */
void addPadding( struct fb** fb, int askedSize);
