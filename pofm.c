#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

void create_file(const char *filename);
void delete_file(const char *filename);
void rename_file(const char *old_name, const char *new_name);
void copy_file(const char *source, const char *destination);
void move_file(const char *source, const char *destination);
void append_text(const char *filename, const char *text);
void insert_text(const char *filename, const char *text, int position);
void clear_text(const char *filename);
void show_content(const char *filename, int lines_per_page);
void show_help(const char *command);
int is_text_file(const char *filename);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: No command provided. Use /h for help.\n");
        return 1;
    }

    if (argc == 3 && strcmp(argv[2], "/h") == 0) {
        show_help(argv[1]);
        return 0;
    }

    if (strcmp(argv[1], "create") == 0) {
        if (argc < 3) {
            show_help("create");
        } else {
            create_file(argv[2]);
        }
    } else if (strcmp(argv[1], "delete") == 0) {
        if (argc < 3) {
            show_help("delete");
        } else {
            delete_file(argv[2]);
        }
    } else if (strcmp(argv[1], "rename") == 0) {
        if (argc < 4) {
            show_help("rename");
        } else {
            rename_file(argv[2], argv[3]);
        }
    } else if (strcmp(argv[1], "copy") == 0) {
        if (argc < 4) {
            show_help("copy");
        } else {
            copy_file(argv[2], argv[3]);
        }
    } else if (strcmp(argv[1], "move") == 0) {
        if (argc < 4) {
            show_help("move");
        } else {
            move_file(argv[2], argv[3]);
        }
    } else if (strcmp(argv[1], "append") == 0) {
        if (argc < 4) {
            show_help("append");
        } else {
            append_text(argv[2], argv[3]);
        }
    } else if (strcmp(argv[1], "insert") == 0) {
        if (argc < 5) {
            show_help("insert");
        } else {
            insert_text(argv[2], argv[3], atoi(argv[4]));
        }
    } else if (strcmp(argv[1], "clear") == 0) {
        if (argc < 3) {
            show_help("clear");
        } else {
            clear_text(argv[2]);
        }
    } else if (strcmp(argv[1], "show") == 0) {
        if (argc < 4) {
            show_help("show");
        } else {
            show_content(argv[2], atoi(argv[3]));
        }
    } else if (strcmp(argv[1], "/h") == 0) {
        show_help(NULL);
    } else {
        printf("Unknown command. Use /h for help.\n");
    }

    return 0;
}

void create_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create file '%s'. %s\n", filename, strerror(errno));
        return;
    }
    fclose(file);
    printf("File '%s' created successfully.\n", filename);
}

void delete_file(const char *filename) {
    if (remove(filename) != 0) {
        printf("Error: Could not delete file '%s'. %s\n", filename, strerror(errno));
        return;
    }
    printf("File '%s' deleted successfully.\n", filename);
}

void rename_file(const char *old_name, const char *new_name) {
    if (rename(old_name, new_name) != 0) {
        printf("Error: Could not rename file '%s' to '%s'. %s\n", old_name, new_name, strerror(errno));
        return;
    }
    printf("File renamed from '%s' to '%s'.\n", old_name, new_name);
}

void copy_file(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    if (!src) {
        printf("Error: Could not open source file '%s'. %s\n", source, strerror(errno));
        return;
    }

    struct stat dest_stat;
    char final_dest[1024];
    if (stat(destination, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
        snprintf(final_dest, sizeof(final_dest), "%s/%s", destination, source);
    } else {
        snprintf(final_dest, sizeof(final_dest), "%s", destination);
    }

    FILE *dest = fopen(final_dest, "wb");
    if (!dest) {
        fclose(src);
        printf("Error: Could not open destination file '%s'. %s\n", final_dest, strerror(errno));
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            printf("Error: Failed to write to destination file '%s'. %s\n", final_dest, strerror(errno));
            fclose(src);
            fclose(dest);
            return;
        }
    }

    fclose(src);
    fclose(dest);
    printf("File '%s' copied to '%s'.\n", source, final_dest);
}

void move_file(const char *source, const char *destination) {
    // Try to rename the file (move it)
    if (rename(source, destination) == 0) {
        printf("File move successful.\n");
        return;
    }

    // If renaming fails, proceed with copying the file

    // Copy the source file to the destination
    FILE *src = fopen(source, "rb");
    if (!src) {
        printf("Error: Could not open source file '%s'. %s\n", source, strerror(errno));
        return;
    }

    struct stat dest_stat;
    char final_dest[1024];
    if (stat(destination, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
        snprintf(final_dest, sizeof(final_dest), "%s/%s", destination, source);
    } else {
        snprintf(final_dest, sizeof(final_dest), "%s", destination);
    }

    FILE *dest = fopen(final_dest, "wb");
    if (!dest) {
        fclose(src);
        printf("Error: Could not open destination file '%s'. %s\n", final_dest, strerror(errno));
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            printf("Error: File copy failed. %s\n", strerror(errno));
            fclose(src);
            fclose(dest);
            return;
        }
    }

    fclose(src);
    fclose(dest);

    // Now delete the source file after successful copy
    if (remove(source) != 0) {
        printf("Error: Failed to delete source file '%s'. %s\n", source, strerror(errno));
        return;
    }

    printf("File move successful.\n");
}

void append_text(const char *filename, const char *text) {
    if (!is_text_file(filename)) {
        printf("Error: '%s' is not a text-based file.\n", filename);
        return;
    }

    // Check if the file exists before attempting to append
    FILE *file = fopen(filename, "r");
    if (!file) {
        // If the file does not exist, print an error and return
        if (errno == ENOENT) {
            printf("Error: File '%s' not found.\n", filename);
        } else {
            printf("Error: Could not open file '%s'. %s\n", filename, strerror(errno));
        }
        return;
    }
    fclose(file);  // File exists, so close the file

    // Open the file in append mode to add text
    file = fopen(filename, "a");
    if (!file) {
        printf("Error: Could not open file '%s' for appending. %s\n", filename, strerror(errno));
        return;
    }

    fprintf(file, "%s\n", text);
    fclose(file);
    printf("Text appended to '%s'.\n", filename);
}

void insert_text(const char *filename, const char *text, int position) {
    if (!is_text_file(filename)) {
        printf("Error: '%s' is not a text-based file.\n", filename);
        return;
    }

    if (position < 0) {
        printf("Error: Position %d is invalid. Position cannot be negative.\n", position);
        return;
    }

    // Open the file for reading and writing
    FILE *file = fopen(filename, "r+");
    if (!file) {
        printf("Error: Could not open file '%s'. %s\n", filename, strerror(errno));
        return;
    }

    // Step 1: Read the entire file into memory
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_content = malloc(file_size + 1);  // +1 for the null terminator
    if (file_content == NULL) {
        printf("Error: Failed to allocate memory for file content.\n");
        fclose(file);
        return;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0';  // Null-terminate the content

    // Step 2: Check if the position is valid
    if (position > file_size) {
        printf("Error: Position %d is beyond the end of the file (length: %ld).\n", position, file_size);
        free(file_content);
        fclose(file);
        return;
    }

    // Step 3: Prepare the new content by inserting the text
    // Allocate memory for the new content
    size_t new_content_size = file_size + strlen(text);
    char *new_content = malloc(new_content_size + 1);  // +1 for the null terminator
    if (new_content == NULL) {
        printf("Error: Failed to allocate memory for new content.\n");
        free(file_content);
        fclose(file);
        return;
    }

    // Step 4: Copy content before the insertion point
    strncpy(new_content, file_content, position);

    // Step 5: Insert the new text
    strcpy(new_content + position, text);

    // Step 6: Copy the content after the insertion point
    strcpy(new_content + position + strlen(text), file_content + position);

    // Step 7: Write the new content back to the file
    freopen(filename, "w", file);  // Reopen the file for writing
    fwrite(new_content, 1, new_content_size, file);

    // Clean up
    free(file_content);
    free(new_content);
    fclose(file);

    printf("Text inserted into '%s' at position %d.\n", filename, position);
}

void clear_text(const char *filename) {
    if (!is_text_file(filename)) {
        printf("Error: '%s' is not a text-based file.\n", filename);
        return;
    }

    // Check if the file exists before clearing
    FILE *file = fopen(filename, "r");
    if (!file) {
        // If the file does not exist, print an error and return
        if (errno == ENOENT) {
            printf("Error: File '%s' not found.\n", filename);
        } else {
            printf("Error: Could not open file '%s'. %s\n", filename, strerror(errno));
        }
        return;
    }
    fclose(file);

    // Open the file in write mode to clear its content
    file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file '%s' for clearing. %s\n", filename, strerror(errno));
        return;
    }
    fclose(file);
    printf("Content cleared from '%s'.\n", filename);
}

void show_content(const char *filename, int lines_per_page) {
    if (!is_text_file(filename)) {
        printf("Error: '%s' is not a text-based file.\n", filename);
        return;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s'. %s\n", filename, strerror(errno));
        return;
    }

    char buffer[1024];
    int line_count = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
        line_count++;
        if (line_count >= lines_per_page) {
            printf("-- Press Enter for next page --\n");
            getchar();
            line_count = 0;
        }
    }

    fclose(file);
}

void show_help(const char *command) {
    if (!command) {
        printf("Available commands:\n"
               "create <filename>\n"
               "delete <filename>\n"
               "rename <oldname> <newname>\n"
               "copy <source> <destination>\n"
               "move <source> <destination>\n"
               "append <filename> <text>\n"
               "insert <filename> <text> <position>\n"
               "clear <filename>\n"
               "show <filename> <lines_per_page>\n");
    } else if (strcmp(command, "create") == 0) {
        printf("Usage: create <filename>\n"
               "Creates a new file with the specified filename.\n");
    } else if (strcmp(command, "delete") == 0) {
        printf("Usage: delete <filename>\n"
               "Deletes the specified file.\n");
    } else if (strcmp(command, "rename") == 0) {
        printf("Usage: rename <oldname> <newname>\n"
               "Renames a file from oldname to newname.\n");
    } else if (strcmp(command, "copy") == 0) {
        printf("Usage: copy <source> <destination>\n"
               "Copies the source file to the destination.\n");
    } else if (strcmp(command, "move") == 0) {
        printf("Usage: move <source> <destination>\n"
               "Moves the source file to the destination.\n");
    } else if (strcmp(command, "append") == 0) {
        printf("Usage: append <filename> <text>\n"
               "Appends the given text to the specified file.\n");
    } else if (strcmp(command, "insert") == 0) {
        printf("Usage: insert <filename> <text> <position>\n"
               "Inserts the given text into the specified file at the position.\n");
    } else if (strcmp(command, "clear") == 0) {
        printf("Usage: clear <filename>\n"
               "Clears all content from the specified file.\n");
    } else if (strcmp(command, "show") == 0) {
        printf("Usage: show <filename> <lines_per_page>\n"
               "Displays content of the specified file with pagination.\n");
    } else {
        printf("Unknown command '%s'. Use /h for help.\n", command);
    }
}

/*int is_text_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s'. %s\n", filename, strerror(errno));
        return 0; // Cannot determine, treat as non-text
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytes; i++) {
            if (!isprint(buffer[i]) && !isspace(buffer[i])) {
                fclose(file);
                return 0; // Non-text character found
            }
        }
    }

    fclose(file);
    return 1; // File passed the check
}*/


int is_text_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    return (strcmp(ext, ".txt") == 0);
}

