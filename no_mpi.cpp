// Above program without openmpi
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHAR 1000

void removePunctuation(char* chunk) {
    // move forward the character if previous character is non-alphabet
    int count = 0;
    int i = 0, j = 0;
    for (i = 0; chunk[i] != '\0'; i++) {
        if ((chunk[i] >= 'a' && chunk[i] <= 'z') || (chunk[i] >= 'A' && chunk[i] <= 'Z')) {
            chunk[j] = chunk[i];
            j++;
        }
    }

    // add null character at the end of the string
    chunk[j] = '\0';
}

void convertToLower(char* chunk) {
    int i = 0;
    while (chunk[i] != '\0') {
        if (chunk[i] >= 'A' && chunk[i] <= 'Z') {
            chunk[i] = chunk[i] + 32;
        }
        i++;
    }
}

int main(int argc, char* argv[]) {
    FILE* fp;
    char str[MAXCHAR];
    char* filename = argv[1];
    char* outfilename = argv[2];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open file %s ", filename);
        return 1;
    }

    int freq[26] = {0};
    while (fgets(str, MAXCHAR, fp) != NULL) {
        removePunctuation(str);
        convertToLower(str);
        int i = 0;
        while (str[i] != '\0') {
            if (str[i] >= 'a' && str[i] <= 'z') {
                freq[str[i] - 'a']++;
            }
            i++;
        }
    }

    fclose(fp);

    // print out freqsum
    printf("-----freqsum------\n");
    for (int i = 0; i < 26; i++) {
        printf("%c: %d ", 'a' + i, freq[i]);
    }

    // write out to the file
    char outbuf[1000];
    int index = 0;
    for (int i = 0; i < 26; i++) {
        index += sprintf(outbuf + index, "%c: %d \n", 'a' + i, freq[i]);
    }

    FILE* outfp;
    outfp = fopen(outfilename, "w");
    if (outfp == NULL) {
        printf("Could not open file %s ", outfilename);
        return 1;
    }
    fprintf(outfp, "%s", outbuf);
    fclose(outfp);

    return 0;
}
