#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512
#define DATASIZE 10000
#define SECTOR_SIZE 32 //한 블록당 섹터의 갯수

typedef struct Memory {
	char memory[SIZE]; //1섹터의 크기 512byte
}MEMORY;

typedef struct Table {
	int lsn;
	int psn;
}TABLE;

typedef struct Dram {
	char dram[SIZE];
}DRAM;
int lcount = 0;
int write_Allcount = 0;
int erase_Allcount = 0;

void menu();
void menu2();
int init_megabyte();
MEMORY *init_memory(int sector);
TABLE *init_table(int sector);
DRAM *init_dram(int sector);
void print_memory(MEMORY *memory, int sector);
void print_table(TABLE *table, int sector);
void ram_init(DRAM *dram);
void print_ram(TABLE *table, int sector);
void Flash_read(MEMORY *memory, TABLE *table, int sector);
void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]);
void FTL_read(MEMORY *memory, TABLE *table, int sector);
void Flash_erase(MEMORY *memory, TABLE *table, int sector);
void Flash_copy(MEMORY *memory, TABLE *table, int sector);
int Flash_copy_check(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n);
int Flash_copy_check2(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n);
void write_text(MEMORY *memory, TABLE *table, int sector, DRAM *dram);
void Flash_copy_original(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n);
void FTL_write(MEMORY *memory, TABLE *table, int sector, DRAM *dram, int lsn, char data[]);

void main()
{



	int choice, sector, n = 0;
	MEMORY *memory = NULL;
	TABLE *table = NULL;
	DRAM *dram = NULL;
	sector = init_megabyte();
	memory = init_memory(sector); // 메모리 생성 및 초기화
	table = init_table(sector); // 매핑테이블 생성
	dram = init_dram(sector);

	while (1)
	{
		menu();
		printf("Number:");
		scanf("%d", &choice);

		switch (choice)
		{
			/*
		case 1:
			FTL_write(memory, table, sector,dram);
			break;
			*/
		case 2:
			FTL_read(memory, table, sector);
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
		fprintf(stderr, "해당하는 파일을 열 수 없습니다.");
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
	printf("1.Write 2.Read 3.Print 4.FILE 0.Exit\n");
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

	if (n <= 0) //크기가 0보다 작으면 에러
	{
		printf("ERROR\n");
		exit(1);
	}

	sector = n * 1024 * 1024 / SIZE; //섹터의 갯수
	block = sector / SECTOR_SIZE; //블록의 갯수 (1블록은 32섹터)

	return sector;
}

MEMORY *init_memory(int sector) //메모리 생성
{
	int i;
	MEMORY *memory = NULL;
	int block = sector / SECTOR_SIZE;

	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector)); //섹터의 크기만큼 구조체생성


	for (i = 0; i < sector; i++)
	{
		strcpy(memory[i].memory, "-1"); //데이터 -1로 초기화
	}

	printf("\n");
	printf("Sector size :%d \n", sector);
	printf("Block size :%d \n", block);

	return memory;
}

TABLE *init_table(int sector) //매핑 테이블 생성
{
	int i, j, k,m;
	int num = 0;
	int sec = sector;
	TABLE *table = NULL;
	int block = sector / SECTOR_SIZE;

	table = (TABLE*)malloc(sizeof(TABLE)*(sector)); //메모리와 마찬가지로 섹터수만큼

	for (i = 0; i < sector; i++)
	{
		table[i].lsn = i; //논리주소는 0부터~ 섹터수까지
	}

	for (j = 0; j <sector; j++)
	{
		table[j].psn = -1; //물리주소는 -1 (빈공간)
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

	dram = (DRAM*)malloc(sizeof(DRAM)*(SECTOR_SIZE));//DRAM 의 크기는 한블록

	for (i = 0; i < SECTOR_SIZE; i++)
	{
		strcpy(dram[i].dram, "-1");
	}

	printf("Dram Size :%d\n", SECTOR_SIZE);
	return dram;
}

void ram_init(DRAM *dram)
{
	int i;

	for (i = 0; i < SECTOR_SIZE; i++)
	{
		strcpy(dram[i].dram, "-1");
	}
}

void print_memory(MEMORY *memory, int sector) //메모리에 저장된 데이터 출력
{
	int i;

	for (i = 0; i < sector; i++)
	{
		printf("psn%d = %s\n", i, memory[i].memory);
	}
	printf("\n");
}

void print_table(TABLE *table, int sector) //매핑테이블 데이터 출력
{
	int i;

	for (i = 0; i < sector; i++)
	{
		printf("lsn%d = %d \t psn%d = %d\n", i, table[i].lsn, i, table[i].psn);
	}
	printf("\n");
}

void print_ram(DRAM *dram, int sector) //램 데이터 출력
{
	int i;

	for (i = 0; i <SECTOR_SIZE; i++)
	{
		printf("Num %d : %s \n",i,dram[i].dram);
	}
}

void FTL_write(MEMORY *memory, TABLE *table, int sector,DRAM *dram,int lsn,char data[]) //FTL 쓰기
{
	int count = 0;
	int count2 = 0;
	int write_count = 0;
	int erase_count = 0;
	int i, j, k,l,m=0;
	int a, b = 0;
	/*
	printf("Input lsn,data :");
	scanf("%d %s", &lsn, data);
	*/

	if (strlen(data) >= SIZE) //입력한 데이터크기 예외처리
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	for (a = SECTOR_SIZE-1; a < sector; a=a+SECTOR_SIZE) //첫번째블록부터 블록단위로 유효데이터 검사
	{
		if (strcmp(memory[a].memory, "-1") == 0) //첫번째블록의 마지막섹터가 비어있으면 업데이트를 할 필요가 없으므로 종료
			break;
		else
		{
			count = Flash_copy_check(memory, table, dram, sector, a);//블록에 유효데이터가 있는지 검사
			if (count == 32) //count가 32 면 한블록이 모두 유효데이터이므로 업데이트필요 x
			{
				continue;
			}
			else if (count == 0)//count가 0이면 빈공간이곳 있는 블록이므로 업데이트필요 x
			{
				continue;
			}
			else //업데이트필요
			{
				count2 = Flash_copy_check2(memory, table, dram, sector, a);//블록에 데이터가 비어있는지 검사
				if (count2 == 0)//다차있으면 업데이트
				{
					Flash_copy(memory, table, dram, sector, a);
					Flash_erase(memory, table, a);
					erase_count++;
					erase_Allcount++;
					Flash_copy_original(memory, table, dram, sector, a);
				}
				else
				{
					continue;
				}
					count2 = 0;
			}
		}
		lcount = 0;
	}
	for (i = 0; i < sector; i++) //데이터넣기
	{
		if (table[i].lsn == lsn) //논리주소찾아서
		{
			for (j = 0; j < sector; j++)
			{
				if (strcmp(memory[j].memory, "-1") == 0)//데이터가비어있는곳에 0부터 순차적으로 쓰기
				{
					Flash_write(memory, table, j, data);
					write_count++;
					write_Allcount++;
					table[i].psn = j;//매핑테이블 업데이트
					break;
				}
			}
		}
	}
	printf("Write_Count : %d\n", write_Allcount); //쓰기연산 횟수 
	printf("Erase_Count : %d\n", erase_Allcount); //지우기 연산 횟수
	printf("\n");

}

void Flash_copy(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n) //램으로 데이터 복사 
{
	int i, j, k = 0;

	for (i = n - SECTOR_SIZE + 1; i <= n; i++)
	{
		for (j = 0; j < sector; j++)
		{
			if (table[j].psn==i)
			{
				if (!(strcmp(memory[i].memory, "-1") == 0))
				{
					strcpy(dram[i % SECTOR_SIZE].dram, memory[i].memory);
					break;
				}
			}
		}
	}

	if(lcount==SECTOR_SIZE)// 한블록 모두가 유효데이터
		printf("Copy FULL Fail lcount:%d ! \n", lcount);
	else if(lcount==0)
	{
		printf("Copy Blank Fail lcount:%d ! \n", lcount);
	}
	else
		printf("Copy To Ram Success lcount:%d ! \n", lcount);
}

int Flash_copy_check(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n) 
{
	int i, j, k = 0;
	int count = 0;
	int lcount = 0;
	for (k = n - SECTOR_SIZE + 1; k <= n; k++)
	{
		if (strcmp(memory[k].memory, "-1") == 0)
		{
			count++;
		}
	}
	if (count == 0)
	{
		for (i = n - SECTOR_SIZE + 1; i <= n; i++)
		{
			for (j = 0; j < sector; j++)
			{
				if (table[j].psn == i)
				{

					lcount++;
					break;

				}
			}
		}
		return lcount;
	}
	else
	{
		return lcount;
	}
}
int Flash_copy_check2(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n)
{
	int i, j, k = 0;
	int count = 0;

	for (k = n - SECTOR_SIZE + 1; k <= n; k++)
	{
		if (strcmp(memory[k].memory, "-1") == 0)
		{
			count++;
		}
	}

	return count;
}
void Flash_copy_original(MEMORY *memory, TABLE *table, DRAM *dram, int sector, int n)
{
	int i, j, k = 0;
	for (i = 0; i < SECTOR_SIZE; i++)
	{
		strcpy(memory[n - SECTOR_SIZE + 1].memory,dram[i].dram);
		n++;
	}
	ram_init(dram);
}

void Flash_erase(MEMORY *memory, TABLE *table, int sector) // 데이터가 있을때 지우기연산 (sector는 매핑테이블의 psn의 값)
{

	int i;
	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i <= ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //블록단위로
	{
		strcpy(memory[i].memory, "-1"); //데이터 지우기(초기화)
	}

	//지워진 블록과 블록을포함하는 섹터 출력
	printf("Block %d (psn%d ~ psn%d) Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE - 1);
	printf("\n");
}

void FTL_read(MEMORY *memory, TABLE *table, int sector) //FTL 읽기
{
	int lsn;
	int i;
	printf("Input lsn:");
	scanf("%d", &lsn); //논리주소 입력
	printf("\n");

	if (lsn < 0)
	{
		printf("ERROR!\n");
		return 0;
	}
	for (i = 0; i < sector; i++)
	{
		if (table[i].lsn == lsn) //논리주소에 대응하는 psn을 찾기위함
		{
			Flash_read(memory, table, table[i].psn);
		}
	}
	printf("\n");
}

void Flash_read(MEMORY *memory, TABLE *table, int sector) // 읽기
{

	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}

void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]) // 쓰기 (sector는 매핑테이블의 psn의 값)
{

	strcpy(memory[sector].memory, data);

	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


