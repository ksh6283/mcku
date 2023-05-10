#include <stdio.h>
#include <stdlib.h>
#include "mcku.h"

#define SCHED	0
#define PGFAULT	1
#define EXIT	2
#define TSLICE	5

struct pcb *current = 0;
char *ptbr = 0;
struct handlers{
       void (*sched)(char);
       void (*pgfault)(char);
       void (*exit)(char); 
}kuos;

void ku_reg_handler(int flag, void (*func)(char)){
	switch(flag){
		case SCHED:
			kuos.sched = func;
			break;
		case PGFAULT:
			kuos.pgfault = func;
			break;
		case EXIT:
			kuos.exit = func;
			break;
		default:
			exit(0);
	}
}

int ku_traverse(char va){
	int pt_index, pa;
	char *pte;

	pt_index = (va & 0xF0) >> 4;
	pte = ptbr + pt_index;

	if(!*pte)
		return -1;
	pa = ((*pte & 0xFC) << 2) + (va & 0x0F);

	return pa;
}


void ku_os_init(void){
	ku_reg_handler(SCHED, ku_scheduler);//round robin으로 구현 context switch 기능 수행 
	ku_reg_handler(PGFAULT, ku_pgfault_handler);
	ku_reg_handler(EXIT, ku_proc_exit);
}


void ku_run_cpu(void){
	unsigned char va;
    char sorf;
	int addr, pa, i;
	do{
		if(!current)
			exit(0);
			
		
		for( i=0 ; i<TSLICE ; i++){
			if(fscanf(current->fd, "%d", &addr) == EOF){
				
				kuos.exit(current->pid);
				break;
			}//va를 읽어온다
			va = addr & 0xFF;
			pa = ku_traverse(va);
			if(pa >= 0){
				sorf = 'S';
			}
			else{
				/* Generates a page fault */
				kuos.pgfault(va);
				pa = ku_traverse(va);

				if(pa < 0){
					/* No free page frames */
					kuos.exit(current->pid);
					break;
				}
				sorf = 'F';
			}

			printf("%d: %d -> %d (%c)\n", current->pid, va, pa, sorf);
		}

		kuos.sched(current->pid);
	}while(1);
}

int main(int argc, char *argv[]){
	/* System initialization */
	ku_os_init();

	/* Per-process initialization */
	// ku_proc_init(atoi(argv[1]), argv[2]);
	ku_proc_init(5, "proc.txt");
	/* Process execution */
	ku_run_cpu();

	return 0;
}