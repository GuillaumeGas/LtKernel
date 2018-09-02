# LtKernel
Petit noyau à but éducatif

# Structure actuelle

**Le bootloader** charge le noyau en **0x1000**.

**La mémoire** (**paginée**) est organisée de cette manière :
 - Les premières pages sont réservées au noyau, la mémoire virtuelle **map le noyau** telle que l'addr Vir 0 == l'addr Phy 0, etc.. (**de 0x0 à 0x20000**)
 - Les pages à partir de **0xA0000** jusqu'à **0x100000** sont réservées pour le **hardware**
 - Un **bitmap** permet de déterminer quelles pages sont libres 

Le noyau décrit les **segments** suivant :
 - Segments code et de donnees noyau sur la même plage, couvrent toute la RAM
 - Segments code et de donnees utilisateur sur la même plage
 - Segment TSS (structure utilisee pour la commutation de tâche)
 
La **pile noyau** est en **0x20000**.

**L'IDT** comprend 255 entrées :
 - Les 35 premières utilisées pour les **exceptions du CPU**
 - Les 16 suivantes pour les **interruptions matérielles**
 - La 48ème pour les **appels systèmes** *(préemptibles)*
 
**Le PIC (Programmable Interrupt Controller)** est configué de telle sorte que les interruptions matérielles de l'**horloge système**, du **clavier** et du port **COM1** soient activées (IRQs 0, 1 et 4)

**Le scheduler** est appelé à chaque interruption de l'horloge du cpu :
 - si aucune tâche ne tourne mais qu'une tâche a bien été chargée, on la lance
 - si une tâche est en cours d'exécution, on sauvegarde son contexte et on permute sur la tâche suivante
 - sinon on ne fait rien

**L'appel système** implémenté permet d'afficher une chaîne depuis une tâche utilisateur. La commutation de tâche est effectuée en utilisant le fonctionnement de l'instruction iret.

Un **logger** permet de rediriger l'affichage de texte soit vers **l'écran** soit vers le **port série** (**COM 1**)
