#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

static 
char hello_name[128] = { 0x0, } ;

static // NOT YET DEFINED 
int hello_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static // NOT YET DEFINED
int hello_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t hello_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[256] ;
	ssize_t toread ;

	sprintf(buf, "Hello %s from kernel!\n", hello_name) ;
	//	"Hello from kernel!"

		if (strlen(buf) >= *offset + size) {
			toread = size ;
		}
		else {
			toread = strlen(buf) - *offset ;
		}

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;

	return toread ;
}

	static 
ssize_t hello_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

	sscanf(buf,"%128s", hello_name) ;
	*offset = strlen(buf) ;

	return *offset ;
}

static const struct file_operations hello_fops = {
	.owner = 	THIS_MODULE,
	.open = 	hello_open, // We didn't define it in this example.
	.read = 	hello_read, 
	.write = 	hello_write,
	.llseek = 	seq_lseek, // also didn't define it
	.release = 	hello_release, // also didn't define it
} ;

static 
int __init hello_init(void) { // for using module_init
	// proc_create function will be invoked and it will make a virtual file.
	proc_create("hellokernelworld", S_IRUGO | S_IWUGO, NULL, &hello_fops) ;
	return 0;
}

static 
void __exit hello_exit(void) { // for using module_exit
	remove_proc_entry("hellokernelworld", NULL) ; // remove_proc_entry will be invoked when we deload this hellokernelworld LKM.
}

module_init(hello_init);
module_exit(hello_exit);
