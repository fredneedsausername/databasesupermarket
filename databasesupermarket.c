
#include "databasesupermarket.h"


int init_database(char* path) {
	if (path == NULL) return -1;
	strcpy_s(databaseFilename, sizeof(databaseFilename), path);
	FILE* file = fopen(databaseFilename, "r");
	if (file != NULL) {
		fclose(file);
		return 0;
	}
	else return -2;
}

Record* line_to_record(char* line) {

	if (line == NULL) exit(100);
	char* dupline = _strdup(line);

	char* name = strtok(dupline, ","); 
	char* storage = strtok(NULL, ","); 
	char* price = strtok(NULL, ","); 

	int istorage = atoi(storage);
	float fprice = (float)atof(price);
	char* sname = _strdup(name);

	Record* ret = (Record*)calloc(sizeof(Record), 1);

	ret->name = sname;
	ret->price = fprice;
	ret->storage = istorage;

	free(dupline);

	return ret;
}

void destroy_record_struct(Record* record) {
	if (record == NULL) return;
	free(record->name);
	free(record);
}

char* record_to_line(Record* record) {
	size_t sizeOfDestination = sizeof(char) * (MAX_LINE_SIZE + 1);
	char* destination = (char*)calloc(sizeOfDestination, 1);
	sprintf_s(destination, sizeOfDestination, "%s,%d,%.2f", record->name, record->storage, record->price);
	return destination;
	
}

void print_record(Record* record) {
	if (record == NULL) return;
	printf("name: %s,\t\t storage: %d,\t\t price: %.2f\n", record->name, record->storage, record->price);
}

void print_all_records() {
	char buffer[MAX_LINE_SIZE + 1];
	FILE* file = fopen(databaseFilename, "r");
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		Record* record = line_to_record(buffer);
		print_record(record);
		destroy_record_struct(record); // MEMORY LEAK: destroy_record_struct()
	}
	fclose(file);
}

int insert_record(char* name, int storage, float price) {
	char buffer[MAX_LINE_SIZE + 1];
	int name_already_exists = 0;
	FILE* file = fopen(databaseFilename, "r+");
	if (file == NULL) {
		return -3; // Could not open file
	}

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		Record* record = line_to_record(buffer);
		if (_stricmp(name, record->name) == 0) {
			name_already_exists = 1;
			destroy_record_struct(record);
			break;
		}
		destroy_record_struct(record);
	}

	if (name_already_exists) {
		fclose(file);
		return -1;
	}

	fseek(file, 0, SEEK_END); // Move to the end of the file for appending
	Record* newRecord = (Record*)calloc(1, sizeof(Record));
	if (newRecord == NULL) {
		fclose(file);
		return -2; // Memory allocation failed
	}

	newRecord->name = _strdup(name);
	if (newRecord->name == NULL) {
		free(newRecord);
		fclose(file);
		return -2; // Memory allocation failed
	}
	newRecord->storage = storage;
	newRecord->price = price;

	fputs("\n", file);
	char* line = record_to_line(newRecord);
	if (line == NULL) {
		free(newRecord->name);
		free(newRecord);
		fclose(file);
		return -2; // Memory allocation failed
	}
	
	int remove_first_line = 0; 
	if (ftell(file) <= 2) { 
		remove_first_line = 1; 
	}

	fputs(line, file);
	free(line);
	destroy_record_struct(newRecord);
	fflush(file);
	fclose(file);
	if (remove_first_line == 1) { 
		remove_line(databaseFilename, 1); 
	} 
	return 0;
}

void remove_line(char* file_path, int line_number) {
	FILE* file = fopen(file_path, "r");
	if (!file) {
		perror("Error opening file for reading");
		return;
	}

	FILE* temp_file = tmpfile();
	if (!temp_file) {
		fclose(file);
		perror("Error creating temporary file");
		return;
	}

	char buffer[MAX_LINE_SIZE + 1];
	int current_line = 1;
	int error_flag = 0;

	while (fgets(buffer, MAX_LINE_SIZE + 1, file)) {
		if (ferror(file)) {
			perror("Error reading from file");
			error_flag = 1;
			break;
		}
		if (current_line != line_number) {
			if (fputs(buffer, temp_file) == EOF) {
				perror("Error writing to temporary file");
				error_flag = 1;
				break;
			}
		}
		current_line++;
	}

	fclose(file);

	if (error_flag) {
		fclose(temp_file);
		return;
	}

	errno_t err = fopen_s(&file, file_path, "w");
	char err_msg[256];

	if (err != 0) {
		strerror_s(err_msg, sizeof(err_msg), err);
		fprintf(stderr, "Error opening file: %s\n", err_msg);
		fclose(temp_file);
		return;
	}

	if (!file) {
		fclose(temp_file);
		perror("Error opening file for writing");
		return;
	}

	rewind(temp_file);
	int first_line_written = 0;
	while (fgets(buffer, MAX_LINE_SIZE + 1, temp_file)) {
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
		}
		if (first_line_written) {
			fputc('\n', file);
		}
		first_line_written = 1;
		if (fputs(buffer, file) == EOF) {
			perror("Error writing to file");
			error_flag = 1;
			break;
		}
	}

	if (fclose(file) == EOF) {
		perror("Error closing the file");
		error_flag = 1;
	}

	if (fclose(temp_file) == EOF) {
		perror("Error closing the temporary file");
		error_flag = 1;
	}

	if (error_flag) {
		fprintf(stderr, "An error occurred during file operations.\n");
	}
}

void remove_record(char* file_path, const char* name) {
	FILE* file = fopen(file_path, "r");
	if (!file) {
		perror("Error opening file for reading");
		return;
	}

	char buffer[MAX_LINE_SIZE];
	int line_number = 1;
	int found = 0;

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		Record* record = line_to_record(buffer);
		if (_stricmp(name, record->name) == 0) {
			found = 1;
			break;
		}
		free(record);
		line_number++;
	}

	fclose(file);

	if (found) {
		remove_line(file_path, line_number);
	}
	else {
		printf("Record with name '%s' not found.\n", name);
	}
}

Record* query_record(char* name) {
	char buffer[MAX_LINE_SIZE];
	FILE* file = fopen(databaseFilename, "r");
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		Record* record = line_to_record(buffer);
		if (_stricmp(name, record->name) == 0) {
			fclose(file);
			return record;
		}
		destroy_record_struct(record); 
	}
	fclose(file);
	return NULL;
}

int update_record(const char* databaseFilename, const char* record_name, int field, void* value) {
	FILE* file = fopen(databaseFilename, "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	FILE* tempFile = fopen("temp.txt", "w");
	if (tempFile == NULL) {
		perror("Error opening temporary file");
		fclose(file);
		return -1;
	}

	char line[256];
	int found = 0;

	while (fgets(line, sizeof(line), file)) {
		char name[100];
		int storage;
		float price;
		sscanf_s(line, "%200[^,],%d,%f", name, sizeof(name), &storage, &price);

		if (strcmp(name, record_name) == 0) {
			found = 1;
			switch (field) {
			case 1:
				strcpy_s(name, sizeof(name), (char*)value);
				break;
			case 2:
				storage = *(int*)value;
				break;
			case 3:
				price = *(float*)value;
				break;
			default:
				printf("Invalid field\n");
				fclose(file);
				fclose(tempFile);
				return -1;
			}
		}
		fprintf(tempFile, "%s,%d,%.2f\n", name, storage, price);
	}

	fclose(file);
	fclose(tempFile);

	if (!found) {
		printf("Record not found\n");
		remove("temp.txt");
		return -1;
	}

	remove(databaseFilename);
	rename("temp.txt", databaseFilename);

	return 0;
}

void init_database_prompt() {
	char init_database_string[1000];
	int retInitDatabase;

	while (1) {
		printf("Enter the database file name:\n");
		fgets(init_database_string, sizeof(init_database_string), stdin);
		init_database_string[strcspn(init_database_string, "\n")] = '\0';

		retInitDatabase = init_database(init_database_string);
		if (retInitDatabase == -1 || retInitDatabase == -2) {
			perror("Error opening file");
		}
		else {
			break;
		}
	}
}

void main_menu(Record* record) {
	char buffer[1000];
	int input;

	while (1) {
		printf("Welcome to the main menu, select one of the following:\n");
		printf("0: exit\n");
		printf("1: print all records\n");
		printf("2: add a record\n");
		printf("3: remove a record\n");
		printf("4: update a record\n");
		printf("5: print a record\n");
		fgets(buffer, sizeof(buffer), stdin);
		input = atoi(buffer);

		switch (input) {
		case 0:
			return;
		case 1:
			print_all_records();
			break;
		case 2:
			insert_record_prompt(record);
			break;
		case 3:
			remove_record_prompt();
			break;
		case 4:
			update_record_prompt();
			break;
		case 5:
			print_record_prompt();
			break;
		default:
			printf("Invalid input\n");
		}
	}
}

void insert_record_prompt(Record* record) {
	char buffer[1000];
	int ret;

	printf("Insert the new record's name:\n");
	fgets(buffer, sizeof(buffer), stdin);
	if (buffer[0] == '\n') {
		printf("Empty name\n");
		return;
	}
	buffer[strcspn(buffer, "\n")] = '\0';
	record->name = _strdup(buffer);

	printf("Insert the new record's storage:\n");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strcspn(buffer, "\n")] = '\0';
	record->storage = atoi(buffer);

	printf("Insert the new record's price:\n");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strcspn(buffer, "\n")] = '\0';
	record->price = atof(buffer);

	ret = insert_record(record->name, record->storage, record->price);
	if (ret == -1) {
		printf("Record with that name already exists\n");
	}
	else if (ret == 2 || ret == 3) {
		printf("Error creating record\n");
	}
	else if (ret == 0) {
		printf("Record successfully added\n");
	}
}

void remove_record_prompt() {
	char buffer[1000];

	printf("Insert name of record to remove:\n");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strcspn(buffer, "\n")] = '\0';
	Record* temp = query_record(buffer);
	if (temp == NULL) {
		printf("Record does not exist\n");
		return;
	}
	remove_record(databaseFilename, buffer);
	printf("Record removed correctly\n");
}

void update_record_prompt() {
	char buffer[1000];
	char* record_name;
	int update_choice;

	printf("Insert name of record to update:\n");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strcspn(buffer, "\n")] = '\0';

	Record* record_temp = query_record(buffer);
	if (record_temp == NULL) {
		printf("Inexistent record\n");
		return;
	}
	destroy_record_struct(record_temp);

	record_name = _strdup(buffer);

	printf("0: Main menu\n");
	printf("1: Update name\n");
	printf("2: Update storage\n");
	printf("3: Update price\n");
	fgets(buffer, sizeof(buffer), stdin);
	update_choice = atoi(buffer);

	switch (update_choice) {
	case 0:
		free(record_name);
		return;
	case 1:
		printf("Enter new name:\n");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';
		update_record(databaseFilename, record_name, 1, buffer);
		break;
	case 2:
		printf("Enter new storage:\n");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';
		int new_storage = atoi(buffer);
		update_record(databaseFilename, record_name, 2, &new_storage);
		break;
	case 3:
		printf("Enter new price:\n");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';
		float new_price = atof(buffer);
		update_record(databaseFilename, record_name, 3, &new_price);
		break;
	default:
		printf("Invalid choice\n");
	}
	free(record_name);
}

void print_record_prompt() {
	char buffer[1000];

	printf("Insert name of record to print:\n");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strcspn(buffer, "\n")] = '\0';

	Record* temp = query_record(buffer);
	if (temp == NULL) {
		printf("Record does not exist\n");
		return;
	}
	print_record(temp);
	destroy_record_struct(temp);
}

void menu() {
	Record* record = (Record*)calloc(1, sizeof(Record));
	if (record == NULL) {
		perror("Memory allocation failed");
		exit(EXIT_FAILURE);
	}

	init_database_prompt();
	main_menu(record);

	destroy_record_struct(record);
}

int main() {
	menu();
}
