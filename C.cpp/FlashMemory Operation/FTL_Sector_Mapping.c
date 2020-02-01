#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512
#define SECTOR_SIZE 32 //�� ��ϴ� ������ ����

typedef struct Memory {
	char memory[SIZE]; //1������ ũ�� 512byte
}MEMORY;

typedef struct Table {
	int lsn;
	int psn;
	char ram[SIZE]; //������� ram�� �����Ѵٰ� ����
}TABLE;

typedef struct Dram {
	char dram[SIZE];
}DRAM;

int write_Allcount = 0;
int erase_Allcount = 0;

void menu();
void menu2();
int init_megabyte();
MEMORY *init_memory(int sector);
TABLE *init_table(int sector);
void print_memory(MEMORY *memory, int sector);
void print_table(TABLE *table, int sector);
void print_ram(DRAM *dram, int sector);
void FTL_write(MEMORY *memory, TABLE *table, int sector, DRAM *dram, int lsn, char data[]);
void Flash_read(MEMORY *memory, TABLE *table, int sector);
void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]);
void FTL_read(MEMORY *memory, TABLE *table, int sector);
void Flash_erase(MEMORY *memory, TABLE *table, int sector);
void Flash_copy(MEMORY *memory, TABLE *table, int sector);
void write_text(MEMORY *memory, TABLE *table, int sector, DRAM *dram);
DRAM *init_dram(int sector);
void print_ram(DRAM *dram, int sector);

void main()
{
	int choice, sector, n = 0;
	MEMORY *memory = NULL;
	TABLE *table = NULL;
	DRAM *dram = NULL;

	sector = init_megabyte();
	memory = init_memory(sector); // �޸� ���� �� �ʱ�ȭ
	table = init_table(sector); // �������̺� ����
	dram = init_dram(sector); //���ʱ�ȭ
	while (1)
	{
		menu();
		printf("Number:");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			//FTL_write(memory, table, sector);
			break;
		case 2:
			FTL_read(memory,table,sector);
			break;
		case 3:
			menu2();
			printf("Number:");
			scanf("%d", &n);
			if (n == 1)
			{
				print_memory(memory, sector);
				break;
			}

			else if (n == 2)
			{
				print_table(table, sector);
				break;
			}
			else if (n == 3)
			{
				print_ram(dram, sector);
				break;
			}
			else if (n == 4)
			{
				printf("Wirte_All_Count : %d\n", write_Allcount);
				printf("Erase_All_Count : %d\n", erase_Allcount);
				printf("\n");
				break;
			}
			else
				break;
		case 4:
			write_text(memory, table, sector, dram);
			break;
		case 0:
			exit(1);
		default:
			break;
		}
	}
}

void write_text(MEMORY *memory, TABLE *table, int sector, DRAM *dram)
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
			FTL_write(memory, table, sector, dram, startSector, "AA");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 1)
		{
			FTL_write(memory, table, sector, dram, startSector, "BB");
		}
		else if (strcmp(readWrite, "w") == 0 && startSector == 17)
		{
			FTL_write(memory, table, sector, dram, startSector, "CC");
		}
		else
		{
			FTL_write(memory, table, sector, dram, startSector, "DD");
		}

	} while (!feof(fp));

}
void menu()
{
	printf("===================================\n");
	printf("1.Write 2.Read 3.Print 0.Exit\n");
	printf("===================================\n");
}

void menu2()
{
	printf("===================================================\n");
	printf("1.FlashMemory  2. MappingTable  3. RamData 4.Count \n");
	printf("===================================================\n");
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

	return sector;
}

MEMORY *init_memory(int sector) //�޸� ����
{
	int i;
	MEMORY *memory = NULL;
	int block = sector / SECTOR_SIZE;
	
	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector)); //������ ũ�⸸ŭ ����ü����


	for (i = 0; i < sector; i++)
	{
		strcpy(memory[i].memory, "-1"); //������ -1�� �ʱ�ȭ
	}

	printf("\n");
	printf("Sector size :%d \n", sector);
	printf("Block size :%d \n", block);

	return memory;
}

TABLE *init_table(int sector) //���� ���̺� ����
{
	int i,j,k;
	int num = 0;
	int sec = sector;
	TABLE *table = NULL;
	int block = sector / SECTOR_SIZE;

	table = (TABLE*)malloc(sizeof(TABLE)*(sector)); //�޸𸮿� ���������� ���ͼ���ŭ

	for (i = 0; i < sector; i++)
	{
		table[i].lsn = i; //���ּҴ� 0����~ ���ͼ�����
	}

	for (j = 0; j <sector; j++)
	{
		table[j].psn =sec-1; //�����ּҴ� ���ͺ���~0���� (�Ųٷ�)
		sec--;   
	}

	printf("MappingTable Size :%d\n", sector);
	return table;
	
}

DRAM *init_dram(int sector)
{
	int i;
	DRAM *dram = NULL;
	int sec = sector;
	int block = sector / SECTOR_SIZE;

	dram = (DRAM*)malloc(sizeof(DRAM)*(SECTOR_SIZE));//DRAM �� ũ��� �Ѻ��

	for (i = 0; i < SECTOR_SIZE; i++)
	{
		strcpy(dram[i].dram, "-1");
	}

	printf("Dram Size :%d\n", SECTOR_SIZE);
	return dram;
}

void print_memory(MEMORY *memory, int sector) //�޸𸮿� ����� ������ ���
{
	int i;

	for (i = 0; i < sector; i++)
	{
		printf("psn%d = %s\n", i, memory[i].memory);
	}
	printf("\n");
}

void print_table(TABLE *table, int sector) //�������̺� ������ ���
{
	int i;

	for (i = 0; i < sector; i++)
	{
		printf("lsn%d = %d \t psn%d = %d\n", i, table[i].lsn, i, table[i].psn);
	}
	printf("\n");
}

void print_ram(DRAM *dram, int sector) //�� ������ ���
{
	int i;

	for (i = 0; i <SECTOR_SIZE; i++)
	{
		printf("Num %d : %s \n", i, dram[i].dram);
	}
}


void FTL_write(MEMORY *memory, TABLE *table, int sector, DRAM *dram, int lsn, char data[]) //FTL ����
{
	int write_count = 0;
	int erase_count = 0;
	int i, j;
	/*
	printf("Input lsn,data :");
	scanf("%d %s", &lsn, data);
	*/
	if (lsn < 0 ||lsn >=sector)
	{
		printf("ERROR!\n");
		return 0;
	}

	if (strlen(data) >= SIZE) //�Է��� ������ũ�� ����ó��
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	
	for (i = 0; i < sector; i++)
	{
		if (table[i].lsn ==lsn) //�Է��� ���ּҿ� �������̺��� ���ּҰ� ��ġ�ϴ� ��ġ�� ã��
		{
			if (!(strcmp(memory[table[i].psn].memory, "-1") == 0)) // ���ּҿ� �����ϴ� �����ּ��� �޸� ���͹�ȣ�� �����Ͱ� ������
			{
				Flash_copy(memory, table, table[i].psn); //���� ������ ���� 
				Flash_erase(memory, table, table[i].psn); //����⿬�� ����(������ �����ʹ� ���� �����)
				erase_count++;
				erase_Allcount++;
				Flash_write(memory, table,table[i].psn,data); //����� �� �� ����
				write_count++;
				write_Allcount++;
				
				for (j = (table[i].psn / SECTOR_SIZE) * SECTOR_SIZE; j <= ((table[i].psn / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; j++) //���� ����� �����͸�
				{
					if (strcmp(memory[j].memory, "-1") == 0) //�޸𸮰� ����ִ°�������
					{
						strcpy(memory[j].memory, table[j].ram); // �������͸� �޸𸮷� �ٽ� ����
					}
				}
			}
			else 
			{
				Flash_write(memory, table, table[i].psn, data); //������� ������ �����;���
				write_count++;
				write_Allcount++;
			}
		}

	}

	printf("Write_ALLCount : %d\n", write_Allcount); //���⿬�� Ƚ�� 
	printf("Erase_ALLCount : %d\n", erase_Allcount); //����� ���� Ƚ��
	printf("\n");

}

void Flash_copy(MEMORY *memory, TABLE *table, int sector) //������ ������ ���� 
{
	int i;
	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i <= ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //��ϴ�����
	{
		strcpy(table[i].ram, memory[i].memory);//�����͸� ��������
	}
	
	printf("Copy To Ram Success ! \n");

}

void Flash_erase(MEMORY *memory, TABLE *table, int sector) // �����Ͱ� ������ ����⿬�� (sector�� �������̺��� psn�� ��)
{

	int i;
	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i <= ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //��ϴ�����
	{
		strcpy(memory[i].memory, "-1"); //������ �����(�ʱ�ȭ)
	}

	//������ ��ϰ� ����������ϴ� ���� ���
	printf("Block %d (psn%d ~ psn%d) Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE-1);
	printf("\n");
}

void FTL_read(MEMORY *memory, TABLE *table, int sector) //FTL �б�
{
	int lsn;
	int i;
	printf("Input lsn:");
	scanf("%d", &lsn); //���ּ� �Է�
	printf("\n");

	if (lsn < 0)
	{
		printf("ERROR!\n");
		return 0;
	}
	for (i = 0; i < sector; i++)
	{
		if (table[i].lsn == lsn) //���ּҿ� �����ϴ� psn�� ã������
		{
			Flash_read(memory, table, table[i].psn);
		}
	}
	printf("\n");
}

void Flash_read(MEMORY *memory,TABLE *table,int sector) // �б�
{
	
	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}

void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]) // ���� (sector�� �������̺��� psn�� ��)
{
	
	strcpy(memory[sector].memory, data);

	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


