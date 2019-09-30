#include "acembler.h"

/** Iterates to the next argument (one past the given delimiter) or until we hit a null byte
 * @param str The string to iterate through
 * @param delim The delimiter to break on
 * @return A char* to the next character in memory after the delim is hit
 */
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

/** Iterates through a string and does a deep copy until a null byte or specified stopping point is hit
 * @param base The base string
 * @param until The ending character
 * @param cpy The char* to copy file contents into
 * @param cpySize The size of cpy
 * @return 1 on success, 0 otherwise
 */
int substr(char* base, char until, char* cpy, int cpySize){
    //Iterate through until we hit a null byte or the delim
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

/** Compares one string against another and returns true if it's an exact match (with only white space following)
 * @param main The main string
 * @param substr The substring being compared against
 * @return 1 if it's an exact match (with only white space following), 0 otherwise
 */
int compareString(char* main, char* substr){
    while(*substr != '\0' && *main != '\0'){
        if(*(main++) != *(substr++)){
            return 0;
        }
    }

    //If there isn't more to the instruction name
    if(*main == '\n' || *main == '\t' || *main == ' ' || *main == '\0'){
        return 1;
    }
    return 0;
}

/** Checks to see if the given instruction is an R-Type
 * @param instr The instruction being checked
 * @return The instruction index if it's an R-Type instruction, -1 otherwise
 */
int compRType(char* instr){
    int register r_vals = sizeof(R_TYPE_FCODE);
    for(int i = 0; i < r_vals; i++){
        if(compareString(instr, R_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

/** Checks to see if the given instruction is an I-Type
 * @param instr The instruction being checked
 * @return The instruction index if it's an I-Type instruction, -1 otherwise
 */
int compIType(char* instr){
    int register i_vals = sizeof(I_TYPE_OPCODE);
    for(int i = 0; i < i_vals; i++){
        if(compareString(instr, I_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

/** Checks to see if the given instruction is a J-Type
 * @param instr The instruction being checked
 * @return The instruction index if it's an J-Type instruction, -1 otherwise
 */
int compJType(char* instr){
    int register j_vals = sizeof(J_TYPE_OPCODE);
    for(int i = 0; i < j_vals; i++){
        if(compareString(instr, J_TYPE_INSTR[i])){
            return i;
        }
    }
    return -1;
}

/** Checks to see what the given register value is
 * @param instr The instruction being checked
 * @return The instruction index if it's valid register, -1 otherwise
 */
int compReg(char* reg){
    int register regs = sizeof(REG);
    for(int i = 0; i < regs; i++){
        if(compareString(reg, REG[i])){
            return i;
        }
    }
    return -1;
}

/** Parses the register from a string
 * @param reg The register string
 * @return The register number
 */
int parseReg(char* reg){
    int register regNum;
    if((regNum = compReg(reg)) != -1){
        return regNum;
    } else {
        return atoi(reg);
    }
}

/** Handles R-Type instructions
 * @param instr The instruction passed in
 * @param instrIdx The instruction index
 * @param new_instr A pointer to the instruction struct that will get altered
 */
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
        
        //Set source register to 0 since we're not using it
        new_instr->rs = 0;
    }
    //If the register instr is a jump register
    else if(instrIdx == 3){
        //Get the source register
        instr = nextArg(instr, '$');
        substr(instr, '\n', val, DELIM_SIZE);
        new_instr->rs = parseReg(val);

        //Set all unused vals to 0
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

/** Handles I-Type instructions
 * @param instr The instruction passed in
 * @param instrIdx The instruction index
 * @param new_instr A pointer to the instruction struct that will get altered
 */
void handleIType(char* instr, int instrIdx, i_instr* new_instr){
    //Setting the opcode
    new_instr->opcode = I_TYPE_OPCODE[instrIdx];

    //Setting the starting point in the loop depening upon the immediate function (gets just rt for store/load, rs & rt for other)
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
        //Getting the immediate (offset)
        instr = nextArg(instr, ',');
        substr(instr, '(', val, DELIM_SIZE);
        new_instr->immediate = atoi(val);

        //Getting the source (register in the parenthesis)
        instr = nextArg(instr, '$');
        substr(instr, ')', val, DELIM_SIZE);
        new_instr->rs = parseReg(val);
    } else {
        //Otherwise, just store the immediate value
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

/** Handles J-Type instructions
 * @param instr The instruction passed in
 * @param instrIdx The instruction index
 * @param new_instr A pointer to the instruction struct that will get altered
 */
void handleJType(char* instr, int instrIdx, j_instr* new_instr){
    //Setting the opcode
    new_instr->opcode = J_TYPE_OPCODE[instrIdx];

    //Setting the address
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

    //Read in options
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

    //Get the content size and allocate a properly-sized string
    fseek(file, 0, SEEK_END);
    int str_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* asm_code = (char*)malloc(str_size);

    //Reading the file in to a string
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

    //Allocate space for all of the instruction structs
    r_instr* r_instruct = malloc(sizeof(r_instr));
    i_instr* i_instruct = malloc(sizeof(i_instr));
    j_instr* j_instruct = malloc(sizeof(j_instr));

    //A cursor for the assembly code
    char* asm_cursor = asm_code;
    //Keeps track of the index of the instruction
    int instrIdx = 0;
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
