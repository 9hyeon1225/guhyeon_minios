#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/ptrace.h>
#include "task.h"
#include <linux/workqueue.h>
static unsigned long flags;
#define PIT_CH0_DATA 0x40
#define PIT_COMMAND  0x43

#define PIT_FREQ 1193182
#define TARGET_HZ 100
#define TIMER_INTERVAL_MS 500
#define PIT_DIVISOR (PIT_FREQ / (1000 / TIMER_INTERVAL_MS))

static struct work_struct my_work;


taskQueue *q;
    
    
#define SIZE 10


static context_t* thread1_ctx;
static context_t* thread2_ctx;
static uint8_t* stack1;
static uint8_t* stack2;

static struct timer_list switch_timer[2];
static int current_thread = 1;


void my_work_handler(struct work_struct *work){
	printk(KERN_ERR "work queue\n");
	if(sched(q)==1){
	SWITCH_CONTEXT(&(old_task->context), &(next_task->context));}
}

static void __exit mymodule_exit(void);

void load_context(context_t* ctx) {
    if (!ctx) {
        printk(KERN_ERR "Invalid context pointer\n");
        return;
    }

    __asm__ volatile (
        "movq 0x88(%0), %%rax\n\t"
        "pushq %%rax\n\t"
        "popfq\n\t"
        "movq 0x80(%0), %%rsp\n\t"
        "movq 0x78(%0), %%rax\n\t"
        "pushq %%rax\n\t"
        "ret \n\t"
        : : "r" (ctx) : "memory", "rax"
    );
}



void thread1(void) {
    int count = 0;
    while (count < 40) {

        printk(KERN_INFO "Thread 1 is running (%d)\n", count + 1);
    mdelay(500);

         count++;
    }
    dequeue(q, 1);
    while(1){        
    mdelay(500);

    printk(KERN_INFO "Thread 1 is sleep\n" );
    }
}

void thread2(void) {

    int count = 0;
    while (count < 20) {
        printk(KERN_INFO "Thread 2 is running (%d)\n", count + 1);
    udelay(10000);
    udelay(10000);
        count++;


    }
    dequeue(q, 2);
    //switch_context(thread2_ctx, thread1_ctx);
    while(1){            
    udelay(10000);
    udelay(10000);
    printk(KERN_INFO "Thread 2 is sleep\n");}
}
static bool init;

        uint64_t *stack_ptr;
void timer_callback(struct timer_list *t) {


	
	printk(KERN_INFO "interrupt\n");

        mod_timer(&switch_timer[0], jiffies + msecs_to_jiffies(500));   
             
	if(sched(q)==1){
	SWITCH_CONTEXT(&(old_task->context), &(next_task->context));
	}


}



static int __init mymodule_init(void) {
    printk(KERN_INFO "Module initialized\n");
    
    q = kmalloc(sizeof(taskQueue), GFP_KERNEL);
    init_taskQueue(q);

    task* t1 = (task*)kmalloc(sizeof(task), GFP_KERNEL);
    t1->PID = 1;
    task* t2 = (task*)kmalloc(sizeof(task), GFP_KERNEL);
    t2->PID = 2;

    stack1 = kmalloc(4096, GFP_KERNEL);
    stack2 = kmalloc(4096, GFP_KERNEL);
    thread1_ctx = kmalloc(sizeof(context_t), GFP_KERNEL);
    thread2_ctx = kmalloc(sizeof(context_t), GFP_KERNEL);

    if (!stack1 || !stack2 || !thread1_ctx || !thread2_ctx) {
        printk(KERN_ERR "Failed to allocate stacks or contexts\n");
        kfree(stack1);
        kfree(stack2);
        kfree(thread1_ctx);
        kfree(thread2_ctx);
        return -ENOMEM;
    }

    memset(stack1, 0, 4096);
    memset(stack2, 0, 4096);
    memset(thread1_ctx, 0, sizeof(context_t));
    memset(thread2_ctx, 0, sizeof(context_t));

    thread1_ctx->rsp = (uint64_t)(stack1 + 4096);
    thread1_ctx->rip = (uint64_t)thread1;
    thread1_ctx->rflags = 0x00000200;

    thread2_ctx->rsp = (uint64_t)(stack2 + 4096);
    thread2_ctx->rip = (uint64_t)thread2;
    thread2_ctx->rflags = 0x00000200;

    t1->context=*thread1_ctx;
    t2->context=*thread2_ctx;
    
    printk(KERN_INFO "Initial context for thread1: RIP=%px, RSP=%px\n", (void*)thread1_ctx->rip, (void*)thread1_ctx->rsp);
    printk(KERN_INFO "Initial context for thread2: RIP=%px, RSP=%px\n", (void*)thread2_ctx->rip, (void*)thread2_ctx->rsp);
    printk(KERN_INFO "Allocated stack1: %px (aligned: %px)\n", stack1, (void*)(stack1 + 4096));
    printk(KERN_INFO "Allocated stack2: %px (aligned: %px)\n", stack2, (void*)(stack2 + 4096));

    if (!thread1_ctx->rip || !thread1_ctx->rsp || !thread2_ctx->rip || !thread2_ctx->rsp) {
        printk(KERN_ERR "Invalid initial context\n");
        kfree(stack1);
        kfree(stack2);
        kfree(thread1_ctx);
        kfree(thread2_ctx);
        return -EINVAL;
    }
    enqueue(q, t1);
    enqueue(q, t2);

    init=false;	
    


   INIT_WORK(&my_work,my_work_handler);
   
   timer_setup(&switch_timer[0], timer_callback, 0);
   mod_timer(&switch_timer[0], jiffies + msecs_to_jiffies(1000));

/*while(1){
    mdelay(500);
    printk(KERN_INFO "Thread 2 is sleep\n");
}*/


    //load_context(thread1_ctx);

    return 0;
}

static void __exit mymodule_exit(void) {
    del_timer(&switch_timer[0]);
    kfree(stack1);
    kfree(stack2);
    kfree(thread1_ctx);
    kfree(thread2_ctx);
    printk(KERN_INFO "Module exited\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Context Switching Kernel Module with Timer Interrupt");
MODULE_AUTHOR("Your Name");
