/* Type definitions local to the File System. */

struct dir_struct {         /* directory entry */
    inode_nr d_inum;        /* inode number */
    char d_name[NAME_SIZE]; /* character string */
};

/* Declaration of the disk inode used in rw_inode(). */
struct d_inode {                  /* disk inode.  Memory inode is in "inotab.h" */
    mask_bits i_mode;             /* file type, protection, etc. */
    uid i_uid;                    /* user id of the file's owner */
    file_pos i_size;              /* current file size in bytes */
    file_pos64 i_size64;          /* 64-bit file size */
    real_time i_modtime;          /* when was file data last changed */
    gid i_gid;                    /* group number */
    links i_nlinks;               /* how many links to this file */
    zone_nr i_zone[NR_ZONE_NUMS]; /* block nums for direct, ind, and dbl ind */
};
