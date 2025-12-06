#include <stdio.h>
#include <string.h>



int main() {
    // printf("hello mini-shell world\n");
    while(1){
    char read[1000];
    printf("$ ");
    if (fgets(read, sizeof(read), stdin) != NULL) {
        read[strcspn(read, "\n")] = '\0';  // Remove the newline character
        printf("You entered: %s", read);
    }
    }
    return 0;
}
