#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

int main(int argc, char **argv) {

    // Initialize hashmap.
    for (int i = 0; i < BDD_HASH_SIZE; i++) {

        *(bdd_hash_map + i) = NULL;
    }
    // Initialize bdd_index_map.
    for (int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map + i) = 0;
    }

    int valid = validargs(argc, argv);
    //debug("Valid args returned %i", valid);
    //debug("Global options is %x", global_options);
    //debug("Valid = %i\n", valid);
    if (global_options & HELP_OPTION) {
        USAGE(*argv, EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }

    if (valid == 0) {
        int inputFormat = global_options & 0xF; // stored in bits 0-3 (4 LSB). & by binary of 0000...1111, or F.
        int outputFormat = (global_options & 0xF0) >> 4; // stored in bits 4-7 -  0000...11110000
        //debug("%i in, %o out\n", inputFormat, outputFormat);

        // input = pgm
        if (inputFormat == 1) {
            switch (outputFormat) {

                // output = pgm. pgm to pgm is invalid.
                case 1:
                    USAGE(*argv, EXIT_FAILURE);
                    return EXIT_FAILURE;

                // output = birp
                case 2:
                    if (pgm_to_birp(stdin, stdout) == -1) { return EXIT_FAILURE;}
                    else { return EXIT_SUCCESS;}

                // output = ascii
                case 3:
                    if (pgm_to_ascii(stdin, stdout) == -1) { return EXIT_FAILURE;}
                    else { return EXIT_SUCCESS;}

            }
        }

        // input = birp
        else {
            switch (outputFormat) {
                // output = pgm
                case 1:
                    if (birp_to_pgm(stdin, stdout) == -1) { return EXIT_FAILURE;}
                    else { return EXIT_SUCCESS;}

                // output = birp
                case 2:
                    if (birp_to_birp(stdin, stdout) == -1) { return EXIT_FAILURE;}
                    else { return EXIT_SUCCESS;}

                // output = ascii
                case 3:
                    if (birp_to_ascii(stdin, stdout) == -1) { return EXIT_FAILURE;}
                    else { return EXIT_SUCCESS;}
            }
        }

        // argv managed to mess up.
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    // argv returned -1.
    else {
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
