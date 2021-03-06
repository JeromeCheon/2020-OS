#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

// A main point is to make this text pointer which is in the heap memory under the kernel.
char * text = 0x0 ;

static 
int listps_open(struct inode *inode, struct file *file) {
	char buf[256] ;

	struct task_struct * t ;
	int idx = 0 ;

	text = kmalloc(2048, GFP_KERNEL) ;
	text[0] = 0x0 ;

	for_each_process(t) {
		sprintf(buf, "%s : %d\n", t->comm, t->pid) ;

		printk(KERN_INFO "%s", buf) ;

		idx += strlen(buf) ;
		if (idx < 2048) 
			strcat(text, buf) ;
		else
			break ;
	}
	return 0 ;
}

static 
int listps_release(struct inode *inode, struct file *file) {
	return 0 ;
}

// read is a role to deliver some context from the user level.
	static
ssize_t listps_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{ // and 'ubuf' will be less than the size.
	ssize_t toread ;
	if (strlen(text) >= *offset + size) {
		toread = size ;
	}
	else {
		toread = strlen(text) - *offset ;
	}
// Here it's necessary to read the information from text + *offset by copying the kernel data into ubuf
	if (copy_to_user(ubuf, text + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;

	return toread ;
}

// Actually there were som lines in write function, but professor had deleted it because writing do basically nothing
	static 
ssize_t listps_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	return 0 ;
}

static const struct file_operations listps_fops = {
	.owner = 	THIS_MODULE,
	.open = 	listps_open,
	.read = 	listps_read,
	.write = 	listps_write,
	.llseek = 	seq_lseek,
	.release = 	listps_release,
} ;

static 
int __init listps_init(void) {
	proc_create("listprocesses", S_IRUGO | S_IWUGO, NULL, &listps_fops) ;
	return 0;
}

static 
void __exit listps_exit(void) {
	remove_proc_entry("listprocesses", NULL) ;
}

module_init(listps_init);
module_exit(listps_exit);
