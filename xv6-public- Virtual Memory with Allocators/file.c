//
// File descriptors
//file.c 

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "mmu.h"    // For NSEGS
#include "x86.h"    // For taskstate structure
#include "proc.h"

static int growfile(struct inode *ip, int size);

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

int
fileseek(struct file *f, int offset)
{
  if(f == 0)
    return -1;
    
  if(f->type != FD_INODE)
    return -1;
    
  // Calculate new offset
  int new_offset = f->off + offset;
  
  // Don't allow negative file positions
  if(new_offset < 0)
    return -1;
    
  // If new position is beyond file size, extend with zeros
  if(new_offset > f->ip->size) {
    begin_op();  // Start the transaction
    
    ilock(f->ip);
    
    int old_size = f->ip->size;
    if(growfile(f->ip, new_offset) < 0) {
      iunlock(f->ip);
      end_op();
      return -1;
    }
      
    // Fill the hole with zeros
    int i;
    char zeros[512];
    memset(zeros, 0, sizeof(zeros));
    
    for(i = old_size; i < new_offset; i += sizeof(zeros)) {
      int n = new_offset - i;
      if(n > sizeof(zeros))
        n = sizeof(zeros);
      if(writei(f->ip, zeros, i, n) != n) {
        iunlock(f->ip);
        end_op();
        return -1;
      }
    }
    
    iunlock(f->ip);
    end_op();  // End the transaction
  }
  
  f->off = new_offset;
  return new_offset;
}



static int
growfile(struct inode *ip, int size)
{
  if(ip->type != T_FILE)
    return -1;
    
  if(size > MAXFILE*BSIZE)
    return -1;
    
  ip->size = size;
  iupdate(ip);
  return 0;
}