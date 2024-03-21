/*
    main.c	- Main program file.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "romimg.h"

static void DisplayROMImgDetails(const ROMIMG *ROMImg)
{
    unsigned int i, TotalSize;
    struct FileEntry *file;
    char filename[11];
    if (ROMImg->date != 0)
        printf("ROM datestamp:\t%04x/%02x/%02x\n", ((unsigned short int *)&ROMImg->date)[1], ((unsigned char *)&ROMImg->date)[1], ((unsigned char *)&ROMImg->date)[0]);
    if (ROMImg->comment != NULL)
        printf("ROM comment:\t%s\n", ROMImg->comment);

    printf("File list:\n"
           GREEN"Name"DEFCOL"      \tSize\n"
           "-----------------------------\n");
    for (i = 0, file = ROMImg->files, TotalSize = 0; i < ROMImg->NumFiles; TotalSize += file->RomDir.size, i++, file++) {
        strncpy(filename, file->RomDir.name, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        printf(GREEN"%-10s"DEFCOL"\t%u\n", filename, file->RomDir.size);
    }

    printf("\nTotal size: %u bytes.\n", TotalSize);
}

static void DisplaySyntaxHelp(void)
{
    printf(REDBOLD"Syntax error"DEFCOL". Syntax:\n"
           "ROMIMG -c <ROM image> <files>\n\tCreate ROM image\n"
           "ROMIMG -l <ROM image>\n\tList files in ROM image\n"
           "ROMIMG -a <ROM image> <file(s)>\n\tAdd file(s) to ROM image\n"
           "ROMIMG -d <ROM image> <file(s)>\n\tDelete file(s) from ROM image\n"
           "ROMIMG -x <ROM image>\n\tExtract all files from ROM image\n"
           "ROMIMG -x <ROM image> <file>\n\tExtract file from ROM image\n");
}

static void DisplayAddDeleteOperationResult(int result, const char *InvolvedFile)
{
    switch (result) {
        case 0: // No error.
            printf(GRNBOLD"done!"DEFCOL"\n");
            break;
        case ENOENT:
            printf(YELBOLD"file not found."DEFCOL"\n");
            break;
        case EIO:
            printf(REDBOLD"Error writing to file: %s"DEFCOL"\n", InvolvedFile);
            break;
        case EEXIST:
            printf(YELBOLD"File already exists."DEFCOL"\n");
            break;
        default:
            printf(REDBOLD"failed! code: %d"DEFCOL"\n", result);
    }
}

int main(int argc, char **argv)
{
    int result;
    FILE *OutputFile;
    ROMIMG ROMImg;
    unsigned int i, FilesAffected;
    char filename[11];
    struct FileEntry *file;

    printf("PlayStation 2 ROM image generator v1.12\n"
           "Compiled on " __DATE__ " - " __TIME__ "\n"
           "---------------------------------------\n\n");
    for (result = 0; result < argc; result++)
        DPRINTF("\t -- argv[%d] = %s\n", result, argv[result]);

    if (argc < 2) {
        DisplaySyntaxHelp();
        DPRINTF("ERROR: LESS THAN TWO ARGS PROVIDED\n");
        return EINVAL;
    }

    if (argc >= 4 && strcmp(argv[1], "-c") == 0) {
        if ((result = CreateBlankROMImg(argv[2], &ROMImg)) == 0) {
            for (FilesAffected = 0, i = 0; i < argc - 3; i++) {
                printf("Adding file '%s'", argv[3 + i]);
                if ((result = AddFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                printf(result == 0 ? GRNBOLD" done!"DEFCOL"\n" : REDBOLD" failed!"DEFCOL"\n");
            }

            if (FilesAffected > 0) {
                printf("Writing image...");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? GRNBOLD"done!"DEFCOL"\n" : REDBOLD"failed!"DEFCOL"\n");
            }
            UnloadROMImg(&ROMImg);
        } else
            ERROR("(Internal fault) Can't create blank image file: %d. Please report.\n", result);
    } else if (argc >= 4 && strcmp(argv[1], "-a") == 0) {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0) {
            for (i = 0, FilesAffected = 0; i < argc - 3; i++) {
                printf("Adding file '%s'", argv[3 + i]);
                if ((result = AddFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                DisplayAddDeleteOperationResult(result, argv[3 + i]);
            }

            if (FilesAffected > 0) {
                printf("Writing image... ");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? GRNBOLD"done!"DEFCOL"\n" : REDBOLD"failed!"DEFCOL"\n");
            }
            UnloadROMImg(&ROMImg);
        } else
            ERROR("Can't load image file: %s\n", argv[2]);
    } else if (argc >= 4 && strcmp(argv[1], "-d") == 0) {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0) {
            for (i = 0, FilesAffected = 0; i < argc - 3; i++) {
                printf("Removing file %s...", argv[3 + i]);
                if ((result = DeleteFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                DisplayAddDeleteOperationResult(result, argv[3 + i]);
            }

            if (FilesAffected > 0) {
                printf("Writing image...");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? GRNBOLD"done!"DEFCOL"\n" : REDBOLD"failed!"DEFCOL"\n");
            }
            UnloadROMImg(&ROMImg);
        } else
            ERROR("Can't load image file: %s\n", argv[2]);
    } else if (argc == 3 && strcmp(argv[1], "-l") == 0) {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0) {
            DisplayROMImgDetails(&ROMImg);
            UnloadROMImg(&ROMImg);
        } else
            ERROR("Can't load image file: %s\n", argv[2]);
    } else if ((argc == 3 || argc == 4) && strcmp(argv[1], "-x") == 0) {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0) {
            if (argc == 3) {
                char FOLDER[256] = "ext_";
                strcat(FOLDER, argv[2]);
                mkdir(FOLDER, 0755);
                chdir(FOLDER);

                printf("File list:\n"
           			   GREEN"Name"DEFCOL"      \tSize\n"
                       "-----------------------------\n");
                for (i = 0, file = ROMImg.files; i < ROMImg.NumFiles; i++, file++) {
                    strncpy(filename, file->RomDir.name, sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    printf(GREEN"%-10s"DEFCOL"\t%u\n", filename, file->RomDir.size);

                    if (file->RomDir.size > 0) {
                        if ((OutputFile = fopen(filename, "wb")) != NULL) {
                            if (fwrite(file->FileData, 1, file->RomDir.size, OutputFile) != file->RomDir.size) {
                                ERROR("Error writing to file %s\n", filename);
                            }
                            fclose(OutputFile);
                        } else {
                            ERROR("Can't create file: %s\n", filename);
                        }
                    }
                }
            } else {
                printf("Extracting file %s...", argv[3]);
                DisplayAddDeleteOperationResult(result = ExtractFile(&ROMImg, argv[3], argv[3]), argv[3]);
            }

            UnloadROMImg(&ROMImg);
        } else
            ERROR("Can't load image file: %s\n", argv[2]);
    } else {
        WARNING("Unrecognized command or incorrect syntax: %s\n", argv[1]);
        DisplaySyntaxHelp();
        result = EINVAL;
    }

    return result;
}
