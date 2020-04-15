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

MODULE_LICENSE("GPL") ;
char filepath[256] = {0x0, } ;
    // This pointer might be necessary later, so let's move on
void ** sctable ; // 이건 시스템 콜 테이블 더블 포인터
// At this point, I have to replace my own function instead of original sys_open function. 

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ;
asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);

// 사용자 재정의 된 sys_open 기능. 첫번째 기능, 유저의 오프닝을 차단하기 위해.
asmlinkage int mousehole_sys_open(const char __user * filename/* jerry로 부터 전달 받은 fname */, int flags, umode_t mode){
    char fname[256] ;
    
	copy_from_user(fname, filename, 256) ;
	if(filepath[0]!= 0x0 && strcmp(filepath, fname) == 0){
//		cred->uid
		
		return -1;
	}
    return orig_sys_open(filename, flags, mode/*여기에 소유자 권한 제거*/) ;
}
// sys_kill 재정의 for function 2. prevent killing of processes
 asmlinkage long mousehole_sys_kill(pid_t pid, int sig){
    // 여기에서 process kill 명령을 막아야 함. for_each_process 매크로 써서.
    
    struct task_struct * t ;
	//char buf[256] ;
    
    
    for_each_process(t){
        //sprintf(buf, "%s : %d \n",t->comm , t->pid) ; // comm(UID)
        if(pid == t->pid && cred->uid == t->uid){ //if uid == tasks_pid, break then put it another signal.
			// sys_kill number change here
			
			printk("process %d couldn't be killed ^^ \n", pid) ;
			return -1;
		}
	}
    return orig_sys_kill(pid, sig );
    
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
	sprintf(buf, "%s\n", filepath);

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;

	*offset += toread ;

	return toread ;
}
// This part will be started as first after initializing module on the kernel 
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
	sscanf(buf, "%s", filepath) ;
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
    orig_sys_open = sctable[__NR_open/*찾아서 수정하기*/] ;
    orig_sys_kill = sctable[__NR_kill] ;
    // 이거 필요한 지는 추후 체크! -> 이따 exit 부분에서 해주네
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
	
