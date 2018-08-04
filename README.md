# LtKernel
Petit noyau à but éducatif

# Structure actuelle

Le bootloader charge le noyau en 0x1000.
Le noyau décrit les segments suivant :
 - Segments code et de donnees noyau sur la même plage, couvrent toute la RAM
 - Segments code et de donnees utilisateur sur la même plage à partir de 0x30000
 - Segment TSS (structure utilisee pour la commutation de tâche)
 
La pile noyau est en 0x20000.

L'IDT comprend 255 entrées :
 - Les 35 premières utilisées pour les exceptions du CPU
 - Les 16 suivantes pour les iterruptions matérielles
 - La 48ème pour les appels systèmes
 
Le PIC (Programmable Interrupt Controller) est configué de telle sorte que les interruptions matérielles de l'horloge système, du clavier et du port COM1 soient activées (IRQs 0, 1 et 4)

L'appel système implémenté permet d'afficher une chaîne depuis une tâche utilisateur. La commutation de tâche est effectuée en utilisant le fonctionnement de l'instruction iret.