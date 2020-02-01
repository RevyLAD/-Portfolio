#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512 //����Ʈ
#define SECTOR_SIZE 32//�� ��ϴ� ������ ����

typedef struct Memory {
	char memory[SIZE]; //1������ ũ�� 512byte
	int spare;
}MEMORY;

typedef struct Table {
	int lbn;
	int pbn;
}TABLE;

int spare_count = 0;
int write_Allcount = 0;
int erase_Allcount = 0;


void menu();
void menu2();
int init_megabyte();
MEMORY *init_memory(int block);
TABLE *init_table(int block);

void print_memory(MEMORY *memory, int block);
void print_table(TABLE *table, int block);
void Flash_erase(MEMORY *memory, TABLE *table, int sector);
void Flash_erase_spare(MEMORY *memory, TABLE *table, int sector);
void Flash_write(MEMORY *memory, TABLE *table, int sector, int pbn, char data[]);
void Flash_read(MEMORY *memory, TABLE *table, int sector);
void Flash_backRead(MEMORY *memory, TABLE *table, int sector, int pbn, int offset);
void FTL_write(MEMORY *memory, TABLE *table, int block);
void FTL_read(MEMORY *memory, TABLE *table, int block);
void write_text(MEMORY *memory, TABLE *table, int sector);
int LogBlock_Check(MEMORY *memory, TABLE *table, int hide, int pbn);
int freeBlock_Check(MEMORY *memory, TABLE *table, int sector);
void Original_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free);
void Log_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free);
void Log_to_Free2(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free);
int Log_pbn_check(MEMORY *memory, TABLE *table, int sector, int offset, int hide);
void FTL_write_FILE(MEMORY *memory, TABLE *table, int block, int lsn, char data[]);
void MERGE_to_Free(MEMORY *memory, TABLE *table, int pbn, int hide, int free);

void main()
{
	int choice, block, n = 0;
	MEMORY *memory = NULL;
	TABLE *table = NULL;

	block = init_megabyte();
	memory = init_memory(block);
	table = init_table(block);

	while (1)
	{
		menu();
		printf("Number:");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			FTL_write(memory, table, block);
			break;
		case 2:
			FTL_read(memory, table, block);
			break;
		case 3:
			menu2();
			printf("Number: ");
			scanf("%d", &n);

			if (n == 1)
			{
				print_memory(memory, block);
				break;
			}
			else if (n == 2)
			{
				print_table(table, block);
				break;
			}
			else if (n == 3)
			{
				printf("Wirte_All_Count : %d\n", write_Allcount);
				printf("Erase_All_Count : %d\n", erase_Allcount);
				printf("\n");
				break;
			}
			else
				break;
		case 4:
			write_text(memory, table, block);
			break;

		case 0:
			exit(1);
		default:
			break;
		}
	}
}

void write_text(MEMORY *memory, TABLE *table, int sector)
{
	char readWrite[10];
	int startSector;


	FILE *fp;
	if ((fp = fopen("kodak.txt", "r")) == NULL)
	{
		fprintf(stderr, "�ش��ϴ� ������ �� �� �����ϴ�.");
		exit(1);
	}
	do
	{
		fscanf(fp, "%s %d", readWrite, &startSector);
		printf("%s %d\n", readWrite, startSector);

		if (strcmp(readWrite, "w") == 0 && startSector == 0)
		{
			FTL_write_FILE(memory, table, sector, startSector, "AA");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 1)
		{
			FTL_write_FILE(memory, table, sector, startSector, "BB");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 17)
		{
			FTL_write_FILE(memory, table, sector, startSector, "CC");
		}
		else
		{
			FTL_write_FILE(memory, table, sector, startSector, "DD");
		}

	} while (!feof(fp));

}
void menu()
{
	printf("\n===================================\n");
	printf("1.Write 2.Read 3.Print 4.FILE 0.Exit\n");
	printf("===================================\n");
}

void menu2()
{
	printf("\n========================================\n");
	printf("1.FlashMemory  2. MappingTable  3. Count  \n");
	printf("=========================================\n");
}

int init_megabyte()
{
	int n, i = 0;
	int sector, block = 0;

	printf("Input Memory Size : (megabyte):");
	scanf("%d", &n);

	if (n <= 0) //ũ�Ⱑ 0���� ������ ����
	{
		printf("ERROR\n");
		exit(1);
	}

	sector = n * 1024 * 1024 / SIZE; //������ ����
	block = sector / SECTOR_SIZE; //����� ���� (1����� 32����)

	return block;
}

MEMORY *init_memory(int block) //�޸� ����
{
	int i;
	MEMORY *memory = NULL;
	int sector = block * SECTOR_SIZE;

	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector + SECTOR_SIZE));


	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		strcpy(memory[i].memory, "-1"); //������ -1�� �ʱ�ȭ
	}

	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		memory[i].spare = -1; //����� ����  -1�� �ʱ�ȭ
	}

	printf("\n");
	printf("Sector Size :%d \n", sector);
	printf("Block Size :%d \n", block);

	return memory;
}

TABLE *init_table(int block) //���� ���̺� ����
{
	int i, j = 0;
	int num = 0;
	int blk = block + 1;
	TABLE *table = NULL;


	table = (TABLE*)malloc(sizeof(TABLE)*(block + 1)); //��ϼ���ŭ ����

	for (i = 0; i < block + 1; i++)
	{
		table[i].lbn = i; //���ּҴ� 0����~ ��ϼ�����
	}

	for (j = 0; j <block + 1; j++)
	{
		table[j].pbn = j;
	}

	printf("Mapping Table Size :%d\n", block);
	return table;

}

void print_memory(MEMORY *memory, int block) //�޸𸮿� ����� ������ ���
{
	int i;
	int sector = block * SECTOR_SIZE;
	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		if (i == sector)//������ �Ѻ���� ���̵� ����
		{
			printf("==================== Hide Memory ====================\n");
			printf("psn%d = %s \t spare=%d\n", i, memory[i].memory, memory[i].spare);
			continue;
		}
		printf("psn%d = %s \t spare=%d\n", i, memory[i].memory, memory[i].spare);
	}
	printf("==================== Hide Memory ====================\n");
	printf("\n");
}

void print_table(TABLE *table, int block) //�������̺� ������ ���
{
	int i;

	for (i = 0; i < block + 1; i++)
	{
		if (i == block)
		{
			printf("==================== Hide Table ====================\n");
			printf("lbn%d = %d \t pbn%d = %d\n", i, table[i].lbn, i, table[i].pbn);
			printf("==================== Hide Table ====================\n");
			continue;
		}
		printf("lbn%d = %d \t pbn%d = %d\n", i, table[i].lbn, i, table[i].pbn);
	}
	printf("\n");
}

void FTL_write(MEMORY *memory, TABLE *table, int block) //FTL ����
{
	int write_count = 0;
int erase_count = 0;
int invalid = -5; //��ȿ������
int i, j, k, temp1, temp2 = 0;
int map_lbn = 0;
int offset = 0;
int sector = block * SECTOR_SIZE;
int hide = sector;
int log_check = 0;
int free_block_pbn = 0;
int log_block_pbn = 0;
int temp;
int lsn;
int lcount = 0;
char data[SIZE];


printf("Input lsn,data :");
scanf("%d %s", &lsn, data);


if (lsn > sector)
{
	printf("ERROR!\n");
	return 0;
}

if (strlen(data) >= SIZE) //�Է��� ������ũ�� ����ó��
{
	printf("data size ERROR!!\n");
	return 0;
}

map_lbn = lsn / SECTOR_SIZE; //�Է¹������� ��ϴ� ���� ������ ���������� �������̺��� ������� ����
offset = lsn % SECTOR_SIZE; //�������� ��ϴ� ���� ������ ���� ������ (���������� ������ ��ȣ�� ã��)

for (i = 0; i < block; i++)
{
	if (table[i].lbn == map_lbn)
	{
		if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)//�޸𸮿� �����Ͱ� �������
		{
			Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
			memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = lsn;
			break;
		}
		else //�޸𸮿� �����Ͱ� ������
		{
			
			log_check = LogBlock_Check(memory, table, hide, table[i].pbn);
			for (j = hide; j < hide + SECTOR_SIZE; j++) //���к�ϰ˻�
			{
				if (strcmp(memory[j].memory, "-1") != 0) //�����Ͱ� ������
				{
					if (memory[j].spare == lsn) //���� lsn���� ������
					{
						memory[j].spare = invalid; //��ȿ������
					}
					continue;
				}
				else
				{
					Flash_write(memory, table, j, table[i].pbn, data);
					memory[j].spare = lsn; //���� lsn�� �Է�
					memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = invalid; //���� �����ʹ� ��ȿ������ ó��
					lcount = 1;
					break;
				}
			}

			if (log_check == 0)//�α׺���� ��á����
			{
				for (k = hide; k < hide + SECTOR_SIZE; k++)
				{
					free_block_pbn = freeBlock_Check(memory, table, sector);
					if (memory[k].spare == invalid) //��ȿ�����ʹ� �ǳʶٰ�
					{
						continue;
					}
					else
					{
						MERGE_to_Free(memory, table, memory[k].spare/SECTOR_SIZE, hide, free_block_pbn*SECTOR_SIZE); //�պ�
					}
				}
				Flash_erase(memory, table, hide); //�α׺�� �����
				Flash_erase_spare(memory, table, hide); //�α׺�� ����� �����
				Flash_write(memory, table, hide, table[i].pbn, data);
				memory[hide].spare = lsn;
			}
		}
	}
}


printf("Write_AllCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
printf("Erase_AllCount : %d\n", erase_Allcount); //����� ���� Ƚ��
printf("\n");
}
void MERGE_to_Free(MEMORY *memory, TABLE *table, int pbn, int hide,int free)
{
	int sector = pbn*SECTOR_SIZE;
	int i, j, k = 0;
	int invalid = -5;
	int offset = 0;
	int temp;
	int free2 = free;

	for (i = sector; i < sector + SECTOR_SIZE; i++) //��������� �����������
	{
		offset = i%SECTOR_SIZE;
		if (strcmp(memory[i].memory, "-1") == 0 || (memory[i].spare == invalid))
		{
			continue;
		}
		else
		{
			Flash_write(memory, table,free+offset,pbn,memory[i].memory );
			memory[free + offset].spare = free + offset;
		}
	}

	for (j= hide; j < hide + SECTOR_SIZE; j++) //�α׺���� �����������
	{
		if (memory[j].spare == invalid)
		{
			continue;
		}
		else
		{
			if (memory[j].spare / SECTOR_SIZE == pbn)
			{
				offset = memory[j].spare%SECTOR_SIZE;
				Flash_write(memory, table, free + offset,pbn, memory[j].memory);
				memory[free + offset].spare = free + offset;
				memory[j].spare = invalid;
			}
		}
	}

	/*for (k = free; k < free + SECTOR_SIZE; k++) //��������� ���� �����ͺ������ 
	{
		offset = k%SECTOR_SIZE;
		if (strcmp(memory[k].memory, "-1") == 0)
		{
			continue;
		}
		else
		{
			Flash_write(memory, table, pbn*SECTOR_SIZE + offset,pbn, memory[k].memory);
			memory[(pbn*SECTOR_SIZE) + offset].spare = memory[k].spare;
		}
	}*/
	Flash_erase(memory, table, sector); //������������
	Flash_erase_spare(memory, table, sector);
	
	
	temp = table[pbn].pbn;
	table[pbn].pbn = free/SECTOR_SIZE;
	table[free/SECTOR_SIZE].pbn = table[pbn].pbn;
}

int LogBlock_Check(MEMORY *memory, TABLE *table, int hide, int pbn) //�α׺������ �ٸ������ �����°� �����ϱ����� üũ
{
	int i;
	int hide_count = 0;
	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (strcmp(memory[i].memory, "-1") == 0)
		{
			hide_count++;
		}
	}
	return hide_count;
}

int freeBlock_Check(MEMORY *memory, TABLE *table, int sector) //������ ��Ϻ��� �����Ͱ� ����ִ� ��� ����
{
	int i;
	int count = 0;
	int pbn;

	for (i = sector - 1; i >= 0; i--)
	{
		if (strcmp(memory[i].memory, "-1") != 0)
		{
			count++;
		}

		if (i%SECTOR_SIZE == 0)
		{
			if (count == 0)
			{
				pbn = i / SECTOR_SIZE;
				return pbn;
			}
			else
			{
				count = 0;
			}
		}
	}
}

int Log_pbn_check(MEMORY *memory, TABLE *table, int sector, int offset, int hide)
{
	int i;

	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (strcmp(memory[i].memory, "-1") != 0)
		{

			return i;
		}
	}
}

void Flash_write(MEMORY *memory, TABLE *table, int sector, int pbn, char data[]) // ���� (sector�� �������̺��� psn�� ��)
{

	strcpy(memory[sector].memory, data);
	write_Allcount++;
	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


void Flash_erase(MEMORY *memory, TABLE *table, int sector) // �����Ͱ� ������ ����⿬�� (sector�� �������̺��� psn�� ��)
{

	int i;


	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i < ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //��ϴ�����
	{
		strcpy(memory[i].memory, "-1"); //������ �����(�ʱ�ȭ)
	}
	erase_Allcount++;
	//������ ��ϰ� ����������ϴ� ���� ���
	printf("Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE - 1);
	printf("\n");
}

void Flash_erase_spare(MEMORY *memory, TABLE *table, int sector) // �����Ͱ� ������ ����⿬�� (sector�� �������̺��� psn�� ��)
{

	int i;


	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i < ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //��ϴ�����
	{
		memory[i].spare = -1;
	}

}

void FTL_read(MEMORY *memory, TABLE *table, int block) //FTL �б�
{
	int lsn, map_lbn;
	int i;
	int offset = 0;
	printf("Input lsn:");
	scanf("%d", &lsn); //���ּ� �Է�
	printf("\n");


	map_lbn = lsn / SECTOR_SIZE; //�Է¹������� ��ϴ� ���� ���� ���������� �������̺��� ������� ����
	offset = lsn % SECTOR_SIZE; //�������� ��ϴ� ���� ������ ���� ������ (���������� ������ ��ȣ�� ã��)

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn) //���ּҿ� �����ϴ� psn�� ã������
		{
			Flash_backRead(memory, table, block*SECTOR_SIZE, lsn, offset);
		}
	}
	printf("\n");
}
void Flash_backRead(MEMORY *memory, TABLE *table, int sector, int lsn, int offset) //������ �齺ĵ
{
	int i;
	for (i = sector + SECTOR_SIZE - 1; i >= 0; i--)
	{
		if (memory[i].spare == lsn)
		{
			Flash_read(memory, table, i);
			break;
		}
	}
}

void Flash_read(MEMORY *memory, TABLE *table, int sector) // �б�
{

	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}

void FTL_write_FILE(MEMORY *memory, TABLE *table, int block, int lsn, char data[]) //FTL ����
{
	int invalid = -5;
	int i, j, k, temp1, temp2 = 0;
	int map_lbn = 0;
	int offset = 0;
	int sector = block * SECTOR_SIZE;
	int hide = sector;
	int log_check = 0;
	int free_block_pbn = 0;
	int log_block_pbn = 0;
	int temp;
	int lcount = 0;


	if (lsn > sector)
	{
		printf("ERROR!\n");
		return 0;
	}

	if (strlen(data) >= SIZE) //�Է��� ������ũ�� ����ó��
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //�Է¹������� ��ϴ� ���� ������ ���������� �������̺��� ������� ����
	offset = lsn % SECTOR_SIZE; //�������� ��ϴ� ���� ������ ���� ������ (���������� ������ ��ȣ�� ã��)

	for (i = 0; i < block; i++)
	{
		if (table[i].lbn == map_lbn)
		{
			if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)//�޸𸮿� �����Ͱ� �������
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
				memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = lsn;
				break;
			}
			else //�޸𸮿� �����Ͱ� ������
			{

				log_check = LogBlock_Check(memory, table, hide, table[i].pbn);
				for (j = hide; j < hide + SECTOR_SIZE; j++) //���к�ϰ˻�
				{
					if (strcmp(memory[j].memory, "-1") != 0) //�����Ͱ� ������
					{
						if (memory[j].spare == lsn) //���� lsn���� ������
						{
							memory[j].spare = invalid; //��ȿ������
						}
						continue;
					}
					else
					{
						Flash_write(memory, table, j, table[i].pbn, data);
						memory[j].spare = lsn; //���� lsn�� �Է�
						memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = invalid; //���� �����ʹ� ��ȿ������ ó��
						lcount = 1;
						break;
					}
				}

				if (log_check == 0)//�α׺���� ��á����
				{
					for (k = hide; k < hide + SECTOR_SIZE; k++)
					{
						free_block_pbn = freeBlock_Check(memory, table, sector);
						if (memory[k].spare == invalid) //��ȿ�����ʹ� �ǳʶٰ�
						{
							continue;
						}
						else
						{
							MERGE_to_Free(memory, table, memory[k].spare / SECTOR_SIZE, hide, free_block_pbn*SECTOR_SIZE); //�պ�
						}
					}
					Flash_erase(memory, table, hide); //�α׺�� �����
					Flash_erase_spare(memory, table, hide); //�α׺�� ����� �����
					Flash_write(memory, table, hide, table[i].pbn, data);
					memory[hide].spare = lsn;
				}
			}
		}
	}



	printf("Write_AllCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
	printf("Erase_AllCount : %d\n", erase_Allcount); //����� ���� Ƚ��
	printf("\n");
}