/* This is the mousehole.c file which roles to prevent any signals of opening some specific files from specific user who was given by 'jerry.c'
 *
 * Key Features : 
 * 1. Block Files Opening of User

 * 2. Prevent Killing of Precesses

*/

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

MODULE LICENSE("GPL") ;
static char user_name[128] = {0x0, } ;
    // This pointer might be necessary later, so let's move on
char * text = 0x0 ;
void ** sctable ; // 이건 시스템 콜 테이블 더블 포인터
// At this point, I have to replace my own function instead of original sys_open function. 

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ;
asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);

// 사용자 재정의 된 sys_open 기능. 첫번째 기능, 유저의 오프닝을 차단하기 위해.
asmlinkage int mousehole_sys_open(const char __user * filename/* jerry로 부터 전달 받은 fname */, int flags, umode_t mode){
    // 일단 이 부분 놔두고
    return orig_sys_open(filename, flags, 077/*여기에 소유자 권한 제거*/) ;
}
// sys_kill 재정의 for function 2. prevent killing of processes
asmlinkage long mousehole_sys_kill(pid_t pid, int sig){
    // 여기에서 process kill 명령을 막아야 함. for_each_process 매크로 써서.
    char buf[256] ;
    
    struct task_struct * t ;
    int idx = 0;
    
    text = kmalloc(2048, GFP_KERNEL);
    text[0] = 0x0 ;
    
    for_each_process(t){
        sprintf(buf, " %d \n", t->pid) ;
        
        printk(KERN_INFO, "%s", buf);
        
        idx += strlen(buf) ;
        if (idx < 2048)
            strcat(text, buf) ;
        else
            break;
    }
    
    return orig_sys_kill(pid, 14 /*9는 절대 안되고, 대신 SIGALRM의 14를 넣어줌*/ );
    
}

static int hole_open(struct inode *inode, struct file *file){
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

	sprintf(buf,"Hello. the mousehole is ready \n") ;

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, text + *offset, toread))
		return -EFAULT ;

	*offset += toread ;

	return toread ;
}
// Writing is unnecessary here 
static ssize_t hole_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset){
    
    char buf[128] ;
    
    if (*offset != 0 || size >128) {
        return -EFAULT ;
    }
    if (copy_from_user(buf, ubuf, size)) {
        return -EFAULT ;
    }
	return 0;
}

static const struct file_operations hole_fops = {
	.owner =	THIS_MODULE,
	.open =		hole_open,
	.read =		hole_read,
	.write =	hole_write,
	.seek =		seq_lseek,
	.release =	hole_release,
    .kill =     mousehole_sys_kill,
	// And I must modify here as well and redefine
	// Maybe I have to replace sys_kill() here referring syscalls.h
};
// This part will make the path, /proc/mousehole
static int __init hole_init(void){
    unsigned int level ;
    pte_t * pte ;
    // to intercept the opening a specific file
    
    proc_create("mousehole", S_IRUGO |S_IWUGO, NULL, &hole_fops) ;
    
    sctable = (void *)kallsyms_lookup_name("sys_call_table") ;
    orig_sys_open = sctable[__NR_open/*찾아서 수정하기*/] ;
    orig_sys_kill = sctable[__NR_kill] ;
    // 이거 필요한 지는 추후 체크! -> 이따 exit 부분에서 해주네
    pte = lookup_address((unsigned long) sctable, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW ;
    
    sctable[__NR_open] = mousehole_sys_open ;
    sctable[__NR_kill] = mousehole_sys_kill ;
	printk("This is for checking if the mousehole LKM was successful to be in the kernel. \n");

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
	
