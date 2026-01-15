#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include "pdfio.h"
#include "pdfio-content.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pdf_file>\n", argv[0]);
        return 1;
    }

    if (getenv("AFL_PERSISTENT")) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
    }

    while (__AFL_LOOP(1000)) {
        const char *filename = argv[1];

        pdfio_file_t *pdf = pdfioFileOpen(filename, NULL, NULL, NULL, NULL);
        if (!pdf) continue;

        size_t num_objs = pdfioFileGetNumObjs(pdf);
        for (size_t i = 0; i < num_objs && i < 100; i++) { 
            pdfio_obj_t *obj = pdfioFileGetObj(pdf, i);
            if (!obj) continue;

            pdfio_stream_t *st = pdfioObjOpenStream(obj, true); 
            if (st) {
                char buffer[4096];
                while (pdfioStreamRead(st, buffer, sizeof(buffer)) > 0);
                pdfioStreamClose(st);
            }
        }

        pdfioFileClose(pdf);
    }

    return 0;
}
