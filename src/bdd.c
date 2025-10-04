#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "debug.h"

#include "studentheaders.h"


/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

// Find the index of the next free spot in bdd_nodes.
int freeSpot() {
    for (int i = 256; i < BDD_NODES_MAX; i++) {
        BDD_NODE *current = bdd_nodes + i;
        //debug("%i level, %i left, %i right\n", current->level, current->left, current->right);
        if (current->level == 0) {
            return i;
        }
    }
    return -1; // Error has occurred, no free spaces.
}


/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {

    // Out of range args
    if (left >= BDD_NODES_MAX || right >= BDD_NODES_MAX || level > BDD_LEVELS_MAX) { return -1; }

    // left and right children are the same, so we just return the index of the child since this node is useless.
    if (left == right) { return left; }

    // otherwise we need to search the hashtable to see if there's a node with these attributes.
    int hash = 0;
    // Our hash function will be key = (LEFT * RIGHT) % 2097169, which is BDD_HASH_SIZE. Collisions will be dealt with using linear probing.

    hash = (left * right) % BDD_HASH_SIZE;
    //debug("%i hash\n", hash);
    //debug("%p pointer\n", *(bdd_hash_map + hash));

    // Check if this hash is in the map.
    if (*(bdd_hash_map + hash) != NULL) {

        for (int i = hash; i < BDD_HASH_SIZE; i++) {
            if (*(bdd_hash_map + i) == NULL) { break; } // Not in the hash table.

            BDD_NODE *current = *(bdd_hash_map + i);
            if ((*current).left == left && (*current).right == right && (*current).level == level) {
                // We found the matching node. Now to find its index.
                for (int j = 0; j < BDD_NODES_MAX; j++) {
                    BDD_NODE *toCompare = bdd_nodes + j;
                    if ((*toCompare).left == left && (*toCompare).right == right && (*toCompare).level == level) { return j;}
                }
            }
        }

        // We need to wrap around.
        for (int i = 0; i < BDD_HASH_SIZE; i++) {
            if (*(bdd_hash_map + i) == NULL) { break; } // Not in the hash table.

            BDD_NODE *current = *(bdd_hash_map + i);
            if ((*current).left == left && (*current).right == right && (*current).level == level) {
                // We found the matching node. Now to find its index.
                for (int j = 0; j < BDD_NODES_MAX; j++) {
                    BDD_NODE *toCompare = bdd_nodes + j;
                    if ((*toCompare).left == left && (*toCompare).right == right && (*toCompare).level == level) { return j;}
                }
            }
        }
    }

    // The hash is NOT in the map, we can make a new node.
    BDD_NODE newNode = {level, left, right};
    //debug("%c\n", newNode.level + 64);
    //debug("NEW NODE: %i level (%c letter), %i left, %i right, %i nodeNum\n", newNode.level, newNode.level + 64, newNode.left, newNode.right, freespot()); // not index - 1 bc havent incremented yet
    //debug("%p newnode ptr\n", &newNode);

    int freeSpotIndex = freeSpot();
    if(freeSpotIndex == -1) return -1;
    //debug("%i\n", freeSpotIndex);
    *(bdd_nodes + freeSpotIndex) = newNode; // Insert new node into the table.

    // Space found so insert into hash table.
    if (*(bdd_hash_map + hash) == NULL) {
        *(bdd_hash_map + hash) = bdd_nodes + freeSpotIndex;
        return freeSpotIndex;
    }

    // Collision at hash
    else {
        //debug("Collision at hash\n");
        for (int i = hash; i < BDD_HASH_SIZE; i++) {
            if (*(bdd_hash_map + i) == NULL) {
                *(bdd_hash_map + i) = bdd_nodes + freeSpotIndex;
                return freeSpotIndex;
            }
        }
    }
    //debug("Wrapping around\n");
    // Wrap around
    for (int i = 0; i < BDD_HASH_SIZE; i++) {
        if (*(bdd_hash_map + i) == NULL) {
            *(bdd_hash_map + i) = bdd_nodes + freeSpotIndex;
            return freeSpotIndex;
        }
    }

    return -1; // Error somewhere with the args that wasn't spotted earlier.
}

int power(int base, int raise) {
    if (raise == 0) { return 1;}
    else { return base * power(base, raise-1);}
}

/**
 * Determine the minimum number of levels required to cover a raster
 * with a specified width w and height h.
 *
 * @param w  The width of the raster to be covered.
 * @param h  The height of the raster to be covered.
 * @return  The least value l >=0 such that w <= 2^(l/2) and h <= 2^(l/2).
 */
int bdd_min_level(int w, int h) {
    int l = 0;

    while (power(2, l/2) < w || power(2, l/2) < h) {
        l++;
    }

    return l;
}

// 0 is row split, 1 is col.
int recursiveBddBuilder(int level, int split, int sizeR, int sizeC, int topLeftR, int topLeftC,  int ogWidth, int ogHeight, unsigned char *raster) {

    if (level == 0) {
        // get color.
        // outside original raster
        if (topLeftR >= ogHeight || topLeftC >= ogWidth) {
            return 0;
        }
        else {
            //debug("(%i,%i) R,C with color %i\n", topLeftR, topLeftC, *(raster + (topLeftR * ogWidth) + topLeftC));
            //debug("COLOR NODE HIT AT: (%i, %i)\n", topLeftR, topLeftC);
            return *(raster + (topLeftR * ogWidth) + topLeftC);
        }
    }

    else {

        int left;
        int right;

        if (split == 0) {
            left = recursiveBddBuilder(level - 1, 1, sizeR / 2, sizeC, topLeftR, topLeftC, ogWidth, ogHeight, raster); // coords dont change if we go left.
        }
        else {
            left = recursiveBddBuilder(level - 1, 0, sizeR, sizeC / 2, topLeftR, topLeftC, ogWidth, ogHeight, raster); // coords dont change if we go left.
        }

        // when going right: if row choice, row coordinate increases by currentrowsize / 2. if col choice, col coord increases by currentcolsize / 2
        // go right while splitting row. next call will split the opposite so we pass the opposite.
        if (split == 0) {
            right = recursiveBddBuilder(level - 1, 1, sizeR / 2, sizeC, topLeftR + (sizeR / 2), topLeftC, ogWidth, ogHeight, raster);
        }
        // split col
        else {
            right = recursiveBddBuilder(level - 1, 0, sizeR, sizeC / 2, topLeftR, topLeftC + (sizeC / 2), ogWidth, ogHeight, raster);
        }

        return bdd_lookup(level, left, right);
    }
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {

    // Initialize hashmap.
    for (int i = 0; i < BDD_HASH_SIZE; i++) {

        *(bdd_hash_map + i) = NULL;
    }
    if (w < 0 || h < 0) { return NULL;} // invalid

    // function returns 2d.
    int levels = bdd_min_level(w, h); // this is the number of levels, or 2d.
    int d = levels / 2; // this is d.
    int dimensions =  power(2, d); // the dimensions are 2^d by 2^d now.
    //debug("%i d, %i w, %i h\n", d, w, h);
    //debug("%i levels, %i dimensions, %i d\n", levels, dimensions, d);
    int root = recursiveBddBuilder(levels, 0, dimensions, dimensions, 0, 0, w, h, raster);
    return (bdd_nodes + root);

}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {

    int offset;
    unsigned char returned;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            offset = (i *w) + j;
            returned = bdd_apply(node, i, j);
            //debug("%i ", returned);
            *(raster + offset) = returned;
        }
        //debug("\n");
    }
}

// Converts given serial numbers to their proper representation and writes them to out.
void writeSerialize(int left, int right, int level, FILE *out) {
    //debug("%c level, %i left, %i right\n", level,left, right);
    //debug("LEFT IN OCTAL: %i\n\n", left);


    // convert to little endian.
    // bytes that go 1/2/3/4 will be stored as 4/3/2/1.
    int left1 = (left & 0xFF000000) >> 24;
    int left2 = (left & 0xFF0000) >> 16;
    int left3 = (left & 0xFF00) >> 8;
    int left4 = (left & 0xFF);

    int right1 = (right & 0xFF000000) >> 24;
    int right2 = (right & 0xFF0000) >> 16;
    int right3 = (right & 0xFF00) >> 8;
    int right4 = (right & 0xFF);

    //debug("integer: %c level, %i/%i/%i/%i and right: %i/%i/%i/%i\n", level, left4, left3, left2, left1, right4, right3, right2, right1);
    //debug("%c L:%i R:%i\n", level, left, right);
    //debug(debug"octal: %c/%o/%o/%o/%o and right: %o/%o/%o/%o\n\n", level, left4, left3, left2, left1, right4, right3, right2, right1);
    //debug("%c%c%c%c%c%c%c%c%c", level, left1, left2, left3, left4, right1, right2, right3, right4);
    fputc(level, out);

    fputc(left4, out);
    fputc(left3, out);
    fputc(left2, out);
    fputc(left1, out);

    fputc(right4, out);
    fputc(right3, out);
    fputc(right2, out);
    fputc(right1, out);
}

// nodeIndex is the index of currentNode.
int postorder_write(BDD_NODE *currentNode, int nodeIndex, int *serial, FILE *out) {

    // base case is that we hit a color. Write that if it's not already here.
    if (nodeIndex < 256) {
        if (*(bdd_index_map + nodeIndex) == 0) {
            fputc('@', out);
            fputc(nodeIndex, out);
            *(bdd_index_map + nodeIndex) = (*serial);
            (*serial) = (*serial) + 1;
            return (*serial) - 1;
        }
        else { return *(bdd_index_map + nodeIndex);}
    }

    // Otherwise we continue w/ the traversal.
    postorder_write(LEFT(currentNode, (*currentNode).level-1), (*currentNode).left, serial, out);
    if (*(bdd_index_map + (*currentNode).left) == 0) {
        *(bdd_index_map + (*currentNode).left) = (*serial);
        (*serial) = (*serial) + 1;
    }

    postorder_write(RIGHT(currentNode, (*currentNode).level-1), (*currentNode).right, serial, out);
    if (*(bdd_index_map + (*currentNode).right) == 0) {
        *(bdd_index_map + (*currentNode).right) = (*serial);
        (*serial) = (*serial) + 1;
    }

    // write currentNode
    if (*(bdd_index_map + nodeIndex) == 0) {
        *(bdd_index_map + nodeIndex) = (*serial);
        (*serial) = (*serial) + 1;

        BDD_NODE *toWrite = (bdd_nodes + nodeIndex);
        int level = (*toWrite).level + 64; // offset to get ascii. lvl 1 = a, so lvl 1 is 1 + 64 (ascii for 'A').
        int left = (*toWrite).left;
        int right = (*toWrite).right;

        // But left and right are improperly indexed, we need to get bdd_index_map + left/right to get the serial.
        left = *(bdd_index_map + left);
        right = *(bdd_index_map + right);

        // Now we have the info to write.
        writeSerialize(left, right, level, out);

        return (*serial) - 1;
    }
    else {
        return *(bdd_index_map + nodeIndex);
    }

}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    if (node == NULL) { return -1;} // invalid.
    // Initialize bdd_index_map.
    for (int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map + i) = 0;
    }
    int serialize_serial = 1;
    int unused_node_index = freeSpot();

    postorder_write(LEFT(node, (*node).level - 1), (*node).left, &serialize_serial, out); // left half
    postorder_write(RIGHT(node, (*node).level - 1), (*node).right, &serialize_serial, out); // right half
    //debug("%i\n", serialize_serial);
    // Manually write root to bdd_index_map if it's not there (seems to be the case most of the time.)
    // to do that we need the index of the root, and then need to check if it has a serial associated with it. If not, then we add it.
    /* Determine the index of the root node. Prefer pointer arithmetic when possible. */
    int rootIndex = -1;
    if (node >= bdd_nodes && node < bdd_nodes + BDD_NODES_MAX) {
        rootIndex = (int)(node - bdd_nodes);
    } else {
        /* Fallback: search for a matching node among allocated nodes. */
        for (int i = 0; i < unused_node_index; i++) {
            BDD_NODE *compare = bdd_nodes + i;
            if ((*compare).left == (*node).left && (*compare).right == (*node).right && (*compare).level == (*node).level) {
                rootIndex = i;
                break;
            }
        }
    }

    if (rootIndex < 0) {
        /* Couldn't determine root index: treat as error. */
        return -1;
    }

    if (*(bdd_index_map + rootIndex) == 0) {
        *(bdd_index_map + rootIndex) = serialize_serial;
        serialize_serial++;
    }

    BDD_NODE *toWrite = (bdd_nodes + unused_node_index-1); // get root
    int level = (*toWrite).level + 64; // offset to get ascii. lvl 1 = a, so lvl 1 is 1 + 64 (ascii for 'A').
    int left = (*toWrite).left;
    int right = (*toWrite).right;

    // But left and right are improperly indexed, we need to get bdd_index_map + left/right to get the serial.
    left = *(bdd_index_map + left);
    right = *(bdd_index_map + right);

    // Now we have the info to write.
    writeSerialize(left, right, level, out);

    /* Return 0 on success to match test expectations. */
    return 0;
}

BDD_NODE *bdd_deserialize(FILE *in) {

    // Initialize hashmap.
    for (int i = 0; i < BDD_HASH_SIZE; i++) {

        *(bdd_hash_map + i) = NULL;
    }
    // Initialize bdd_index_map.
    for (int i = 0; i < BDD_NODES_MAX; i++) {
        *(bdd_index_map + i) = 0;
    }

    int deserialize_serial = 1;
    int readingNodes = 0; // initially false since we might have to go through the header and leaf nodes.
    int character;
    character = fgetc(in);

    while (character != EOF) {

        if (character == '@') {
            readingNodes = 1;
            int colorVal = fgetc(in); // we have the color value now.
            if (*(bdd_index_map + colorVal) == 0) {
                *(bdd_index_map + colorVal) = deserialize_serial; // Update bdd_index_map with a serial number.
                deserialize_serial++;
            }
        }

        if (character >= 'A' && character <= 'Z') {  /* or 65..90 */
            readingNodes = 1;
            int level = character - 64;

            int left1 = fgetc(in); // LSB
            int left2 = fgetc(in); // 2nd from right
            int left3 = fgetc(in); // 3rd from right
            int left4 = fgetc(in); // MSB

            int right1 = fgetc(in); // LSB
            int right2 = fgetc(in); // 2nd from right
            int right3 = fgetc(in); // 3rd from right
            int right4 = fgetc(in); // MSB

            // stored as 1,2,3,4 but we want it as 4,3,2,1.
            //debug("%c level, %i/%i/%i/%i left and %i/%i/%i/%i right.\n", level + 64, left1, left2, left3, left4, right1, right2, right3, right4);
            int bigLeft = 0;
            bigLeft += left1;
            bigLeft += left2 << 8;
            bigLeft += left3 << 16;
            bigLeft += left4 << 24;

            int bigRight = 0;
            bigRight += right1;
            bigRight += right2 << 8;
            bigRight += right3 << 16;
            bigRight += right4 << 24;
            //debug("now we have %c level, %i left, %i right\n", level + 64, bigLeft, bigRight);

            // Now we have the level, left serial, and right serial of a node.
            // The left and right serials are guaranteed to exist already, so we just need to find their corresponding index and create a new node.

            // big left and right are serials.

            int leftIndex;
            int rightIndex;

            for (int i = 0; i < BDD_NODES_MAX; i++) {
                if (*(bdd_index_map + i) == bigLeft) {
                    leftIndex = i;
                    break;
                }
            }

            for (int i = 0; i < BDD_NODES_MAX; i++) {
                if (*(bdd_index_map + i) == bigRight) {
                    rightIndex = i;
                    break;
                }
            }

            //debug("NEW: %i level, %i left, %i right\n", level, bigLeft, bigRight);
            int nodeIndex = bdd_lookup(level, leftIndex, rightIndex);
            if (nodeIndex == -1) {return NULL;}

            // Node successfully created. Store the serial in the index map.
            // the index of the newly created node is returned and stored.
            *(bdd_index_map + nodeIndex) = deserialize_serial;
            deserialize_serial++;

        }
        character = fgetc(in);
        if (readingNodes == 0) {continue;}
    }
    //debug("%i serials in deserialize\n", deserialize_serial);
    //debug("%i nodes created.\n", (freespot()- 256));
    //debug("%i level, %i left, %i right root\n", (*root).level, (*root).left, (*root).right);

    return (bdd_nodes + freeSpot()-1);
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {

    // This is the root node so we can get d from this.
    int d = (*node).level / 2;

    int bitsToExpect = 2 * d;

    int direction;
    int rMask = 0;
    int rPtr = 0; // bit ptr to determine the shift

    int cMask = 0;
    int cPtr = 0;
    BDD_NODE *current = node;

    // row -> col -> row -> col | if i is even, then take the next row bit. else take the next col bit.
    for (int i = 0; i < bitsToExpect; i++) {

        // even, take row bit.
        if (i % 2 == 0) {
            rMask = 1 << (d - 1 - rPtr);
            direction = (r & rMask) >> (d - 1 - rPtr);
            rPtr++;
        }
        // Odd, take col bit.
        else {
            cMask = 1 << (d - 1 - cPtr);
            direction = (c & cMask) >> (d - 1 - cPtr);
            cPtr++;

        }
        //debug("cptr: %i, rptr: %i\n", cPtr, rPtr);
        if (direction == 0) { // left
            if ((*LEFT(current, (*current).level - 1)).level == 0) { // If left is color node, return that.
                //debug("LEFT: %i left, %i right\n", (*current).left, (*current).right);
                return (*current).left;
            }
            // We have "dead bits" that weren't included in the bdd. skip over those. (ie going from lvl 5 to 2 with nothing in between.)
            if ((*LEFT(current, (*current).level - 1)).level != (*current).level - 1) {
                //debug("(%i, %i) SKIPPING LEFT FROM %i TO %i\n", r, c, (*current).level, (*current).level -(*LEFT(current, (*current).level - 1)).level-1);

                // we need to determine how many column and row bits we skip when jumping down.
                // Just loop from 0 to the bit difference, which ptr we increment first depends on whether we're currently on a row or col bit.
                // If we're on a col bit, then we increment row first and vice versa.
                if (i % 2 == 0) { // on a row bit, increment col first.
                    for (int j = 0; j < (*current).level -(*LEFT(current, (*current).level - 1)).level - 1; j++) {
                        if (j % 2 == 0) {cPtr++;}
                        else {rPtr++;}
                    }
                }
                else {
                    for (int j = 0; j < (*current).level -(*LEFT(current, (*current).level - 1)).level - 1; j++) {
                        if (j % 2 == 0) {rPtr++;}
                        else {cPtr++;}
                    }
                }
                i += ((*current).level -(*LEFT(current, (*current).level - 1)).level - 1); // ie: 5 to 2 means we skip 3 levels (current.level - next.level)
            }
            current = LEFT(current, (*current).level - 1); // Otherwise go left.
        }


        else { // right
            if ((*RIGHT(current, (*current).level - 1)).level == 0) {
                //debug("RIGHT: %i left, %i right\n", (*current).left, (*current).right);
                return (*current).right;
            }

            // We have "dead bits" that weren't included in the bdd. skip over those. (ie going from lvl 5 to 2 with nothing in between.)
            if ((*RIGHT(current, (*current).level - 1)).level != (*current).level - 1) {
                //debug("(%i, %i) SKIPPING RIGHT FROM %i TO %i\n", r, c, (*current).level, (*current).level -(*RIGHT(current, (*current).level - 1)).level-1);

                // we need to determine how many column and row bits we skip when jumping down.
                // Just loop from 0 to the bit difference, which ptr we increment first depends on whether we're currently on a row or col bit.
                // If we're on a col bit, then we increment row first and vice versa.
                if (i % 2 == 0) { // on a row bit, increment col first.
                    for (int j = 0; j < (*current).level -(*RIGHT(current, (*current).level - 1)).level - 1; j++) {
                        if (j % 2 == 0) {cPtr++;}
                        else {rPtr++;}
                    }
                }
                else {
                    for (int j = 0; j < (*current).level -(*RIGHT(current, (*current).level - 1)).level - 1; j++) {
                        if (j % 2 == 0) {rPtr++;}
                        else {cPtr++;}
                    }
                }
                i += ((*current).level -(*RIGHT(current, (*current).level - 1)).level - 1); // ie: 5 to 2 means we skip 3 levels(current.level - next.level - 1)
            }

            current = RIGHT(current, (*current).level - 1); // Otherwise go left.
        }
        //debug("cptr: %i, rptr: %i\n", cPtr, rPtr);
    }
    return -1; // Error has occurred.
}

int postorder_apply(BDD_NODE *current, int nodeNum, unsigned char (*func)(unsigned char), int *index) {
    // base case: we hit a value to apply the function to.
    if ((*current).level == 0) {
        //debug("%i color, %i transformed", (int)nodeNum, (int)((*func)(nodeNum)));
        return  (*func)(nodeNum);
    }

    else {
        int left = postorder_apply(LEFT(current, (*current).level - 1), (*current).left, func, index);
        int right = postorder_apply(RIGHT(current, (*current).level - 1), (*current).right, func, index);


        //debug("%i level, %i left, %i right\n", (*current).level, left, right);
        int node = bdd_lookup((*current).level, left, right);
        //debug("index: %i\n", freeSpot());
        //debug("node created at %i with left %i and right %i with level %i\n", node, (*(bdd_nodes + node)).left, (*(bdd_nodes + node)).right, (*(bdd_nodes + node)).level);
        if (node == (*index)) { (*index) = (*index) + 1;} // new node created.
        return node;
    }
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // the new bdd starts at index freeSpot(). We can start there.
    //debug("%i is where old bdd ends.\n", freeSpot() - 1);
    int newbdd_unused_node_index = freeSpot();

    int newRoot = postorder_apply(node, freeSpot() - 1, func, &newbdd_unused_node_index); // pass index - 1 since that's the root of the previous bdd.
    //debug("%i is the root of the new bdd. newroot is %i.\n", newRoot, freeSpot()-1);
    return (bdd_nodes + newRoot);
}


// 0 = row, 1 = col
int recursive_bdd_rotator(int level, int sizeR, int sizeC, int topLeftR, int topLeftC, BDD_NODE *root, int *index) {

    if (level == 0) {
        //debug("(%i, %i)\n", topLeftR, topLeftC);
        return bdd_apply(root, topLeftR, topLeftC);
    }

    else {
        int topLeft, topRight;
        int botLeft, botRight;
        // A B
        // C D

        topLeft = recursive_bdd_rotator(level - 2, sizeR / 2, sizeC / 2, topLeftR, topLeftC, root, index);

        topRight = recursive_bdd_rotator(level - 2, sizeR / 2, sizeC / 2, topLeftR, topLeftC + (sizeC / 2), root, index);

        botLeft = recursive_bdd_rotator(level - 2, sizeR / 2, sizeC / 2, topLeftR + (sizeR / 2), topLeftC, root, index);

        botRight = recursive_bdd_rotator(level - 2, sizeR / 2, sizeC / 2, topLeftR + (sizeR / 2), topLeftC + (sizeC / 2), root, index);

        //if (sizeR == 2) {
            //debug("%i/%i\n%i/%i", topLeft, topRight, botLeft, botRight);
        //}
        int topSliver = bdd_lookup(level - 1, topRight, botRight);
        if (topSliver == (*index)) { (*index) = (*index) + 1;}

        int botSliver = bdd_lookup(level - 1, topLeft, botLeft);
        if (botSliver == (*index)) { (*index) = (*index) + 1;}

        int new = bdd_lookup(level, topSliver, botSliver);
        if (new == (*index)) {
            (*index) = (*index) + 1;
        }
        //debug("node created at %i with left %i and right %i with level %i\n", new, (*(bdd_nodes + new)).left, (*(bdd_nodes + new)).right, (*(bdd_nodes + new)).level);
        //if (sizeR == 2) {
            //debug("becomes: %i/%i\n%i/%i", (bdd_nodes + topSliver)->left, (bdd_nodes + topSliver)->right, (bdd_nodes + botSliver)->left, (bdd_nodes + botSliver)->right);
        //}
        return new;

    }

}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // We should be guaranteed to only work with a square here.
    // The dimensions are equal to 2^d, where d = levels in the bdd / 2.
    int dimensions = power(2, (*node).level / 2);
    //debug("%ix%i dimensions\n", dimensions, dimensions);
    //debug("%i is where old bdd ends.\n", freeSpot());

    int newbdd_unused_node_index = freeSpot();
    int newRoot = recursive_bdd_rotator(node->level, dimensions, dimensions, 0, 0, node, &newbdd_unused_node_index);
    //debug("%i level, %i left, %i right\n", (bdd_nodes + newRoot)->level-1, (bdd_nodes + newRoot)->left, (bdd_nodes + newRoot)->right);
    return bdd_nodes + newRoot;
}

// newLevel keeps track of the level of the new BDD.
int postorder_zoom_in(BDD_NODE *current, int nodeNum, int levelIncrease, int *index) {
    if ((*current).level == 0) {
        return nodeNum;
    }

    else {
        int left = postorder_zoom_in(LEFT(current, (*current).level - 1), (*current).left, levelIncrease, index);
        int right = postorder_zoom_in(RIGHT(current, (*current).level - 1), (*current).right, levelIncrease, index);

        int node = bdd_lookup((*current).level + levelIncrease, left, right);
        if (node == (*index)) {
            (*index) = (*index) + 1;
        }
        //debug("NEW NODE: %i level, (%c letter), %i left, %i right\n", (*current).level + levelIncrease, (*current).level + levelIncrease + 64, left, right);
        return node;
    }
}

int checkReplacement(BDD_NODE *node, int nodeNum) {
    if (node->level == 0) {
        if (nodeNum == 0) { return 0;} // black
        else { return 255;} // white
    }

    else {
        int left = checkReplacement(LEFT(node, node->level - 1), node->left);
        if (left == 255) { return 255;}

        int right = checkReplacement(RIGHT(node, node->level - 1), node->right);
        if (right == 255) { return 255;}

        return 0;
    }
}
int postorder_zoom_out(BDD_NODE *current, int nodeNum, int levelDecrease, int* index) {

    if (current->level <= levelDecrease) {
        //debug("node hit at level %i with left %i and right %i\n", current->level, current-> left, current-> right);
        //debug("This node will be replaced with %i", checkReplacement(current, nodeNum));
        return checkReplacement(current, nodeNum);
    }
    else if (current-> level == 0) { return nodeNum;}

    else {
        int left = postorder_zoom_out(LEFT(current, current->level-1), current->left, levelDecrease, index);
        int right = postorder_zoom_out(RIGHT(current, current->level-1), current->right, levelDecrease, index);

        int node = bdd_lookup(current->level - levelDecrease, left, right);
        if (node == (*index)) { (*index) = (*index) + 1;}
        //debug("NEW NODE: %i level, (%c letter), %i left, %i right %i index\n", (*current).level - levelDecrease, (*current).level - levelDecrease + 64, left, right, node);
        return node;
    }
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    //debug("%i node level, %i level, %i factor\n", node->level, level, factor);
    if (level < 0 || level > BDD_LEVELS_MAX) { return NULL;}
    //debug("zoom factor of %i\n", factor);
    if (factor == 0) { return node;} // Identity zoom by a factor of 1 (2^0 = 1)

    else if (factor > 0) {
        // the new bdd starts at index freeSpot(). We can start there.
        //debug("%i is where old bdd ends.\n", freeSpot()-1);
        int newbdd_unused_node_index = freeSpot();
        int newRoot = postorder_zoom_in(node, newbdd_unused_node_index - 1, (2* factor), &newbdd_unused_node_index);
        return bdd_nodes + newRoot;
    }

    else if (factor < 0){ // zoom out
        //debug("zoom factor of %i\n", factor);
        factor = factor * -1;
        int newbdd_unused_node_index = freeSpot();
        int newRoot = postorder_zoom_out(node, newbdd_unused_node_index- 1, (2* factor), &newbdd_unused_node_index);
        return bdd_nodes + newRoot;
    }
    else {
        return NULL; // invalid.
    }
}