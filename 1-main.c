#include <stdio.h>

int main(void)
{
	
	int x[] = {2, 4, 6, 8, 12, 23, -2, 0};
	int sum = 0;

	for (int i = 0; i <= 8; i++)
	{

		sum += x[i];
	

	}	


	printf("%d", sum);

	return 0;
}
