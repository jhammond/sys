#ifndef _SYS_LUSTRE_H_
#define _SYS_LUSTRE_H_
#include <inttypes.h>
#include <sys/ioctl.h>

#define XATTR_NAME_LOV     "trusted.lov"
#define XATTR_NAME_LMA     "trusted.lma"
#define XATTR_NAME_LMV     "trusted.lmv"
#define XATTR_NAME_LINK    "trusted.link"
#define XATTR_NAME_FID     "trusted.fid"
#define XATTR_NAME_VERSION "trusted.version"

#define LL_IOC_PATH2FID _IOR ('f', 173, long)

struct lu_fid {
  /**                                                                                           
   * FID sequence. Sequence is a unit of migration: all files (objects)                         
   * with FIDs from a given sequence are stored on the same server.                             
   * Lustre should support 2^64 objects, so even if each sequence                               
   * has only a single object we can still enumerate 2^64 objects.                              
   */
  uint64_t f_seq;
  /** FID number within sequence. */
  uint32_t f_oid;
  /**                                                                                           
   * FID version, used to distinguish different versions (in the sense                          
   * of snapshots, etc.) of the same file system object. Not currently                          
   * used.                                                                                      
   */
  uint32_t f_ver;
};

#define PRI_FID "[0x%"PRIx64":0x%"PRIx32":0x%"PRIx32"]"
#define PRI_FID_ARGS(f) (f)->f_seq, (f)->f_oid, (f)->f_ver

struct lustre_mdt_attrs {
  /*
   * Bitfield for supported data in this structure. From enum
   * lma_compat.  lma_self_fid and lma_flags are always available.
   */
  uint32_t lma_compat;
  /*
   * Per-file incompat feature list. Lustre version should support all
   * flags set in this field. The supported feature mask is available
   * in LMA_INCOMPAT_SUPP.
   */
  uint32_t lma_incompat;
  /** FID of this inode */
  struct lu_fid lma_self_fid;
  /** mdt/ost type, others */
  uint64_t lma_flags;
  /* IO Epoch SOM attributes belongs to */
  uint64_t lma_ioepoch;
  /** total file size in objects */
  uint64_t lma_som_size;
  /** total fs blocks in objects */
  uint64_t lma_som_blocks;
  /** mds mount id the size is valid for */
  uint64_t lma_som_mountid;
};

#endif
