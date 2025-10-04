#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#include "bdd.h"
#include "image.h"

static int skip_whitespace(FILE *f) {
    int c;
    while(isspace(c = fgetc(f)))
	;
    if(c == EOF)
	return EOF;
    ungetc(c, f);
    return 0;
}

static int skip_comment(FILE *f) {
    int c;
    if((c = fgetc(f)) == '#') {
	while((c = fgetc(f)) != '\n')
	    ;
    }
    if(c == EOF)
	return EOF;
    ungetc(c, f);
    return 0;
}

static int skip_ws_or_comment(FILE *f) {
    int c;
    for (;;) {
        /* Skip whitespace first */
        while (isspace(c = fgetc(f))) {
            if (c == EOF) return EOF;
        }
        if (c == EOF) return EOF;
        if (c == '#') {
            /* skip until end of line */
            while ((c = fgetc(f)) != '\n') {
                if (c == EOF) return EOF;
            }
            /* continue to skip any whitespace/comments after the newline */
            continue;
        }
        /* We read one character too far (not whitespace or comment) -- push it back */
        ungetc(c, f);
        return 0;
    }
}

// Common code for reading the file header, after the magic.
static int img_read_header(FILE *file, char *type, int *wp, int *hp) {
    int c, err, max;
    /* Skip any leading whitespace/comments between the magic and header ints. */
    if (skip_ws_or_comment(file) == EOF) {
	fprintf(stderr, "Invalid %s file (bad header)\n", type);
	goto bad;
    }
    /* Read width, height and max value separately for robustness. */
    if (fscanf(file, "%d", wp) != 1) {
            fprintf(stderr, "Invalid %s file (bad header parameters)\n", type);
            goto bad;
        }
        if (fscanf(file, "%d", hp) != 1) {
            fprintf(stderr, "Invalid %s file (bad header parameters)\n", type);
            goto bad;
        }
        if (fscanf(file, "%d", &max) != 1) {
            fprintf(stderr, "Invalid %s file (bad header parameters)\n", type);
            goto bad;
        }
    /* After reading the three integers, we must encounter a single separating
     * whitespace character before the raster/binary data. Comments (lines
     * starting with '#') may appear in between and should be skipped. We
     * therefore loop consuming comments and whitespace until we find the
     * separating whitespace. */
    for (;;) {
        int cc = fgetc(file);
        if (cc == EOF) {
            fprintf(stderr, "Invalid %s file (bad comment/no data)\n", type);
            goto bad;
        }
        if (cc == '#') {
            /* skip to end of line */
            while ((cc = fgetc(file)) != '\n') {
                if (cc == EOF) {
                    fprintf(stderr, "Invalid %s file (bad comment/no data)\n", type);
                    goto bad;
                }
            }
            /* continue scanning after the newline */
            continue;
        }
        if (isspace(cc)) {
            /* This is the required single whitespace separator. Proceed to data. */
            break;
        }
        /* Any other character here is invalid: we expected whitespace or comment. */
        fprintf(stderr, "Invalid %s file (no data)\n", type);
        goto bad;
    }
    if(max >= 256) {
	fprintf(stderr, "%s file maximum pixel value %d is too large (255 max supported)\n",
		type, max);
	goto bad;
    }
    return 0;

bad:
    return -1;
}

// Spec: http://netpbm.sourceforge.net/doc/pgm.html
int img_read_pgm(FILE *file, int *wp, int *hp, unsigned char *raster, size_t size) {
    int err;
    int cc;
    unsigned int max;
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P5") != 0) {
        fprintf(stderr, "Invalid PGM file (missing/bad magic)\n");
        goto bad;
    }
    if((err = img_read_header(file, "PGM", wp, hp)) < 0)
        goto bad;

    // Check that there is enough space to hold the data.
    if(*wp * *hp * sizeof(unsigned char) > size)
    goto bad;

    // Read the raster.
    unsigned char *dp = raster;
    for(int i = 0; i < *hp; i++) {
        for(int j = 0; j < *wp; j++) {
            if ((cc = fgetc(file)) == EOF) {
                fprintf(stderr, "PGM file image data truncated\n");
                goto bad;
            }
            *dp++ = (unsigned char) cc;
        }
    }
    return 0;

 bad:
    return -1;
}

int img_write_pgm(unsigned char *data, int w, int h, FILE *file) {
    if (file == NULL) {
        return -1;
    }
    fprintf(file, "P5 %d %d 255\n", w, h);
    for(int i = 0; i < h; i++) {
	for(int j = 0; j < w; j++) {
	    fputc(data[i * w + j], file);
	}
    }
    return fflush(file);
}

BDD_NODE *img_read_birp(FILE *file, int *wp, int *hp) {
    int c;
    unsigned int max;
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "B5") != 0) {
        fprintf(stderr, "Invalid BIRP file (missing/bad magic)\n");
        goto bad;
    }
    int err;
    if((err = img_read_header(file, "BIRP", wp, hp)) < 0)
	goto bad;

    // Read the serialized BDD.
    BDD_NODE *node = bdd_deserialize(file);
    return node;

 bad:
    return NULL;
}

int img_write_birp(BDD_NODE *node, int w, int h, FILE *file) {
    if (file == NULL) {
        return -1;
    }
    fprintf(file, "B5 %d %d 255\n", w, h);
    bdd_serialize(node, file);
    return fflush(file);
}
