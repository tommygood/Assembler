#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_opcode.c"
#include <math.h>
#include <ctype.h>

// all types of registers, which value are equal to index number 
char registers[9][2] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};
// all types of mnemonic which are under the format 2 (related to handle register)
char register_mnemonic[10][6] = {"ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO", "SHIFTL", "SUBR", "SVC", "TIXR"};

// total records of object program
char ** object_program;
int object_program_size;
int cur_t_record_size = 0; // size of current t record
int cur_t_record_length_index = 7; // index of length of object code in this t record in bytes(hex)
int cur_t_record_new_line = 0; // add a new line in t record if this value turn to 1
char ** forward_ref_table_format_four_address;
char ** m_records;

// base register, which will store the address of a symbol
int base;
char * base_symbol;

// record if error have been occuered
int error_occured = 0;

// current number of line
int line_num;

// record the line use the forward reference symbol
char ** symbol_line_table;

void throwError(char * error_msg) {
	printf("line %d, Error : %s!!!!!!!!!\n", line_num, error_msg);
	error_occured = 1; // not print the object program at last if there have a error occured
}

int getLenStrArr(char ** s) {
    // get the length of string array
    int len = 0;
    while (s[len] != NULL) {
		len++;
    }
    return len;
}

void printObjectProgram() {
	// print the all records in object program
	if (!error_occured) {
		//printf("output the complete object program :\n=========================================================\n");
		for (int i = 0;i < object_program_size;i++) {
			if (object_program[i] != NULL) {
				printf("%s", object_program[i]);
			}
			else {
				printf("\n");
				break;
			}
		}
		//printf("=========================================================\n");
	}
	else {
		printf("stop outputting the object program, as the error occur\n");
	}
}

char * fillZeroAtFirst(char * s, int num) {
	// fill zeros at the first of string to make the length of s same as num
	if (s == NULL) {
		s = "";
	}
	char * n_str = calloc(num, sizeof(char));
	for (int i = 0;i < num-strlen(s);i++) {
		n_str[i] = '0';
	}
	strcat(n_str, s);
	return n_str;
}

char * fillCharAtLast(char * s, int num, char c) {
	// fill zeros at the last of string to make the length of s same as num
	if (s == NULL) {
		s = "";
	}
	char * n_str = calloc(num, sizeof(char));
	strcat(n_str, s);
	for (int i = strlen(s);i < num;i++) {
		n_str[i] = c;
	}
	return n_str;
}

// add t record in object program
void addTRecord(int address, char * s, int format) {
	if (cur_t_record_size == 0 || cur_t_record_size + strlen(s) > 60 || cur_t_record_new_line) {
		// first t record or over the size limit of t record, add a new t record
		char * t_record = calloc(30, sizeof(char));
		strcat(t_record, "\nT ");
		char str_address[intLength(address)];
		sprintf(str_address, "%X", address); // convert int adddress to string
		strcat(t_record, fillZeroAtFirst(str_address, 6));
		int object_program_length = getLenStrArr(object_program);
		object_program[object_program_length] = strdup(strcat(t_record, " "));
		object_program_length++;
		// set the length of object code
		char str_cur_t_record_size[intLength(cur_t_record_size)];
		sprintf(str_cur_t_record_size, "%X", cur_t_record_size/2); // convert int address to string
		object_program[cur_t_record_length_index] = fillZeroAtFirst(strdup(str_cur_t_record_size), 2); // set the length of object code of last t record
		cur_t_record_length_index = object_program_length;
		object_program[cur_t_record_length_index] = "00"; // init the length of object code
		// reset the size of t record
		cur_t_record_size = 0;
		// reset the new line
		cur_t_record_new_line = 0;
	}
	// set the object code of t record
	char * t_record = calloc(strlen(s)+1, sizeof(char));
	t_record[0] = ' '; // add a space in front of object code to seperate different object code in this t record
	strcat(t_record, s);
	object_program[getLenStrArr(object_program)] = t_record;
	cur_t_record_size += strlen(s);

	// add a m record if is format 4
	if (format == 4) {
		char * m_record = calloc(30, sizeof(char));
		strcat(m_record, "\nM ");
		char str_address[intLength(address+1)];
		sprintf(str_address, "%X", address+1); // convert int adddress to string
		strcat(m_record, fillZeroAtFirst(str_address, 6));
		strcat(m_record, " ");
		char str_len[intLength(strlen(s)-3)];
		sprintf(str_len, "%X", strlen(s)-3); // convert int adddress to string
		char * temp = fillZeroAtFirst(str_len, 2);
		strcat(m_record, temp);
		m_records[getLenStrArr(m_records)] = m_record;
	}
}

void addHRecordStart(char * program_name, int start_address) {
	// add the info of header record except the length of object program
	start_address = (start_address);
	object_program[0] = "H ";
	object_program[1] = "";
	object_program[2] = strcat(fillCharAtLast(program_name, 6, ' '), " ");
	char temp[intLength(start_address)];
	sprintf(temp, "%X", start_address);
	object_program[3] = fillZeroAtFirst(temp, 6);
	// reservre the place to put the length of object program
	object_program[4] = " ";
	object_program[5] = "";
}

void addHRecordLength(int last_address) {
	// add the length of object program in the header record by bytes(hex)
	int start_address = strToHex(object_program[3]);
	int length = (last_address - start_address);
	char temp[intLength(length)];
	sprintf(temp, "%X", length);
	char * n_temp = fillZeroAtFirst(temp, 6); 
	object_program[5] = n_temp; 
}

// get length of a int
int intLength(int n) {
	if (n == 0) {
		return 0;
	}
	else {
		if (n < 0) {
			n *= -1;
		}
		return (ceil(log10(n))+1)*sizeof(char);
	}
}

int strToHex(char * s) {
	// convert string to hex int
	return strtol(s, NULL, 16);
}

int strConvertToHex(char * s) {
	char temp[intLength(strToHex(s))];
	sprintf(temp, "%X", strToHex(s));
	return atoi(temp);
}

int decimalIntConvertToHex(int n) {
	char temp[intLength(n)];
	sprintf(temp, "%X", n);
	return atoi(temp);
}

char * twosComplement(char * num) {
	// return the two's complement of num
	char * s = calloc(strlen(num), sizeof(char));

	for (int i = 0;i < strlen(num);i++) {
		char temp[1];
		temp[0] = num[i];
		int col = 15 - strToHex(temp);
		//printf("col : %X %X %c\n", col, 15-strToHex(temp), num[i]);
		sprintf(temp, "%X", 15-strToHex(temp));
		s[i] = temp[0];
	}
	int result = strToHex(s)+1;
	sprintf(s, "%X", result);
	return s;
}

char ** split(char * s, char split_char) {
    // split the string with space and newline
	if (s == NULL) {
		return NULL;
	}

    int s_len = strlen(s);
    char ** s_arr = calloc(s_len, sizeof(char *));
    char * temp = calloc(s_len, sizeof(char));
    int index = 0;
    
    for (int i = 0;i < s_len;i++) {
        if (s[i] != split_char && s[i] != '\t' && s[i] != '\n') {
	    	//printf("%d %c is not space, but is %d\n", i, s[i], s[i]);
            // add char into temp if not counter the space or newline
            strncat(temp, &s[i], 1); // append the char to string
        }
        else {
			//printf("%d %c is space, but is %d\n", i, s[i], s[i] == '\0');
			if (temp[0] != '\0' && temp != NULL) {
				// place temp into array and clear the temp
				s_arr[index++] = strdup(temp);
			}
            // clear the temp
            temp[0] = '\0';
        }
    }
    if (temp[0] != '\0' && temp != NULL) {
        // put the temp into array and clear the temp
        s_arr[index++] = strdup(temp);
    }
    //for (int i = 0;i < s_len;i++)
    //	printf("s arr %d : %s\n", i, s_arr[i]);
    
    return s_arr;
}

void addERecord(char * first_executable_instruction, char ** symbol_table, int record_len, int end_line_num) {
	// add the end record in the last of program
	// set the length of object code in last t record
	char str_cur_t_record_size[intLength(cur_t_record_size)];
	sprintf(str_cur_t_record_size, "%X", cur_t_record_size/2); // convert int address to string
	object_program[cur_t_record_length_index] = fillZeroAtFirst(strdup(str_cur_t_record_size), 2); // set the length of object code of last t record

	// add end record
	char * e_record = calloc(9, sizeof(char));
	e_record[0] = '\n';
	e_record[1] = 'E';
	e_record[2] = ' ';
	if (first_executable_instruction != NULL && first_executable_instruction != "null") {
		char * temp = find(first_executable_instruction, symbol_table, record_len);
		if (temp == "0") {
			line_num = end_line_num;
			throwError("symbol of first_executable_instruction is not existed");
		}
		else {
			temp = split(temp, ' ')[1];
			strcat(e_record, fillZeroAtFirst(temp, 6));
		}
	}
	else {
		throwError("ERROR : first executable instruction is missed !\n"); // throw error and not print the object program at last
	}
	object_program[getLenStrArr(object_program)] = e_record;
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
			if (i == comma_index + 1) {
				if (line[i] != ' ') {
					new_str[index++] = line[i];
				}
			}
			else {
				new_str[index++] = line[i];
			}
		}
		return new_str;
    }
}		    

char * removeComment(char * line) {
    // remove comments, which are behind the dot
    char * n_str = calloc(strlen(line), sizeof(char));
	
	int have_comment = 0;
    for (int i = 0;i < strlen(line);i++) {
		if (line[i] == '.') {
			have_comment = 1;
			break;
		}
		else {
			n_str[i] = line[i];
		}
    }

	if (!have_comment) {
		return line;
	}
	else {
		return n_str;
	}
}

int isBasic(char * line) {
    char * second_place = split(line, ' ')[1];
    char * first_place = split(line, ' ')[0];
	int stop = 0;
	if (first_place != NULL && strcmp((first_place), "END") == 0) {
		stop = 1;
	}
		
	
    if (!stop && (second_place == NULL || second_place == '\0')) // avoid  
		return 0;
    if (!stop && strcmp(second_place, "START") == 0)
		return 1;
	else if (first_place != NULL && (strcmp(first_place, "END") == 0 || strcmp(first_place, "END") == 0))
    	return 2;
	else if (first_place != NULL && strcmp(first_place, "BASE") == 0 || strcmp(first_place, "BASE") == 0)
    	return 7;
    else if (strcmp(second_place, "WORD") == 0)
    	return 3;
    else if (strcmp(second_place, "RESW") == 0)
    	return 4;
    else if (strcmp(second_place, "BYTE") == 0)
    	return 5;
    else if (strcmp(second_place, "RESB") == 0)
    	return 6;
    else 
    	return 0;
}

char * isMnemonic(char * first_str, char * sec_str, char * third_str, char ** opcode_table, int record_len) {
    // check whether the instruction contains a valid mnemonic
    if (sec_str != NULL && find(sec_str, opcode_table, record_len) != "0") {
		// this instruction have three sperate string means the first string can be the symbol
		return "TEST Mnemonic";
    }
    else if (first_str != NULL && find(first_str, opcode_table, record_len) != "0") {
		return "Mnemonic";
    }
    else {
		if (first_str != NULL) {
			char temp[] = "mnemonic error, not a valid mnemonic : ";
			strcat(temp, first_str);
			throwError(temp);
		}
		else if (sec_str != NULL) {
			char temp[] = "mnemonic error, not a valid mnemonic : ";
			strcat(temp, sec_str);
			throwError(temp);
		}
		
		return "null";
    }
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

// check if the operand string is a number
int isNumber(char * s) {
	// omit the first char, which is not the real value of operand
	for (int i = 1;i < strlen(s);i++) {
		if (s[i] != 10 && !isdigit(s[i])) {
			// not a digit
			return 0;
		}
	}
	return 1;
}

char * makeObjectCodeFront(char * opcode, int n, int i, int x, int b, int p, int format) {
	// deal with the first 12 bit of object code, and return the final three char
	if (format == 2) {
		return opcode;
	}
	// use format to determine the e
	int e = 0; // extended
	if (format == 4) {
		e = 1;
	}

	char * first_three_char = calloc(3, sizeof(char));
	// first char is same as the opcode
	first_three_char[0] = opcode[0];

	// second char = 2 bits(opcode second num) + 2 bits(n,i)
	int second_char = 0;
	char * temp = calloc(1, sizeof(char));
	// part of 2 bits(opcode second num)
	int opcode_second_num = opcode[1] - '0'; // convert char to int
	if (opcode_second_num-8 >= 0) {
		opcode_second_num -= 8;
		second_char += 8;
	}
	if (opcode_second_num-4 >= 0) {
		second_char += 4;
	}
	// part of 2 bits(n,i)
	if (n) {
		second_char += 2;
	}
	if (i) {
		second_char += 1;
	}
	sprintf(temp, "%X", second_char);
	first_three_char[1] = temp[0];

	// third char = (x,b,p,e)
	int third_char = 0;
	char * temp1 = calloc(1, sizeof(char));
	if (x) {
		third_char += 8;
	}
	if (b) {
		third_char += 4;
	}
	if (p) {
		third_char += 2;
	}
	if (e) {
		third_char += 1;
	}
	sprintf(temp1, "%X", third_char);
	first_three_char[2] = temp1[0];

	return first_three_char;
}

char * omitFirstVal(char * s) {
	// omit the first char of string
	char * n_str = calloc(strlen(s)-1, sizeof(char));
	for (int i = 1;i < strlen(s);i++) {
		n_str[i-1] = s[i];
	}
	return n_str;
}

int getIndexOfVal(char * val, char ** arr, int arr_len) {
	// get the index of val in arr
	val = split(val, ',')[0];

	for (int i = 0;i < arr_len;i++) {
		if (strcmp(val, registers[i]) == 0) {
			return i;
		}
	}
	return -1;
}

char * makeObjectCodeLast(int address, int format, char * str_operand) {
	// num = number of half-byte char
	if (format == 2) {
		// return the index of register when use the format 2
		char * last_two_object_code = calloc(2, sizeof(char));
		char ** operand_registers = split(str_operand, ',');
		for (int i = 0;i < getLenStrArr(operand_registers);i++) {
			int index = getIndexOfVal(split(operand_registers[i], ' ')[0], registers, 9);
			char temp[intLength(index)];
			sprintf(temp, "%X", index);
			strcat(last_two_object_code, temp);
		}
		return fillCharAtLast(last_two_object_code, 2, '0');
	}

	int is_negative = 0;
	if (address < 0) {
		// negative, need to convert to two's complement
		is_negative = 1;
		address *= -1;
	}

	int object_code_address_length = formatAddressLen(format); // use format to determine the length of address in object code
	
	char operand[intLength(address)];
	sprintf(operand, "%X", address); // convert int address to string

	char * last_char = calloc(object_code_address_length, sizeof(char));
	char * temp = calloc(object_code_address_length, sizeof(char));
	sprintf(temp, "%X", strToHex(operand));
	int fill_zero_num = object_code_address_length - strlen(temp); // num of zero need to insert into the start of last three char to fill the empty char
	for (int i = 0;i < fill_zero_num;i++) {
		last_char[i] = '0';
	}
	int index = 0;
	for (int i = fill_zero_num;i < object_code_address_length;i++) {
		last_char[i] = temp[index++];
	}

	if (is_negative) {
		// convert to two's complement
		//printf("%s need to convert to two's complement\n", last_char);
		last_char = twosComplement(last_char);
	}
	return last_char;
}

int isProgramCounterRelative(int displacement) {
	if (displacement > -2048 && displacement < 2047) {
		// program counter relative
		return 1;
	}
	else {
		// base relative
		return 0;
	}
}

int getBaseAddress(char ** symbol_table) {
	// return the address of base from symbol table
}

char * getObjectCode(char * opcode, char * symbol_address, int program_counter, int format, int n, int i, int x, int opernad) {
	// count the displacement and use it and opcode to make a object code

	char * first_three_char;
	char * last_three_char;
	if (symbol_address == NULL && format != 2) {
		// forward reference
		first_three_char = makeObjectCodeFront(opcode, n, i, x, 0, 0, format);
		last_three_char = fillZeroAtFirst(NULL, formatAddressLen(format));
	}
	else {
		int displacement = strToHex(symbol_address) - program_counter;
		// first three char

		if (isProgramCounterRelative(displacement)) {
			first_three_char = makeObjectCodeFront(opcode, n, i, x, 0, 1, format);
		}
		else {
			//printf("opcode %s at %s, pc = %X is going to use base relative, as the displacement %d\n", opcode, symbol_address, program_counter, displacement);
			if (base == NULL) {
				throwError("BASE Register is not defined !"); // throw error and not print the object program at last
			}
			else {
				displacement = strConvertToHex(symbol_address) - decimalIntConvertToHex(base);
				//printf("new displacement is : %d, as base is %X, symbol : %X\n", displacement, decimalIntConvertToHex(base), strConvertToHex(symbol_address));
			}
			first_three_char = makeObjectCodeFront(opcode, n, i, x, 1, 0, format);
		}
		// last three char
		//printf("displacment : %d = sa : %X - pc : %X\n", displacement, strToHex(symbol_address), program_counter);
		last_three_char = makeObjectCodeLast(displacement, format, opernad);
	}
	//printf("symbol address : %s%s, format : %d\n", first_three_char, last_three_char, format);
	strcat(first_three_char, last_three_char); // combine first and last string to a complete object code of this line
	return first_three_char;
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

void insertValueInForwardRefTable(char * symbol, int address, int program_counter, char ** hash_table, int size, int format) {
	// record the forward reference symbol and the address need to be modified

	if (isEmptyStr(symbol) || getIndexOfVal(symbol, registers, 9) != -1) {
		// check if symbol is empty or is a register
		return;
	}

	char * temp_symbol = strdup(symbol);

	int val = getCodeOpcode(symbol);
	int index = val % size;
	
	char str_address[intLength(address)];
	sprintf(str_address, "%X", address); // convert int adddress to string
	if (format == 4) {
		// record the operand address which is format 4
		forward_ref_table_format_four_address[getLenStrArr(forward_ref_table_format_four_address)] = strdup(str_address);
	}
	strcat(str_address, "$"); // add specify symbol $ to seperate the address and program counter

	char str_program_counter[intLength(program_counter)];
	sprintf(str_program_counter, "%X", program_counter); // convert int adddress to string

	char * address_program_counter = strcat(str_address, str_program_counter);
	
	if (hash_table[index] == NULL) { // the bucket not have value
		strcat(symbol, " ");
		strcat(symbol, str_address);
		hash_table[index] = symbol;
	}
	else { // probing other empty bucket
		// linear probing the empty bucket
		int empty_bucket = -1;
		int have_looped = 0;
		for (int i = index;i < size;i++) {
			char ** s_arr = split(hash_table[i], ' ');
			if (s_arr != NULL && strcmp(s_arr[0], symbol) == 0) {
				strcat(hash_table[i], " ");
				strcat(hash_table[i], address_program_counter);
				return;
			}
			if (hash_table[i] == NULL) {
				empty_bucket = i;
				break;
			}
			if (i == size-1 && !have_looped) {
				// loop from head if not traverse all array
				size = index;
				i = 0;
				have_looped = 1;
			}
		}

		// check whether have empty bucket
		if (empty_bucket == -1) {
			printf("no more empty bucket %s\n", symbol);
		}
		else {
			strcat(symbol, " ");
			strcat(symbol, address_program_counter);
			hash_table[empty_bucket] = symbol;
		}
	}

	// record the line of forward reference
	char * str_line = calloc(sizeof(intLength(line_num)), sizeof(char));
	sprintf(str_line, "%d", line_num);
	strcat(temp_symbol, " ");
	strcat(temp_symbol, str_line);
	insertValueInHashTable(temp_symbol, symbol_line_table, size);
}

int formatAddressLen(int format) {
	// use format to determine the length of address part by how many half-bytes

	if (format == 2) {
		return 2;
	}
	else if (format == 3) {
		return 3;
	}
	else if (format == 4) {
		return 5;
	}
}

int isEmptyStr(char * s) {
	// check if string contains a valid symbol name
	for (int i = 0;i < strlen(s);i++) {
		if (s[i] != NULL && s[i] != ' ' && s[i] != '\t' && s[i] != '\n') {
			return 0;
		}
	}
	return 1;
}

// check the addressing type of operand, and make the object code
void dealWithAddressingType(char * mnemonic, char * operand, int format, char ** symbol_table, int record_len, int address, char ** opcode_table, char ** forward_ref_table, char ** object_code_table) {
	if (isIndexAddressing(mnemonic, operand)) {
        //printf("This is index addressing\n");
		char * opcode = getOpcode(find(mnemonic, opcode_table, record_len));
		int program_counter = address + format;
		char * symbol = split(operand, ',')[0]; // symbol is the string before comma
		char * symbol_address = find(symbol, symbol_table, record_len);
		if (symbol_address[0] != '0') {
			// symbol is existed in current symbol table
			symbol_address = split(symbol_address, ' ')[1];
			addTRecord(address, getObjectCode(opcode, symbol_address, program_counter, format, 1, 1, 1, operand), format);
		}
		else {
			// symbol is not existed in current symbol table
			insertValueInForwardRefTable(symbol, address, program_counter, forward_ref_table, record_len, format);
			addTRecord(address, getObjectCode(opcode, NULL, program_counter, format, 1, 1, 1, operand), format);
		}
    }
    else if (isInDirectAddresing(operand)) {
        //printf("This is indirect addressing\n");
		// make object code
		char * opcode = getOpcode(find(mnemonic, opcode_table, record_len));
		int program_counter = address + format;
		char * symbol = operand;
		char * symbol_address = find(symbol, symbol_table, record_len);
		if (symbol_address[0] != '0') {
			// symbol is existed in current symbol table
			symbol_address = split(symbol_address, ' ')[1];
			addTRecord(address, getObjectCode(opcode, symbol_address, program_counter, format, 1, 0 ,0, operand), format);
		}
		else {
			// symbol is not existed in current symbol table
			//printf("indirect forward addressing %s : %X\n", symbol, address);
			insertValueInForwardRefTable(omitFirstVal(symbol), address, program_counter, forward_ref_table, record_len, format);
			addTRecord(address, getObjectCode(opcode, NULL, program_counter, format, 1, 0, 0, operand), format);
		}
    }
    else if (isImmediateAddresing(operand)) {
        //printf("This is immediate addressing\n");
		// make object code
		char * opcode = getOpcode(find(mnemonic, opcode_table, record_len));
		//printf("opcode of %s is %s\n", mnemonic, opcode);
		if (isNumber(operand)) {
			// if the operand is a num, then the displacement part is equal to the num
			char * first_three_char = makeObjectCodeFront(opcode, 0, 1, 0, 0, 0, format);
			// convert decimal num to hex
			char temp[intLength(strToHex(omitFirstVal(operand)))];
			sprintf(temp, "%X", strToHex(omitFirstVal(operand)));
			char * last_three_char = makeObjectCodeLast(atoi(temp), format, operand);
			strcat(first_three_char, last_three_char);
			addTRecord(address, first_three_char, NULL); // although the extended format, it still does not need the m record with immediate addressing
		}
		else {
			// the operand is a symbol, then the displacement = symbol address - program counter
			int program_counter = address + format;
			char * symbol = omitFirstVal(operand);
			char * symbol_address = find(symbol, symbol_table, record_len);
			if (symbol_address[0] != '0') {
				// symbol is existed in current symbol table
				symbol_address = split(symbol_address, ' ')[1];
				addTRecord(address, getObjectCode(opcode, symbol_address, program_counter, format, 0, 1, 0, operand), NULL); // although the extended format, it still does not need the m record with immediate addressing
			}
			else {
				// symbol is not existed in current symbol table
				insertValueInForwardRefTable(symbol, address, program_counter, forward_ref_table, record_len, format);
				addTRecord(address, getObjectCode(opcode, NULL, program_counter, format, 0, 1, 0, operand), NULL); // although the extended format, it still does not need the m record with immediate addressing
			}
		}
    }
    else {
		//printf("This is relative addressing\n");
		// make object code
		char * opcode = getOpcode(find(mnemonic, opcode_table, record_len));
		int program_counter = address + format;
		char * symbol = operand;
		char * symbol_address = find(symbol, symbol_table, record_len);
		if (symbol_address[0] != '0') {
			// symbol is existed in current symbol table
			symbol_address = split(symbol_address, ' ')[1];
			addTRecord(address, getObjectCode(opcode, symbol_address, program_counter, format, 1, 1, 0, operand), format);
		}
		else {
			// symbol is not existed in current symbol table
			insertValueInForwardRefTable(symbol, address, program_counter, forward_ref_table, record_len, format);
			addTRecord(address, getObjectCode(opcode, NULL, program_counter, format, 1, 1, 0, operand), format);
		}
    }
}

void findSymbolAddress(char ** symbol_table, int record_len, char * symbol) {
	// find address of correspond symbol from symbol table
    printf("Symbol Table : {");
    int first_symbol = 1;
	for (int i = 0;i < record_len;i++) {
	    if (symbol_table[i] != NULL) {
			char ** str = split(symbol_table[i], ' ');
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

void printSymbolTable(char ** symbol_table, int record_len) {
    printf("Symbol Table : {");
    int first_symbol = 1;
	for (int i = 0;i < record_len;i++) {
	    if (symbol_table[i] != NULL) {
			char ** str = split(symbol_table[i], ' ');
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

int getFormatType(char * mnemonic) {
	// get format of instruction
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
		return 3;
	}
}

int isExtendedAddress(char * address) {
	for (int i = 0;i < getLenStrArr(forward_ref_table_format_four_address);i++) {
		if (strcmp(forward_ref_table_format_four_address[i], address) == 0) {
			// this address is in the array of extended format address records
			return 1;
		}
	}
	return 0;
}

void dealForwardRef(char * symbol, char ** forward_ref_table, int record_len, int symbol_address) {
	// deal with the instructions are forward reference relative symbol
	char * symbol_refs = find(symbol, forward_ref_table, record_len); // all lines forward reference this symbol 

	if (symbol_refs != "0") {
		// add all T records about previous forward reference
		char ** ref_records = split(symbol_refs, ' ');
		int ref_records_len = getLenStrArr(ref_records);

		for (int i = 1;i < ref_records_len;i++) {
			char * t_record = calloc(30, sizeof(char));
			// get address and program counter of relative symbol
			char ** address_program_counter = split(ref_records[i], '$');
			int address = strToHex(address_program_counter[0]);
			int program_counter = strToHex(address_program_counter[1]);
			int displacement = symbol_address - program_counter;
			// put the address of forward reference
			//printf("T : %X 02 ", address+1);
			strcat(t_record, "\nT ");
			char str_address[intLength(address+1)];
			sprintf(str_address, "%X", address+1); // convert int adddress to string
			strcat(t_record, fillZeroAtFirst(str_address, 6));
			strcat(t_record, " 02 ");
			// check if origin forward reference address is extended
			char str_origin_address[intLength(address)];
			sprintf(str_origin_address, "%X", address); // convert int adddress to string
			if (isExtendedAddress(str_origin_address)) {
				// put the real symbol address back to origin forward reference address
				//printf("%s is extended address\n", str_address);
				strcat(t_record, omitFirstVal(makeObjectCodeLast(symbol_address, 4, NULL)));
			}
			else {
				if (isProgramCounterRelative(displacement)) {
					// program counter relative
					// printf("2%s\n", makeObjectCodeLast(displacement, 3));
					strcat(t_record, "2");

				}
				else {
					// base relative
					// printf("4%s\n", makeObjectCodeLast(displacement, 3));
					strcat(t_record, "4");
				}
				// put the displacement back to origin forward reference address
				strcat(t_record, makeObjectCodeLast(displacement, 3, NULL));
			}
			
			object_program[getLenStrArr(object_program)] = t_record;
			cur_t_record_new_line = 1;
		}
		// clean the forward reference records of this symbol
		symbol_refs = "0";
	}
}

void checkUndefinedSymbol(char ** forward_ref_table, char ** symbol_table, int record_len) {
	// check if still exist the redundant forward reference symbol which not be resolved
	for (int i = 0;i < record_len;i++) {
		// printf("aa : %d\n", forward_ref_table[i] == NULL);
		if (forward_ref_table[i] != NULL) {
			// check if this symbol exist
			char * symbol = split(forward_ref_table[i], ' ')[0];
			if (find(symbol, symbol_table, record_len) == "0") {
				// symbol not defined, make error msg
				char * error_msg = calloc(30, sizeof(char));
				strcat(error_msg, "Undefined Symbol : ");
				strcat(error_msg, symbol);
				char * symbol_line = find(symbol, symbol_line_table, record_len);
				line_num = atoi(split(symbol_line, ' ')[1]);			
				throwError(error_msg);
			}
		}
	}
}

void addMRecord() {
	// add the m records into the object program
	int object_program_len = getLenStrArr(object_program);
	for (int i = 0;i < getLenStrArr(m_records);i++) {
		if (m_records[i] == NULL) {
			break;
		}
		else {
			object_program[object_program_len++] = m_records[i];
		}
	}
}

int checkSymbol(char * symbol, char * operand, char ** opcode_table, int record_len) {
	if (find(symbol, opcode_table, record_len) != "0") {
		throwError("Symbol name conflict with mnemonic");
	}
	else if (operand != NULL && strcmp(symbol, operand) == 0) {
		throwError("Symbol name conflict with operand");
	}
	else {
		return 1;
	}
	return 0; // error occured
}

char * numIsDecimal(char * n) {
	for (int i = 0;i < strlen(n);i++) {
		if (!(n[i] >= '0' && n[i] <= '9')) {
			throwError("Operand must be decimal in the WORD, RESW, RESB");
			return 0;
		}
	}
	return 1;
}

int onlyCommand(char * s) {
	// if this line only contains the command, then return 1
	for (int i = 0;i < strlen(s);i++) {
		if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n') {
			if (s[i] != '.') {
				return 0;
			}
			else {
				return 1;
			}
		}
	}
	return 1;
}

void scanSoureProgram(char ** all_str, int record_len, char ** opcode_table, int opcode_table_record_len) {
    // scan the source program one time
    char ** symbol_str_arr = calloc(record_len, sizeof(char *));
    char ** symbol_table = hashTable("", 0, record_len);

	int end_line_num;
    int is_started = 0;
	int address;
    char ** object_code_table = calloc(record_len, sizeof(char *)); // object code table
	int object_code_table_index = 0;
	char ** forward_ref_table = calloc(record_len, sizeof(char *)); // forward reference table
	char * first_executable_instruction = "null";
	symbol_line_table = calloc(record_len, sizeof(char *)); // record the symbol is on which line of source program
	m_records = calloc(record_len, sizeof(char *));
    for (int i = 0;i < record_len;i++) {
		char * line = all_str[i];
		
		line = commaRemoveSpace(line); // remove a space behind the comma 
		line = removeComment(line); // remove comment, which is behind the dot

		// check this line is belong to normal instruction or basic function to use different way to locate
		line_num = i+1;
		//printf("\n%d : %s\n", i+1, line);
		int basic_instruction = isBasic(line);
		//printf("basic instruction is %d\n", basic_instruction);
		//printf("address : %x, basic instruction : %d\n", address, basic_instruction);
		
		char * object_code; // object code of this line
		int failed_symbol = 0;

		if (basic_instruction == 1) {
			// START
			is_started = 1;
			char ** s_arr = split(line, ' ');
			address = (int)strtol(s_arr[2], NULL, 16);
			//printf("Program name is %s\n", s_arr[0]);
			//printf("start from this line\n\n");
			// add info of header record except the length of object program
			addHRecordStart(s_arr[0], address);
		}
		else if (basic_instruction == 7) {
			// virtual address space
			//printf("BASE is virtual instruction code\n");
			char ** s_arr = split(line, ' ');
			base_symbol = s_arr[1]; // record the symbol which pointered by base
		}
		else {
			char ** s_arr = split(line, ' ');
			if (isSymbol(s_arr)) {
				// print the address of symbol
				char * symbol_name = s_arr[0];

				// check symbol is meet the limits
				if (checkSymbol(symbol_name, s_arr[2], opcode_table, record_len)) {

					// check if is symbol
					if (base_symbol != NULL && strcmp(symbol_name, base_symbol) == 0) {
						base = address;
					}

					// convert int adddress to string
					// count the length of int(address)
					int address_length;
					if (address == 0) {
						address_length = 0;
					}
					else {
						address_length = ((ceil(log10(address))+1)*sizeof(char)); // length of int
					}

					// check if symbol existed
					if (find(symbol_name, symbol_table, record_len) != "0") {
						// duplicated symbol, throw error
						char temp[] = "Duplicated symbol : ";
						strcat(temp, symbol_name);
						throwError(temp);
					}
					else {
						char str_address[address_length];
						sprintf(str_address, "%X", address); // convert int adddress to string
						// concat the symbol name and a space and address
						char * temp_symbol_name = strdup(symbol_name);
						char * hash_value = strcat(temp_symbol_name, " ");
						hash_value = strcat(temp_symbol_name, str_address);

						// insert into symbol table
						insertValueInHashTable(hash_value, symbol_table, record_len);

						// deal with forward reference symbol
						dealForwardRef(symbol_name, forward_ref_table, record_len, address);
					}
				}
				else {
					failed_symbol = 1;
				}
			}
			if (basic_instruction) {
				// contains a basic instruction
				if (!is_started) {
					if (!onlyCommand(line)) {
						throwError("instruction before the program start");
					}
				}
				else {
					char ** s_arr = split(line, ' ');

					if (basic_instruction == 2) {
						// END
						//printf("END of the program\n");
						first_executable_instruction = s_arr[1];
						end_line_num = i+1;
					}
					else if (basic_instruction == 3) {
						// WORD
						//printf("WORD is pesudo instruction code\n");
						if (numIsDecimal(s_arr[2])) {
							int space = 3;
							address += space;
							object_code = calloc(6, sizeof(char));	
							sprintf(object_code, "%d", s_arr[2]); // convert int operand to string
							object_code_table[object_code_table_index++] = object_code;
							addTRecord(address, object_code, 3);
						}
					}
					else if (basic_instruction == 4) {
						// RESW
						//printf("RESW is pesudo instruction code\n");
						if (numIsDecimal(s_arr[2])) {
							int space = atoi(s_arr[2]) * 3;
							address += space;
							cur_t_record_new_line = 1;
						}
					}
					else if (basic_instruction == 5) {
						// BYTE
						//printf("BYTE is pesudo instruction code\n");
						int space;
						object_code = calloc(6, sizeof(char));	
						if (s_arr[2][0] == 'C') {
							// char
							// the nums of char
							int char_nums = strlen(s_arr[2])-3; // minus the two single quotation and one type and a space at the last char
							space = char_nums;
							//printf("cccc char nums %s : %d\n", s_arr[2], char_nums);
							//printf("c char nums : %d %c %c", strlen(s_arr[1]), s_arr[2][5], s_arr[2][6]==' ');
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
						object_code_table[object_code_table_index++] = object_code;
						addTRecord(address, object_code, 3);
						address += space;
					}
					else if (basic_instruction == 6) {
						// RESB
						if (numIsDecimal(s_arr[2])) {
							//printf("RESB is pesudo instruction code\n");
							int space = atoi(s_arr[2]);
							address += space;
							cur_t_record_new_line = 1;
						}
					}
				}	
			}
			else {
				// not the basic instruction
				if (is_started) {
					// check whether it is in opcode table
					char * mnemonic = isMnemonic(s_arr[0], s_arr[1], s_arr[2], opcode_table, opcode_table_record_len);
					if (mnemonic != "null") {
						//printf("this is a valid mnemonic %s\n", mnemonic);
						// valid mnemonic
						int operand_index = getLenStrArr(s_arr)-1; // operand is the last index of a line
						char * operand = "";
						if (operand_index > 0) {
							// operand is not the only string in line
							operand = s_arr[operand_index]; // operand is the last index of a line
						}

					    // check format of instruction
						int mnemonic_index = operand_index-1;
						if (mnemonic_index < 0) {
							mnemonic_index = 0;
						}
						int format_size = getFormatType(s_arr[mnemonic_index]);

						
						// check if mnemonic is test or normal
						if (mnemonic == "Mnemonic") {
							// check the addressing type of operand
							if (s_arr[0] != NULL && strcmp(s_arr[0], "RSUB") == 0) {
								char * temp = getOpcode(find(s_arr[0], opcode_table, record_len));
								char * first_object_code = fillCharAtLast(makeObjectCodeFront(temp, 1, 1, 0, 0, 0, 3), 6, '0');
								addTRecord(address, first_object_code, format_size);
								//printf("rsub object code : %s\n", first_object_code);
								// 
								if (s_arr[1] != NULL) {
									throwError("RSUB can not have oeprand");
								}
							}

							else {
								dealWithAddressingType(s_arr[mnemonic_index], operand, format_size, symbol_table, record_len, address, opcode_table, forward_ref_table, object_code_table);
							}
						}
						else {
							// TEST Mnemonic
							
							//printf("Label : %s Mnemonic: %s operand : %s\n", s_arr[0], s_arr[1], operand);
							// print the address of symbol
							char * symbol_name = s_arr[0];

							// check symbol is meet the limits
							if (!failed_symbol && checkSymbol(symbol_name, s_arr[2], opcode_table, record_len)) {
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
								sprintf(str_address, "%X", address); // convert int adddress to string
								// concat the symbol name and a space and address
								char * temp_symbol_name = strdup(symbol_name);
								char * hash_value = strcat(temp_symbol_name, " ");
								hash_value = strcat(temp_symbol_name, str_address);
								// insert into symbol table
								insertValueInHashTable(hash_value, symbol_table, record_len);

								if (s_arr[1] != NULL && strcmp(s_arr[1], "RSUB") == 0) {
									char * temp = getOpcode(find(s_arr[1], opcode_table, record_len));
									char * first_object_code = fillCharAtLast(makeObjectCodeFront(temp, 1, 1, 0, 0, 0, 3), 6, '0');
									addTRecord(address, first_object_code, format_size);
									//printf("rsub object code : %s\n", first_object_code);
									// 
									if (s_arr[2] != NULL) {
										throwError("RSUB can not have oeprand");
									}
								}
								else {
									dealWithAddressingType(s_arr[mnemonic_index], operand, format_size, symbol_table, record_len, address, opcode_table, forward_ref_table, object_code_table);
								}
								
								/*
								// check if symbol existed
								if (find(symbol_name, symbol_table, record_len) != "0") {
									// duplic  ated symbol, throw error
									char temp[] = "Duplicated symbol : ";
									strcat(temp, symbol_name);
									throwError(temp);
								}
								else {
									char str_address[address_length];
									sprintf(str_address, "%X", address); // convert int adddress to string
									// concat the symbol name and a space and address
									char * temp_symbol_name = strdup(symbol_name);
									char * hash_value = strcat(temp_symbol_name, " ");
									hash_value = strcat(temp_symbol_name, str_address);

									// insert into symbol table
									insertValueInHashTable(hash_value, symbol_table, record_len);

									// deal with forward reference symbol
									dealForwardRef(symbol_name, forward_ref_table, record_len, address);
								}
								*/

								// check the addressing type of operand
								//dealWithAddressingType(s_arr[mnemonic_index], NULL, format_size, symbol_table, record_len, address, opcode_table, forward_ref_table, object_code_table);
							}
						}
						// normal instruction take 3B
						address += format_size;
					}
				}
				else {
					if (!onlyCommand(line)) {
						throwError("instruction before the program start");
					}
				}
			}
		}
		/*
		*/
    }
	
	addHRecordLength(address);
	addMRecord();
	addERecord(first_executable_instruction, symbol_table, record_len, end_line_num);
	checkUndefinedSymbol(forward_ref_table, symbol_table, record_len);
    return symbol_table;
}

char ** mkOpcodeTable(int source_program_record_len) {
    // find relative opcode and output
    char * filename = "./opcode.txt";
    int record_len = recordLen(filename);
	if (source_program_record_len > record_len) {
		record_len = source_program_record_len;
	}
    char ** all_str = readFile(filename);
    char ** hash_table = hashTable(all_str, recordLen(filename), record_len); // make a hash table with data in opcode.txt
    return hash_table;
}

int main() {
    char * filename = "./testprog3.S"; // source program name
    int record_len = recordLen(filename);
    char ** all_str = readFile(filename);
	
    // make opcode table
    int opcode_table_record_len = recordLen("opcode.txt");
    char ** opcode_table = mkOpcodeTable(record_len);
	
	// make object program array
	object_program_size = record_len*3;
	object_program = calloc(object_program_size, sizeof(char *));
	forward_ref_table_format_four_address = calloc(object_program_size, sizeof(char *));

    // scan the source program one time
    scanSoureProgram(all_str, record_len, opcode_table, opcode_table_record_len);

	// print object program, if no error occured
	printObjectProgram();
}
