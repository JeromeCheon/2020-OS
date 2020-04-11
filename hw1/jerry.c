/* This is jerry.c file which roles to intermediate between mousehole and user to communicate in some way. 
 *
 * jerry has to get a specific user name and fname from user (using pid functions maybe) then report it to mousehole to prevent.
 * Also, through this jerry.c, we have to contact with /proc/mousehole in user level.
 * We need to use the proper API what we've learned to implement jerry.c
 *
 * Key Features :
 * 1. Block Files Opening of User
 *	- (Hint) Replace handler routine for sys_open()
 *	- (Hint) Get a user ID using linux/cred.h and translate the string uname to integer
 * 2. Prevent Killing of Processes
 *	- a user will pass a username to jerry(get fname using pointer?)
 *	- 
 *
 * */

#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cred.h>

MODULE LICENSE("GPL") ;

char filepath[128] = {0x0, } ;
void ** sctable ;
int count = 0;

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode){
	char fname[256] ; // do I have to initialise with a specific name?

	copy_from_user(fname, filename, 256) ;

	if(filepath[0] != 0x0 && strcmp(filepath, fname) == 0) {
		count++; // I have to think of if it's necessary with this program
	}
	return orig_sys_open(filename, flags, mode);
}
// WE don't need this opening handler
static int jerry_proc_open(struct inode *inode, struct file *file){
	return 0;
}

static int jerry_proc_release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t jerry_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset){
	char buf[256];
	ssize_t toread ;

	sprintf(buf,"%s: %d\n", filepath, count);

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;
	
	*offset = *offset + toread ;

	return toread ;
}

static ssize_t jerry_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset){

	char buf[128];

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

	sscanf(buf, "%s", filepath) ;

	count = 0;
	*offset = strlen(buf) ;

	return *offset ;
}

static const struct file_operations jerry_fops = {
	.owner = THIS_MODULE,
	.open = jerry_proc_open,
	.read = jerry_proc_read,
	.write = jerry_proc_write, 
	.llseek = seq_lseek,
	.release = jerry_proc_release,
};

static int __init jerry_init(void){
	unsigned int level ;
	pte_t * pte ;

	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &jerry_fops);

	sctable = (void* ) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	if( pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW ;

	sctable[__NR_open] = jerry_sys_open ;

	return 0;

}

static void __exit jerry_exit(void){
	unsigned int level ;
	pte_t * pte ;

	remove_proc_entry("jerry", NULL) ;

	sctable[__NR_open] = orig_sys_open ;

	pte = lookup_address((unsigned long) sctable, &level);
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(jerry_init);
module_exit(jerry_exit);