#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int counter;

int main(void) {

counter = 0x0;
for(;;)
{
printf("\r%08x",counter++);

}

return EXIT_SUCCESS;

}
