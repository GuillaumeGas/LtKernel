# LtKernel
Petit noyau à but éducatif

# Organisation de la mémoire

**Le bootloader** charge le noyau en **0x100000**.

**La mémoire physique** est organisée de cette manière :

 | Debut | Fin | Utilisation |
 | --- | --- | --- |
 | 0x0 | 0x1000 | GDT/IDT |
 | 0x1000 | 0x2000 | Répertoire de pages du noyau |
 | 0x2000 | 0xA0000 | Pile noyau |
 | 0xA0000 | 0x100000 | Hardware |
 | 0x100000 | 0x400000 | Code Noyau |
 | 0x400000 | 0x800000 | Tables de pages du noyau |
 
**La mémoire virtuelle** est organisée de cette manière :

 | Debut | Fin | Utilisation |
 | --- | --- | --- |
 | 0x0 | 0x800000 | "Identity mapping" pour le noyau *(v_addr == p_addr)* |
 | 0x800000 | 0x1000000 | "Heap" de pages |
 | 0x1000000 | 0x10000000 | *Libre* |
 | 0x10000000 | 0x40000000 | "Heap" noyau |
 | 0x40000000 | 0xE0000000 | Espace libre utilisateur (code, données...) |
 | 0xE0000000 | ... | Pile Utilisateur |
 
En résumer, **1Go** pour le noyau et **3Go** pour le monde utilisateur.
 
# Autres détails techniques
 
Concernant la **pagination**, pages de **4ko**, utilisation d'un **bitmap** pour savoir quelles pages sont libres/utilisées.

Le noyau décrit les **segments** suivant :
 - Segments code et de donnees noyau sur la même plage, couvrent toute la RAM
 - Segments code et de donnees utilisateur sur la même plage
 - Segment TSS (structure utilisee pour la commutation de tâche)

**L'IDT** comprend 255 entrées :
 - Les 35 premières utilisées pour les **exceptions du CPU**
 - Les 16 suivantes pour les **interruptions matérielles**
 - La 48ème pour les **appels systèmes** *(préemptibles)*
 
**Le PIC (Programmable Interrupt Controller)** est configué de telle sorte que les interruptions matérielles de l'**horloge système**, du **clavier** et du port **COM1** soient activées (IRQs 0, 1 et 4)

**Le scheduler** est appelé à chaque interruption de l'horloge du cpu :
 - si aucune tâche ne tourne mais qu'une tâche a bien été chargée, on la lance
 - si une tâche est en cours d'exécution, on sauvegarde son contexte et on permute sur la tâche suivante
 - sinon on ne fait rien

**Les appels système** :
 - [0] : print
 - [1] : scan
 - [2] : exec (utilisé pour tester le noyau..., exécute une tâche utilisateur)
 - [3] : wait (pid)
 - [4] : exit
 - [10] : listProcess
 
 **Système de fichier et exécutable ELF** :
 Support de l'ext2, chargement de fichier exécutable au format ELF + exécution.
 
Un **logger** permet de rediriger l'affichage de texte soit vers **l'écran** soit vers le **port série** (**COM 1**)
