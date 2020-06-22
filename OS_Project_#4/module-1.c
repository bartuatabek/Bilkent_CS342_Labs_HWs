/*
 *  module-1.c − Demonstrates module documentation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/fs_struct.h>
#include <linux/rcupdate.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/buffer_head.h>
#include <linux/genhd.h>
#include <linux/types.h>

#define DRIVER_AUTHOR "Utku Görkem Ertürk & Bartu Atabek"
#define DRIVER_DESC   "A sample driver"

static int processid = 0;

module_param(processid, int, 0);
MODULE_PARM_DESC(processid, "pid");

static int __init init_module_1(void)
{
  pid_t pid = processid;

  struct task_struct *task;

  task = pid_task(find_vpid(pid), PIDTYPE_PID);

  spin_lock(&(task->files->file_lock));
  struct file *currfd;
  int i;
  for(i = 0;(currfd = task->files->fdt->fd[i]) != NULL;i++){
    //cwd = d_path(&(currfd->f_path),buffer,100*sizeof(char));
    printk(KERN_ALERT "fd: %d, ", i);
    //printk(KERN_CONT "path: %s, ", cwd);
    printk(KERN_CONT "current file position ptr: %lld, ", currfd->f_pos);
    printk(KERN_CONT "user's id: %d, ", currfd->f_owner.euid.val);
    printk(KERN_CONT "access mode:%d, ", currfd->f_mode);
    printk(KERN_CONT "file name: %s, ", currfd->f_path.dentry->d_name.name);
    printk(KERN_CONT "inode no: %ld, ", currfd->f_path.dentry->d_inode->i_ino);
    printk(KERN_CONT "file length: %lld, ", currfd->f_path.dentry->d_inode->i_size);
    printk(KERN_CONT "no of blocks: %ld\n", currfd->f_path.dentry->d_inode->i_blocks);
    //buffer cache
    unsigned long totalNumOfPages  = currfd->f_mapping->nrpages;
    int k;
    int cacheCounter = 0;
    for(k=0; (k < totalNumOfPages) && (k < 100);k++){
	struct page *currpage= find_get_page(currfd->f_mapping, k);
        
        if(currpage != NULL){
	    cacheCounter = cacheCounter + 1;
	    struct buffer_head *bf = (struct buffer_head*) currpage->private;
	    struct buffer_head *nbf = bf;
		
	    
            if(nbf != NULL){
			printk(KERN_ALERT "Storage device the block is in: %s, ", nbf->b_bdev->bd_disk->disk_name);
			printk(KERN_ALERT "the block number: %lu, ", nbf->b_blocknr);
			printk(KERN_ALERT "use count: %d\n ", nbf->b_count.counter);
			nbf = nbf-> b_this_page;
                while(bf != nbf){
			printk(KERN_ALERT "Storage device the block is in: %s, ", nbf->b_bdev->bd_disk->disk_name);
			printk(KERN_ALERT "the block number: %lu, ", nbf->b_blocknr);
			printk(KERN_ALERT "use count: %d\n ", nbf->b_count.counter);
			nbf = nbf-> b_this_page;     
	        }
	    }
	}
    }
  }
  printk(KERN_CONT "\n");
  printk(KERN_CONT "pwd: %s\n",task->fs->pwd.dentry->d_name.name);

  spin_unlock(&(task->files->file_lock));
  return 0;
}

static void __exit cleanup_module_1(void)
{
        printk(KERN_INFO "Goodbye, world 4\n");
}
module_init(init_module_1);
module_exit(cleanup_module_1);
/*
 *  You can use strings, like this:
 */
/*
 * Get rid of taint message by declaring code as GPL.
 */
MODULE_LICENSE("GPL");
/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);        /* What does this module do */
