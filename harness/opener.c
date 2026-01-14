#include <pdfio.h>
#include <pdfio-content.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* in pdfio the function to open a pdf file refrenced as [pdfioFileOpen] takes in 5 parameters and 1 
   of them is a error callback parameter which as said in the docs at 
   https://github.com/michaelrsweet/pdfio/blob/master/doc/pdfio.md  -- > 
   The error callback is called for both errors and warnings and accepts 
   1 -- > the pointer
   2 -- > the message string 
   3 -- > pointer value
   simply return true to contnute processing
   return false to stop we set it to true since we are fuzzing !
*/
bool error_cb(pdfio_file_t *pdf, const char *message, void *data) {
    return true; 
}

/*https://github.com/michaelrsweet/pdfio/blob/master/doc/pdfio.md*/
int main(int argc, char *argv[]) {
    pdfio_file_t *pdf; //A PDF file (for reading or writing)
    pdfio_obj_t  *page; //An object in a PDF file
    pdfio_stream_t *st; //  An object stream
    char         buffer[8192]; // holder
    size_t       i, j, num_pages; // indexers

    if (argc < 2) return 1; // if no file supplied exit the program

    /* open the file
       pass cb -- > 0
       pass data -- > 0 
       error_data -- > 0
       error cb  --> 1=0
    */
    pdf = pdfioFileOpen(argv[1], NULL, NULL, error_cb, NULL);
    if (!pdf) {
        printf("[-] failed: %s\n", argv[1]);
        return 0;
    } // idk it says make it 0 so afl doenst false positive hangs 
    
    num_pages = pdfioFileGetNumPages(pdf);
    printf("[+] opened : %s (pages: %zu)\n", argv[1], num_pages);

    //since most seeds are 1 , and afl wont make a big pdf -_-
    for (i = 0; i < num_pages && i < 5; i++) { 
        page = pdfioFileGetPage(pdf, i);
        if (!page) continue;

        //addons
        int num_streams = pdfioPageGetNumStreams(page);
        for (j = 0; j < (size_t)num_streams; j++) {
            st = pdfioPageOpenStream(page, j, true);
            if (st) {
                while (pdfioStreamRead(st, buffer, sizeof(buffer)) > 0);
                pdfioStreamClose(st);
            }
        }

        //more
        pdfio_dict_t *dict = pdfioObjGetDict(page);
        pdfioDictGetArray(dict, "MediaBox");
        pdfioDictGetArray(dict, "Annots");
    }

    //done
    for (i = 1; i < 50; i++) {
        pdfio_obj_t *obj = pdfioFileFindObj(pdf, i);
        if (obj) {
            st = pdfioObjOpenStream(obj, true);
            if (st) pdfioStreamClose(st);
        }
    }

    pdfioFileClose(pdf);
    return 0;
}
