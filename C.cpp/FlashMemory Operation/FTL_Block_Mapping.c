#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512 //����Ʈ
#define SECTOR_SIZE 32 //�� ��ϴ� ������ ����

typedef struct Memory {
	char memory[SIZE]; //1������ ũ�� 512byte
}MEMORY;

typedef struct Table {
	int lbn;
	int pbn;
}TABLE;

int temp = 0; //temp �� overwrite�� ��Ÿ�� ���� ����� ù��°����
int temp2 = 0; //temp2 �� overwrite�� ��Ÿ�� ���� �������̺��� lbn 
int count = 0;
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
void Flash_erase_Hide(MEMORY *memory, TABLE *table, int sector);
void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]);
void Flash_read(MEMORY *memory, TABLE *table, int sector);
void FTL_write(MEMORY *memory, TABLE *table, int block, int lsn, char data[]);
void Flash_Copy_Hide(MEMORY *memory, TABLE *table, int sector, int offset, int hide);
void Flash_Copy_Original(MEMORY *memory, TABLE *table, int sector, int offset,int hide);
void FTL_read(MEMORY *memory, TABLE *table, int block);
void write_text(MEMORY *memory, TABLE *table, int sector);

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
			//FTL_write(memory, table, block);
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
			write_text(memory, table,block);
			break;

		case 0:
			exit(1);
		default:
			break;
		}
	}
}

void write_text(MEMORY *memory, TABLE *table,int sector)
{
	char readWrite[10];
	int startSector;


	FILE *fp;
	if ((fp = fopen("trace1.txt", "r")) == NULL)
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
			FTL_write(memory, table, sector,startSector, "AA");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 1)
		{
			FTL_write(memory, table, sector,startSector, "BB");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 17)
		{
			FTL_write(memory, table, sector,startSector, "CC");
		}
		else
		{
			FTL_write(memory, table, sector,startSector, "DD");
		}

	} while (!feof(fp));

}
void menu()
{
	printf("\n===================================\n");
	printf("1.Write 2.Read 3.Print 0.Exit\n");
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

	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector+SECTOR_SIZE)); //������ ũ�⸸ŭ ����ü����


	for (i = 0; i < sector+SECTOR_SIZE; i++)
	{
		strcpy(memory[i].memory, "-1"); //������ -1�� �ʱ�ȭ
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
	int blk = block+1;
	TABLE *table = NULL;
	

	table = (TABLE*)malloc(sizeof(TABLE)*(block+1)); //��ϼ���ŭ ����

	for (i = 0; i < block+1; i++)
	{
		table[i].lbn = i; //���ּҴ� 0����~ ��ϼ�����
	}

	for (j = 0; j <block+1; j++)
	{
		table[j].pbn = j; //�����ּҴ� ��Ϻ���~0���� (�Ųٷ�)
	}

	printf("Mapping Table Size :%d\n", block);
	return table;

}

void print_memory(MEMORY *memory, int block) //�޸𸮿� ����� ������ ���
{
	int i;
	int sector = block * SECTOR_SIZE;
	for (i = 0; i < sector+SECTOR_SIZE; i++)
	{
		if (i == sector)//������ �Ѻ���� ���̵� ����
		{
			printf("==================== Hide Memory ====================\n");
			printf("psn%d = %s\n", i, memory[i].memory);
			continue;
		}
		printf("psn%d = %s\n", i, memory[i].memory);
	}
	printf("==================== Hide Memory ====================\n");
	printf("\n");
}

void print_table(TABLE *table, int block) //�������̺� ������ ���
{
	int i;

	for (i = 0; i < block+1; i++)
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

void FTL_write(MEMORY *memory, TABLE *table, int block,int lsn,char data[]) //FTL ����
{
	int write_count = 0;
	int erase_count = 0;
	int i, j=0;
	int map_lbn = 0;
	int map_pbn = 0;
	int offset = 0;
	int sector = block * SECTOR_SIZE;
	int hide = sector;
	

	/*printf("Input lsn,data :");
	scanf("%d %s", &lsn, data);

	if (lsn <SECTOR_SIZE) // ���̵� ������ �����͸� �Է��ϸ� ���� 
	{
		printf("ERROR!\n");
		return 0;
	}
	*/
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
		if (table[i].lbn == map_lbn) // lbn�� �����ϴ� pbn�� ���� �������̺��� ã��
		{
			if (!(strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)) //�޸𸮿� �����Ͱ� ������
			{
				if (table[i].pbn == block) // pbn�� �� ����Ű�� ���� ���̵� �� ��
				{
					Flash_Copy_Original(memory, table, temp, offset, hide);
					Flash_erase_Hide(memory, table, hide); //���̵������ �����
					erase_count++;
					erase_Allcount++;
					Flash_write(memory, table, temp+offset, data); //������Ͽ� ������ ����
					write_count++;
					write_Allcount++;
					table[i].pbn = temp / SECTOR_SIZE;
					count--;
					break;
				}
				else
				{
					if (count != 0) // ���̵������ �����Ͱ� ���� ��
					{
						Flash_Copy_Original(memory, table, temp, offset, hide);//���̵��� �����͸� ���� ��������Ʈ�� �Ͼ ���� ���� 
						table[temp2].pbn = temp / SECTOR_SIZE; //������ �羲�Ⱑ �Ͼ ���� ���̺��� �ٽ� ������Ʈ 
						Flash_erase_Hide(memory, table, hide); //���̵������ �����
						erase_count++;
						erase_Allcount++;
						Flash_Copy_Hide(memory, table, table[i].pbn * SECTOR_SIZE, offset, hide); //���̵忡 ������ ����
						Flash_write(memory, table, hide + offset, data); //���̵忡 ���ο� ������ ����
						write_count++;
						write_Allcount++;
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); //���� �ִ� ������ ��� �����
						erase_count++;
						erase_Allcount++;
						temp = table[i].pbn * SECTOR_SIZE;
						temp2 = table[i].lbn;
						table[i].pbn = hide / 32; //�������̺��� ��������� ���̵�������� ������Ʈ
						
						break;
					}
					else // ���̵������ �����Ͱ� ���� ��
					{
						Flash_Copy_Hide(memory, table, table[i].pbn * SECTOR_SIZE, offset, hide); //���̵忡 ������ ����
						Flash_write(memory, table, hide + offset, data); //���̵忡 ���ο� ������ ����
						write_count++;
						write_Allcount++;
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); //���� �ִ� ������ ��� �����
						erase_count++;
						erase_Allcount++;
						temp = table[i].pbn * SECTOR_SIZE;
						temp2 = table[i].lbn;
						table[i].pbn = hide / 32; //�������̺��� ��������� ���̵�������� ������Ʈ
						count++;
						break;
					}
				}
			}
			else
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, data);
				write_count++;
				write_Allcount++;
			}

		}
		
		}

	printf("Write_AllCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
	printf("Erase_AllCount : %d\n", erase_Allcount); //����� ���� Ƚ��
	printf("\n");
}

void Flash_Copy_Hide(MEMORY *memory, TABLE *table, int sector,int offset,int hide) //���̵������ ������ ����
{
	int i;

	for (i = sector; i < sector + SECTOR_SIZE; i++)
	{
		if (i == sector + offset)  // overwrite�� �Ͼ �ڸ��� �����͸� �����ϰ� ������
		{   
			strcpy(memory[hide].memory, "-1");
			hide++;
			continue;
		}
		strcpy(memory[hide].memory, memory[i].memory);
		hide++;
		
	}

	printf("Copy To Hide Memory Success\n");
}

void Flash_Copy_Original(MEMORY *memory,TABLE *table,int sector,int offset,int hide)
{
	int i;

	for (i = hide; i < hide + SECTOR_SIZE; i++)
	{
		if (i == hide + offset)
		{
			strcpy(memory[sector].memory, "-1");
			sector++;
			continue;
			
		}
		strcpy(memory[sector].memory, memory[i].memory);
		sector++;
	}

	printf("Copy To Original Memory Success\n");
}
void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]) // ���� (sector�� �������̺��� psn�� ��)
{

	strcpy(memory[sector].memory, data);

	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


void Flash_erase_Hide(MEMORY *memory, TABLE *table, int sector) // �����Ͱ� ������ ����⿬�� (sector�� �������̺��� psn�� ��)
{

	int i;


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //��ϴ�����
	{
		strcpy(memory[i].memory, "-1"); //������ �����(�ʱ�ȭ)
	}

	//������ ��ϰ� ����������ϴ� ���� ���
	printf("Hide Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE-1);
	printf("\n");
}

void Flash_erase(MEMORY *memory, TABLE *table, int sector) // �����Ͱ� ������ ����⿬�� (sector�� �������̺��� psn�� ��)
{

	int i;


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //��ϴ�����
	{
		strcpy(memory[i].memory, "-1"); //������ �����(�ʱ�ȭ)
	}

	//������ ��ϰ� ����������ϴ� ���� ���
	printf("Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE - 1);
	printf("\n");
}

void FTL_read(MEMORY *memory, TABLE *table, int block) //FTL �б�
{
	int lsn,map_lbn;
	int i;
	int offset=0;
	printf("Input lsn:");
	scanf("%d", &lsn); //���ּ� �Է�
	printf("\n");
	
	if (lsn < SECTOR_SIZE)
	{
		printf("ERROR!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //�Է¹������� ��ϴ� ���� ���� ���������� �������̺��� ������� ����
	offset = lsn % SECTOR_SIZE; //�������� ��ϴ� ���� ������ ���� ������ (���������� ������ ��ȣ�� ã��)

	for (i = 1; i < block; i++)
	{
		if (table[i].lbn == map_lbn) //���ּҿ� �����ϴ� psn�� ã������
		{
			Flash_read(memory, table, table[i].pbn*SECTOR_SIZE+offset);
		}
	}
	printf("\n");
}


void Flash_read(MEMORY *memory, TABLE *table, int sector) // �б�
{

	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}