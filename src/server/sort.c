#include <server/sort.h>
#include <shared/debug.h>


void
Swap(
	uint16_t* a,
	uint16_t* b
	)
{
	uint16_t Temp = *a;
	*a = *b;
	*b = Temp;
}


int32_t
Partition(
	uint16_t* Array,
	int32_t Low,
	int32_t High
	)
{
	uint16_t x = Array[High];
	int32_t j = Low - 1;

	for(int32_t i = Low; i <= High - 1; ++i)
	{
		if(Array[i] <= x)
		{
			++j;
			Swap(Array + j, Array + i);
		}
	}

	++j;
	Swap(Array + j, Array + High);

	return j;
}



void
QuickSort(
	uint16_t* Array,
	int32_t len
	)
{
	if(len <= 1)
	{
		return;
	}

	int32_t Low = 0;
	int32_t High = len - 1;

	int32_t Stack[High - Low + 1];
	Stack[0] = Low;
	Stack[1] = High;

	int32_t* top = Stack + 2;

	do
	{
		High = *(--top);
		Low = *(--top);

		int32_t Pivot = Partition(Array, Low, High);

		if(Pivot - 1 > Low)
		{
			*(top++) = Low;
			*(top++) = Pivot - 1;
		}

		if(Pivot + 1 < High)
		{
			*(top++) = Pivot + 1;
			*(top++) = High;
		}
	}
	while(top != Stack);
}
