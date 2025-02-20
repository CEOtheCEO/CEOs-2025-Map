#include <conio.h> 
#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>

struct pcxheader {
    unsigned char manufacturer;
    unsigned char version;
    unsigned char encoding;
    unsigned char bitsPerPixel;
    unsigned short xMin;
    unsigned short yMin;
    unsigned short xMax;
    unsigned short yMax;
    unsigned short hDpi;
    unsigned short vDpi;
    unsigned char palette[48];
    unsigned char reserved;
    unsigned char colorPlanes;
    unsigned short bytesPerLine;
    unsigned short paletteType;
    char filler[58];
};

int showMenu();
int openMap(const char *filename);
int setVideoMode(int mode);
int resetVideoMode();
int drawPCXImage(const char *filename);
int strcasecmp_custom(const char *s1, const char *s2);
void clrscr(void);
void textbackground(int color);
void textcolor(int color);

main() { 
    char choice[10];
    
    clrscr();
    setVideoMode(0x03); 

    while(1) {
        showMenu();
        printf("Enter your choice (1-7) or\n");
        printf("EXIT to quit: ");

        if (kbhit()) {
            if (getch() == 27) {
                printf("\nExiting program...\n");
                break;
            }
        }

        scanf("%s", choice);
        fflush(stdin);

        if (strcasecmp_custom(choice, "EXIT") == 0) {
            printf("Exiting program...\n");
            break;
        }

        switch(choice[0]) {
            case '1': openMap("bin/afr.pcx"); break;
            case '2': openMap("bin/asia.pcx"); break;
            case '3': openMap("bin/erp.pcx"); break;
            case '4': openMap("bin/na.pcx"); break;
            case '5': openMap("bin/sa.pcx"); break;
            case '6': openMap("bin/Aus.pcx"); break;
            case '7': openMap("bin/Wrld.pcx"); break;
            default: printf("Invalid choice, please try again.\n");
        }
    }

    resetVideoMode();
    return 0;
}

showMenu() {
    textbackground(0);
    textcolor(15);

    printf("========================\n");
    printf("   CEO's Map 2025   \n");
    printf("========================\n");
    printf("1. Africa\n");
    printf("2. Asia\n");
    printf("3. Europe\n");
    printf("4. North America\n");
    printf("5. South America\n");
    printf("6. Australia\n");
    printf("7. World Map\n");
    printf("Type 'EXIT' to quit\n");
    printf("========================\n");
}

openMap(const char *filename) {
    setVideoMode(0x13);
    drawPCXImage(filename);

    while (!kbhit()) {
        delay(100);
    }

    getch();

    setVideoMode(0x03);
    return 0;
}

setVideoMode(int mode) {
    union REGS r;
    r.h.ah = 0x00;
    r.h.al = mode;
    int86(0x10, &r, &r);
    return 0;
}

resetVideoMode() {
    setVideoMode(0x03);
    textbackground(0);
    textcolor(15);
    return 0;
}

drawPCXImage(const char *filename) {
    FILE *file;
    struct pcxheader header;
    unsigned char byte;
    int runLength;
    unsigned char palette[768];
    unsigned char far *vgaPtr;
    int i, pixelIndex, chunkSize;

    file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening %s\n", filename);
        delay(2000);
        return 1;
    }

    fread(&header, sizeof(struct pcxheader), 1, file);

    if (header.manufacturer != 0x0A) {
        printf("Invalid PCX file\n");
        fclose(file);
        delay(2000);
        return 1;
    }

    if (((int)(header.xMax - header.xMin + 1)) != 320 ||
        ((int)(header.yMax - header.yMin + 1)) != 200) {
        printf("Invalid image size\n");
        fclose(file);
        delay(2000);
        return 1;
    }

    vgaPtr = (unsigned char far *)MK_FP(0xA000, 0);

    for (i = 0; i < 32000; i++) {
        vgaPtr[i] = 0;
    }

    fseek(file, -769L, SEEK_END);
    if (fgetc(file) != 0x0C) {
        printf("No palette found\n");
        fclose(file);
        delay(2000);
        return 1;
    }

    fread(palette, 1, 768, file);

    disable();
    outportb(0x3C8, 0);
    for (i = 0; i < 768; i++) {
        outportb(0x3C9, palette[i] >> 2);
    }
    enable();

    fseek(file, 128L, SEEK_SET);

    pixelIndex = 0;
    chunkSize = 800;

    while (pixelIndex < 32000) {
        byte = fgetc(file);
        if (feof(file)) break;

        if ((byte & 0xC0) == 0xC0) {
            runLength = (int)(byte & 0x3F);
            byte = fgetc(file);
            for (i = 0; i < runLength && pixelIndex < 32000; i++) {
                vgaPtr[pixelIndex++] = byte;
                if (pixelIndex % chunkSize == 0) {
                    delay(10);
                }
            }
        } else {
            vgaPtr[pixelIndex++] = byte;
            if (pixelIndex % chunkSize == 0) {
                delay(10);
            }
        }
    }

    while ((unsigned int)pixelIndex < 64000) {
        byte = fgetc(file);
        if (feof(file)) break;

        if ((byte & 0xC0) == 0xC0) {
            runLength = (int)(byte & 0x3F);
            byte = fgetc(file);
            for (i = 0; i < runLength && (unsigned int)pixelIndex < 64000; i++) {
                vgaPtr[pixelIndex++] = byte;
                if ((unsigned int)pixelIndex % chunkSize == 0) {
                    delay(10);
                }
            }
        } else {
            vgaPtr[pixelIndex++] = byte;
            if ((unsigned int)pixelIndex % chunkSize == 0) {
                delay(10);
            }
        }
    }

    fclose(file);
    return 0;
}

int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
            return (unsigned char)*s1 - (unsigned char)*s2;
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}
