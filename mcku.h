#include <string.h>
#include <stdio.h>
#include <stdlib.h>
struct pcb
{
	char pid;
	FILE *fd;
	char *pgtable;
	/* Add more fields if needed */
};

struct pcb **plist;
char dummyFlag;
int psize;

extern struct pcb *current;
extern char *ptbr;

int freelist[64];
void ku_scheduler(char pid)
{

	/* Your code here */
	int cnt = 0;

	int flag = 0;
	for (int i = 0; i < psize; i++)
	{
		if (plist[i] != NULL){
			flag = 1;
			break;
		}
	}

	if(flag==0) {
		current = NULL;
		ptbr = NULL;
		return;
	}

	do
	{

		if (dummyFlag != -1 || plist[cnt] && plist[cnt]->pid == pid)
		{
			if (dummyFlag != -1)
				cnt = dummyFlag;

			cnt++;
			if (cnt == psize)
				cnt = 0;
			do
			{

				if (plist[cnt] != NULL)
				{
					break;
				}
				cnt++;
				if (cnt == psize)
					cnt = 0;
			} while (1);
			break;
		}
		cnt++;
	} while (1);

	current = plist[cnt];
	ptbr = current->pgtable;

	if(dummyFlag!=-1)
		dummyFlag = -1;
}

void ku_pgfault_handler(char va)
{

	/* Your code here */
	int cnt = 0;
	do
	{

		if (freelist[cnt] == 0)
		{
			int vpn = (unsigned char)va >> 4;
			current->pgtable[vpn] = (cnt << 2) + 1;
			freelist[cnt] = 1;

			break;
		}
		cnt++;

	} while (cnt!=64);
	
}

void ku_proc_exit(char pid)
{

	/* Your code here */
	int cnt = 0;
	do
	{

		if (plist[cnt] && plist[cnt]->pid == pid)
		{
			for (int i = 0; i < 16; i++)
			{
				if (plist[cnt]->pgtable[i])
				{
					int pfn = (unsigned char)plist[cnt]->pgtable[i] >> 2;
					freelist[pfn] = 0;
				}
			}
			free(plist[cnt]->pgtable);
			fclose(plist[cnt]->fd);
			free(plist[cnt]);
			plist[cnt] = NULL;

			dummyFlag = cnt;
			break;
		}
		cnt++;
	} while (1);
}

void ku_proc_init(int nprocs, char *flist)
{

	/* Your code here */
	psize = nprocs;

	for (int i = 0; i < 64; i++)
		freelist[i] = 0;

	FILE *fp = fopen(flist, "r");

	plist = malloc(nprocs * sizeof(struct pcb *));

	for (int i = 0; i < nprocs; i++)
	{

		plist[i] = malloc(sizeof(struct pcb));
		plist[i]->pid = i;

		char fname[256];
		fscanf(fp, "%s", fname);
		plist[i]->fd = fopen(fname, "r");
		plist[i]->pgtable = (char *)malloc(16 * sizeof(char));
		for (int j = 0; j < 16; j++)
			plist[i]->pgtable[j] = 0;
	}
	current = plist[0];
	ptbr = current->pgtable;

	dummyFlag = -1;

	fclose(fp);
}
