#include <kernel/init/vmm.h>

#include "task_manager.h"

struct page_directory_entry * create_task()
{
	struct page_directory_entry * pd = (struct page_directory_entry*)get_free_page();
	struct page_table_entry * pt = (struct page_table_entry*)get_free_page();
	unsigned int index = 0;

	/*
		init répertoire de pages
		 pd0 init pour le noyau pour qu'il ait accès à tout, égal au pd0 actuel
		 pd[USER_TASK_V_ADDR >> 12] pour mapper 0x400000 à 0x100000, privilege de user

		 dans le task_switcher : maj cr3 pour changer de pd
		 on doit surement garder en mémoire le pd actuel non ? quoi que là on s'en fou I guess
		 on va jamais revenir dans le context précédent
	*/

	init_pages_directory(pd);
	set_page_directory_entry(pd, (u32)kernel_pt, IN_MEMORY | WRITEABLE);

	for (; index < NB_PAGES_PER_TABLE; index++)
		set_page_table_entry(&(pt[index]), 0, EMPTY);

	set_page_directory_entry(&pd[USER_TASK_V_ADDR >> 22], (u32)pt, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);
	set_page_table_entry(pt, USER_TASK_P_ADDR, IN_MEMORY | WRITEABLE | NON_PRIVILEGED_ACCESS);

	return pd;
}