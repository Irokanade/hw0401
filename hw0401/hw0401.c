//
//  hw0401.c
//  hw0401
//
//  Created by michaelleong on 09/05/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

#define MIN_SIZE 1000

struct option long_options[] = {
    {"split", 1, NULL, 's'},
    {"recover", 1, NULL, 'r'},
    {"size", 1, NULL, 128}, //ascii has 127
    {"help", 0, NULL, 129},
    { 0, 0, 0, 0},
};

void printHelp(void);

int main(int argc, char *argv[]) {
    /*
    printf( "argc: %d\n", argc );
    
    
    for( int32_t i = 0 ; i < argc ; i++ )
    {
        printf( "argv[%d]: %s\n", i, argv[i] );
    }
    
    puts( "---" );
     */
    
    // getopt_long
    
    int32_t c = 0;
    int32_t index = 0;
    char fileName[129] = {0};
    char outFileName[129] = {0};
    long int size = 0;
    char *pEnd = NULL;
    uint8_t split = 0; //split option
    uint8_t recover = 0; //recover option
    
    while ( ( c = getopt_long( argc, argv, "s:r:", long_options, &index ) ) != -1 )
    {
        //printf( "index: %d\n", index );
        switch( c )
        {
            case 's':
                //printf("option: -s, %s\n", optarg);
                split = 1; //split option true
                strncpy(fileName, optarg, strlen(optarg));
                //printf("fileName: %s\n", fileName);
                break;
            case 'r':
                //printf("option: -r, %s\n", optarg);
                recover = 1;
                strncpy(outFileName, optarg, strlen(optarg));
                //printf("output file name: %s\n", outFileName);
                break;
            case 128:
                //printf("option --size, %s\n", optarg);
                size = strtol(optarg, &pEnd, 10);
                if(pEnd == NULL) {
                    printf("Conversion unsuccessful\n");
                    return 1;
                }
                //printf("size: %ld\n", size);
                break;
            case 129:
                printHelp();
                return 0;
            case '?':
                //printf( "option: ?, %s\n", optarg );
                printf("invalid option use '--help' for more options\n");
                return 1;
            default:
                //printf( "option: unknown\n" );
                printf("invalid option use '--help' for more options\n");
                return 1;
        }
    }
    
    int smolStartingIndex = optind;
    
    for (size_t i = smolStartingIndex; i < argc; i++) {
        //remaining arguements are assumed to be input files
        //printf("non options: %s\n", argv[i]);
    }
    
    if(split) {
        //if split option
        //check if size is valid
        if(size < MIN_SIZE) {
            printf("error size is less than %d\n", MIN_SIZE);
            printf("default small size is %d\n", MIN_SIZE);
            size = MIN_SIZE;
        }
        
        FILE *oriFile = NULL;
        long int oriSize = 0;
        FILE **smallFileList = NULL; //array of small file pointers
        long double numberOfPartitions = 0;
        uint8_t buffer;
        
        if((oriFile = fopen(fileName, "rb")) == NULL) {
            printf("File open error\n >:[\n");
            return 1;
        }
        
        fseek(oriFile, 0, SEEK_END);
        oriSize = ftell(oriFile);
        rewind(oriFile);
        
        //printf("oriSize: %ld\n", oriSize);
        
        numberOfPartitions = (long double)oriSize/size;
        //printf("number of partitions: %Lf\n", numberOfPartitions);
        if(fmod(numberOfPartitions, 1.0) != 0) {
            numberOfPartitions = (int)numberOfPartitions;
            numberOfPartitions++;
            //printf("number of partitions: %Lf\n", numberOfPartitions);
        }
        
        //printf("sizeofchar: %ld\n", sizeof(char));
        smallFileList = (FILE**)malloc(sizeof(FILE*)*numberOfPartitions);
        //buffer = (unsigned char*)malloc(sizeof(unsigned char)*size);
        
        //open all the files as write
        for(size_t i = 0; i < numberOfPartitions; i++) {
            char fileNameTemp[129] = {0};
            char intChar[2] = {0};
            intChar[0] = i+1+'0';
            strncat(fileNameTemp, intChar, 1);
            strncat(fileNameTemp, fileName, strlen(fileName));
            
            printf("%s ", fileNameTemp);
            
            //printf("fileNameTemp: %s\n", fileNameTemp);
            if((smallFileList[i] = fopen(fileNameTemp, "wb")) == NULL) {
                printf("Error opening files\n");
                return 1;
            }
        }
        
        printf("\n");
        
        for(size_t i = 0; i < oriSize; i += size) {
            //first write the numbering
            size_t cardinal = i/size;
            fwrite(&cardinal, sizeof(i), 1, smallFileList[i/size]);
            
            for(size_t j = 0; j < size && (i+j < oriSize); j++) {
                //read byte by byte from the original file until size is reached
                buffer = fgetc(oriFile);
                //printf("%x", buffer);
                
                fwrite(&buffer, 1, 1, smallFileList[i/size]);
            }
            //printf("\n");
        }
        
        
        //free all pointers
        free(smallFileList);
        
        //close all the files
        for(size_t i = 0; i < numberOfPartitions; i++) {
            fclose(*(smallFileList+i));
        }
        fclose(oriFile);
        
    } else if(recover){
        //else if recover option
        //if both options only perform split
        FILE *outFile = NULL;
        FILE **smallFileList = NULL; //array of small file pointers
        int numOfSmallFiles = 0;
        
        if((outFile = fopen(outFileName, "wb")) == NULL) {
            printf("Error opening output file\n");
            return 1;
        }
        
        numOfSmallFiles = argc - smolStartingIndex;
        //printf("numOfSmallFile: %d\n", numOfSmallFiles);
        if(numOfSmallFiles > 0) {
            smallFileList = (FILE**)malloc(sizeof(FILE*)*numOfSmallFiles);
        } else {
            printf("error as there is no small files\n");
            return 1;
        }
        
        for(size_t i = 0; i < numOfSmallFiles; i++) {
            //open all the files
            //printf("small file: %s\n", argv[i+smolStartingIndex]);
            if((smallFileList[i] = fopen(argv[i+smolStartingIndex], "rb")) == NULL) {
                printf("Error opening files\n");
                return 1;
            }
        }
        
        
        //sort small file list by cardinal
        for(size_t i = 0; i < numOfSmallFiles; i++) {
            size_t cardinalj = 0;
            size_t cardinaljplus1 = 0;
            
            for(size_t j = 0; j < numOfSmallFiles-1; j++) {
                rewind(smallFileList[j]);
                rewind(smallFileList[j+1]);
                fread(&cardinalj, sizeof(size_t), 1, smallFileList[j]);
                fread(&cardinaljplus1, sizeof(size_t), 1, smallFileList[j+1]);
                
                //printf("cardinalj: %lu\n", cardinalj);
                //printf("cardinaljplus1: %lu\n", cardinaljplus1);
                
                if(cardinalj > cardinaljplus1) {
                    //sort file pointers
                    FILE *temp = NULL;
                    temp = smallFileList[j];
                    smallFileList[j] = smallFileList[j+1];
                    smallFileList[j+1] = temp;
                    
                    //sort argv
                    char nameTemp[129] = {0};
                    strncpy(nameTemp, argv[j+smolStartingIndex], strlen(argv[j+smolStartingIndex]));
                    strncpy(argv[j+smolStartingIndex], argv[j+1+smolStartingIndex], strlen(argv[j+1+smolStartingIndex]));
                    strncpy(argv[j+1+smolStartingIndex], nameTemp, strlen(nameTemp));
                }
            }
        }
        
        for(size_t i = 0; i < numOfSmallFiles; i++) {
            //printf("%s ", argv[i+smolStartingIndex]);
        }
        
        for(size_t i = 0; i < numOfSmallFiles; i++) {
            rewind(smallFileList[i]);
            fseek(smallFileList[i], 0, SEEK_END);
            size_t smallFileSize = ftell(smallFileList[i]) - sizeof(size_t);
            fseek(smallFileList[i], sizeof(size_t), SEEK_SET);
            for(size_t j = 0; j < smallFileSize; j++) {
                char charTemp;
                charTemp = fgetc(smallFileList[i]);
                fputc(charTemp, outFile);
            }
        }
        
        //free all pointers
        free(smallFileList);
        
        //close all the files
        for(size_t i = 0; i < numOfSmallFiles; i++) {
            fclose(*(smallFileList+i));
        }
        fclose(outFile);
    }
    
    return 0;
}

void printHelp() {
    printf("___to split files___\n");
    printf("use './hw0401 -s [input file name]' --size [size to split]\n");
    printf("___to recover files___\n");
    printf("use './hw0401 -r [output file]' [small files]\n");
    printf("to recover file please input ALL the small files.\n");
    printf("this program does not guarantee successfull recovery if not all the small files are inputed\n");
}
