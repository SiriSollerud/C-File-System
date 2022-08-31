#include "allocation.h"
#include "inode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BLOCKSIZE 4096

// inode_counter is used to set ids for new inodes in the create functions
int inode_counter = 0;


/* Create a file below the inode parent. Parent must
 * be a directory. The size of the file is size_in_bytes,
 * and create_file calls the allocate_block() function
 * enough number of times to reserve blocks in the simulated
 * disk to store all of these bytes.
 * Returns a pointer to file's inodes.
 */
struct inode* create_file( struct inode* parent, char* name, char readonly, int size_in_bytes ) {
    // if there is no parent, we can't add an inode_file
    if (!parent) return NULL;

    // checking if there's a file with the same name
    if (parent->is_directory == 1) {
        for (int i = 0; i < parent->num_entries; i++) {
            if (!strcmp(((struct inode*) parent->entries[i])->name, name) &&
               !((struct inode*) parent->entries[i])->is_directory) {
                return NULL;
            }
        }
    }

    // making a new file_inode
    struct inode *file_inode = malloc(sizeof(struct inode));
    if (!file_inode) {
         fprintf(stderr, "malloc failed\n");
         return NULL;
    }

    file_inode->id = inode_counter++;
    file_inode->name = strdup(name);
    file_inode->is_directory = 0;
    file_inode->is_readonly = readonly;
    file_inode->filesize = size_in_bytes;

    // calculating how many blocks the file needs
    file_inode->num_entries = size_in_bytes / BLOCKSIZE;
    if (size_in_bytes % BLOCKSIZE) {
        file_inode->num_entries++;
    }

    file_inode->entries = malloc(sizeof(size_t) * file_inode->num_entries );
    for (int i = 0; i < file_inode->num_entries; i++) {
        int block_index = allocate_block();

        // checking if there are no available blocks - meaning, the file is too big
        if (block_index == -1) {
            free(file_inode->entries);
            free(file_inode);
            perror("Disk is full.");
            return NULL;
        } else {
            file_inode->entries[i] = (size_t) block_index;
        }
    }

    // updating the parent node's num_entries and entries
    parent->num_entries++;
    parent->entries = (size_t*) realloc(parent->entries, sizeof(size_t) * parent->num_entries);
    parent->entries[parent->num_entries- 1] = file_inode;
    return file_inode;
}


/* Create a directory below the inode parent. Parent must
 * be a directory.
 * Returns a pointer to file's inodes.
 */
struct inode* create_dir( struct inode* parent, char* name ) {
    if ((!parent) || parent->is_directory == 1) {
        // if there is a parent --> checking if there's a directory with the same name
        if (parent) {
            for (int i = 0; i < parent->num_entries; i++) {
                if (!strcmp(((struct inode*) parent->entries[i])->name, name) &&
                   ((struct inode*) parent->entries[i])->is_directory) {
                    return NULL;
                }
            }
            // if there is a parent --> increase amount of entries with 1
            parent->num_entries++;
            parent->entries = realloc(parent->entries, sizeof(size_t) * parent->num_entries);
        }

        // creating a new inode_dir
        struct inode *inode_dir = malloc(sizeof(struct inode));
        inode_dir->id = inode_counter++;
        inode_dir->name = strdup(name);
        inode_dir->is_directory = 1;
        inode_dir->is_readonly = 0;
        inode_dir->filesize = 0;
        inode_dir->num_entries = 0;

        // entries set to NULL if it is the root
        inode_dir->entries = NULL;

        // if there is a parent --> adding the inode_dir to parent->entries
        if (parent) {
            parent->entries[parent->num_entries- 1] = inode_dir;
        }
        return inode_dir;
    }
    return NULL;
}


/* Check all the inodes that are directly referenced by
 * the node parent. If one of them has the name "name",
 * its inode pointer is returned.
 * parent must be directory.
 */
 // I have decided not to use find_inode_by_name() in my create functions
 // because I got paranoid/confused if I had to account for the possibility
 // that a filename and a directory name could be the same.
struct inode* find_inode_by_name( struct inode* parent, char* name ) {
    if (parent->is_directory == 1) {
        for (int i = 0; i < parent->num_entries; i++) {
            if (!strcmp(((struct inode*) parent->entries[i])->name, name)) {
                return parent->entries[i];
            }
        }
    }
    return NULL;
}

// recurivesly creates all inodes in the file tree, returns the root inode
struct inode* create_inode(FILE* superblock_file) {
    // inode_counter is used in the create functions to give a new
    // file_inode and dir_inode a correct id
    inode_counter++;

    struct inode *inode = malloc(sizeof(struct inode));
    fread(&inode->id, sizeof(int), 1, superblock_file);

    int name_len;
    fread(&name_len, sizeof(int), 1, superblock_file);

    inode->name = malloc(sizeof(char) * name_len);
    fread(inode->name, sizeof(char), name_len, superblock_file);

    fread(&inode->is_directory, sizeof(char), 1, superblock_file);
    fread(&inode->is_readonly, sizeof(char), 1, superblock_file);
    fread(&inode->filesize, sizeof(int), 1, superblock_file);
    fread(&inode->num_entries, sizeof(int), 1, superblock_file);

    inode->entries = malloc(sizeof(long) * inode->num_entries);
    fread(inode->entries, sizeof(long), inode->num_entries, superblock_file);

    // linking the entries with recursion
    if (inode->is_directory == 1) {
        for (int i = 0; i < inode->num_entries; i++) {
            inode->entries[i] = (size_t*) create_inode(superblock_file);
        }
    }
    return inode;
}

/* Read the file superblock and create an inode in memory
 * for every inode that is stored in the file. Set the pointers
 * between inodes correctly.
 * The file superblock remains unchanged.
 */
struct inode* load_inodes() {
    FILE *superblock_file;

    //opening the superblock file
    superblock_file = fopen("superblock", "rb");
    if (superblock_file == NULL) {
        printf("fopen");
        fclose(superblock_file);
        exit(1);
    }

    // making the root_inode, closing file and returning the root_inode
    struct inode* root_inode = create_inode(superblock_file);
    fclose(superblock_file);
    return root_inode;
}


/* This function releases all dynamically allocated memory
 * recursively for all the referenced inodes are visited
 * and their memory is released, finally also for the
 * node that is passed as a parameter.
 * The simulated disk and the file superblock are
 * not changed.
 */
void fs_shutdown( struct inode* inode ) {
    if (!inode) {
        return;
    }

    // recusively frees all dir_inodes
    if (inode->is_directory) {
        for (int i = 0; i < inode->num_entries; i++) {
            fs_shutdown((struct inode*) inode->entries[i]);
        }
    }

    // frees file_inodes
    free(inode->name);
    if (inode -> entries) {
        free(inode->entries);
    }
    free(inode);
}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;


/* This function is handed out.
 *
 * It prints the node that is receives as a parameter,
 * and recurivesly all inodes that are stored below it.
 */
void debug_fs( struct inode* node ) {
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");
    if( node->is_directory ) {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_entries; i++ )
        {
            struct inode* child = (struct inode*)node->entries[i];
            debug_fs( child );
        }
        indent--;
    }
    else {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_entries; i++ )
        {
            printf("%d ", (int)node->entries[i]);
        }
        printf(")\n");
    }
}
