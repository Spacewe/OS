//miscdev_map.c
// 通过内存空间映射，实现设备驱动程序与用户程序的通信
// 注意: 杂项设备不需要再显示地使用mknod创建设备节点
// $ make
// $ sudo insmod miscdev_map.ko
// $ sudo chmod a+rw /dev/mymap
//测试使用miscdev_maptest.c程序

#include <linux/miscdevice.h>  
#include <linux/delay.h>  
#include <linux/kernel.h>   
#include <linux/init.h>   
#include <linux/fs.h>  
#include <linux/types.h>  
#include <linux/delay.h>  
#include <linux/moduleparam.h>  
#include <linux/slab.h>  
#include <linux/errno.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/string.h>  
#include <linux/list.h>  
#include <linux/pci.h>  
#include <linux/gpio.h>  
#include <linux/module.h>	// for init_module() 
#include <linux/proc_fs.h>	// for create_proc_read_entry() 
#include <linux/seq_file.h>	// for sequence files
#include <linux/mm.h>		// for 'struct vm_area_struct'
#include <linux/sched.h>	// for 'struct task_struct'
#include <linux/mount.h>
#include <linux/dcache.h>
#include <linux/string.h>
#define DEVICE_NAME "mymap"  
#define DEVICE_INFO "I am the devive mymap, this is my test output"   

static unsigned char *buffer;    

static void * my_seq_start(struct seq_file *m, loff_t *pos)
{
  if (0 == *pos)  
  {  
     ++*pos;  
     return (void *)1; 
   }  

        return NULL;
}
static void * my_seq_next(struct seq_file *m, void *p, loff_t *pos)
{
        // do nothing
        return NULL;
}
static void my_seq_stop(struct seq_file *m, void *p)
{
				//// do nothing        
}
static int my_seq_show(struct seq_file *m, void *p)
{	
	struct task_struct	*tsk = current;
	struct vm_area_struct	*vma;
	unsigned long 		ptdb;
	int i=0;
	//struct mm_struct	*mm = tsk->mm;
	//unsigned long		stack_size = (mm->stack_vm << PAGE_SHIFT);
	//unsigned long		down_to = mm->start_stack - stack_size;	
	seq_printf( m, "List of the Virtual Memory Areas " );
	seq_printf( m, "for task \'%s\' ", tsk->comm );
	seq_printf( m, "(pid=%d)", tsk->pid );
	// loop to traverse the list of the task's vm_area_structs
	vma = tsk->mm->mmap;
	while ( vma )	{		
		char	ch;	
		seq_printf( m, "\n%3d ", ++i );
		seq_printf( m, " vm_start=%08lX ", vma->vm_start );
		seq_printf( m, " vm_end=%08lX  ", vma->vm_end );

		ch = ( vma->vm_flags & VM_READ ) ? 'r' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_WRITE ) ? 'w' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_EXEC ) ? 'x' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_SHARED ) ? 's' : 'p';
		seq_printf( m, "%c", ch );
		//LIST_HEAD(modname);               //自己加的
		//seq_printf( m, " list_head name =%c  ", list_first_entry((vma->anon_vma_chain)->next,str,modname );//自己加的
		vma = vma->vm_next;
	}
	//seq_printf( m, "\n" );

	// display additional information about tsk->mm
	asm(" movl %%cr3, %%ecx \n movl %%ecx, %0 " : "=m" (ptdb) );
	seq_printf( m, "\nCR3=%08lX ", ptdb );
	seq_printf( m, " mm->pgd=%p ", tsk->mm->pgd );
	seq_printf( m, " mm->map_count=%d ", tsk->mm->map_count );
	seq_printf( m, "\n" );
  	return	0;
}
static struct seq_operations my_seq_fops = {
        .start  = my_seq_start,
        .next   = my_seq_next,
        .stop   = my_seq_stop,
        .show   = my_seq_show
};
 
static int my_open(struct inode *inode, struct file *file)  
{  
     return seq_open(file, &my_seq_fops);   //没什么用
}  
  
  
static int my_map(struct file *filp, struct vm_area_struct *vma)  
{     
		//char	ch; 
//struct seq_file *m;
    unsigned long page;  
	//char bu[200]; 
	//char *ddd = "vm_start=";
	//char *jjj = "vm_end=";
    unsigned long start = (unsigned long)vma->vm_start;   
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);  
    vma->vm_flags |= VM_IO;
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP); //VM_RESERVED
  
    //得到物理地址  
    page = virt_to_phys(buffer);      
    //将用户空间的一个vma虚拟内存区映射到以page开始的一段连续物理页面上  
    if(remap_pfn_range(vma,start,page>>PAGE_SHIFT,size,PAGE_SHARED))//第三个参数是页帧号，由物理地址右移PAGE_SHIFT得到  
        return -1;    
    //往该内存写数据  
	//strcpy(bu,DEVICE_INFO);
	//strcat(bu,ddd);
	//strcat(bu,(char*)vma->vm_start);
	//strcat(bu,jjj);
	//strcat(bu,(char*)vma->vm_end);
    sprintf(buffer, "%s,vm_start=%08lX,vm_end=%08lX,%c%c%c%c", DEVICE_INFO, vma->vm_start, vma->vm_end ,(( vma->vm_flags & VM_READ ) ? 'r' : '-'),(( vma->vm_flags & VM_WRITE ) ? 'w' : '-'),(( vma->vm_flags & VM_EXEC ) ? 'x' : '-'),(( vma->vm_flags & VM_SHARED ) ? 's' : 'p'));	
	
		//seq_printf( m, "\n%3d ", ++i );
		//sprintf( buffer, " vm_start=%08lX ", vma->vm_start );
		///sprintf( buffer, " vm_end=%08lX  ", vma->vm_end );

		//ch = ( vma->vm_flags & VM_READ ) ? 'r' : '-';
		//sprintf( buffer, "%c", ch );
		//ch = ( vma->vm_flags & VM_WRITE ) ? 'w' : '-';
		//sprintf( buffer, "%c", ch );
		//ch = ( vma->vm_flags & VM_EXEC ) ? 'x' : '-';
		//sprintf( buffer, "%c", ch );
		//ch = ( vma->vm_flags & VM_SHARED ) ? 's' : 'p';
		//sprintf( buffer, "%c", ch );
    //seq_open(filp, &my_seq_fops);  
    return 0;  
}  
  
  
static struct file_operations dev_fops = {  
    .owner    = THIS_MODULE,  
    .open    = my_open,  
    .mmap   = my_map,  
};  
  
static struct miscdevice misc = {  
    .minor = MISC_DYNAMIC_MINOR,  
    .name = DEVICE_NAME,  
    .fops = &dev_fops,  
};    
  
static int __init dev_init(void)  
{  
    int ret;        
    ret = misc_register(&misc);  //注册混杂设备
    buffer = (unsigned char *)kmalloc(PAGE_SIZE,GFP_KERNEL);//内存分配       
    SetPageReserved(virt_to_page(buffer));//将该段内存设置为保留   
    return ret;  
}  
  
  
static void __exit dev_exit(void)  
{  
    //注销设备  
    misc_deregister(&misc);  
    //清除保留  
    ClearPageReserved(virt_to_page(buffer));  
    //释放内存  
    kfree(buffer);  
}  
  
  
module_init(dev_init);  
module_exit(dev_exit);  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("LKN@SCUT");  


