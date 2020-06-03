#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
//#include <linux/cred.h>
MODULE_LICENSE("GPL");
// this filepath was given from user application system through 'proc file system'
char filepath[128] = { 0x0, } ;
void ** sctable ;
int count = 0 ; //  this is a global variable. 
// *orig_sys_open is a function pointer. 
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage int openhook_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ; // kernel data in kernel space. Once get some file name from user space, it delievers into fname.
    
	copy_from_user(fname, filename, 256) ;
								// this filepath is the array like above
	if (filepath[0] != 0x0 && strcmp(filepath, fname) == 0) {
		// if the target file is specified with proc file sys, plus count.
		// and it will be retrieved at sprintf function below.
		count++ ; 
	}
	return orig_sys_open(filename, flags, mode) ;
}


static // we didn't define here, open feature. 
int openhook_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static // also we didn't define here, release part 
int openhook_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

	static
ssize_t openhook_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[256] ;
	ssize_t toread ;

	sprintf(buf, "%s:%d\n", filepath, count) ;

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;

	return toread ;
}

	static 
ssize_t openhook_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

	sscanf(buf,"%s", filepath) ;
	count = 0 ;
	*offset = strlen(buf) ;

	return *offset ;
}

static const struct file_operations openhook_fops = {
	.owner = 	THIS_MODULE,
	.open = 	openhook_proc_open,
	.read = 	openhook_proc_read,
	.write = 	openhook_proc_write,
	.llseek = 	seq_lseek,
	.release = 	openhook_proc_release,
} ;

static 
int __init openhook_init(void) { // openhook init part
	unsigned int level ; 
	pte_t * pte ;
	
	// this proc_create is a kind of an agent. Remember, I've already dealt with this 'agent concept' in Hellokernelworld example. 
	proc_create("openhook", S_IRUGO | S_IWUGO, NULL, &openhook_fops) ;
	
	// The pointer of pointer sctable is used here to bring up the address of sys call handler table.  
	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;	

	// we will check what type is delivered to system call 
	sctable[__NR_open] = openhook_sys_open ;

	return 0;
}

static 
void __exit openhook_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	// A procedure of deloading the LKM 'openhook' from the kernel
	remove_proc_entry("openhook", NULL) ;

	sctable[__NR_open] = orig_sys_open ; // yes. after deallocate the module, we will put the original open system call back into __NR_open
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(openhook_init);
module_exit(openhook_exit);
