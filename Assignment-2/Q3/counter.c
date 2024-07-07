#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Q3");
MODULE_DESCRIPTION("Custom System Call Module");

static int count(void) {
    struct task_struct *task;
    int c = 0;
    for_each_process(task){
        if (task->__state == TASK_RUNNING){
            c++;
        }
    }   
    return c;
}
static int __init custom_syscall_init(void){
    printk(KERN_INFO "Count Running processes module loaded\n"); 
    int c = count();
    printk(KERN_INFO "Number of running processes: %d\n", c);
    return 0;
}
static void __exit custom_syscall_exit(void){
    printk(KERN_INFO "Custom syscall module unloaded\n");
}
module_init(custom_syscall_init);
module_exit(custom_syscall_exit);
