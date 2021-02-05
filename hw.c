#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#include<vld.h>
#include<malloc.h>

#define PN_SIZE 8
#define PM_SIZE 32
#define DISK_SIZE 64
#define PMP_SIZE 4

///*  list create  */
struct List {
	int val;
	struct List * next;
};
int cap = 0;
void erase_head(struct List** list)
{
	//struct List* res = list;
	if ((*list) == NULL) return;//can not occur
	struct List * p = *list;
	(*list) = p->next;
	//free(p);
	cap -= 1;
	return;
}
void erase(struct List** list, int value)
{
	struct List* res = (*list);
	if ((*list) == NULL) {//这种情况不会发生
		//printf("the list is null\n");
	}
	//printf("head-val:%d\n", (*list)->val);
	if ((*list)->val == value) {
		erase_head(list);
		return;
	}

	while ((*list)->next != NULL && (*list)->next->val != value)
	{
		(*list) = (*list)->next;
	}

	if ((*list)->next == NULL)//这种情况不会发生，既然在内存中，就一定是存在链表中的
	{
		//*list = res;
		printf("not found target\n");
		return ;
	}
	else
	{
		struct List* p = (*list)->next;
		(*list)->next = p->next;	
		cap = cap - 1;
		*list = res;
		//free(p);
		return ;
	}
	
}

void insert_tail(struct List** list, int value)
{
	struct List *res = (*list);
	
	struct List *tail = (struct List*)malloc(sizeof(struct List*));
	tail->val = value;
	tail->next = NULL;

	if (res == NULL) {
		(*list) = tail;
		return;
	}

	while (res->next != NULL)
	{
		res = res->next;
	}
	res->next = tail;
	cap += 1;
}

void myp(struct List* list)
{
	while (list != NULL)
	{
		//printf("%d<-", list->val);
		list = list->next;
	}
	//printf("\n");
}


/*  页表大小  */
struct Pagetable {
	int Valid_Bit[PN_SIZE];//0在磁盘，1在内存
	int Dirty_Bit[PN_SIZE];//0已经写入磁盘，1未写入，VB为0时无意义
	int Page_Number[PN_SIZE];//页号
};
struct Pagetable page_table;

void initial(struct Pagetable *page_table,int *main_memory,int* disk)
{
	for (int i = 0; i < PN_SIZE; ++i)
	{
		page_table->Valid_Bit[i] = 0;
		page_table->Dirty_Bit[i] = 0;
		page_table->Page_Number[i] = i;
	}

	for (int i = 0; i < PM_SIZE; ++i)
		main_memory[i] = -1;

	for (int i = 0; i < DISK_SIZE; ++i)
		disk[i] = -1;
}

/*  主存大小  */
int main_memory[PM_SIZE];

/*  磁盘大小  */
int disk[DISK_SIZE];

int partition(char* str,char* command)
{
	int len = strlen(str);

	if (str == "") return -1;
	int pos1 = -1;
	for (int i = 0; i < strlen(str); ++i)
	{
		if ((str[i] == ' '||str[i]=='\n')&&pos1 == -1) {//" "与‘ ’不一样
			pos1 = i;
			break;
		}
	}	
	if (pos1 == -1)//unused
	{
		strcpy(command, str);
		return 1;
	}
	else
	{
		for (int i = 0; i < pos1; ++i)
			command[i] = str[i];
		command[pos1] = '\0';

		char tmp[100] = "";
		int id = 0;
		for (int j = pos1 + 1; j < len; ++j)
		{
			tmp[id++] = str[j];
		}
		tmp[id] = '\0';
		strcpy(str, tmp);
	}
	return 1;
}

void page_replacement_alg(struct Pagetable* page_table,struct List** list,int page_number)
{
	if (cap == (PM_SIZE / PN_SIZE))
	{
		//printf("go on cap == (PM_SIZE / PN_SIZE)\n");
		int head_page = (*list)->val;
		int disk_page = 0;
		for (; disk_page < PN_SIZE; ++disk_page)
		{
			if (page_table->Valid_Bit[disk_page] == 1 && page_table->Page_Number[disk_page] == head_page)
				break;
		}
		//printf("disk_page:%d", disk_page);

		if (page_table->Dirty_Bit[head_page] == 1)//需要写磁盘,需要找到head_page对应的diskpage
		{
			//write to disk
			for (int i = 0; i < PN_SIZE; ++i)
			{
				disk[disk_page * 8 + i] = main_memory[head_page * 8 + i];
			}
			
		}
		//不需要写磁盘，需要将对应的位置0
		page_table->Valid_Bit[disk_page] = 0;
		page_table->Dirty_Bit[disk_page] = 0;
		page_table->Page_Number[disk_page] = disk_page;

		//写完磁盘和不需要写磁盘，均需要进行拷贝
		erase_head(list);
		insert_tail(list, head_page);
		for (int j = 0; j < PN_SIZE; ++j)
		{
			main_memory[head_page*PN_SIZE + j] = disk[page_number*PN_SIZE + j];
			disk[page_number*PN_SIZE + j] = -1;
		}
		page_table->Page_Number[page_number] = head_page;
		page_table->Valid_Bit[page_number] = 1;

	}
	else if (cap < (PM_SIZE / PN_SIZE))
	{
		//printf("go on cap < (PM_SIZE / PN_SIZE)\n");
		//if ((*list) == NULL) NULL;//printf("NULL\n");
		int next_page = 0;

		struct List *tmp = (*list);
		while (tmp != NULL && tmp->next != NULL)
			tmp = tmp->next;
		if (tmp)
		{
			//printf("the list is not empty\n");
			next_page = tmp->val + 1;			
		}
		//增加链表节点
		
		if ((*list) == NULL)
		{
			//printf("NULL\n");
			struct List* p= (struct List*)malloc(sizeof(struct List*));
			p->next=NULL;
			p->val = next_page;
			(*list) = p;
			cap += 1;
		}
		else {
			struct List* p = (struct List*)malloc(sizeof(struct List*));
			p->val = next_page;
			p->next = NULL;

			//struct List *res = (*list);
			//while (res->next != NULL) res = res->next;
			tmp->next = p;
			cap += 1;
			//free(tmp);
		}
		
		for (int j = 0; j < PN_SIZE; ++j)
		{
			main_memory[next_page*PN_SIZE + j] = disk[page_number*PN_SIZE + j];
			//printf("%d\n", disk[page_number*PN_SIZE + j]);
			disk[page_number*PN_SIZE + j] = -1;
		}
		page_table->Page_Number[page_number] = next_page;
		page_table->Valid_Bit[page_number] = 1;
	}
	
	
}

void read_virtual(int logical, struct List** list, unsigned int islru)
{
	// get logical page number
	int page_number = logical / 8;
	int offset = logical % 8;

	if (page_table.Valid_Bit[page_number] == 0)//在磁盘不能直接读
	{
		printf("A Page Fault Has occurred\n");
		page_replacement_alg(&page_table, list, page_number);
	}
	else//本来就在内存，读取，需要删除
	{
		if (islru) {
			int memory_page = page_table.Page_Number[page_number];
			//printf("in memory,memory_page=%d,cap=%d\n", memory_page, cap);
			erase(list, memory_page);
			//printf("after erase cap=%d", cap);
			insert_tail(list, memory_page);
			//printf("after insert cap=%d", cap);
		}
		else
		{
			//FIFO不需要操作
		}
	}

	//read data
	int physical_addr = (page_table.Page_Number[page_number]) * 8 + offset;
	printf("%d\n", main_memory[physical_addr]);

}

void write_virtual(int logical, int value,struct List** list,unsigned int islru)
{
	// get page number
	int page_number = logical / 8;
	int offset = logical % 8;

	if (page_table.Valid_Bit[page_number] == 0)//写时在磁盘
	{
		//printf("write in disk\n");
		printf("A Page Fault Has Occurred\n");
		//执行页面置换算法,对cap进行分类
		page_replacement_alg(&page_table, list, page_number);
		
	}
	else
	{
		if (islru) {
			//printf("write in memory\n");
			int memory_page = page_table.Page_Number[page_number];
			erase(list, memory_page);
			insert_tail(list, memory_page);
		}
		else
		{
			//写时已经在磁盘，不需要操作
		}
	}

	int physical_addr = (page_table.Page_Number[page_number]) * 8 + offset;
	main_memory[physical_addr] = value;
	page_table.Dirty_Bit[page_number] = 1;//修改位置1
}

void showmain(int page_num)
{
	if (page_num >= 4||page_num<0)  return;
	int start_addr = page_num * 8;
	for (int i = start_addr; i < start_addr + 8; ++i)
	{
		printf("%d:%d\n", i, main_memory[i]);
	}
}
void showdisk(int page_num)
{
	if (page_num >= 8||page_num<0) return;
	int start_addr = page_num * 8;
	for (int i = start_addr; i < start_addr + 8; ++i)
	{
		printf("%d:%d\n", i, disk[i]);
	}
}
void showptable()
{
	for (int i = 0; i < 8; ++i)
	{
		printf("%d:%d:%d:%d\n", i, page_table.Valid_Bit[i], page_table.Dirty_Bit[i], page_table.Page_Number[i]);
	}
}




int main(int argc, char** argv)
{
	//_CrtSetBreakAlloc(85);

	
	unsigned int islru=0;
	if(argc>1)
		if (strcmp(argv[1], "LRU") == 0) islru = 1;

	/*  list of page  */
	struct List * page_list = NULL;
	initial(&page_table,main_memory,disk);

	while (1)
	{
		char str[100];
		printf("> ");
		fgets(str,100,stdin);//遇到回车键会读取回车键

		char command[100];
		partition(str, command);

		//printf("%s,\n", command);
		if (!strcmp(command, "quit")) break;
		else if (!strcmp(command, "read"))
		{
			char num[100];
			partition(str, num);
			if (strcmp(num, "") == 0) {
				printf("please enter correct command!\n");
				continue;
			}
			int virtual_addr = atoi(num);
			//printf("\n%d", virtual_addr);
			read_virtual(virtual_addr, &page_list,islru);
		}
		else if (!strcmp(command ,"write"))
		{
			char num1[100];
			partition(str, num1);
			if (strcmp(num1, "") == 0) {
				printf("please enter correct command!\n");
				continue;
			}
			int virtual_addr = atoi(num1);
			char num2[100];
			partition(str, num2);
			if (strcmp(num2, "") == 0) {
				printf("please enter correct command!\n");
				continue;
			}
			int value = atoi(num2);
			//printf("\n%d,%d", virtual_addr,value);
			write_virtual(virtual_addr, value, &page_list,islru);
			
		}
		else if (!strcmp(command,"showmain")) {
			//printf("go on showmain\n");
			char num[100];
			partition(str, num);
			int page_num = atoi(num);
			showmain(page_num);
		}
		else if (!strcmp(command,"showdisk"))
		{
			char num[100];
			partition(str, num);
			int page_num = atoi(num);
			showdisk(page_num);
		}
		else if (!strcmp(command,"showptable")) {
			showptable();
		}
		else
		{
			continue;
		}
		//printf("cap=%d\n", cap);
		//myp(page_list);

		
	}
	struct List *p,*pp;
	pp = page_list;
	while (pp != NULL)
	{
		p = pp;
		pp = p->next;
		free(p);
		//printf("go on\n");
	}
	//_CrtDumpMemoryLeaks();
	return 0;
}