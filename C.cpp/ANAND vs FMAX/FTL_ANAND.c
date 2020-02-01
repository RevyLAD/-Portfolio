#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512 //����Ʈ
#define SECTOR_SIZE 32 //�� ��ϴ� ������ ����

typedef struct Memory {
	char memory[SIZE]; //1������ ũ�� 512byte
	int spare;
}MEMORY;

typedef struct Table {
	int lbn;
	int pbn;
}TABLE;


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
			printf("psn%d = %s \t spare=%d\n", i, memory[i].memory,memory[i].spare);
			continue;
		}
		printf("psn%d = %s \t spare=%d\n", i, memory[i].memory,memory[i].spare);
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
	int i, j, temp1, temp2 = 0;
	int map_lbn = 0;
	int map_pbn = 0;
	int offset = 0;
	int sector = block * SECTOR_SIZE;
	int hide = sector;
	int log_check = 0;
	int free_block_pbn = 0;
	int log_block_pbn = 0;
	int temp;
	int lsn;
	int data[SIZE];

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

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn)
		{
			if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
				memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = table[i].pbn; //�������� ��Ϲ�ȣ ǥ��
				break;
			}
			else
			{
				log_check = LogBlock_Check(memory, table, hide, table[i].pbn);

				if (log_check == 0)
				{
					if (strcmp(memory[hide + offset].memory, "-1") == 0)
					{
						Flash_write(memory, table, hide + offset, table[i].pbn, data);
						memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block; //������ ��ȿ������Ȯ��������
						memory[hide + offset].spare = table[i].pbn; //��ȿ�����ʹ� �α׺�Ͽ�
						break;
					}
					else
					{
						free_block_pbn = freeBlock_Check(memory, table, sector);
						printf("FREE BLOCK = %d\n", free_block_pbn);
						Original_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //���� ������ ����� ��ȿ�����͸� ����������� 
						Log_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // �α׺���� ��ȿ�����͸� ����������� �պ�
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); // ���� �����ͺ�� �����
						Flash_erase_spare(memory, table, table[i].pbn*SECTOR_SIZE);
						Flash_erase(memory, table, hide); //�α׺�� �����
						Flash_erase_spare(memory, table, hide);
						Flash_write(memory, table, (free_block_pbn*SECTOR_SIZE) + offset, free_block_pbn, data);
						temp = table[i].pbn;
						table[free_block_pbn].pbn = temp;
						table[i].pbn = free_block_pbn;
						break;
					}
				}
				else
				{
					printf("!!!\n");
					free_block_pbn = freeBlock_Check(memory, table, sector);
					printf("FREE BLOCK = %d\n", free_block_pbn);
					log_block_pbn = Log_pbn_check(memory, table, sector, offset, hide);
					temp1 = memory[log_block_pbn].spare;
					Original_to_Free(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //���� ������ ����� ��ȿ�����͸� ����������� 
					Log_to_Free2(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // �α׺���� ��ȿ�����͸� ����������� �պ�
					Flash_erase(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE); // ���� �����ͺ�� �����
					Flash_erase_spare(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE);
					Flash_erase(memory, table, hide); //�α׺�� �����
					Flash_erase_spare(memory, table, hide);
					Flash_write(memory, table, hide + offset, table[i].pbn, data);
					memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block;
					temp2 = table[temp1].pbn;
					table[temp1].pbn = temp1;
					table[temp2].pbn = table[free_block_pbn].pbn;
					table[free_block_pbn].pbn = temp2;
					break;

				}
			}
		}
	}

	printf("Write_AllCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
	printf("Erase_AllCount : %d\n", erase_Allcount); //����� ���� Ƚ��
	printf("\n");
}



void Original_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide,int free) // ���� ������ ��Ͽ��� ����������� ��ȿ������ �պ�
{
	int i;
	int pbn = sector / SECTOR_SIZE;
	int free_pbn = free / SECTOR_SIZE;
	for (i = sector; i < sector + SECTOR_SIZE; i++)
	{
		if (strcmp(memory[i].memory, "-1") != 0)
		{
			if (memory[i].spare ==pbn )
			{
				strcpy(memory[free].memory, memory[i].memory);
				memory[free].spare = free_pbn;
				write_Allcount++;
				free++;
			}
			else
			{
				free++;
			}

		}
		
	}
}

void Log_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free) //�α׺���� ��ȿ������ ����������� �պ�
{
	int i;
	int pbn = sector/SECTOR_SIZE;
	int free_pbn = free / SECTOR_SIZE;

	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (strcmp(memory[i].memory, "-1") != 0)
		{
			if (memory[i].spare == pbn)
			{
				if (i == hide + offset)
				{
					free++;
					continue;
				}
				else
				{
					strcpy(memory[free].memory, memory[i].memory);
					memory[free].spare = free_pbn;
					write_Allcount++;
					free++;
				}

			}

		}

	}
}

void Log_to_Free2(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free) //�α׺���� ��ȿ������ ����������� �պ�
{
	int i;
	int pbn = sector / SECTOR_SIZE;
	int free_pbn = free / SECTOR_SIZE;

	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (strcmp(memory[i].memory, "-1") != 0)
		{
			if (memory[i].spare == pbn)
			{

				strcpy(memory[free].memory, memory[i].memory);
				memory[free].spare = free_pbn;
				write_Allcount++;
				free++;
				

			}

		}

	}
}

int LogBlock_Check(MEMORY *memory, TABLE *table, int hide, int pbn) //�α׺������ �ٸ������ �����°� �����ϱ����� üũ
{
	int i;
	int hide_count = 0;
	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (memory[i].spare != -1)
		{
			if (memory[i].spare != pbn)
			{
				hide_count++;
				break;
			}

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

void Flash_write(MEMORY *memory, TABLE *table, int sector,int pbn, char data[]) // ���� (sector�� �������̺��� psn�� ��)
{

	strcpy(memory[sector].memory, data);
	memory[sector].spare = pbn;
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


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //��ϴ�����
	{
		memory[i].spare = -1;
	}
	//������ ��ϰ� ����������ϴ� ���� ���

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
			Flash_backRead(memory, table, block*SECTOR_SIZE,table[i].pbn,offset);
		}
	}
	printf("\n");
}
void Flash_backRead(MEMORY *memory, TABLE *table, int sector,int pbn, int offset) //������ �齺ĵ
{
	int i;
	for (i = sector + SECTOR_SIZE - 1; i >= 0; i--)
	{
		if (memory[i].spare == pbn)
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
	int write_count = 0;
	int erase_count = 0;
	int i, j, temp1, temp2 = 0;
	int map_lbn = 0;
	int map_pbn = 0;
	int offset = 0;
	int sector = block * SECTOR_SIZE;
	int hide = sector;
	int log_check = 0;
	int free_block_pbn = 0;
	int log_block_pbn = 0;
	int temp;

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

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn)
		{
			if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
				memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = table[i].pbn; //�������� ��Ϲ�ȣ ǥ��
				break;
			}
			else
			{
				log_check = LogBlock_Check(memory, table, hide, table[i].pbn);

				if (log_check == 0)
				{
					if (strcmp(memory[hide + offset].memory, "-1") == 0)
					{
						Flash_write(memory, table, hide + offset, table[i].pbn, data);
						memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block; //������ ��ȿ������Ȯ��������
						memory[hide + offset].spare = table[i].pbn; //��ȿ�����ʹ� �α׺�Ͽ�
						break;
					}
					else
					{
						free_block_pbn = freeBlock_Check(memory, table, sector);
						Original_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //���� ������ ����� ��ȿ�����͸� ����������� 
						Log_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // �α׺���� ��ȿ�����͸� ����������� �պ�
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); // ���� �����ͺ�� �����
						Flash_erase_spare(memory, table, table[i].pbn*SECTOR_SIZE);
						Flash_erase(memory, table, hide); //�α׺�� �����
						Flash_erase_spare(memory, table, hide);
						Flash_write(memory, table, (free_block_pbn*SECTOR_SIZE) + offset, free_block_pbn, data);
						temp = table[i].pbn;
						table[free_block_pbn].pbn = temp;
						table[i].pbn = free_block_pbn;
						break;
					}
				}
				else
				{
					printf("!!!\n");
					free_block_pbn = freeBlock_Check(memory, table, sector);
					log_block_pbn = Log_pbn_check(memory, table, sector, offset, hide);
					temp1 = memory[log_block_pbn].spare;
					Original_to_Free(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //���� ������ ����� ��ȿ�����͸� ����������� 
					Log_to_Free2(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // �α׺���� ��ȿ�����͸� ����������� �պ�
					Flash_erase(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE); // ���� �����ͺ�� �����
					Flash_erase_spare(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE);
					Flash_erase(memory, table, hide); //�α׺�� �����
					Flash_erase_spare(memory, table, hide);
					Flash_write(memory, table, hide + offset, table[i].pbn, data);
					memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block;
					temp2 = table[temp1].pbn;
					table[temp1].pbn = temp1;
					table[temp2].pbn = table[free_block_pbn].pbn;
					table[free_block_pbn].pbn = temp2;
					break;

				}
			}
		}
	}

	printf("Write_AllCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
	printf("Erase_AllCount : %d\n", erase_Allcount); //����� ���� Ƚ��
	printf("\n");
}