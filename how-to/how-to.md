# CHIP8 Démarrage


## 1 Importer le code binaire (ROM)

Le code est entièrement chargé en mémoire d'un coup. 
La technique est de l'ouvrir en "rb". Le souci, c'est que la structure FILE * du C ne permet pas de connaitre la taille. 
Il faut donc :
* aller au bout du fichier, 
* récupérer le curseur à la fin (qui est est la taille en octet)
* revenir au début 
* réserver l'espace memoire 
* lire tout le fichier 
* fermer le flow FILE *

````C
FILE * f = fopen("./fichier.ch8", "rb");
fseek(f, 0, SEEK_END); // zero à partir de la fin
unsigned int size = ftell(f); // where is the cursor 
unsigned char * code = (unsigned char*) malloc(size * sizeof(unsigned char));
fread(f, code, size);
fclose(f);
````
Probablement le plus simple est de faire une fonction qui prend une string C comme nom de fichier et modifie des variables
globales "code" et "size" - probablement donner des noms plus explicites. 

## 2 Lire les instructions 

Les instructions sont des codes sur 2 (deux) octets. Il faut lire la ROM (code) par paire.
En hexadécimal un octet occupe 2 digit (on dit aussi nibble) donc 2 octet c'est 4 digits. 
Un octet 0xab avec a dans [0, F] et b dans [0, F] ces nombres vont jusqu'à 255. 
En C, on peut créer des octets (taille 8 bits) et des 2-octest (taille 16 bits)

La liste d'instruction est organisé en catégorie a partir du premier nibble:
les 3 autres nibbles servent au code lui même. 
Exemples : 
* 0x00E0 : clear the screen
* 0xA2A2 : met le pointeur de mémoire à l'adresse 2A2 
* 0x8XYN : fait une opération au registre X et Y le type d'opération est décidé par N

De manière générale il faut pouvoir découper les nibbles d'un opcode à 2 octets (donc 4 nibbles)
Pour cela il faut utiliser des masques avec l'opérateur & (AND bit à bit) et des décalage << ou >> à gauche ou a droite. 
Par exemple :
* 0x1234 & 0x0000 -> 0x0000 
* 0x1234 & 0xf000 -> 0x1000
* 0x1234 & 0x0f00 -> 0x0200 

masquer avec des "f" (tous les bits à 1) permet de garder tout le nibble. 

* 0x1234 & 0x00ff -> 0x034
* 0x1234 & 0x0f0f -> 0x0204

Le décalage permet de récupérer le nibble sous forme d'octet ou à l'inverse de passer un nibble dans un nombre :

Exemple:
* 0x1234 >> 1 -> 0x0123
* 0x1234 >> 2 -> 0x0012
* 0x1234 >> 3 -> 0x0001

En combinant les masques et les décalages on obtient le nibble qu'on veut:
Exemple:
* 0x1234 & 0x0f00 >> 2 -> 0x0002 -> 0x2 
* 0x1234 & 0xff00 >> 2 -> 0x0012 -> 0x12

Les insctructions sont sur deux octets et peuvent avoir le format :
* 0xIXYN -> on veut récupéer I, X et Y 
* 0xIXNN -> on veut récupérer I, X et NN (qui est sur 1 octet du coup)
* 0xINNN -> on veut récupérer I, NNN qui est sur 12 bits

Ce qui est récupéré va dépendre de I -> il faudra faire un switch sur ce I avec de décoder le reste. 

````C
uint_16 opcode; 
uint_8 nibble1 = (uint_8) (opcode & 0xF000) >> 3;
switch(nibble1) {
    case 0x0 : {
        // do expression 0x0...}
        break;
    case 0x1 : // etc...  
````
Il faut noter plusieurs choses :
opcode est un uint_16 (2 octets ) mais nibble1 est un unint_8 (unsigne char) donc 
il faut forcer la conversion via un 'cast' la commande (uint_8) devant l'opération. 
Il faut aussi faire gaffe aux parenthèses. 

La liste de toutes les instructions est la https://en.wikipedia.org/wiki/CHIP-8 
le mieux est de faire un switch et une fonction qui décrit ce qui est fait mais ne fait encore rien. 

## 3 Registres, lecture du flow et décodage
Les machines ont 16 registres appelées VX avec X dans [0x0, 0xF]. Il y a un pointeur 'PC' qui indique l'endroit du code 
en exécution. Si on a chargé le code sous forme d'octet l'instruction doit augmenter de 2 pour passer à l'opcode suivant. 
* code[0] et code[1] contiennent les deux octets de la première instruction. 

Pour passer de deux octets (uint_8) à 16 bits il faut forcer le type et décaler. Du coup pour lire l'instruction à l'adresse i :
```C
uint_16 instruction = ((uint_16)code[i] << 4) + (uint_16) code[i+1]
```
on peut faire un + ou un OU (|)

Du coup on peut faire un code simple qui :
* charge la rom
* prend un compteur PC à 0 
* lit toutes les instructions dans l'ordre 
* appelle une fonction par instruction qui décrit ce qu'elle fait
* commencer par lire le logo IBM pour voir ce que ça donne.



## 4 Pointeur de mémoire et d'instructinn

### Pointeur de mémoire
Il faut créer un pointeur de mémoire I qui pointe sur un bout de la mémoire. 
Le plus simple est de créer un bloc de 4096 octets et l'appeler memory. Ensuite on peut copier le code à l'adresse 0x200
qui sera la référence. L'instruction 0xANNN va justement copier l'adresse NNN dans I. 
Il s'agit souvent d'un data block - dans le cas de IBM c'est les données pour dessiner le logo. 
### Pointeur d'instruction 
Le pointeur d'instruction commence à 0x200 si le code est copié à cette adresse sinon 0x0. 
Il s'incrémente de 2 car on lit 2 octets : 
````C
uint_16 opcode = ((uint_16) code[PC] << 8) | (uint_16) code[PC+1]; 
PC += 2; // next instruction
````
### Pointeur de pile 
Le pointeur de pile SP (stack pointeur). On peut faire une pile soit même qui n'a pas pas besoin de résider dans la mémoire totale. 
Genre :
````C
uint_8 stack[256]; // stacke de 256 octets max
uint_8 SP = 0; // pointeur actuel 
````
on peut faire des fonctions pour ajouter un élément de la pile et enlever un élement de la pile 

````C
void push_to_stack(uint_8 X) {
    stack[sh] = (uint_8) X;
    SP +=1;
}
uint_8 pop_from_stack() {
    uint_8 X = (uint_8) stack[sh];
    SP += -1;
    return X;
}
````
En supposant que le pointeur SP soit une variable globale


## 5 Display : ecrire à l'écran 
Il y a un seul opcode pour le dessin : 0xDXYN 
Ici ça veut dire dessiner aux coordonnées VX et VY le N octets de  la memoire à I
donc :
````C
for (int i = 0; i < N) {
    uint_8 sprite = memory[I + i]};
    // plot the sprite sprite 
    // at posituion Y = VY + i 
````
Un sprite ici est  un octet qui s'écrit en binaire : 0bxxxxxxxx
donc avec 8 bits. Chaque bit va donner une coordonnée X à partir de 0 jusqu'à 8 exclut 


````C
for (int i = 0; i < N) {
    uint_8 sprite = memory[I + i]};
    // plot the sprite sprite 
    // at posituion Y = VY + i 
    uint_8 ay == vY + i;
    for (int j = 0; j< 8; j++) {
        // si le jieme bit de sprite est 1 flip le pixel
        uint_8 ax = VX + j 
        flip_screen(ax, ay);
        }
````

Il faut vérifier si le jième bit is on :
````C
bool is_bit_on(u_int8 x, u_int8 bit) {
        u_int8 flag = 1 << bit; 
        return (x & flag) == flag; // the bit is on 
    }
````

Pour commencer le plus simple est de dessiner dans un tableau de 64 par 32 et de l'imprimer à l'écran une fois le code fini. 

