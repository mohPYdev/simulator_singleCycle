#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


// main memory and the register file as a list in the simulator
int DATA_MEMORY [1000];
int REGISTER_FILE[16];



char * convertToBinary(long n){

    int c, d, t;
    char *p;

    t = 0;
    p = (char*)malloc(32+1);

    for (c = 31 ; c >= 0 ; c--)
    {
        d = n >> c;

        if (d & 1)
            *(p+t) = 1 + '0';
        else
            *(p+t) = 0 + '0';

        t++;
    }
    *(p+t) = '\0';

    return  p;
}

int convertToDecimal(char* Bnum){
    int res = 0;
    for (int i = 0 ; i < 32 ; i++){
        if (Bnum[i] == '1')
            res += pow(2 , 31-i);
    }
    return res;
}

long * readFile(char* address){

    long *nums;
    nums = (long*)malloc(8*65536);
    char line[32+1];
    char *eptr;
    FILE *file;
    file = fopen(address , "r");
    int t = 0;
    if (file){
        while (fgets(line , sizeof(line) , file)){
            *(nums + t) = strtol(line , &eptr , 10);
            t++;
        } 
        fclose(file);
    }
    return nums;
}

int ALU(int rs , int b ,char* opcode , bool* Btaken){

    int result;
    if (strcmp(opcode , "0000") == 0 ||strcmp(opcode , "0101") == 0 || strcmp(opcode , "1001") == 0 || strcmp(opcode , "1010") == 0)
    {
        result = rs + b;
    }
    else if ( strcmp(opcode , "0001") == 0) result = rs - b;
    else if ( strcmp(opcode , "0010") == 0 || strcmp(opcode , "0110") == 0) {
        if (rs > b) result = 0;
        else result = 1;
    }
    else if ( strcmp(opcode , "0011") == 0 || strcmp(opcode , "0111") == 0) result = rs | b;
    else if (strcmp(opcode , "0100") == 0) result = rs & b;
    else if ( strcmp(opcode , "1000") == 0) result = b << 16;

    else if ( strcmp(opcode , "1011") == 0)
    {
        result = rs - b;
        if (result == 0)
            *Btaken = true; 
    }

    return result;
}

void controlUnit(char* opcode , bool* regdest , bool* alusrc,bool* MemToReg,bool* JalrToReg,bool* BranchToPc, bool* JumpToPc ,  bool* regWrite , bool* memWrite , int* memCount){
    // R instructions
    if ( strcmp(opcode , "0000") == 0 || strcmp(opcode , "0001") == 0 || strcmp(opcode , "0010") == 0 || strcmp(opcode , "0011") == 0 ||strcmp(opcode , "0100") == 0){
        *regdest = true;
        *regWrite = true;
    }
    
    // some I instructions
    else if (strcmp(opcode , "0101") == 0 || strcmp(opcode , "0110") == 0 ||strcmp(opcode , "0111") == 0 || strcmp(opcode ,"1000") == 0){
        *alusrc = true;
        *regWrite = true;
    }

    // lw
    else if (strcmp(opcode , "1001") == 0){
        *alusrc = true;
        *MemToReg = true;
        *regWrite = true;
        (*memCount)++;
    }
    
    // sw
    else if (strcmp(opcode , "1010") == 0){
        *alusrc = true;
        *memWrite = true;
        (*memCount)++;
    }

    // beq
    else if ( strcmp(opcode , "1011") == 0){
        *BranchToPc = true;
    }

    //jalr
    else if (strcmp(opcode , "1100") == 0){
        *JalrToReg = true;
        *JumpToPc = true;
    }

    //j
    else if (strcmp(opcode , "1101") == 0){
        *JumpToPc = true;
    }

}

char* mux(char* a , char* b , bool choose){
    if (choose) 
        return a;
    return b;
}

char* adder(char* Sa , char* Sb){
    int a = convertToDecimal(Sa);
    int b = convertToDecimal(Sb);

    long result = a + b;

    char* Bnum;
    Bnum = convertToBinary(result);
    return Bnum;
}

void extend16To32bits(char* n , char* extN){
    char* zeros = "0000000000000000" ;
    strcat(extN, zeros);
    strcat(extN , n );
}

void extend4To32bits(char* n , char* extN){
    char* zeros = "0000000000000000000000000000" ;
    strcat(extN, zeros);
    strcat(extN , n );
}

void showRegInfo(int cycleCount){
    printf("\tcycle : %d\n"  , cycleCount);
    for (int i = 0 ; i < 18 ; i++)
        printf("_");
    printf("\n|");
    // int colums = 2;
    for (int i = 0; i < 16 ; i++){
        printf("R%d = %d" , i , REGISTER_FILE[i]);
        for(int j = 0 ; j < 2 ; j++)
            printf("\t");
        printf("|\n|");
        // if (i == 9) colums = 1;
    }
    for (int i = 0 ; i < 18 ; i++)
            printf("_"); 
}

void calculateRegPercent(double* regPercent){
    int sum = 0 ;
    for (int i = 0 ; i < 16 ; i++)
        sum += regPercent[i];
    for( int i = 0 ; i < 16 ; i++)
        regPercent[i] = (regPercent[i]/sum) * 100;
}

void showInfo(double* regPercent , int cycleCount , int memCount){
    printf("\n          *** INFORMATION ***            \n");

    for (int i = 0 ; i < 50 ; i++)
        printf("_");

    printf("\n|");
    printf("\t register percentage info:");
    for(int j = 0 ; j < 2 ; j++)
        printf("\t");
    printf("|\n|");
    // int columns = 22;
    for (int i = 0; i < 16 ; i++){
        printf("R%d use percentage = %.2f%%  " , i , regPercent[i]);
        for(int j = 0 ; j < 3 ; j++)
            printf("\t");
        printf("|\n|");
        // if (i == 9) columns = 21;
    }
    for (int i = 0 ; i < 50 ; i++)
            printf("_");

    printf("\n|");
    printf("\t memory usage = %d times " , memCount );
    for(int j = 0 ; j < 2 ; j++)
        printf("\t");
    printf("|\n|");

    for (int i = 0 ; i < 50 ; i++)
            printf("_");
    printf("\n|");

    printf("number of instruction that have executed : %d" , cycleCount);
    for(int j = 0 ; j < 1 ; j++)
        printf("\t");
    printf("|\n|");
    for (int i = 0 ; i < 50 ; i++)
            printf("_");

}

int main(){


    DATA_MEMORY[6] = 5;
    DATA_MEMORY[7] = -1;
    DATA_MEMORY[8] = 2;

    char *Bnum;
    long *instructions; 
    instructions = readFile("program.mc");
    unsigned int pc = 0;
    int cycleCount = 0;
    int memCount = 0;

    double regPercent[16];
    for (int i = 0; i < 16; i++)
        regPercent[i] = 0;

    
    char *opcode;
    opcode = (char*)malloc(4+1);

    char *rs;
    rs = (char*)malloc(4+1);

    char *rt;
    rt = (char*)malloc(4+1);

    char *rd;
    rd = (char*)malloc(4+1);

    char *imm;
    imm = (char*)malloc(16+1);
    
    char *jumpTarget;
    jumpTarget = (char*)malloc(16+1);


    // control unit signals
    bool RegDest = false;
    bool ALUsrc = false;
    bool MemToReg = false;
    bool JalrToReg = false;
    bool BranchToPc = false;
    bool JumpToPc = false;
    bool regWrite = false;
    bool regRead = false;
    bool MemWrite = false;
    bool Btaken = false;
    

    // declaring a key for user to press if he/she wants to check the next cycle.

    printf("\n\n           ***WELCOME TO THE SIMULATOR ***             \n");

    while (1){
        // if the instruction is halt, end the execution
        if (instructions[pc] == 0 || instructions[pc] == 234881024) break;

        // if the instruction is directive , put the value in the data memory.
        else if (instructions[pc] < 65536) {
            DATA_MEMORY[pc] = instructions[pc]; 
            pc++;
            continue;
        }

        Bnum = convertToBinary(instructions[pc]);

        RegDest = false;
        ALUsrc = false;
        MemToReg = false;
        JalrToReg = false;
        BranchToPc = false;
        JumpToPc = false;
        regWrite = false;
        MemWrite = false;
        Btaken = false;

        
        // opcode
        int t = 0;
        for (int i = 4 ; i < 8 ; i++){
            *(opcode + t) = Bnum[i];
            t++;
        }
        *(opcode + t) = '\0';

        // rs
        t = 0;
        for (int i = 8 ; i < 12 ; i++){
            *(rs + t) = Bnum[i];
            t++;
        }
        *(rs + t) = '\0';
        
        // rt
        t = 0;
        for (int i = 12 ; i < 16 ; i++){
            *(rt + t) = Bnum[i];
            t++;
        }
        *(rt + t) = '\0';
        
        // rd
        t = 0;
        for (int i = 16 ; i < 20 ; i++){
            *(rd + t) = Bnum[i];
            t++;
        }
        *(rd + t) = '\0';

        // immediate
        t = 0;
        for (int i = 16 ; i < 32 ; i++){
            *(imm + t) = Bnum[i];
            t++;
        }
        *(imm + t) = '\0';

        // jump target address
        t = 0;
        for (int i = 16 ; i < 32 ; i++){
            *(jumpTarget + t) = Bnum[i];
            t++;
        }
        *(jumpTarget + t) = '\0';

        
        // determine the opcode and calculate the control signals
        controlUnit(opcode , &RegDest , &ALUsrc , &MemToReg , &JalrToReg , &BranchToPc , &JumpToPc , &regWrite , &MemWrite , &memCount);


        // extend the immediate value , jump target address , rs to 32 bits
        char extImm [33] = "";
        char extJaddress [33] = "";
        char extRs [33] = "";
        char extRt [33] = "";
        char extRd [33] = "";
        extend16To32bits(imm , extImm);
        extend16To32bits(jumpTarget , extJaddress);
        extend4To32bits(rs , extRs);
        extend4To32bits(rt , extRt);
        extend4To32bits(rd , extRd);
        
        
        // pc path 
        char* pcS;
        pcS = convertToBinary(pc);
        
        //  pc + 1
        char* pcP1; 
        pcP1 = adder("00000000000000000000000000000001" , pcS);


        // register file
        int rsDec = convertToDecimal(extRs);
        int rtDec = convertToDecimal(extRt);
        int rdDec = convertToDecimal(extRd);
        
        int rsValue = REGISTER_FILE[rsDec];
        int rtValue = REGISTER_FILE[rtDec];
        regPercent[rsDec]++;
        regPercent[rtDec]++;
        
        char* aluSrcNum = mux(extImm, convertToBinary(rtValue) , ALUsrc);

        // ALU section
        int result = ALU(rsValue , convertToDecimal(aluSrcNum) , opcode , &Btaken);
        


        // memory section
        if (MemWrite)
            DATA_MEMORY[result] = rtDec;

        else {
            int MemToMux = DATA_MEMORY[result];
            char *MuxToMux = mux(convertToBinary(MemToMux) , convertToBinary(result) , MemToReg);
            char* MuxToReg = mux(pcP1 , MuxToMux , JalrToReg);

            // write back to the register file
            if (regWrite){
                // choose the destination register from rs and rt
                char* destinationReg = mux(extRd , extRt , RegDest);
                REGISTER_FILE[convertToDecimal(destinationReg)] = convertToDecimal(MuxToReg);
                regPercent[convertToDecimal(destinationReg)]++;
            }
        }

        // continue the pc path

        // pc + 1 + offset
        char* pcP2;
        pcP2 = adder(pcP1 , extImm);
        
        // choose from pc + 1 and pc + 1 + offset
        bool Bsignal = BranchToPc & Btaken;
        pcS = mux(pcP2 , pcP1 , Bsignal);

        // choose from rs and jump target address
        char* pcP3;
        pcP3 = mux(extRs , extJaddress , JalrToReg);

        // choose from jump / jalr pc address or branch/pc+1 address
        pcS = mux( pcP3 , pcS , JumpToPc);

        pc = convertToDecimal(pcS);

        //show the register info to the user
        cycleCount++;
        showRegInfo(cycleCount);

        // wait for the user to input a character and continue the program
        char input;
        while(1){
        printf("\npress N to go to the next cycle | press E to exit : ");
        scanf(" %c" , &input);
        printf("\n");
        if (input == 'N' || input == 'E')
            break;
        }
        if (input == 'E')
            break;
    }

    calculateRegPercent(regPercent);
    printf("\n");
    showInfo(regPercent , cycleCount , memCount);

    return 0;
}
