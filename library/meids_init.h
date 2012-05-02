#ifndef __KERNEL__
# ifndef _MEIDS_INIT_H_
#  define _MEIDS_INIT_H_

typedef struct addr_list
{
	char* addr;
	int flag;
	struct addr_list* next;
}addr_list_t;

int  CreateInit(char* addr, addr_list_t** conf);
void DestroyInit(addr_list_t** conf);
int  CleanInit(addr_list_t** conf);
int  SearchInit(char* addr, addr_list_t* conf);

# endif	//_MEIDS_INIT_H_
#endif	//__KERNEL__
