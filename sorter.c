#include <stdio.h>
#include <stdlib.h>

int array[1024];
int counter = 0;

void syntax ()
{
    printf("SYNTAX: \n\n");
    printf("sorter <filein.txt> <fileout.txt>");
    printf("Sorts integers in filein.txt and write them in fileout.txt");
    exit(1);
}

void error (const char* msg)
{
    printf("ERROR: %s\n", msg);
    exit(1);
}

int cmpfunc (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}

void read_ints (const char* filename)
{
  FILE* file = fopen (filename, "r");
  if(file == NULL){
      error("Cannot open filein.txt");
  }
  
  int i = 0;
  fscanf (file, "%d", &i);    
  while (!feof (file))
    {  
      array[counter] = i;
      counter++;
      fscanf (file, "%d", &i);      
    }
  fclose (file);        
}

void write_ints (const char* filename)
{
  FILE* file = fopen (filename, "w");
  if(file == NULL){
      error("Cannot open fileout.txt");
  }

  int i = 0;
  while (i < counter)
    {  
      fprintf (file, "%d\n", array[i]);
      i++;      
    }
  fclose (file);        
}

int main (int argc, char* argv[])
{
    if(argc != 3){
        syntax();
    }

    read_ints(argv[1]);
    qsort(array, counter, sizeof(int), cmpfunc);
    write_ints(argv[2]);

    return 0;
}