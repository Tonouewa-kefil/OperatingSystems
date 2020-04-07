/* Sudoku Solution Validator
 *Create (9) threads that checks that each column contains the digits 1 through 9.
 *Create (9) threads that checks that each row conatins the digits 1 through 9.
 *Create (9) threads to check that each of the (3 x 3) subgrids contains the digits 1 through 9.
 */



#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define T_NUM 9


void* runner(void*);//this is ran by the thread.
int CountDuplicate_col (int);/*Checks if there is any duplicate in the col number given an an input*/
bool oneToNine_col(int);/*Checks if the number in the column are from one to nine*/
int CountDuplicate_row (int);/*Checksif there is any duplicate in the row number given as an input*/
bool oneToNine_row(int);/*Checks if the number in the row are from one to nine*/
int CountDuplicate_three_by_three(int,int,int);/*Checks if the three by three array specified has a duplicate*/
bool oneToNine_three_by_three(int,int);/*Checks if the three by three array contains numbers only ranging from one to nine*/
bool popSudoku(FILE*);//This will covert the file to the sudoku

int Sudoku_Board[9][9]; 

//This array will be used to tell whether the row and column is valide of not
bool valid[T_NUM]={true,true,true,true,true,true,true,true,true};
bool valid2[T_NUM]={true,true,true,true,true,true,true,true,true};

//This will be used to pass argument to the runner function
typedef struct
{
	int row;
	int column;
	int count;
}parameters;

int main(){
	int i,j;
	FILE * infile;//source file for reading sudoku
	infile = fopen("input.txt","r");
	
	if(!popSudoku(infile)) {
		printf("Error Reading Sudoku from File.\n");
		return 1;
	}
	
	//This is where the code starts for real
	int threadNumbers;
	pthread_t tid[T_NUM * 2];
	pthread_t tid2[T_NUM];
	pthread_attr_t attr;
	bool isRowColValid = true;


	//column creation
	//We replace row by -1 since we are working with cols
	//Make the column number equal to the index of the for loop
	//We make count -1 because there is no 3 by 3
	for(threadNumbers = 0; threadNumbers<T_NUM;  threadNumbers++)
	{	
		pthread_attr_init(&attr);
		parameters* rowcol = (parameters *)malloc(sizeof(parameters));	
		rowcol->row = -1;
		rowcol->column = threadNumbers;
		rowcol->count = -1;	
		pthread_create(&tid[threadNumbers],&attr,runner,rowcol);

	}

	//row creation
	//We replace col by -1 since we are working with rows
	//Make the row number equal to the index of the for loop
	for(threadNumbers = T_NUM; threadNumbers<(T_NUM*2); threadNumbers++)
	{	
		pthread_attr_init(&attr);
		parameters* rowcol = (parameters *)malloc(sizeof(parameters));	
		rowcol->row = threadNumbers-T_NUM;
		rowcol->column = -1;
		rowcol->count = -1;
		pthread_create(&tid[threadNumbers],&attr,runner,rowcol);

	}

	//join the thread
	for(threadNumbers = 0; threadNumbers<(2*T_NUM); threadNumbers++)
	{	
		pthread_join (tid[threadNumbers],NULL);
	}
	

	//Checking if all the conditions are satisfied for the row and columns
	for (threadNumbers = 0; threadNumbers<T_NUM; threadNumbers++)
	{
		isRowColValid = isRowColValid & valid[threadNumbers];	
	}

	//if the conditions for row and columns are satified, we will proceed to do the thread for the three by three
	//Else for eficiency purpose, we need to stop the process and tell the user that the sudoku is not a valid one.
	if (isRowColValid)
	{
		int count = 0;
		for(i = 0; i < T_NUM; i++)
		{
			for(j = 0; j < T_NUM; j++)
			{
				// If these indecies are divisible by 3 then this is something we want to take a 3 by 3 box and verify for
				if((i%3 == 0) && (j%3 == 0))
				{
					pthread_attr_init(&attr);
					parameters* rowcol = (parameters *)malloc(sizeof(parameters));
					/*Here we will have row and column (0,0);(0,3);(0,6);(3,0);(3,3);(3,6);(6,0);(6,3);(6,6)*/
					rowcol->row = i;
					rowcol->column = j;
					rowcol->count = count;

					pthread_create(&tid2[count],&attr,runner,rowcol);
					count++;
				}
			}
		}
		//join the threads (ie. wait for them)
		for(threadNumbers = 0; threadNumbers<T_NUM; threadNumbers++)
		{	
			pthread_join (tid2[threadNumbers],NULL);
		}

		// Assigning a solid boolean value as to whether the 3 by 3 boxes are valid
		for (threadNumbers = 0; threadNumbers<T_NUM; threadNumbers++)
		{
			isRowColValid = isRowColValid & valid2[threadNumbers];	
		}

		if(isRowColValid) {
			printf("CONGRATS, this is a valid sudoku.\n");
		}else {
			printf("This is not a valid sudoku.\n");
		}

	}
	else { 
		printf("The is not a valid sudoku.\n");
	}

	return 0;
}

bool popSudoku(FILE* infile) {
	int i, j;
	i = j = 0;
	char *line = NULL;
	char *num = NULL;
	size_t n = 0;
	bool fileGood = true;

	if (infile != NULL)
	{
		while(getline(&line, &n, infile) != -1)
		{	
			FILE * stream;
			stream = fmemopen(line,strlen(line),"r");//converting the line into a stream
			while(getdelim(&num, &n , ' ', stream) !=-1)//getting a character (number) before the space 
			{	
				Sudoku_Board[i][j] = atoi(num);//populating the Sudoku board
				j++;
			}
			j=0;
			i++;
		}

		free(line);//freeing line (This is crutial!!!)

		printf("This is the Sudoku Board: \n");
		for(i=0;i<9;i++)
		{
			for(j=0;j<9;j++)
			{
				printf("%d ",Sudoku_Board[i][j]);
				
			}

			printf("\n");
		}
			
		fclose(infile);
		//exit(EXIT_SUCCESS);
	}
	else
	{
		printf("File not properly read!! \n");
		fileGood = false;
	}
	return fileGood;
}


int CountDuplicate_col (int col)
{
	int i;
	int j;
	int count = 0;
	bool validCol = false;
	//We traverse the col  and check for duplicate.
	//if there is a duplicate, we increment the count value.
	for(i=0; i<9; i++)
	{
		for(j=i+1; j<9; j++)
		{
			/* If duplicate found then increment count by 1 */
			if(Sudoku_Board[i][col] == Sudoku_Board[j][col])
			{
				count++;
				break;
			}
		}
	}
	
	//if the count is 0 then the col is valid(True)
	if(count == 0)
	{
		validCol = true;
	}	
	//we and the validcol with the value contained in the valid array 
	valid[col] = valid[col] & validCol;
	return count;

}

int CountDuplicate_row (int row)
{	
	int i;
	int j;
	int count = 0;
	bool validRow = false;	
	//We traverse the row  and check for duplicate.
	//if there is a duplicate, we increment the count value.
	for(i=0; i<9; i++)
	{
		for(j=i+1; j<9; j++)
		{
			/* If duplicate found then increment count by 1 */
			if(Sudoku_Board[row][i] == Sudoku_Board[row][j])
			{
				count++;
				break;
			}
		}
	}

	//if the count is 0 then the row is valid(True)
	if(count == 0)
	{
		validRow = true;
	}

	//we and the validRow with the value contained in the valid array 
	valid[row] = valid[row] & validRow;

	return count;

}

bool oneToNine_col(int col)
{
	bool allOneToNine = true;
	bool OneToNine = false;
	int i;
	//We traverse the array (column of the sudoku) and check if all the
	//the numbers are between 1 and 9.
	//If valid, we set OneToNine to true and false if it is not valid	
	for(i=0; i<9; i++)
	{
		if (Sudoku_Board[i][col]>0 && Sudoku_Board[i][col]<=9)
		{
			OneToNine = true;
		}
		else 
		{
			OneToNine = false;
		} 

		allOneToNine = allOneToNine & OneToNine;

	}
	//all OneToNine is anded with the value present in the valid array.	
	valid[col] = valid[col] & allOneToNine;
	return allOneToNine;

}

bool oneToNine_row(int row)
{
	bool allOneToNine = true;
	bool OneToNine = false;
	int i;

	//We traverse the array (row of the sudoku) and check if all the
	//the numbers are between 1 and 9.
	//If valid, we set OneToNine to true and false if it is not valid
	for(i=0; i<9; i++)
	{
		if (Sudoku_Board[row][i]>0 && Sudoku_Board[row][i]<=9)
		{
			OneToNine = true;
		}
		else 
		{
			OneToNine = false;
		} 

		allOneToNine = allOneToNine & OneToNine;

	}
	//all OneToNine is anded with the value present in the valid array.
	valid[row] = allOneToNine & valid[row];
	return allOneToNine;
}

int CountDuplicate_three_by_three(int row, int col, int countInArray)
{
	int i;
	int j;
	int count = 0;
	int AuxiliaryArr[T_NUM];
	int k = 0;
	bool isValid3by3 = false;
	//We transform the 3 by 3 table into an unidementional array 
	//in order to facilitate how we traverse the table.
	for (i=row; i<row+3 ; i++)
	{
		for(j=col;j<col+3;j++)
		{
			AuxiliaryArr[k] = Sudoku_Board[i][j];
			k++;
		}
	}
	
	//We traverse the array and check for duplicate.
	//if there is a duplicate, we increment the count value.
	for(i=0; i<T_NUM; i++)
	{
		for(j=i+1; j<T_NUM; j++)
		{
			/* If duplicate found then increment count by 1 */
			if(AuxiliaryArr[i] == AuxiliaryArr[j])
			{
				count++;
				break;
			}
		}
	}

	//if count is 0, we our valid boolean array a specific index to true.
	//else we set it to false.
	if(count == 0)
	{
		isValid3by3 = true;
	}

	valid2[countInArray] = isValid3by3;
	return count;
}

void* runner(void* param)
{
	parameters* rowcol = (parameters*) param; 
	
	/*In parameter when the row is not -1 and column is not -1
	  we will be checking fo the validity of a 3x3 table
	  by checking for duplicate since we already confirm that all
	  the rows and columns have number between 1-9.
	*/
	if(rowcol->row != -1 && rowcol->column !=-1)
	{

		CountDuplicate_three_by_three(rowcol->row, rowcol->column, rowcol->count);

	}
	
	/*In parameter when the row is -1 and column is not -1
	  we will be checking fo the validity of the column by running
	  the function that checks for number between 1-9 and the function 
	  that checks for duplicate.
	*/
	else if (rowcol->row == -1)
	{
		oneToNine_col(rowcol->column);
		CountDuplicate_col(rowcol->column);		
	}

	/*In parameter when the row is not -1 and column is -1
	  we will be checking fo the validity of the row by running
	  the function that checks for number between 1-9 an the function 
	  that checks for duplicate
	*/	
	else if (rowcol->column == -1)
	{	
		oneToNine_row(rowcol->row);
		CountDuplicate_row(rowcol->row);
	}

	free (param);
	pthread_exit(0);
}
