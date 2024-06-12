#include "task.h"
#include <linux/slab.h>
#include <linux/kernel.h>


task* next_task=NULL;
task* old_task=NULL;
task* current_task = NULL;
int sched_counter = 0;

void init_taskQueue(taskQueue* q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->current_t = NULL;
}

uint8_t isEmpty(taskQueue* q) {
    return q->head == NULL;
}

void enqueue(taskQueue* q, task* new_task) {
    new_task->next = NULL;
    if (isEmpty(q)) {
        q->head = new_task;
        q->tail = new_task;
        q->current_t = new_task;
    } else {
        q->tail->next = new_task;
        q->tail = new_task;
    }
    //current_task=q->tail;
    q->count++;
}

task* get_next_task(taskQueue* q) {

    if (isEmpty(q)) {
    	printk(KERN_INFO "Empty\n");
        return NULL;
    }
    if (q->current_t == NULL) {
        q->current_t = q->head;
    } else {
        q->current_t = q->current_t->next;
        if (q->current_t == NULL) {
            q->current_t = q->head;
        }
    }
    return q->current_t;
}



int sched(taskQueue* q) {
	asm volatile (        "sti\n\t");
	printk(KERN_INFO "sched\n");
    sched_counter++;
    if (sched_counter >= 1) {
        sched_counter = 0;
        next_task = get_next_task(q);
        if (next_task != NULL) {
            if (current_task != NULL) {
                old_task = current_task;
                current_task = next_task;
		return 1;
            } else {
                current_task = next_task;
                return 0;
            }
        }
    }
    return 0;
}

void dequeue(taskQueue* q, uint8_t PID) {
    if (isEmpty(q)) {
        return;
    }
    task* current_t = q->head;
    task* previous = NULL;

    while (current_t != NULL) {
        if (current_t->PID == PID) {
            if (previous == NULL) {
                q->head = current_t->next;
            } else {
                previous->next = current_t->next;
            }
            if (current_t == q->tail) {
                q->tail = previous;
            }
            if (current_t == q->current_t) {
                q->current_t = q->head;
            }
            kfree(current_t);
            q->count--;
            return;
        }
        previous = current_t;
        current_t = current_t->next;
    }
}


void print_context(context_t *context) {
    pr_info("Context: r15=%px, r14=%px, r13=%px, r12=%px, r11=%px, r10=%px, r9=%px, r8=%px\n",
            context->r15, context->r14, context->r13, context->r12, context->r11, context->r10, context->r9, context->r8);
    pr_info("        rdi=%px, rsi=%px, rbp=%px, rbx=%px, rdx=%px, rcx=%px, rax=%px\n",
            context->rdi, context->rsi, context->rbp, context->rbx, context->rdx, context->rcx, context->rax);
    pr_info("        rip=%px, rsp=%px, rflags=%px\n",
            context->rip, context->rsp, context->rflags);
}

/*
void switch_context(context_t* old_ctx, context_t* new_ctx) {
    if (!old_ctx || !new_ctx) {
        printk(KERN_ERR "Invalid context pointer(s)\n");
        return;
    }
    print_context(old_ctx);
    print_context(new_ctx);
	printk(KERN_ERR "switch\n");
    __asm__ volatile (
    	"cli\n\t"
        "movq %%rsp, 0x80(%0)\n\t"
        "pushfq\n\t"
        "popq %%rax\n\t"
        "movq %%rax, 0x88(%0)\n\t"
        "movq %%rbx, 0x58(%0)\n\t"
        "movq %%rcx, 0x68(%0)\n\t"
        "movq %%rdx, 0x60(%0)\n\t"
        "movq %%rsi, 0x48(%0)\n\t"
        "movq %%rdi, 0x40(%0)\n\t"
        "movq %%rbp, 0x50(%0)\n\t"
        "movq %%r8, 0x38(%0)\n\t"
        "movq %%r9, 0x30(%0)\n\t"
        "movq %%r10, 0x28(%0)\n\t"
        "movq %%r11, 0x20(%0)\n\t"
        "movq %%r12, 0x18(%0)\n\t"
        "movq %%r13, 0x10(%0)\n\t"
        "movq %%r14, 0x08(%0)\n\t"
        "movq %%r15, 0x00(%0)\n\t"
        "leaq 1f(%%rip), %%rax\n\t"
        "movq %%rax, 0x78(%0)\n\t"
        "movq %1, %%rax\n\t"
        "movq 0x88(%%rax), %%rbx\n\t"
        "pushq %%rbx\n\t"
        "popfq\n\t"
        "movq 0x80(%%rax), %%rsp\n\t"
        "movq 0x58(%%rax), %%rbx\n\t"
        "movq 0x68(%%rax), %%rcx\n\t"
        "movq 0x60(%%rax), %%rdx\n\t"
        "movq 0x48(%%rax), %%rsi\n\t"
        "movq 0x40(%%rax), %%rdi\n\t"
        "movq 0x50(%%rax), %%rbp\n\t"
        "movq 0x38(%%rax), %%r8\n\t"
        "movq 0x30(%%rax), %%r9\n\t"
        "movq 0x28(%%rax), %%r10\n\t"
        "movq 0x20(%%rax), %%r11\n\t"
        "movq 0x18(%%rax), %%r12\n\t"
        "movq 0x10(%%rax), %%r13\n\t"
        "movq 0x08(%%rax), %%r14\n\t"
        "movq 0x00(%%rax), %%r15\n\t"
         "sti\n\t"
    	"movq 0x78(%%rax), %%rbx\n\t" // Return address
    	"movq %%rbx, 0x8(%%rsp)\n\t" // Set return address on stack

        "1:\n\t"
        : : "r" (old_ctx), "r" (new_ctx) : "memory", "rax"
    );
    	printk(KERN_ERR "swich end\n");
}*/


void print_taskQueue(taskQueue *q) {
    task *task_current = q->head;
    pr_info("Task Queue Contents:\n");
    while (task_current != NULL) {
        pr_info("Task PID: %d\n", task_current->PID);
        print_context(&task_current->context);
        task_current = task_current->next;
    }
}

