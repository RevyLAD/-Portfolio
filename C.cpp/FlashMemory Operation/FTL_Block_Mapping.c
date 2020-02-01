#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512 //바이트
#define SECTOR_SIZE 32 //한 블록당 섹터의 갯수

typedef struct Memory {
	char memory[SIZE]; //1섹터의 크기 512byte
}MEMORY;

typedef struct Table {
	int lbn;
	int pbn;
}TABLE;

int temp = 0; //temp 는 overwrite가 나타난 곳의 블록의 첫번째섹터
int temp2 = 0; //temp2 는 overwrite가 나타난 곳의 매핑테이블의 lbn 
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
		fprintf(stderr, "해당하는 파일을 열 수 없습니다.");
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

	if (n <= 0) //크기가 0보다 작으면 에러
	{
		printf("ERROR\n");
		exit(1);
	}

	sector = n * 1024 * 1024 / SIZE; //섹터의 갯수
	block = sector / SECTOR_SIZE; //블록의 갯수 (1블록은 32섹터)

	return block;
}

MEMORY *init_memory(int block) //메모리 생성
{
	int i;
	MEMORY *memory = NULL;
	int sector = block * SECTOR_SIZE;

	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector+SECTOR_SIZE)); //섹터의 크기만큼 구조체생성


	for (i = 0; i < sector+SECTOR_SIZE; i++)
	{
		strcpy(memory[i].memory, "-1"); //데이터 -1로 초기화
	}

	printf("\n");
	printf("Sector Size :%d \n", sector);
	printf("Block Size :%d \n", block);

	return memory;
}

TABLE *init_table(int block) //매핑 테이블 생성
{
	int i, j = 0;
	int num = 0;
	int blk = block+1;
	TABLE *table = NULL;
	

	table = (TABLE*)malloc(sizeof(TABLE)*(block+1)); //블록수만큼 생성

	for (i = 0; i < block+1; i++)
	{
		table[i].lbn = i; //논리주소는 0부터~ 블록수까지
	}

	for (j = 0; j <block+1; j++)
	{
		table[j].pbn = j; //물리주소는 블록부터~0까지 (거꾸로)
	}

	printf("Mapping Table Size :%d\n", block);
	return table;

}

void print_memory(MEMORY *memory, int block) //메모리에 저장된 데이터 출력
{
	int i;
	int sector = block * SECTOR_SIZE;
	for (i = 0; i < sector+SECTOR_SIZE; i++)
	{
		if (i == sector)//마지막 한블록은 하이드 공간
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

void print_table(TABLE *table, int block) //매핑테이블 데이터 출력
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

void FTL_write(MEMORY *memory, TABLE *table, int block,int lsn,char data[]) //FTL 쓰기
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

	if (lsn <SECTOR_SIZE) // 하이드 영역에 데이터를 입력하면 오류 
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
	if (strlen(data) >= SIZE) //입력한 데이터크기 예외처리
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //입력받은값을 블록당 섹터 개수로 나눈값으로 매핑테이블의 논리블록을 구함
	offset = lsn % SECTOR_SIZE; //오프셋은 블록당 섹터 개수로 나눈 나머지 (오프셋으로 섹터의 번호를 찾음)

	for (i = 0; i < block; i++)
	{
		if (table[i].lbn == map_lbn) // lbn에 대응하는 pbn의 값을 매핑테이블에서 찾음
		{
			if (!(strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)) //메모리에 데이터가 있으면
			{
				if (table[i].pbn == block) // pbn이 이 가르키는 곳이 하이드 일 때
				{
					Flash_Copy_Original(memory, table, temp, offset, hide);
					Flash_erase_Hide(memory, table, hide); //하이드공간을 지우고
					erase_count++;
					erase_Allcount++;
					Flash_write(memory, table, temp+offset, data); //기존블록에 데이터 쓰기
					write_count++;
					write_Allcount++;
					table[i].pbn = temp / SECTOR_SIZE;
					count--;
					break;
				}
				else
				{
					if (count != 0) // 하이드공간에 데이터가 있을 때
					{
						Flash_Copy_Original(memory, table, temp, offset, hide);//하이드의 데이터를 전에 오버라이트가 일어난 곳에 복사 
						table[temp2].pbn = temp / SECTOR_SIZE; //이전에 재쓰기가 일어난 곳의 테이블을 다시 업데이트 
						Flash_erase_Hide(memory, table, hide); //하이드공간을 지우고
						erase_count++;
						erase_Allcount++;
						Flash_Copy_Hide(memory, table, table[i].pbn * SECTOR_SIZE, offset, hide); //하이드에 데이터 복사
						Flash_write(memory, table, hide + offset, data); //하이드에 새로운 데이터 쓰기
						write_count++;
						write_Allcount++;
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); //원래 있던 데이터 블록 지우기
						erase_count++;
						erase_Allcount++;
						temp = table[i].pbn * SECTOR_SIZE;
						temp2 = table[i].lbn;
						table[i].pbn = hide / 32; //매핑테이블의 물리블록을 하이드공간으로 업데이트
						
						break;
					}
					else // 하이드공간에 데이터가 없을 때
					{
						Flash_Copy_Hide(memory, table, table[i].pbn * SECTOR_SIZE, offset, hide); //하이드에 데이터 복사
						Flash_write(memory, table, hide + offset, data); //하이드에 새로운 데이터 쓰기
						write_count++;
						write_Allcount++;
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); //원래 있던 데이터 블록 지우기
						erase_count++;
						erase_Allcount++;
						temp = table[i].pbn * SECTOR_SIZE;
						temp2 = table[i].lbn;
						table[i].pbn = hide / 32; //매핑테이블의 물리블록을 하이드공간으로 업데이트
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

	printf("Write_AllCount : %d\n", write_Allcount); //쓰기연산 횟수 
	printf("Erase_AllCount : %d\n", erase_Allcount); //지우기 연산 횟수
	printf("\n");
}

void Flash_Copy_Hide(MEMORY *memory, TABLE *table, int sector,int offset,int hide) //하이드공간에 데이터 복사
{
	int i;

	for (i = sector; i < sector + SECTOR_SIZE; i++)
	{
		if (i == sector + offset)  // overwrite가 일어난 자리의 데이터를 제외하고 복사함
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
void Flash_write(MEMORY *memory, TABLE *table, int sector, char data[]) // 쓰기 (sector는 매핑테이블의 psn의 값)
{

	strcpy(memory[sector].memory, data);

	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


void Flash_erase_Hide(MEMORY *memory, TABLE *table, int sector) // 데이터가 있을때 지우기연산 (sector는 매핑테이블의 psn의 값)
{

	int i;


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //블록단위로
	{
		strcpy(memory[i].memory, "-1"); //데이터 지우기(초기화)
	}

	//지워진 블록과 블록을포함하는 섹터 출력
	printf("Hide Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE-1);
	printf("\n");
}

void Flash_erase(MEMORY *memory, TABLE *table, int sector) // 데이터가 있을때 지우기연산 (sector는 매핑테이블의 psn의 값)
{

	int i;


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //블록단위로
	{
		strcpy(memory[i].memory, "-1"); //데이터 지우기(초기화)
	}

	//지워진 블록과 블록을포함하는 섹터 출력
	printf("Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE - 1);
	printf("\n");
}

void FTL_read(MEMORY *memory, TABLE *table, int block) //FTL 읽기
{
	int lsn,map_lbn;
	int i;
	int offset=0;
	printf("Input lsn:");
	scanf("%d", &lsn); //논리주소 입력
	printf("\n");
	
	if (lsn < SECTOR_SIZE)
	{
		printf("ERROR!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //입력받은값을 블록당 섹터 개수 나눈값으로 매핑테이블의 논리블록을 구함
	offset = lsn % SECTOR_SIZE; //오프셋은 블록당 섹터 개수로 나눈 나머지 (오프셋으로 섹터의 번호를 찾음)

	for (i = 1; i < block; i++)
	{
		if (table[i].lbn == map_lbn) //논리주소에 대응하는 psn을 찾기위함
		{
			Flash_read(memory, table, table[i].pbn*SECTOR_SIZE+offset);
		}
	}
	printf("\n");
}


void Flash_read(MEMORY *memory, TABLE *table, int sector) // 읽기
{

	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}