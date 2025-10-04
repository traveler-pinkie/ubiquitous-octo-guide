/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"
#include <stdio.h>

#include "studentheaders.h"

int pgm_to_birp(FILE *in, FILE *out) {
    int rasterWidth = 0;
    int rasterHeight = 0;
    int readPgm = img_read_pgm(in, &rasterWidth, &rasterHeight, raster_data, RASTER_SIZE_MAX);

    if (readPgm == -1) {
        fprintf(stderr, "An error has occurred.\n");
        return -1;
        } // An error has occurred.

    BDD_NODE *topNode = bdd_from_raster(rasterWidth, rasterHeight, raster_data);
    if (topNode == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;} // Some sort of error has occurred.

    // call img_write_birp to finish up.
    if (img_write_birp(topNode, rasterWidth, rasterHeight, out) == -1) {
        fprintf(stderr, "An error has occurred.\n");
        return -1;
    }
    else {
        return 0;
    }
}

int birp_to_pgm(FILE *in, FILE *out) {

    int height = 0;
    int width = 0;
    BDD_NODE *root = img_read_birp(in, &width, &height);

    bdd_to_raster(root, width, height, raster_data);

    if (img_write_pgm(raster_data, width, height, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
    return 0;
}

unsigned char negate(unsigned char in) {
    return 255 - in;
}

unsigned char threshold(unsigned char in) {
    int parameter = (global_options & 0xFF0000) >> 16;
    if (in >= parameter) { return 255;}
    else { return 0;}
}

int birp_to_birp(FILE *in, FILE *out) {
    int rasterWidth = 0;
    int rasterHeight = 0;
    BDD_NODE *root = img_read_birp(in, &rasterWidth, &rasterHeight);
    if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;} // Some sort of error has occurred.

    // Obtain operation from global_options.
    int transformation = (global_options & 0xF00) >> 8;
    int parameter = (global_options & 0xFF0000) >> 16;

    // Identity transformation
    if (transformation == 0) {
        if (img_write_birp (root, rasterWidth, rasterHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
        return 0;
    }

    // Negative
    else if (transformation == 1) {
        root = bdd_map(root, &negate);
        if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;} // Some sort of error has occurred.
        if (img_write_birp (root, rasterWidth, rasterHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
        return 0;
    }

    // Threshold
    else if (transformation == 2) {
        root = bdd_map(root, &threshold);
        if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;} // Some sort of error has occurred.
        if (img_write_birp (root, rasterWidth, rasterHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
        return 0;
    }

    // Zoom
    else if (transformation == 3) {

        // First, check if zoom factor is 0. If so, we can just return.
        if (parameter == 0) {
            if (img_write_birp (root, rasterWidth, rasterHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
            return 0;
        }

        // Zoom out bc negative parameter, check sign bit.
        if (((parameter & 0x80) == 0x80)) {
            parameter = ((~parameter + 1) & 0x00FF); // negative value stored in here. we never formally make it negative until the actual function call.

            int newWidth = rasterWidth;
            int newHeight = rasterHeight;

            if (newWidth % power(2, parameter) == 0) {
                newWidth /= power(2, parameter);
            }
            else {
                newWidth /= power(2, parameter);
                newWidth++;
            }

            if (newHeight % power(2, parameter) == 0) {
                newHeight /= power(2, parameter);
            }
            else {
                newHeight /= power(2, parameter);
                newHeight++;
            }

            //debug("%i is the old highest level. %i is the old d value. %ix%i is the old dimension.\n", (*root).level, (*root).level/2, rasterWidth, rasterHeight);
            //debug("%i is the new highest level. %i is the new d value. %ix%i is the new dimension.\n", newHighest, d, newDimensions, newDimensions);

            root = bdd_zoom(root,(*root).level - (2* parameter) ,(-1*parameter)); // negate the factor before passing it.
            if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;}
            if (img_write_birp(root, newWidth, newHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
            return 0;
        }

        // Zoom in because pos parameter.
        else {
            //debug("%i zoom in factor\n", parameter);

            int newWidth = rasterWidth;
            int newHeight = rasterHeight;

            newWidth *= power(2, parameter);
            newHeight *= power(2, parameter);
            //debug("%i is the old highest level. %i is the old d value. %ix%i is the old dimension.\n", (*root).level, (*root).level/2, rasterWidth, rasterHeight);
            //debug("%i is the new highest level. %i is the new d value. %ix%i is the new dimension.\n", newHighest, d, newDimensions, newDimensions);

            root = bdd_zoom(root,(*root).level + (2* parameter) ,parameter);
            if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;}
            // Zoom in changes the size of the bdd.
            if (img_write_birp(root, newWidth, newHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
            return 0;
        }
    }

    // Rotate
    else {
        root = bdd_rotate(root, (*root).level);
        if (root == NULL) { fprintf(stderr, "An error has occurred.\n"); return -1;}
        if (img_write_birp(root, rasterWidth, rasterHeight, out) == -1) { fprintf(stderr, "An error has occurred.\n"); return -1;}
        return 0;
    }
    fprintf(stderr, "An error has occurred.\n");
    return -1;

}

int pgm_to_ascii(FILE *in, FILE *out) {

    int rasterWidth = 0;
    int rasterHeight = 0;
    int readPgm = img_read_pgm(in, &rasterWidth, &rasterHeight, raster_data, RASTER_SIZE_MAX);
    int offset = 0;

    if (readPgm == -1) { fprintf(stderr, "An error has occurred.\n"); return -1; } // An error has occurred.

    for (int i = 0; i < rasterHeight; i++) {
        for (int j = 0; j < rasterWidth; j++) {
            unsigned char toPrint = *(raster_data + offset);
            if (toPrint <= 63) { fputc(' ', out);}
            else if (toPrint <= 127) { fputc('.', out); }
            else if (toPrint <= 191) { fputc('*', out); }
            else { fputc('@', out); }
            offset++;
        }
        fputc('\n', out);
    }
    return 0;
}

int birp_to_ascii(FILE *in, FILE *out) {
    int width = 0;
    int height = 0;

    BDD_NODE *root = img_read_birp(in, &width, &height);

    bdd_to_raster(root, width, height, raster_data);
    int offset = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char toPrint = *(raster_data + offset);
            if (toPrint <= 63) { fputc(' ', out);}
            else if (toPrint <= 127) { fputc('.', out); }
            else if (toPrint <= 191) { fputc('*', out); }
            else { fputc('@', out); }
            offset++;
        }
        fputc('\n', out);
    }
    return 0;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {

    // If for some reason, argc is negative, the entire thing is invalid.
    if (argc < 0 ) {
        return -1;
    }

    // Store the string of arguments in a pointer to the 1st char of the array
    // Something like "-i birp -o ascii" turns into "-i", "birp", "-o", and etc.

    int offset = 0; // Keep track of which argument we're looking at.
    int argsProcessed = 0;
    global_options = 0; // Initialize global_options to 0.

    // check for bin/birp in the args. if it's there, ignore it by increasing the offset and argschecked.
    offset = 1;
    argsProcessed = 1;

    // no flags. according to piazza @146, this is ok.
    if (argsProcessed == argc) {
        global_options += 2; // 4 LSB are set to 0x2 for birp input.
        global_options += 2 << 4; // bits 4-7 are set to 0x2 for birp output.
        return 0;
    }

    // values to determine what type of i/o we're dealing with.
    int in = 'b';
    int out = 'b';
    char flag;

    // boolean sentinal values.
    int done = 0; // false
    int first = 1; // true

    int seenInput = 0; // false
    int seenOutput = 0; // false

    //debug("%d args.\n", argc);
    // Check for positional arguments.
    while (!done) {
        char *current = *(argv + offset); // Load arg into current.

        if (current == NULL) {
            done = 1;// Otherwise we're done.
        }

        else if (*current == '-') { // Indicate we have a flag to process.

            flag = *(current + 1);
            char *format;
            switch (flag) {

                case 'h':
                    if (!first) { return -1; } // h must be the first positional arg.
                    else {
                        global_options = 1 << 31; // Update MSB to 1.
                        return 0; // anything past this is meaningless.
                    }
                    break; // Should always return but just to be safe. :)

                case 'i':
                    if (seenInput) { return -1; } // duplicate argument, not allowed as per piazza question.
                    offset++; // Look at next arg. We're expecting a file format in either pgm or birp.
                    format = *(argv + offset);

                    if (format == NULL) { return -1; } // Missing format.

                    if ((*format == 'p') && (*(++format) == 'g') && (*(++format) == 'm') && (*(++format) == '\0')) {
                        in = 'p';
                    }

                    else if ((*format == 'b') && (*(++format) == 'i') && (*(++format) == 'r') && (*(++format) == 'p') && (*(++format) == '\0')) {
                        in = 'b';
                    }

                    else {
                        return -1;
                    }
                    offset++; // we're done looking at the format. look for a flag in the next arg.
                    argsProcessed += 2; // we looked at the flag and format ie - "-i pgm" is 2 arguments.
                    first = 0;
                    seenInput = 1; // true
                    break;

                case 'o':
                    if (seenOutput) { return -1; } // duplicate arg.
                    offset++;
                    format = *(argv + offset);
                    if (format == NULL) { return -1; } // Missing format.

                    if ((*format == 'p') && (*(++format) == 'g') && (*(++format) == 'm') && (*(++format) == '\0')) {
                        out = 'p';
                    }

                    else if ((*format == 'b') && (*(++format) == 'i') && (*(++format) == 'r') && (*(++format) == 'p') && (*(++format) == '\0')) {
                        out = 'b';
                    }

                    else if ((*format == 'a') && (*(++format) == 's') && (*(++format) == 'c') && (*(++format) == 'i') && (*(++format) == 'i') && (*(++format) == '\0')) {
                        out = 'a';
                    }

                    else {
                        return -1;
                    }

                    offset++; // we're done looking at the format. look for a flag in the next arg.
                    argsProcessed += 2; // we looked at the flag and format ie - "-i pgm" is 2 arguments.
                    first = 0;
                    seenOutput = 1; // true
                    break;
                default:
                    // Current flag is neither h/i/o. That means either: we're done with the positional args or there's an optional arg completely out of place. we can check the latter in the next part.
                    first = 0;
                    done = 1;
            }
        }
        else { return -1; } // if the first char isn't -,
    }

    //debug("in: %c, out: %c\n", in, out);
    // Now if we're dealing with birp for both input and output, we have to parse optional arguments.
    int optionalArgInt = 0;
    char *optionalArg = 0;
    int place = 1;

    if (in == 'b' && out == 'b') { // Dealing with birp for input/output. no need for a while loop this time since we can expect only 1 flag and 1 arg at most.
        //debug("%i args processed out of %i argc.\n", argsProcessed, argc);
        if (argsProcessed == argc) {
            global_options += 2;
            global_options += 2 << 4;
            return 0;
         } // no flag, identity transformation.
        char *current = *(argv + offset); // Load arg into current.

        if (*current == '-') { // We have a flag to process.

            flag = *(current + 1);
            switch (flag) {
                case 'n':
                    if (argsProcessed + 1 != argc) { return -1; } // too many args were given.
                    global_options += 2; // 4 LSB are set to 0x2 for birp input.
                    global_options += 2 << 4; // bits 4-7 are set to 0x2 for birp output.
                    global_options += 1 << 8; // bits 8-11 are set to 0x1 for a negation.
                    return 0; // No extra arg needed.
                    break;

                case 'r':
                    if (argsProcessed + 1 != argc) { return -1; } // too many args were given.
                    global_options += 2; // 4 LSB are set to 0x2 for birp input.
                    global_options += 2 << 4; // bits 4-7 are set to 0x2 for birp output.
                    global_options += 4 << 8; // bits 8-11 are set to 0x4 for a rotation.
                    return 0; // No extra arg needed.
                    break;

                case 't':
                    if (argsProcessed + 2 != argc) { return -1; } // too many args were given.
                    // Look for threshold from 0, 255
                    offset++;
                    optionalArg = *(argv + offset);
                    if (optionalArg == NULL) { return -1; } // Missing format.

                    // Determine place values and whether the number is actually a number.
                    for (char *i = *(argv + offset); *i != '\0'; i++) {
                        if ((*i >= '0') && (*i <= '9')) { place *= 10; } // valid digit.
                        else { return -1; }
                    }
                    place /= 10; // Went one over.

                    // Actually calculate the integer value of the string.
                    for (char *i = *(argv + offset); *i != '\0'; i++) {
                        optionalArgInt += (*i - '0') * place;
                        place /= 10;
                    }

                    if (optionalArgInt < 0 || optionalArgInt > 255) { return -1; } // Not in the range.
                    global_options += 2; // 4 LSB are set to 0x2 for birp input.
                    global_options += 2 << 4; // bits 4-7 are set to 0x2 for birp output.
                    global_options += 2 << 8; // bits 8-11 are set to 0x2 for a threshold operation.
                    global_options += optionalArgInt << 16;// bits 6-23 (operation parameter) no need to do anything else, since it's always positive.
                    return 0; // In range.
                    break;

                case 'z':
                case 'Z':
                    if (argsProcessed + 2 != argc) { return -1; } // too many args were given.
                    // Look for threshold from 0, 16
                    offset++;
                    optionalArg = *(argv + offset);
                    if (optionalArg == NULL) { return -1; } // Missing format.

                    // Determine place values and whether the number is actually a number.
                    for (char *i = *(argv + offset); *i != '\0'; i++) {
                        if (*i >= '0' && *i <= '9') { place *= 10; } // valid digit.
                        else { return -1; }
                    }
                    place /= 10; // Went one over.

                    // Actually calculate the integer value of the string.
                    for (char *i = *(argv + offset); *i != '\0'; i++) {
                        optionalArgInt += (*i - '0') * place;
                        place /= 10;
                    }

                    if (optionalArgInt < 0 || optionalArgInt > 16) { return -1; } // Not in the range.
                    global_options += 2; // 4 LSB are set to 0x2 for birp input.
                    global_options += 2 << 4; // bits 4-7 are set to 0x2 for birp output.
                    global_options += 3 << 8; // bits 8-11 are set to 0x3 for any sort of zoom.
                    // bits 16-23 (operation parameter)
                    if (flag == 'Z') { global_options += optionalArgInt << 16;} // Z means zoom in, use a positive factor.
                    // z means zoom out (negative factor) flip and add 1 (get 2's comp), but we have 8 extra padding bits of 1's, so we have to remove them before we can add it to global_options.
                    else { global_options += ((~optionalArgInt + 1) & 0x00FF) << 16; }

                    return 0; // In range.
                    break;

                default:
                    return -1; // A positional arg mage its way here somehow. Invalid, since they had their chance to show up earlier.

            }
        }

        else { return -1; } // invalid arg.

    }

    else {
        // If birp is not set as I/O and there are outstanding args, we have to return -1.
        if (argsProcessed != argc) { return -1; }

        // otherwise no need to check optional args, we can just return 1 after updating global_options.
        else {
            // Update global_options based on the i/o format.
            switch (in) {
                case 'p':
                    global_options += 1;
                    break;
                case 'b':
                    global_options += 2;
                    break;
                default:
                    return -1; // Somehow input format is incorrect. Should never hit here, but just to be safe.
            }

            switch (out){
                case 'p':
                    global_options += 1 << 4;
                    break;
                case 'b':
                    global_options += 2 << 4;
                    break;
                case 'a':
                    global_options += 3 << 4;
                    break;
                default:
                    return -1; // Same as above switch, just a failsafe.
            }

            return 0;
        }
    }
}
