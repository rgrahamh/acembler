#include "acembler.h"

char* nextArg(char* str, char delim){
    //Iterate through until we hit a null byte or the deliminator
    while(*str != delim && *str != '\0'){
        str++;
    }

    //Iterate through all instances of the deliminator
    while(*str == delim){
        str++;
    }

    //Iterate through all whitespace characters
    while(*str == ' ' || *str == '\t'){
        str++;
    }

    //If the str isn't at a null byte, return the pointer.
    if(*str != '\0'){
        return str;
    }
    return NULL;
}

int substr(char* base, char until, char* cpy, int cpySize){
    //Iterate through until we hit a null byte or the deliminator
    int i;
    for(i = 0; *base != until && i < 32; i++){
        *(cpy++) = *(base++);
    }
    if(i == 32){
        return 0;
    }
    *cpy = '\0';
    return 1;
}

int compareString(char* main, char* substr){
    while(*substr != '\0' && *main != '\0'){
        if(*(main++) != *(substr++)){
            return 0;
        }
    }

    //If there isn't more to the instruction name
    if(*main == '\n' || *main == ' ' || *main == '\0'){
        return 1;
    }
    return 0;
}

//Comparing R-Type instructions to the given instruction
int compRType(char* instr){
    int register r_vals = sizeof(R_TYPE_FCODE);
    for(int i = 0; i < r_vals; i++){
        if(compareString(instr, R_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

//Comparing I-Type instructions to the given instruction
int compIType(char* instr){
    int register i_vals = sizeof(I_TYPE_OPCODE);
    for(int i = 0; i < i_vals; i++){
        if(compareString(instr, I_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

//Comparing J-Type instructions to the given instruction
int compJType(char* instr){
    int register j_vals = sizeof(J_TYPE_OPCODE);
    for(int i = 0; i < j_vals; i++){
        if(compareString(instr, J_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

//Comparing J-Type instructions to the given instruction
int compReg(char* reg){
    int register regs = sizeof(REG);
    for(int i = 0; i < regs; i++){
        if(compareString(reg, REG[i])){
            return i;
        }
    }
    return -1;
}

int parseReg(char* reg){
    int register regNum;
    if((regNum = atoi(reg)) != 0){
        return regNum;
    } else {
        return compReg(reg);
    }
}

void handleRType(char* instr, int instrIdx, r_instr* new_instr){
    //Setting the opcode
    new_instr->opcode = 0;
    new_instr->funct = R_TYPE_FCODE[instrIdx];

    //If the register instr is a shift instruction
    if(instrIdx == 8 || instrIdx == 9){
        //Get the destination and additioanl registers
        for(int i = 0; i < 2; i++){
            instr = nextArg(instr, '$');
            substr(instr, ',', val, DELIM_SIZE);
            if(i == 0){
                new_instr->rd = parseReg(val);
            } else {
                new_instr->rt = parseReg(val);
            }
        }
        instr = nextArg(instr, ',');
        substr(instr, '\n', val, DELIM_SIZE);
        new_instr->shamt = atoi(val);
        
        //If atoi didn't work
        if(!new_instr->shamt){
            printf("Could not parse the shift amount properly!");
        }
        new_instr->rs = 0;
    }
    //If the register instr is a jump register
    else if(instrIdx == 3){
        instr = nextArg(instr, '$');
        substr(instr, '\n', val, DELIM_SIZE);
        new_instr->rs = parseReg(val);
        new_instr->rt = 0;
        new_instr->rd = 0;
        new_instr->shamt = 0;
    } else {
        for(int i = 0; i < 3; i++){
            instr = nextArg(instr, '$');
            substr(instr, ((i<2)? ',' : '\n'), val, DELIM_SIZE);
            if(i == 0){
                //Get destination register
                new_instr->rd = parseReg(val);
            }
            else if(i == 1){
                //Get source register
                new_instr->rs = parseReg(val);
            } else {
                //Get additional register
                new_instr->rt = parseReg(val);
            }
        }
        new_instr->shamt = 0;
    }

    if(VERBOSE){
        printf("Instr: %s\n", R_TYPE_INSTR[instrIdx]);
        printf("Dest: %d\n", new_instr->rd);
        printf("Source: %d\n", new_instr->rs);
        printf("Two: %d\n", new_instr->rt);
        printf("shamt: %d\n\n", new_instr->shamt);
    }
}

void handleIType(char* instr, int instrIdx, i_instr* new_instr){
    //Setting the opcode
    new_instr->opcode = I_TYPE_OPCODE[instrIdx];

    //Setting the starting point in the loop depening upon the immediate function
    int register i = (instrIdx > 4 && instrIdx < 14)? 1 : 0;

    for(; i < 2; i++){
        instr = nextArg(instr, '$');
        substr(instr, ',', val, DELIM_SIZE);
        if(i == 0){
            new_instr->rs = parseReg(val);
        } else {
            new_instr->rt = parseReg(val);
        }
    }
    //If it's a store/load instruction
    if(instrIdx > 4 && instrIdx < 14){
        instr = nextArg(instr, ',');
        substr(instr, '(', val, DELIM_SIZE);
        new_instr->immediate = atoi(val);

        instr = nextArg(instr, '$');
        substr(instr, ')', val, DELIM_SIZE);
        new_instr->rs = parseReg(val);
    //Otherwise
    } else {
        instr = nextArg(instr, ',');
        substr(instr, '\n', val, DELIM_SIZE);
        new_instr->immediate = atoi(val);
    }

    if(VERBOSE){
        printf("Instr: %s\n", I_TYPE_INSTR[instrIdx]);
        printf("Source: %d\n", new_instr->rs);
        printf("Two: %d\n", new_instr->rt);
        printf("Immediate: %d\n\n", new_instr->immediate);
    }
}

void handleJType(char* instr, int instrIdx, j_instr* new_instr){
    new_instr->opcode = J_TYPE_OPCODE[instrIdx];
    instr = nextArg(instr, ' ');
    substr(instr, '\n', val, DELIM_SIZE);
    new_instr->addr = atoi(instr);

    if(VERBOSE){
        printf("Instr: %s\n", J_TYPE_INSTR[instrIdx]);
        printf("Addr: %d\n", new_instr->addr);
    }
}

int main(int argc, char** argv){
    char* in_file = "in.mips";
    char* out_file = "out.mips";
    int opt;

    while((opt = getopt(argc, argv, ":vf:o:")) != -1){
        switch (opt){
            case 'f':
                in_file = optarg;
                break;
            case 'o':
                out_file = optarg;
                break;
            case 'v':
                VERBOSE = 1;
                break;
            case 'h':
                printf("-f <input_file>: Specified the input file for the MIPS assembly\n-o <output_file>: Specified the output file for the binary\n-v: Sets the verbose option\n");
                break;
            case '?':
                printf("-%c was not a recognized option! For help, use the -h.\n", opt);
                break;
        }
    }

    //Opening the input file for reading
    FILE* file = fopen(in_file, "r");
    if(file == NULL){
        printf("Could not open %s for reading!\n", in_file);
        return 1;
    }

    //Get the content size
    fseek(file, 0, SEEK_END);
    int str_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* asm_code = (char*)malloc(str_size);

    char tempChar = getc(file);
    for(int i = 0; !feof(file); i++){
        asm_code[i] = tempChar;
        tempChar = getc(file);
    }

    fclose(file);

    if(VERBOSE){
        printf("Code:\n%s\n", asm_code);
    }

    //Opening the output file for writing
    file = fopen(out_file, "w");
    if(file == NULL){
        printf("Could not open %s for writing!\n", out_file);
        return 1;
    }

    char* asm_cursor = asm_code;
    int instrIdx = 0;
    r_instr* r_instruct = malloc(sizeof(r_instr));
    i_instr* i_instruct = malloc(sizeof(i_instr));
    j_instr* j_instruct = malloc(sizeof(j_instr));
    while(asm_cursor != NULL){
        //Handle R-Type instructions
        if((instrIdx = compRType(asm_cursor)) != -1){
            handleRType(asm_cursor, instrIdx, r_instruct);
            for(int i = 0; i < 4; i++){
                fprintf(file, "%c", ((char*)r_instruct)[i]);
            }
        }
        //Handle I-Type instructions
        else if((instrIdx = compIType(asm_cursor)) != -1){
            handleIType(asm_cursor, instrIdx, i_instruct);
            for(int i = 0; i < 4; i++){
                fprintf(file, "%c", ((char*)i_instruct)[i]);
            }
        }
        //Handle J-Type instructions
        else if((instrIdx = compJType(asm_cursor)) != -1){
            handleJType(asm_cursor, instrIdx, j_instruct);
            for(int i = 0; i < 4; i++){
                fprintf(file, "%c", ((char*)j_instruct)[i]);
            }
        }
        else{
            printf("Unrecognized instruction! Skipping...\n");
        }
        asm_cursor = nextArg(asm_cursor, '\n');
    }

    fclose(file);
    printf("Finished assembling!\n");
    return 0;
}
