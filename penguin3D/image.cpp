#include <stdio.h>

#include "image.h"
#include "gl.h"

extern int Win[2];


void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw = true) {
    FILE* fp = fopen(filename, "wt");

    if ( fp == NULL ) {
        printf("WARNING: Can't open output file %s\n", filename);
        return;
    }

    if ( raw ) {
        fprintf(fp, "P5\n%d %d\n%d\n", width, height, 255);
        for (int y = height - 1; y >= 0; y--) {
            fwrite(&buffer[y * width], sizeof (GLubyte), width, fp);
        }
    } else {
        fprintf(fp, "P2\n%d %d\n%d\n", width, height, 255);
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                fprintf(fp, "%d ", int(buffer[x + y * width]));
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw = true) {
    FILE* fp = fopen(filename, "wt");

    if ( fp == NULL ) {
        printf("WARNING: Can't open output file %s\n", filename);
        return;
    }

    if ( raw ) {
        fprintf(fp, "P6\n%d %d\n%d\n", width, height, 255);
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                GLubyte* pix = &buffer[4 * (x + y * width)];

                fprintf(fp, "%c%c%c", int(pix[RED_OFFSET]),
                        int(pix[GREEN_OFFSET]),
                        int(pix[BLUE_OFFSET]));
            }
        }
    } else {
        fprintf(fp, "P3\n%d %d\n%d\n", width, height, 255);
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                GLubyte* pix = &buffer[4 * (x + y * width)];

                fprintf(fp, "%d %d %d ", int(pix[RED_OFFSET]),
                        int(pix[GREEN_OFFSET]),
                        int(pix[BLUE_OFFSET]));
            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}



void writeFrame(char* filename, bool pgm, bool frontBuffer) {
    static GLubyte* frameData = NULL;
    static int currentSize = -1;

    int size = (pgm ? 1 : 4);

    if ( frameData == NULL || currentSize != size * Win[0] * Win[1] ) {
        if (frameData != NULL)
            delete [] frameData;

        currentSize = size * Win[0] * Win[1];

        frameData = new GLubyte[currentSize];
    }

    glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

    if ( pgm ) {
        glReadPixels(0, 0, Win[0], Win[1],
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
        writePGM(filename, frameData, Win[0], Win[1]);
    } else {
        glReadPixels(0, 0, Win[0], Win[1],
                     GL_RGBA, GL_UNSIGNED_BYTE, frameData);
        writePPM(filename, frameData, Win[0], Win[1]);
    }
}
