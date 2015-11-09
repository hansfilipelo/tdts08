#define TAB_SIZE        1048576   /*  1 MB */
#define CACHE_SIZE	  16384   /* 16 kB */

int main(void) {

    int		A[TAB_SIZE];
    int		sum = 0;
    int		i, j;

    /*
     * Initialization
     */
    sum = 0;

    for (i = 0; i < TAB_SIZE; i++) {
	A[i] = 1;
    }

    /*
     * Data referencing
     */
    for (i = 0; i < TAB_SIZE; i++) {
	for (j = 0; j < 5; j++) {
	    sum += A[i];
	}
    }

    /* Compensation for functionality */
    for (i = CACHE_SIZE; i < TAB_SIZE - CACHE_SIZE; i++) {
	for (j = 0; j < 5; j++) {
	    sum += A[i];
	}
    }

    printf("Sum = %d\n", sum);

    return 0;
}

