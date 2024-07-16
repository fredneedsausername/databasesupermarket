#pragma once

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 300
char databaseFilename[1000];

typedef struct {
	char* name;
	int storage;
	float price;
} Record;

int init_database(char* path);
/* return values:
* 0: all successful
* -1: path was NULL
* -2: file did not exist/open correctly
*/
Record* line_to_record(char* line);
void destroy_record_struct(Record* record);
char* record_to_line(Record* record);
void print_record(Record* record);
void print_all_records();
int insert_record(char* name, int storage, float price);
/* return values:
	0: record inserted succesfully
	-1: record with that name already exists
	-2: memory allocation failed
	-3: could not open file
*/
void remove_line(char*, int);
void remove_record(char* file_path, const char* name);
Record* query_record(char* name);

int update_record(const char* databaseFilename, const char* record_name, int field, void* value);
void init_database_prompt();
void main_menu(Record* record);
void insert_record_prompt(Record* record);
void remove_record_prompt();
void update_record_prompt();
void print_record_prompt();

void menu();
