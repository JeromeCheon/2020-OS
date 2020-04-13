/* This is the mousehole.c file which roles to prevent any signals of opening some specific files from specific user who was given by 'jerry.c'
 *
 * Key Features : 
 * 1. Block Files Opening of User 
 *	- 
 * 2. Prevent Killing of Precesses
 *	- mousehole makes no other processes kill a specific process created by the user, until the user commands to release it
 *  - (Hint) using 'signaling', mousehole can make it prevent kill the process directly. and to do that, we have to redefine 'sys_kill'
 *  - (Hint) using for_each_process macro, a process will be replaced as a task_struct object instead in kernel.
*/


// based on listprocesses.c source
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>


MODULE LICENSE("GPL") ;
// This pointer might be necessary later, so let's move on
char * text = 0x0 ;
// At this point, I have to replace my own function instead of original sys_open function. 
static int hole_open(struct inode *inode, struct file *file){
	char buf[256] ;

	struct tast_struct * t;
	int idx = 0;

	text = kmalloc(2048, GFP_KERNEL) ;
	text[0] = 0x0 ; 

	for_each_process(t) {
		// At this point, I must modify as 'pid'
		sprintf(buf, "%s : %d\n", t->comm, t->pid) ; 

		printk(KERN_INFO "%s", buf) ;

		idx += strlen(buf) ;
		if (idx < 2048)
			strcat(text, buf) ;
		else
			break ;
	}
	return 0;

}
// I think I might have to modify here as well
static int hole_release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t hole_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset){
	// ubuf will be less than the size
	ssize_t toread ;
	if(strlen(text) >= *offset + size) {
		toread = size ;
	}
	else {
		toread = strlen(text) - *offset ;
	}

	if (copy_to_user(ubuf, text + *offset, toread))
		return -EFAULT ;

	*offset += toread ;

	return toread ;
}
static ssize_t hole_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset){
	return 0;
}

static const struct file_operations hole_fops = {
	.owner =	THIS_MODULE,
	.open =		hole_open,
	.read =		hole_read,
	.write =	hole_write,
	.seek =		seq_lseek,
	.release =	hole_release,
	// And I must modify here as well and redefine
	// Maybe I have to replace sys_kill() here referring syscalls.h
};

static int __init hole_init(void){
	proc_creat("mousehole", S_IRUGO |S_IWUGO, NULL, &hole_fops) ;
	return 0;
}

static void __exit hole_exit(void){
	remove_proc_entry("mousehole", NULL) ;
}

module_init(hole_init) ;
module_exit(hole_exit) ;
	
/* This is jerry.c file which roles to intermediate between mousehole and user to communicate in some way. 
 *
 * jerry has to get a specific user name and fname from user (using pid functions maybe) then report it to mousehole to prevent.
 * Also, through this jerry.c, we have to contact with /proc/mousehole in user level. (Maybe through using echo ? )
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
// filepath will be given from user application system through proc file sys.
char filepath[128] = {0x0, } ;

// sctable means 'system call table'
void ** sctable ;
int count = 0;

//asmlinkage long sys_kill(pid_t pid, int sig);
// Yeah, I should redefine sys_open() here in this part as like jerry
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode);

asmlinkage int jerry_sys_open(const char __user *filename, int flags, umode_t mode)
{
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

	proc_create("jerry", S_IRUGO | S_IWUGO, NULL, &jerry_fops);
	// To check if jerry is running in proc as LKM
	printk("jerry is running for detecting a specific user. \n") ;
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
	//This part indicates to restore the original state back
	sctable[__NR_open] = orig_sys_open ;

	pte = lookup_address((unsigned long) sctable, &level);
	pte->pte = pte->pte &~ _PAGE_RW ;
	// Do double check later if this is necessary to print out
	printk("jerry has been removed. \n") ;
}

module_init(jerry_init);
module_exit(jerry_exit);
