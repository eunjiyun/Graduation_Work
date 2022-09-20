#include "MemPool.h"

class CDataMP : public MemoryPool<CDataMP>
{
public:
	BYTE a[1024];
	BYTE b[1024];
};

class CDataMP2
{
public:
	BYTE a[1024];
	BYTE b[1024];
};

int main()
{
	int FOR_COUNT = 1000000;
	clock_t start_time, end_time;

	start_time = clock();

	for (int i = 0; i < FOR_COUNT; i++)
	{
		CDataMP* pData = new CDataMP();
		delete pData;
	}

	end_time = clock();
	printf("Time:%f\n", ((double)(end_time - start_time)) / CLOCKS_PER_SEC);

	start_time = clock();

	for (int i = 0; i < FOR_COUNT; i++)
	{
		CDataMP2* pData = new CDataMP2();
		delete pData;
	}

	end_time = clock();
	printf("Time:%f\n", ((double)(end_time - start_time)) / CLOCKS_PER_SEC);

	// 전체적으로 약 4배 정도의 속도 차이가 난다.
}