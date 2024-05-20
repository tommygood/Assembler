#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int recordLen(char * filename) {
    // get record nums in opcode.txt
    FILE* ptr;
    char str[50];
    ptr = fopen(filename, "r");
 
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }
 
    int record_len = 0;
 
    // counting the length of record 
    while (fgets(str, 50, ptr) != NULL) {
        record_len++;
    }
    fclose(ptr);

    return record_len;
}

char ** readFile(char * filename)
{
    FILE* ptr;
    char str[50];
    ptr = fopen(filename, "r");
    int index = 0;
    int record_len = recordLen(filename);
    char ** all_str = calloc(record_len, sizeof(char *));

    // storing each record
    while (fgets(str, 50, ptr) != NULL) {
        all_str[index++] = strdup(str);
    }

    fclose(ptr);
    return all_str;
}

int getCodeOpcode(char * c) {
    int code = 0;
    for (int i = 0;i < strlen(c);i++) {
        if (c[i] == ' ') {
            break;
        }
        code += (int)c[i];
    }
    return code;
}

char ** hashTable(char ** all_str, int record_len, int size) {
    // make and return a hash table
    char ** hash_table = calloc(size, sizeof(char*));
    hash_table[0] = NULL;
    hash_table[1] = NULL;

    for (int i = 0;i < record_len;i++) {
        int val = getCodeOpcode(all_str[i]);
        int index = val % record_len;
        //printf("%s, %d\n", all_str[i], index);
        if (hash_table[index] == NULL) { // the bucket not have value
            hash_table[index] = all_str[i];
        }
        else { // probing other empty bucket
            // linear probing the empty bucket
            int empty_bucket = -1;
            for (int i = 0;i < record_len;i++) {
                if (hash_table[i] == NULL) {
                    empty_bucket = i;
                    break;
                }
            }

            // check whether have empty bucket
            if (empty_bucket == -1) {
                printf("no more empty bucket %s\n", all_str[i]);
            }
            else {
                hash_table[empty_bucket] = all_str[i];
            }
        }
    }

    return hash_table;
}

char * removeLastRedundantChar(char * s) {
	// remove last character if is junk value
	char * n_str = calloc(strlen(s), sizeof(char));
	int index = 0;
	for (int i = 0;i < strlen(s);i++) {
		if (s[i] != NULL && s[i] != ' ' && s[i] != '\n') {
			n_str[index++] = s[i];
		}
	}
	return n_str;
}

int cmpMnemonic(char * str, char * c, int record_len) {
    for (int i = 0;i < record_len;i++) {
        if (str[i] == ' ') {
            if (i != strlen(c)) { // not same length, then not same string
                // minus a char of new line
	    	    //printf("%s and %s are not the same string, as length different %s %s\n", str, c, strlen(str), strlen(c));
	    	    //printf("%s and %s are not the same string, as length different, %d %d [%c]\n", str, c, strlen(c), i, c[4]);
                return 1; // not same string
            }
            break;
        }
        if (str[i] != c[i]) {
	        //printf("%s and %s are not the same string, as the char different %c %c\n", str, c, str[i], c[i]);
            return 1; // not same string
        }
    }
    //printf("%s and %s are same string\n", str, c);
    return 0; // str and c are same string
}

char * getOpcode(char * c) { // get opcode part of each line
    char * opcode = malloc(strlen(c) * sizeof(char));
    int index = 0;
    int start_opcode = 0;
    char * temp = malloc(sizeof(char));
    for (int i = 0;i < strlen(c);i++) {
        if (c[i] == ' ') {
            start_opcode = 1;
            continue;
        }
        if (start_opcode) {
            temp[0] = c[i]; // temp is a char pointer to temp store the char
            opcode[index++] = c[i];
        }
    }
    for (int i = index;i < strlen(opcode);i++) {
        opcode[i] = 0;
    }
    return opcode;
}

int getCode(char * c) {
    int code = 0;
    // last char is new line, so ignore it
    for (int i = 0;i < strlen(c)-1;i++) {
        code += (int)c[i];
    }
    return code;
}

char * find(char * mnemonic, char ** hash_table, int record_len) {
    // omit the extended symbol
	if (mnemonic[0] == '+') {
		char * n_str = calloc(strlen(mnemonic), sizeof(char));
		int index = 0;
		for (int i = 1;i < strlen(mnemonic);i++) {
			n_str[index++] = mnemonic[i];
		}
		mnemonic = n_str;
	}

    // remove last character if is junk value
    mnemonic = removeLastRedundantChar(mnemonic);

    int mnemonic_index = getCode(mnemonic);
    //int record_len = sizeof(hash_table) / sizeof(hash_table[0]);
    mnemonic_index = mnemonic_index % record_len;
    int temp_mnemonic_index = mnemonic_index;

    while (cmpMnemonic(hash_table[mnemonic_index], mnemonic, record_len)) {
	//printf("hash value : %s %s\n", hash_table[mnemonic_index], mnemonic);
        mnemonic_index++;
        if (mnemonic_index == temp_mnemonic_index) {
            // stop finding when back to start index
            //printf("can not found the opcode of %s\n", mnemonic);
            //printf("invalid\n");
            return "0";
        }
        if (mnemonic_index >= record_len) {
            // find from 0
            mnemonic_index = 0;
        }
    }

    // output
    //printf("the relative opcode of %s = %s", mnemonic, getOpcode(hash_table[mnemonic_index]));     
    char * opcode = getOpcode(hash_table[mnemonic_index]);
    //printf("opCode : %s\n", opcode);
    return opcode;
}

/*
int main() {

    // input menmonic
    //char mnemonic[20];
    //printf("search (Input a mnemonic) : ");
    //fgets(mnemonic, 20, stdin);

    // find relative opcode and output
    char s[3] = "ADD";
    find(s);

    return 0;
}
*/
