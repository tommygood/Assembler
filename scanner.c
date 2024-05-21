#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_opcode.c"
#include <math.h>

// all types of registers, which value are equal to index number 
char registers[9][2] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};
// all types of mnemonic which are under the format 2 (related to handle register)
char register_mnemonic[10][6] = {"ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO", "SHIFTL", "SUBR", "SVC", "TIXR"};

char ** split(char * s) {
    // split the string with space and newline
    int s_len = strlen(s);
    char ** s_arr = calloc(s_len, sizeof(char *));
    char * temp = calloc(s_len, sizeof(char));
    int index = 0;
    
    for (int i = 0;i < s_len;i++) {
        if (s[i] != ' ' && s[i] != '\t') {
	    	//printf("%d %c is not space, but is %d\n", i, s[i], s[i] == '\0');
            // add char into temp if not counter the space or newline
            strncat(temp, &s[i], 1); // append the char to string
        }
        else {
			//printf("%d %c is space, but is %d\n", i, s[i], s[i] == '\0');
			if (temp[0] != '\0') {
				// place temp into array and clear the temp
				s_arr[index++] = strdup(temp);
			}
            // clear the temp
            temp[0] = '\0';
        }
    }
    if (temp[0] != '\0') {
        // place temp into array and clear the temp
        s_arr[index++] = strdup(temp);
    }
    //for (int i = 0;i < s_len;i++)
    //	printf("s arr %d : %s\n", i, s_arr[i]);
    
    return s_arr;
}

char * commaRemoveSpace(char * line) {
    // remove a space behind the comma
    int comma_index = -1;
    for (int i = 0;i < strlen(line);i++) {
	if (line[i] == ',') {
	    comma_index = i; // which index is comma
	    break;
	}
    }
    // return origin string, if no comma
    // remove a space behind comma, otherwise
    if (comma_index == -1) {
	return line;
    }
    else {
	char * new_str = malloc((strlen(line)-1) * sizeof(char));
	int index = 0;
	for (int i = 0;i < strlen(line);i++) {
	    if (i != comma_index + 1) {
		new_str[index++] = line[i];
	    }
	}
	return new_str;
    }
}		    

char * removeComment(char * line) {
    // remove comments, which are behind the dot
    char * n_str = calloc(strlen(line), sizeof(char));
    n_str[0] = '\0';
    for (int i = 0;i < strlen(line);i++) {
	if (line[i] == '.') {
	    break;
	}
	else {
	    n_str[i] = line[i];
	}
    }

    return n_str;
}

int isBasic(char * line) {
    char * second_place = split(line)[1];
    char * first_place = split(line)[0];
    if (second_place == '\0') // avoid  
		return 0;
    if (strcmp(second_place, "START") == 0)
		return 1;
    else if (strcmp(first_place, "END") == 0)
    	return 2;
    else if (strcmp(second_place, "WORD") == 0)
    	return 3;
    else if (strcmp(second_place, "RESW") == 0)
    	return 4;
    else if (strcmp(second_place, "BYTE") == 0)
    	return 5;
    else if (strcmp(second_place, "RESB") == 0)
    	return 6;
	else if (strcmp(first_place, "BASE") == 0)
    	return 7;
    else 
    	return 0;
}

char * isMnemonic(char * first_str, char * sec_str, char ** opcode_table, int record_len) {
    // check whether the instruction contains a valid mnemonic
    if (first_str != NULL && find(first_str, opcode_table, record_len) != "0") {
		return "Mnemonic";
    }
    else if (sec_str != NULL && find(sec_str, opcode_table, record_len) != "0") {
		return "TEST Mnemonic";
    }
    else {
		return "null";
    }
}

int getLenStrArr(char ** s) {
    // get the length of string array
    int len = 0;
    while (s[len] != NULL) {
	len++;
    }
    return len;
}

int isSymbol(char ** s) {
    // the way to check this line contains a symbol is to check whether valid string number is 3
    
    // get the length of string array
    int len = getLenStrArr(s);

    int num = 0;
    for (int i = 0;i < len;i++) {
		if (s[i] != '\0') {
			num++;
		}
    }
    return num == 3;
}

int isIndexAddressing(char * mnemonic, char * operand) {
    // check if operand is index addressing
    int result = 0;

	// it's not index addressing when mnemonic is format 2
	if (isRegisterMnemonic(mnemonic)) {
		return result;
	}

	// is index adddressing when the operand contain a comma
    for (int i = 0;i < strlen(operand);i++) {
		if (operand[i] == ',') {
			result = 1;
			break;
		}
    }
    return result;
}

int isInDirectAddresing(char * operand) {
    if (operand[0] == '@') {
		return 1;
    }
    else {
		return 0;
    }
}

int isImmediateAddresing(char * operand) {
    if (operand[0] == '#') {
		return 1;
    }
    else {
		return 0;
    }
}

// check if operand is which type of addressing
void checkAddressingType(char * mnemonic,char * operand) {
    if (isIndexAddressing(mnemonic, operand)) {
        printf("This is index addressing\n");
    }
    else if (isInDirectAddresing(operand)) {
        printf("This is indirect addressing\n");
    }
    else if (isImmediateAddresing(operand)) {
        printf("This is immediate addressing\n");
    }
    else {
		printf("This is direct addressing\n");
    }
}

void insertValueInHashTable(char * str, char ** hash_table, int size) {
    // find a empty space in hash table to place the value

	int val = getCodeOpcode(str);
	int index = val % size;
	//printf("%s, %d\n", all_str[i], index);
	if (hash_table[index] == NULL) { // the bucket not have value
		hash_table[index] = str;
	}
	else { // probing other empty bucket
		// linear probing the empty bucket
		int empty_bucket = -1;
		for (int i = 0;i < size;i++) {
			if (hash_table[i] == NULL) {
				empty_bucket = i;
				break;
			}
		}

		// check whether have empty bucket
		if (empty_bucket == -1) {
			printf("no more empty bucket %s\n", str);
		}
		else {
			hash_table[empty_bucket] = str;
		}
	}
}

void printSymbolTable(char ** symbol_table, int record_len) {
    printf("Symbol Table : {");
    int first_symbol = 1;
	for (int i = 0;i < record_len;i++) {
	    if (symbol_table[i] != NULL) {
			char ** str = split(symbol_table[i]);
			char * symbol = str[0];
			int address = atoi(str[1]);
			// print a comma at the end of each symbol if not the last symbol
			if (first_symbol) {
				printf("'%s' : '0x%x'", symbol, address);
				first_symbol = 0;
			}
			else {
				printf(", '%s' : '0x%x'", symbol, address);
			}
	    }
	}
    printf("}\n");
}

int isRegisterMnemonic(char * mnemonic) {
	// is register relaive mnemonic

	int register_mnemonic_len = sizeof(register_mnemonic) / sizeof(register_mnemonic[0]);
	for (int i = 0;i < register_mnemonic_len;i++) {
		if (strcmp(mnemonic, register_mnemonic[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

int checkFormatType(char * mnemonic) {
	// check format of instruction
	if (mnemonic[0] == '+') {
		// extended, format 4
		return 4;
	}
	else if (isRegisterMnemonic(mnemonic)) {
		// format 2
		// is register relaive mnemonic
		return 2;
	}
	else {
		// format 3
		printf("format 3\n");
		return 3;
	}
} 
char ** mkSymbolTable(char ** all_str, int record_len, char ** opcode_table, int opcode_table_record_len) {
    // make the string array needed to put into symbol table
    char ** symbol_str_arr = calloc(record_len, sizeof(char *));
    char ** symbol_table = hashTable("", 0, record_len);

    int is_started = 0;
	int address;
    char ** object_code_table = calloc(record_len, sizeof(char *));
    for (int i = 0;i < record_len;i++) {
		char * line = all_str[i];
		line = commaRemoveSpace(line); // remove a space behind the comma 
		printf("\n%d : %s\n", i+1, line);
		line = removeComment(line); // remove comment, which is behind the dot

		// check this line is belong to normal instruction or basic function to use different way to locate
		int basic_instruction = isBasic(line);
		
		//printf("basic instruction is %d\n", basic_instruction);
		printf("address : %x\n", address);
		
		char * object_code; // object code of this line
		if (basic_instruction == 1) {
			// START
			is_started = 1;
			char ** s_arr = split(line);
			address = (int)strtol(s_arr[2], NULL, 16);
			printf("Program name is %s\n", s_arr[0]);
			printf("start from this line\n\n");
		}
		else if (basic_instruction == 7) {
			// virtual address space
			printf("BASE is virtual instruction code\n");
		}
		else {
			if (basic_instruction) {
				// contains a basic instruction
				if (!is_started) {
					printf("error : basic instruction before the program start");
				}
				else {
					char ** s_arr = split(line);
					if (isSymbol(s_arr)) {
						// print the address of symbol
						char * symbol_name = s_arr[0];
						// convert int adddress to string
						// count the length of int(address)
						int address_length;
						if (address == 0) {
							address_length = 0;
						}
						else {
							address_length = ((ceil(log10(address))+1)*sizeof(char)); // length of int
						}
						char str_address[address_length];
						sprintf(str_address, "%d", address); // convert int adddress to string
						// concat the symbol name and a space and address
						char * temp_symbol_name = strdup(symbol_name);
						char * hash_value = strcat(temp_symbol_name, " ");
						hash_value = strcat(temp_symbol_name, str_address);
						// insert into symbol table
						insertValueInHashTable(hash_value, symbol_table, record_len);
					}

					if (basic_instruction == 2) {
						// END
						printf("END of the program\n");
					}
					else if (basic_instruction == 3) {
						// WORD
						printf("WORD is pesudo instruction code\n");
						int space = 3;
						address += space;
						object_code = calloc(6, sizeof(char));	
						sprintf(object_code, "%d", s_arr[2]); // convert int operand to string
						printf("object code : %s\n", object_code);
					}
					else if (basic_instruction == 4) {
						// RESW
						printf("RESW is pesudo instruction code\n");
						int space = atoi(s_arr[2]) * 3;
						address += space;
					}
					else if (basic_instruction == 5) {
						// BYTE
						printf("BYTE is pesudo instruction code\n");
						int space;
						object_code = calloc(6, sizeof(char));	
						if (s_arr[2][0] == 'C') {
							// char
							// the nums of char
							int char_nums = strlen(s_arr[2])-4; // minus the two single quotation and one type and a space at the last char
							space = char_nums;
							printf("c char nums : %d %c %c", strlen(s_arr[1]), s_arr[2][5], s_arr[2][6]==' ');
							// make object code
							for (int j = 2;j < strlen(s_arr[2]);j++) {
							    if (s_arr[2][j] == 39) {
								// stop when counter the char of '
								break;
							    }
							    else {
								int temp_int = s_arr[2][j];
								char * temp_str = calloc(3, sizeof(char));
								sprintf(temp_str, "%X", temp_int); // convert operand str to hex
								object_code = strcat(object_code, temp_str);
							    }
							}
						}
						else if (s_arr[2][0] == 'X') {
							// int
							space = 1;
							// make object code
							for (int j = 2;j < strlen(s_arr[2]);j++) {
							    if (s_arr[2][j] == 39) {
								// stop when counter the char of '
								break;
							    }
							    else {
								char * temp_str = calloc(1, sizeof(char));
								temp_str[0] = s_arr[2][j];
								object_code = strcat(object_code, temp_str);
							    }
							}
						}
						printf("object code : %s\n", object_code);
						address += space;
					}
					else if (basic_instruction == 6) {
						// RESB
						printf("RESB is pesudo instruction code\n");
						int space = atoi(s_arr[2]);
						address += space;
					}
				}	
			}
			else {
				// not the basic instruction
				if (is_started) {
					// check whether it is in opcode table
					char ** s_arr = split(line);
					char * mnemonic = isMnemonic(s_arr[0], s_arr[1], opcode_table, opcode_table_record_len);
					if (mnemonic != "null") {
						int operand_index = getLenStrArr(s_arr)-1; // operand is the last index of a line
						char * operand = "";
						if (operand_index != 0) {
							// operand is not the only string in line
							operand = s_arr[operand_index]; // operand is the last index of a line
						}

					    // check format of instruction
						int mnemonic_index = operand_index-1;
						if (mnemonic_index < 0) {
							mnemonic_index = 0;
						}
						int format_size = checkFormatType(s_arr[mnemonic_index]);

						// check if operand is which type of addressing
						checkAddressingType(s_arr[mnemonic_index], operand);

						// check if mnemonic is test or normal
						if (mnemonic == "Mnemonic") {
							printf("Label : Mnemonic: %s operand : %s\n", s_arr[0], operand);
						}
						else {
							// TEST Mnemonic
							printf("Label : TEST Mnemonic: %s operand : %s\n", s_arr[1], operand);
							// print the address of symbol
							char * symbol_name = s_arr[0];
							
							// convert int adddress to string
							int address_length; // length of int
							if (address == 0) {
								address_length = 1;
							}
							else {
								address_length = (int)((ceil(log10(address))+1)*sizeof(char));
							}
							//printf("%d\n", address);
							char str_address[address_length];
							sprintf(str_address, "%d", address); // convert int adddress to string
							// concat the symbol name and a space and address
							char * temp_symbol_name = strdup(symbol_name);
							char * hash_value = strcat(temp_symbol_name, " ");
							hash_value = strcat(temp_symbol_name, str_address);
							// insert into symbol table
							insertValueInHashTable(hash_value, symbol_table, record_len);
						}
						// normal instruction take 3B
						address += format_size;
					}
				}
			}
		}
		/*
		*/
    }
    return symbol_table;
}

char ** mkOpcodeTable() {
    // find relative opcode and output
    char * filename = "./opcode.txt";
    int record_len = recordLen(filename);
    char ** all_str = readFile(filename);

    char ** hash_table = hashTable(all_str, record_len, record_len); // make a hash table with data in opcode.txt
    return hash_table;
}

int main() {
    char * filename = "./testprog3.S"; // program name
    int record_len = recordLen(filename);
    char ** all_str = readFile(filename);
    // make opcode table
    int opcode_table_record_len = recordLen("opcode.txt");
    char ** opcode_table = mkOpcodeTable();
    // make symbol table
    char ** symbol_table = mkSymbolTable(all_str, record_len, opcode_table, opcode_table_record_len);
    printSymbolTable(symbol_table, record_len); // print values in symbol table
}
