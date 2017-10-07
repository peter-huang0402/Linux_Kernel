#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/desc.h>

//#if defined(CONFIG_X86_64)
#define DIVIDE_ERROR 0x00
//#else
//#error "This module is only for X86_64 Kernel"
//#endif
 
/* global varaible */
char msg[10240];
int length = 0;
char *str = "test";
struct desc_ptr newidtr, oldidtr,tempidtr;
gate_desc *newidt, *oldidt, *tempidt;
int counter = 0;
unsigned long old_stub = 0xc17120c4;


struct tty_struct *my_tty;

#if 1
int write_console(char *str) {
	struct tty_struct *my_tty;
	if ((my_tty=current->signal->tty) != NULL) {
		((my_tty->driver->ops->write) (my_tty, str, strlen(str)));
		return 0;
	} else return -1;
}
#endif

/* my C handler */
void my_func(void)
{
    counter = counter + 1;
    /* add the counter and send messge to console */
    sprintf(msg + length, "Interrupt 0 is changed by Peter, counter is %d\n", counter);
    length = strlen(msg);
}

 
/* global function */
//extern asmlinkage void new_stub(void);
asmlinkage void my_divide_error(void);
#if 0
asm(".text");
asm(".type my_divide_error,@function");
asm("my_divide_error:");
asm("pushfq");
asm("pushq %rax");
asm("pushq %rcx");
asm("pushq %rdx");
asm("pushq %rbx");
asm("pushq %rsp");
asm("pushq %rbp");
asm("pushq %rsi");
asm("pushq %rdi");
asm("call my_func");
asm("popq %rdi");
asm("popq %rsi");
asm("popq %rbp");
asm("popq %rsp");
asm("popq %rbx");
asm("popq %rdx");
asm("popq %rcx");
asm("popq %rax");
asm("popfq");
asm("jmpq *old_st");
#endif
//asm("data16 %cs:0x0(%rax,%rax,1)");
 
/* active idt_table by loading new idt pointer to the register */
static void load_IDTR(void *addr)
{
    asm volatile("lidt %0"::"m"(*(unsigned short *)addr));
}

#if 1
/* my Assembly handler */
void my_dummy(void)
{
        __asm__ (
	".globl my_divide_error\n\t"
        ".align 4, 0x90\n\t"
        "my_divide_error:\n\t"
	"pushfl\n\t"
	"pushal\n\t"
	"pushl %%esp\n\t"
	"pushl %%ebp\n\t"
#if 0
        "pushfq\n\t"
        "pushq %%rax\n\t"
        "pushq %%rcx\n\t"
        "pushq %%rdx\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rsp\n\t"
        "pushq %%rbp\n\t"
        "pushq %%rsi\n\t"
        "pushq %%rdi\n\t"
#endif
        "call my_func\n\t"
#if 0
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rbp\n\t"
        "popq %%rsp\n\t"
        "popq %%rbx\n\t"
        "popq %%rdx\n\t"
        "popq %%rcx\n\t"
        "popq %%rax\n\t"
        "popfq\n\t"
#endif
	"popl %%ebp\n\t"
	"popl %%esp\n\t"
	"popal\n\t"
	"popfl\n\t"
        "jmp *old_stub\n\t"
         ::);
}

#endif

int __init hook_init(void){
 
   
    printk("Peter -- CS680 homework #8 start\n");
 
    /* initialize tty for console print */
    my_tty = current->signal->tty;
 
    /* create new idt_table copied from old one */
    store_idt(&oldidtr);
	printk("Peter -- old idtr is %p\n", &oldidtr);
	printk("Peter -- old idtr address is %p\n", (void *) oldidtr.address);
    oldidt = (gate_desc *)oldidtr.address;
	printk("Peter -- old idt is %p\n", oldidt);
    newidtr.address = __get_free_page(GFP_KERNEL);
//    newidtr.address = (int) kmalloc(oldidtr.size, GFP_KERNEL);
	printk("Peter -- newidtr.address is %p\n", (void *) newidtr.address);
    if(!newidtr.address)
        return -1;
    newidtr.size = oldidtr.size;
	printk("Peter -- size is %d\n", newidtr.size);
    newidt = (gate_desc *)newidtr.address;
	printk("Peter -- new idt is %p\n", newidt);
    memcpy(newidt, oldidt, oldidtr.size);
 
    /* modify the divide_error entry to point to my assembly handler */
    pack_gate(&newidt[DIVIDE_ERROR], GATE_INTERRUPT, (unsigned long)my_divide_error, 0, 0, __KERNEL_CS);
 
    /* active the new idt_table */
    load_IDTR((void *)&newidtr);
 
    /* for smp architecture */
     smp_call_function(load_IDTR, (void *)&newidtr, 0);
 
    return 0; 
} 

void __exit hook_exit(void){
      
    /* active old idt_table */
    load_IDTR(&oldidtr);
    printk("%s", msg); 
    /* for smp architecture */
     smp_call_function(load_IDTR, (void *)&oldidtr, 0);
 
    /* free the allocated page for new idt_table */
    if(newidtr.address)
        free_page(newidtr.address); 
    printk("Peter -- CS680 homework #8 end\n");
}
  
module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
