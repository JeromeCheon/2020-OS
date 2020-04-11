#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

static 
int __init baremin_init(void) { // initialising part. Double under bar means this is a kind of macro. 
	printk("baremininum : Hi.\n") ; // It will be remained in kernel space. and then I will be able to get this through 'dmesg' command.
	return 0; 
} 

static 
void __exit baremin_exit(void) { // finalising
	printk("bareminimum : Bye.\n") ;
}

module_init(baremin_init);
module_exit(baremin_exit);
