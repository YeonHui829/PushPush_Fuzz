#include <stdio.h>
#include <string.h>

#ifdef SYMBOL_NAME
int main(){
    fprintf(stderr, "main1\n");
}
#endif

#ifdef SYMBOL_NAME2
int main(){
    fprintf(stderr, "main2\n");
}
#endif