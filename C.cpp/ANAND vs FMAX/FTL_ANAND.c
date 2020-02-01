#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 512 //바이트
#define SECTOR_SIZE 32 //한 블록당 섹터의 갯수

typedef struct Memory {
	char memory[SIZE]; //1섹터의 크기 512byte
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
		fprintf(stderr, "해당하는 파일을 열 수 없습니다.");
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

	memory = (MEMORY*)malloc(sizeof(MEMORY)*(sector + SECTOR_SIZE));


	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		strcpy(memory[i].memory, "-1"); //데이터 -1로 초기화
	}

	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		memory[i].spare = -1; //스페어 영역  -1로 초기화
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
	int blk = block + 1;
	TABLE *table = NULL;


	table = (TABLE*)malloc(sizeof(TABLE)*(block + 1)); //블록수만큼 생성

	for (i = 0; i < block + 1; i++)
	{
		table[i].lbn = i; //논리주소는 0부터~ 블록수까지
	}

	for (j = 0; j <block + 1; j++)
	{
		table[j].pbn = j;
	}

	printf("Mapping Table Size :%d\n", block);
	return table;

}

void print_memory(MEMORY *memory, int block) //메모리에 저장된 데이터 출력
{
	int i;
	int sector = block * SECTOR_SIZE;
	for (i = 0; i < sector + SECTOR_SIZE; i++)
	{
		if (i == sector)//마지막 한블록은 하이드 공간
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

void print_table(TABLE *table, int block) //매핑테이블 데이터 출력
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

void FTL_write(MEMORY *memory, TABLE *table, int block) //FTL 쓰기
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

	if (strlen(data) >= SIZE) //입력한 데이터크기 예외처리
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //입력받은값을 블록당 섹터 개수로 나눈값으로 매핑테이블의 논리블록을 구함
	offset = lsn % SECTOR_SIZE; //오프셋은 블록당 섹터 개수로 나눈 나머지 (오프셋으로 섹터의 번호를 찾음)

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn)
		{
			if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
				memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = table[i].pbn; //스페어영역에 블록번호 표시
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
						memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block; //기존의 무효데이터확인을위해
						memory[hide + offset].spare = table[i].pbn; //유효데이터는 로그블록에
						break;
					}
					else
					{
						free_block_pbn = freeBlock_Check(memory, table, sector);
						printf("FREE BLOCK = %d\n", free_block_pbn);
						Original_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //기존 데이터 블록의 유효데이터를 프리블록으로 
						Log_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // 로그블록의 유효데이터를 프리블록으로 합병
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); // 기존 데이터블록 지우기
						Flash_erase_spare(memory, table, table[i].pbn*SECTOR_SIZE);
						Flash_erase(memory, table, hide); //로그블록 지우기
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
					Original_to_Free(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //기존 데이터 블록의 유효데이터를 프리블록으로 
					Log_to_Free2(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // 로그블록의 유효데이터를 프리블록으로 합병
					Flash_erase(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE); // 기존 데이터블록 지우기
					Flash_erase_spare(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE);
					Flash_erase(memory, table, hide); //로그블록 지우기
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

	printf("Write_AllCount : %d\n", write_Allcount); //쓰기연산 횟수 
	printf("Erase_AllCount : %d\n", erase_Allcount); //지우기 연산 횟수
	printf("\n");
}



void Original_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide,int free) // 기존 데이터 블록에서 프리블록으로 유효데이터 합병
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

void Log_to_Free(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free) //로그블록의 유효데이터 프리블록으로 합병
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

void Log_to_Free2(MEMORY *memory, TABLE *table, int sector, int offset, int hide, int free) //로그블록의 유효데이터 프리블록으로 합병
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

int LogBlock_Check(MEMORY *memory, TABLE *table, int hide, int pbn) //로그블록으로 다른블록이 들어오는걸 방지하기위한 체크
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

int freeBlock_Check(MEMORY *memory, TABLE *table, int sector) //마지막 블록부터 데이터가 비어있는 블록 선정
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

void Flash_write(MEMORY *memory, TABLE *table, int sector,int pbn, char data[]) // 쓰기 (sector는 매핑테이블의 psn의 값)
{

	strcpy(memory[sector].memory, data);
	memory[sector].spare = pbn;
	write_Allcount++;
	printf("write(%d,%s) Success ! \n", sector, data);
	printf("\n");
}


void Flash_erase(MEMORY *memory, TABLE *table, int sector) // 데이터가 있을때 지우기연산 (sector는 매핑테이블의 psn의 값)
{

	int i;


	for (i = (sector / SECTOR_SIZE) * SECTOR_SIZE; i < ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; i++) //블록단위로
	{
		strcpy(memory[i].memory, "-1"); //데이터 지우기(초기화)
	}
	erase_Allcount++;
	//지워진 블록과 블록을포함하는 섹터 출력
	printf("Block %d (psn%d ~ psn%d)  Erase\n", sector / SECTOR_SIZE, (sector / SECTOR_SIZE) * SECTOR_SIZE, ((sector / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE - 1);
	printf("\n");
}

void Flash_erase_spare(MEMORY *memory, TABLE *table, int sector) // 데이터가 있을때 지우기연산 (sector는 매핑테이블의 psn의 값)
{

	int i;


	for (i = (sector / 32) * 32; i < ((sector / 32) * 32) + 32; i++) //블록단위로
	{
		memory[i].spare = -1;
	}
	//지워진 블록과 블록을포함하는 섹터 출력

}

void FTL_read(MEMORY *memory, TABLE *table, int block) //FTL 읽기
{
	int lsn, map_lbn;
	int i;
	int offset = 0;
	printf("Input lsn:");
	scanf("%d", &lsn); //논리주소 입력
	printf("\n");


	map_lbn = lsn / SECTOR_SIZE; //입력받은값을 블록당 섹터 개수 나눈값으로 매핑테이블의 논리블록을 구함
	offset = lsn % SECTOR_SIZE; //오프셋은 블록당 섹터 개수로 나눈 나머지 (오프셋으로 섹터의 번호를 찾음)

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn) //논리주소에 대응하는 psn을 찾기위함
		{
			Flash_backRead(memory, table, block*SECTOR_SIZE,table[i].pbn,offset);
		}
	}
	printf("\n");
}
void Flash_backRead(MEMORY *memory, TABLE *table, int sector,int pbn, int offset) //데이터 백스캔
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

void Flash_read(MEMORY *memory, TABLE *table, int sector) // 읽기
{

	printf("psn : %d\n", sector);
	printf("data : %s\n", memory[sector].memory);

}

void FTL_write_FILE(MEMORY *memory, TABLE *table, int block, int lsn, char data[]) //FTL 쓰기
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

	if (strlen(data) >= SIZE) //입력한 데이터크기 예외처리
	{
		printf("data size ERROR!!\n");
		return 0;
	}

	map_lbn = lsn / SECTOR_SIZE; //입력받은값을 블록당 섹터 개수로 나눈값으로 매핑테이블의 논리블록을 구함
	offset = lsn % SECTOR_SIZE; //오프셋은 블록당 섹터 개수로 나눈 나머지 (오프셋으로 섹터의 번호를 찾음)

	for (i = 0; i < block + 1; i++)
	{
		if (table[i].lbn == map_lbn)
		{
			if (strcmp(memory[(table[i].pbn * SECTOR_SIZE) + offset].memory, "-1") == 0)
			{
				Flash_write(memory, table, table[i].pbn*SECTOR_SIZE + offset, table[i].pbn, data);
				memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = table[i].pbn; //스페어영역에 블록번호 표시
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
						memory[(table[i].pbn*SECTOR_SIZE) + offset].spare = block; //기존의 무효데이터확인을위해
						memory[hide + offset].spare = table[i].pbn; //유효데이터는 로그블록에
						break;
					}
					else
					{
						free_block_pbn = freeBlock_Check(memory, table, sector);
						Original_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //기존 데이터 블록의 유효데이터를 프리블록으로 
						Log_to_Free(memory, table, table[i].pbn*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // 로그블록의 유효데이터를 프리블록으로 합병
						Flash_erase(memory, table, table[i].pbn*SECTOR_SIZE); // 기존 데이터블록 지우기
						Flash_erase_spare(memory, table, table[i].pbn*SECTOR_SIZE);
						Flash_erase(memory, table, hide); //로그블록 지우기
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
					Original_to_Free(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); //기존 데이터 블록의 유효데이터를 프리블록으로 
					Log_to_Free2(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE, offset, hide, free_block_pbn*SECTOR_SIZE); // 로그블록의 유효데이터를 프리블록으로 합병
					Flash_erase(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE); // 기존 데이터블록 지우기
					Flash_erase_spare(memory, table, (memory[log_block_pbn].spare)*SECTOR_SIZE);
					Flash_erase(memory, table, hide); //로그블록 지우기
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

	printf("Write_AllCount : %d\n", write_Allcount); //쓰기연산 횟수 
	printf("Erase_AllCount : %d\n", erase_Allcount); //지우기 연산 횟수
	printf("\n");
}