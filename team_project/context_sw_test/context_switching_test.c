#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t rip, rsp;
    uint64_t rflags;
} __attribute__((packed)) context_t; //x86 64 ��Ű������ cpu �������͵��� ������ �����ϱ����� ����ü(���߿� ������ task ����ü�� �Ϻ�)

static context_t* thread1_ctx; //���ؽ�Ʈ ����Ī�� �����ϱ����� thread1
static context_t* thread2_ctx;
static uint8_t* stack1; //thread1�� �Ҵ�� stack ����(sp)
static uint8_t* stack2;

void load_context(context_t* ctx) {//�� ó�� thraed1�� �����ϱ����� �Լ�
    if (!ctx) {
        printk(KERN_ERR "Invalid context pointer\n");
        return;
    }

    printk(KERN_INFO "Loading context: \n");
    __asm__ volatile (
        "movq 0x88(%0), %%rax\n\t"
        "pushq %%rax\n\t"
        "popfq\n\t"
        "movq 0x80(%0), %%rsp\n\t"
        /* "movq 0x70(%0), %%rax\n\t"
         "movq 0x68(%0), %%rcx\n\t"
         "movq 0x60(%0), %%rdx\n\t"
         "movq 0x58(%0), %%rbx\n\t"
         "movq 0x50(%0), %%rbp\n\t"
         "movq 0x48(%0), %%rsi\n\t"
         "movq 0x40(%0), %%rdi\n\t"
         "movq 0x38(%0), %%r8\n\t"
         "movq 0x30(%0), %%r9\n\t"
         "movq 0x28(%0), %%r10\n\t"
         "movq 0x20(%0), %%r11\n\t"
         "movq 0x18(%0), %%r12\n\t"
         "movq 0x10(%0), %%r13\n\t"
         "movq 0x08(%0), %%r14\n\t"
         "movq 0x00(%0), %%r15\n\t"     */
        "movq 0x78(%0), %%rax\n\t"
        "jmp *%%rax\n\t"
        : : "r" (ctx) : "memory", "rax"
        );
}

void switch_context(context_t* old_ctx, context_t* new_ctx) {//���ؽ�Ʈ ����Ī�� �� �Լ�
    printk(KERN_INFO "Switching context from %px to %px\n", old_ctx, new_ctx);

    __asm__ volatile (
        "movq %%rsp, 0x80(%0)\n\t"     // Save the stack pointer to old_ctx->rsp
        "pushfq\n\t"                   // Save flags register to the stack
        "popq %%rax\n\t"
        "movq %%rax, 0x88(%0)\n\t"     // Save flags to old_ctx->rflags
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
        "leaq 1f(%%rip), %%rax\n\t"    // Load address of next instruction into rax
        "movq %%rax, 0x78(%0)\n\t"     // Save it to old_ctx->rip
        "movq %1, %%rax\n\t"           // Load new_ctx address into rax


        "movq 0x88(%%rax), %%rbx\n\t"  // Move flags from new_ctx->rflags to rbx temporarily
        "pushq %%rbx\n\t"              // Push rbx (flags) to the stack
        "popfq\n\t"
        "movq 0x80(%%rax), %%rsp\n\t"     // Restore stack pointer from new_ctx->rsp
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
        "jmp *0x78(%%rax)\n\t"         // Jump to new_ctx->rip
        "1:\n\t"
        : : "r" (old_ctx), "r" (new_ctx) : "memory", "rax"
        );
}


void thread1(void) {//���ؽ�Ʈ ����Ī�� �� thraed1
    int count = 0;
    while (count < 20) {
        printk(KERN_INFO "Thread 1 is running (%d)\n", count + 1);
        msleep(100);  // 0.1�� ������
        switch_context(thread1_ctx, thread2_ctx);//thread2�� context switching
        count++;
    }
}

void thread2(void) {
    int count = 0;
    while (count < 20) {
        printk(KERN_INFO "Thread 2 is running (%d)\n", count + 1);
        msleep(100);  // 0.1�� ������
        switch_context(thread2_ctx, thread1_ctx);//thread1�� context switching
        count++;
    }
}

static int __init mymodule_init(void) {//������ Ŀ�� ����� �ʱ�ȭ Ŀ�� �����main�Լ���� �� �� ����
    printk(KERN_INFO "Module initialized\n");

    stack1 = kmalloc(4096, GFP_KERNEL);//stack1�� �޸� �Ҵ�
    stack2 = kmalloc(4096, GFP_KERNEL);
    thread1_ctx = kmalloc(sizeof(context_t), GFP_KERNEL);//thread1�� task ����ü�� �޸� �Ҵ�
    thread2_ctx = kmalloc(sizeof(context_t), GFP_KERNEL);

    if (!stack1 || !stack2 || !thread1_ctx || !thread2_ctx) {//�Ҵ� �����ϸ� ����
        printk(KERN_ERR "Failed to allocate stacks or contexts\n");
        kfree(stack1);
        kfree(stack2);
        kfree(thread1_ctx);
        kfree(thread2_ctx);
        return -ENOMEM;
    }

    // ���� �޸� �ʱ�ȭ
    memset(stack1, 0, 4096);
    memset(stack2, 0, 4096);
    memset(thread1_ctx, 0, sizeof(context_t));
    memset(thread2_ctx, 0, sizeof(context_t));//�� �޸𸮰��� 0���� �ʱ�ȭ

    thread1_ctx->rsp = (uint64_t)(stack1 + 4096);
    thread1_ctx->rip = (uint64_t)thread1;
    thread1_ctx->rflags = 0x0000000000000202;//thread1�� �ʱ�ȭ, stack�� �������ּ� ���� �� ��ɾ��ּ�, �ʱ� flag��

    thread2_ctx->rsp = (uint64_t)(stack2 + 4096);
    thread2_ctx->rip = (uint64_t)thread2;
    thread2_ctx->rflags = 0x0000000000000202;//thread2�� �ʱ�ȭ, stack�� �������ּ� ���� �� ��ɾ��ּ� , �ʱ� flag��


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

    load_context(thread1_ctx);//thread1 ����

    return 0;
}

static void __exit mymodule_exit(void) {
    kfree(stack1);
    kfree(stack2);
    kfree(thread1_ctx);
    kfree(thread2_ctx);
    printk(KERN_INFO "Module exited\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Context Switching Kernel Module");
MODULE_AUTHOR("Your Name");