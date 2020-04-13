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

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ;

asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode){
	char fname[256];

	copy_from_user(fname, filename, 256) ;

	if(filepath[0] != 0x0 && strcmp(filepath, fname) == 0){
		count++;
	}
	return orig_sys_open(filename, flags, mode) ;
}
}
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
	char buf[256] ;
	ssize_t toread ;

	sprintf(buf,"%s :%d\n", filepath, count) ;

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, text + *offset, toread))
		return -EFAULT ;

	*offset += toread ;

	return toread ;
}
// Writing is unnecessary here 
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
// This part will make the path, /proc/mousehole
static int __init hole_init(void){
	proc_creat("mousehole", S_IRUGO |S_IWUGO, NULL, &hole_fops) ;
	printk("This is for checking if the mousehole LKM was successful to be in the kernel. \n");

	return 0;
}

static void __exit hole_exit(void){
	remove_proc_entry("mousehole", NULL) ;
	printk("Okay. This module has been removed. Thanks.! \n") ;
}

module_init(hole_init) ;
module_exit(hole_exit) ;
	
