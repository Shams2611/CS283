#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len) {
    if (user_str == NULL || buff == NULL) { // Conditions to implement null pointer error
        return -2;
    }
    int write_index = 0;
    int readIndex = 0;
    int user_str_len = 0;
    int lastWasSpace = 1;
    while (user_str[readIndex] == ' ' || user_str[readIndex] == '\t') {
        readIndex++;
    }
    
    while (user_str[readIndex] != '\0' && write_index < len) {
        if (user_str[readIndex] == ' ' || user_str[readIndex] == '\t') { // checking to see white space or tab then adding to the buff as a single one
            if (!lastWasSpace) {
                buff[write_index++] = ' ';
                lastWasSpace = 1;
            }
        } else {
            buff[write_index++] = user_str[readIndex];
            lastWasSpace = 0;
        }
        readIndex++;
    }
    
    user_str_len = write_index;

    while (write_index < len) {
        buff[write_index++] = '.';
    }

    return user_str_len;
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len) {
    if (buff == NULL || str_len < 0 || str_len > len) {
        return -1; // Error if buffer is null or str_len is invalid
    }

    int wordCount = 0;
    int i = 0;

    while (i < str_len) {
        // Skip leading spaces and tabs
        while (i < str_len && (buff[i] == ' ' || buff[i] == '\t')) {
            i++;
        }

        // If a non-space character is found, it's the start of a word
        if (i < str_len && buff[i] != ' ' && buff[i] != '\t') {
            wordCount++;
        }

        // Skip characters of the current word
        while (i < str_len && buff[i] != ' ' && buff[i] != '\t') {
            i++;
        }
    }

    return wordCount;
}

int countChars(char *buff, int len, int str_len) {
    if (buff == NULL || str_len < 0 || str_len > len) {
        return -1;
    }
    int charCount = 0;
    for (int i = 0; i < str_len; i++) {
        charCount++;
    }
    return charCount;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

void charSwap(char *a, char *b){
    char temp = *a;
    *a = *b;
    *b = temp;
}
// two pointer approach to reverse the characters in buff while calling the charSwap function
void reverseString(char *str, int len){
    int i = 0;
    int j = len - 1;
    while (i < j){
        charSwap(&str[i], &str[j]);
        i++;
        j--;
    }
}

// Function to print words in order.
void wordPrint(char* buff, int len)
{
    int words_count = count_words(buff, len, len);  
    int char_count = 0; 
    int index = 0;       

    printf("Word Print\n");
    printf("----------\n");

    // Loop through the words using words_count.
    for (int i = 0; i < words_count; i++){
        printf("%d. ", i + 1);  
        while (buff[index] != ' ' && index != len)
        {
            printf("%c", buff[index]);
            index++;
            char_count++;
        }

        printf(" (%d)\n", char_count);  
        char_count = 0;  
        // Skip over any spaces between words.
        while (buff[index] == ' ')
        {
            index++;
        }}
    printf("\n");
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    // It is safe as it uses the first codition to which is checked first and only when it is dismissed it will try to access argv[1]. Segmentation fault does not occur and the argument gets passed, ensuring at least one argument and it starts with '-'.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    // This check makes sure that an input string is provided as an argument. Here, without the three argument of argv it will not run. This prevents accessing undefined memory that would occur if we tried to use argv[2] when it wasn't provided.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ * sizeof(char));
    if (buff == NULL) {
        printf("Failed to allocate memory\n");
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    printf("%d\n", user_str_len);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        case 'r':
            reverseString(buff,user_str_len);
            break;
        case 'w':
            wordPrint(buff, user_str_len);
            break;
        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
// 1) Prevents buffer overflows due to explicitly stated boundaries.
// 2) The pointer allows easy manipulation of data for the buffer
// 3) Passing both buffer and function allows functions to work with buffers of any size.
// 4) Enables functions to validate their inputs by ensuring operations don't exceed buffer bounds. 