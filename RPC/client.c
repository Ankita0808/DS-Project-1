#include "header.h"
#include <math.h>
#include <time.h>

typedef unsigned long long timestamp_t;

static timestamp_t get_timestamp()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}


void init(float *a, int m, int n)
{
	int i, j, count=0;
	
	for (i=0; i<m; i++)
		for (j = 0; j<n; j++)
			{
				(*(a+i*n+j)) = j;
			}
}

int testMultiply(int m,int n,int l) {
	char *program_name;
	int program_version, i,j;

	
	float a[m][n], b[n][l], c[m][l];
	init((float *)a,m,n);
	init((float *)b,n,l);
	
	program_name = "matrix";
	program_version = 1;
	
	printf("\n/***********MULTIPLY*********/\n");
	
	if (multiply(program_name, program_version, (float *)a, (float *) b, (float *)c, m, n, l) == 0)
		return 0;
	printf("Result matrix is: \n");
	
	for (i=0 ; i< m; i++)
	{
		for (j = 0; j < l; j++)
		{
			printf("%.0f ", c[i][j]);
		}
		printf("\n");
	}		
	
	
	printf("\n/***********END OF MULTIPLY OPERATION*********/\n");	
return 1;
}

int testSort(int size) {
	char *program_name;
	int program_version, i,j;

	float a[size], b[size];
	init((float *)a,1,size);
	
	program_name = "matrix";
	program_version = 1;
	
	printf("\n/***********SORT*********/\n");
	if (sort(program_name, program_version, (float *)a, (float *)b, size) == 0)
		return 0;
	printf("Sorted array is: \n");
	for (i=0; i< size; i++)
		{
				printf ("%.0f ",b[i]);
		}
	printf("\n/***********END OF SORTING*********/\n");
return 1;
}

int testMin(int size) {
	char *program_name;
	int program_version, i,j;
	
	float a[size], b;
	init((float *)a,1,size);
	
	program_name = "matrix";
	program_version = 1;
	
	printf("\n/***********MIN*********/\n");
	if (min(program_name, program_version, (float *)a, &b, size) == 0)
		return 0;
	printf ("MIN is: %.0f \n", b);
	printf("\n/***********END OF MIN OPERATION*********/\n");
return 1;
}

int testMax(int size) {
	char *program_name;
	int program_version, i,j;

	
	float a[size], b;
	init((float *)a,1,size);
	
	program_name = "matrix";
	program_version = 1;
	
	printf("\n/***********MAX*********/\n");
	if (max(program_name, program_version, (float *)a, &b, size) == 0)
		return 0;
	printf ("MAX is: %.0f \n", b);
	printf("\n/***********END OF MAX OPERATION*********/\n");
return 1;
}

int main()
{
	int choice = 0, times = 10; 
	int m, n, l;
	timestamp_t begin, end;
	double time_spent;
	
	int i = 0;
	double a[times];
	double standard= 0, average = 0;
	
	while (choice != 5)
	{
	printf("Enter your choice:\n1: Multiply\n2: Sort\n3: Min\n4: Max\n5: Quit\n");
	scanf("%d",&choice);
	if (choice == 1) {
	printf("Enter two matrix sizes for a[m][n] and b[n][l]: m, n, l\n");
	scanf("%d %d %d", &m, &n, &l);
	testMultiply(m,n,l);
	}
	else if (choice == 2) {
	printf("Enter vector size\n");
	scanf("%d", &m);
	
	
	testSort(m);
	
	}
	else if (choice == 3) {
	printf("Enter vector size\n");
	scanf("%d", &m);
	
	testMin(m);
	
	}
	else if (choice == 4) {
	printf("Enter vector size\n");
	scanf("%d", &m);
	testMax(m);
	}
	}
	return 0;
}