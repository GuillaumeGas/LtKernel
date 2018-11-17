[BITS 32]

global PicInit
	
;;; Initialisation et configuration du PIC (Programmable Interrupt Controller)

PicInit:	
	;; ICW1, port 0x20 (et 0xA0 pour esclave)
	;; 0 0 0 1 . 0 0 0 0
	;; Les 4 derniers bits sont pour :
	;;  - le déclenchement par niveau (1) ou front (0)
	;;  - /
	;;  - un seul controleur (1) ou cascade (0)
	;;     (?) J'ai essaye le mode simple, mais il ne semble pas supporte !
	;;  - avec ICW4 (1) ou sans (0)
	mov al, 0x11
	out 0x20, al
	jmp tmp1
tmp1:	
	
	;; outb (0xA0, 0x11);
	
	;; ICW2, port 0x21 (et 0xA1 pour esclave)
	;; x x x x x 0 0 0
	;; Les bits de poids fort servent à stocker l'adresse de base du vecteur d'interruption
	;; Il correspondent en fait à un offset dans l'idt
	;; Ici, on ne configure que le controleur maître, donc les IRQs 0-7, pour qu'il utilise
	;; les interruptions à partir de l'offset 0x20 dans l'idt.
	;; Sous archi type x86, les 32 premiers vecteurs sont réservés à la geENABLE_IRQon des exceptions.
	mov al, 0x20
	out 0x21, al
	jmp tmp2
tmp2:	
	;; outb (0xA1, 0x70);

	;; On utilise pas de controlleur esclave pour le moment, donc pas besoin de renseigner les
	;; registres ICW3 et ICW4
	
	;; finalement si
	;; outb (0x21, 0x04);
	;; outb (0xA1, 0x01);
	
	;; outb (0x21, 0x01);
	;; outb (0xA1, 0x01);
    
	;; OCW1, port 0x21 (et 0xA1 pour esclave)
	;; x x x x x x x x
	;; Chaque bit permet de masquer une interruption (1) ou non (0)
	;; 1 1 1 0 0 1 0 0 ;; on masque tout sauf l'horloge système (IRQ0), le clavier (IRQ1) et le port COM1 (IRQ4) et COM2
	;; mov al, 0xE4
	mov al, 0xE4
	out 0x21, al
	;; outb (0x21, 0xFC);
	;; outb (0xA1, 0xFF);

	ret

	
