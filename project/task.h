#ifndef TASK_H
#define TASK_H

#include <linux/types.h>
                //

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t rip, rsp;
    uint64_t rflags;
} __attribute__((packed)) context_t;

typedef struct task {
    uint8_t PID;
    context_t context;
    struct task* next;
} task;

typedef struct {
    task* head;
    task* tail;
    uint8_t count;
    task* current_t;
} taskQueue;

extern task* next_task;
extern task* old_task;

extern task* current_task;


extern int sched_counter;

void init_taskQueue(taskQueue* q);
uint8_t isEmpty(taskQueue* q);
void enqueue(taskQueue* q, task* new_task);
void dequeue(taskQueue* q, uint8_t PID);
task* get_next_task(taskQueue* q);

//void context_switch(task* current, task* next);
int sched(taskQueue* q);

//void switch_context(context_t* old_ctx, context_t* new_ctx);
void print_taskQueue(taskQueue* q);
#endif // TASK_H
            //return 0; \
            
#define SWITCH_CONTEXT(old_ctx, new_ctx) \
    do { \
        if (!(old_ctx) || !(new_ctx)) { \
            printk(KERN_ERR "Invalid context pointer(s)\n"); \
            return;\
        } \
        __asm__ volatile ( \
            "movq %%rsp, 0x80(%0)\n\t" \
            "pushfq\n\t" \
            "popq %%rax\n\t" \
            "movq %%rax, 0x88(%0)\n\t" \
            "movq %%rbx, 0x58(%0)\n\t" \
            "movq %%rcx, 0x68(%0)\n\t" \
            "movq %%rdx, 0x60(%0)\n\t" \
            "movq %%rsi, 0x48(%0)\n\t" \
            "movq %%rdi, 0x40(%0)\n\t" \
            "movq %%rbp, 0x50(%0)\n\t" \
            "movq %%r8, 0x38(%0)\n\t" \
            "movq %%r9, 0x30(%0)\n\t" \
            "movq %%r10, 0x28(%0)\n\t" \
            "movq %%r11, 0x20(%0)\n\t" \
            "movq %%r12, 0x18(%0)\n\t" \
            "movq %%r13, 0x10(%0)\n\t" \
            "movq %%r14, 0x08(%0)\n\t" \
            "movq %%r15, 0x00(%0)\n\t" \
            "leaq 1f(%%rip), %%rax\n\t" \
            "movq %%rax, 0x78(%0)\n\t" \
            "movq %1, %%rax\n\t" \
            "movq 0x88(%%rax), %%rbx\n\t" \
            "pushq %%rbx\n\t" \
            "popfq\n\t" \
            "movq 0x58(%%rax), %%rbx\n\t" \
            "movq 0x68(%%rax), %%rcx\n\t" \
            "movq 0x60(%%rax), %%rdx\n\t" \
            "movq 0x48(%%rax), %%rsi\n\t" \
            "movq 0x40(%%rax), %%rdi\n\t" \
            "movq 0x50(%%rax), %%rbp\n\t" \
            "movq 0x38(%%rax), %%r8\n\t" \
            "movq 0x30(%%rax), %%r9\n\t" \
            "movq 0x28(%%rax), %%r10\n\t" \
            "movq 0x20(%%rax), %%r11\n\t" \
            "movq 0x18(%%rax), %%r12\n\t" \
            "movq 0x10(%%rax), %%r13\n\t" \
            "movq 0x08(%%rax), %%r14\n\t" \
            "movq 0x00(%%rax), %%r15\n\t" \
	    "movq 0x78(%%rax), %%rax\n\t" \
	    "movq %%rax, 0x10(%%rsp)\n\t" \
            "1:\n\t" \
            : : "r" (old_ctx), "r" (new_ctx) : "memory", "rax" \
        ); \
    } while (0)
    
            //"movq 0x80(%%rax), %%rsp\n\t" \
	    //"jmp *0x78(%%rax)\n\t"\
	    "movq 0x78(%%rax), %%rax\n\t" \
	    "movq %%rax, 0x8(%%rsp)\n\t" \
