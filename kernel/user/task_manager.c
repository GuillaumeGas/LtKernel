#include <kernel/init/vmm.h>
#include <kernel/lib/memory.h>

#include "task_manager.h"

/*
	Permet de créer une tâche utilisateur :
	 - réserve une page pour le répertoire de pages de la tâche
	 -         une page pour la table de pages de la tâche
	 - on utilise la table de pages déjà existant du noyau afin que ce dernier ait accès
	   à toute la mémoire en cas d'interruption/appel système
	 - réserve une page pour y stocker le code de la tâche
	 - initialise les répertoire et table de pages
*/
struct page_directory_entry * create_task(u8 * task_addr, unsigned int size)
{
	struct page_directory_entry * pd = (struct page_directory_entry*)get_free_page();
	struct page_table_entry * pt = (struct page_table_entry*)get_free_page();
	unsigned int index = 0;

	// récupération d'une page pour y stocker le code à exécuter
	u8 * new_task_addr = (u8*)get_free_page();

	// copie de la tâche dans la zone réservée à la tâche utilisateur
	mmcopy(task_addr, new_task_addr, size);

	init_pages_directory(pd);
	set_page_directory_entry(pd, (u32)kernel_pt, IN_MEMORY | WRITEABLE);

	for (; index < NB_PAGES_PER_TABLE; index++)
		set_page_table_entry(&(pt[index]), 0, EMPTY);

	set_page_directory_entry(&pd[USER_TASK_V_ADDR >> 22], (u32)pt, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);
	set_page_table_entry(pt, (u32)new_task_addr, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);

	return pd;
}