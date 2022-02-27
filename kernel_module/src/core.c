//////////////////////////////////////////////////////////////////////
//                     University of California, Riverside
//
//
//
//                             Copyright 2022
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng, Yu-Chia Liu
//
//   Description:
//     Core of Kernel Module for CSE202's Resource Container
//
////////////////////////////////////////////////////////////////////////
#include "blockmma.h"
#include <asm/segment.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include "core.h"
extern struct miscdevice blockmma_dev;
/**
 * Enqueue a task for the caller/accelerator to perform computation.
 */

struct task {
    struct blockmma_cmd* data;
    struct task* next;
};

struct task_container {
    struct task* main;
    struct task_container* next;
};

int count = 1;
struct task_container* buffer = NULL;

long blockmma_send_task(struct blockmma_cmd __user *user_cmd) {
    struct blockmma_cmd cmd;
    struct task * testing;
    struct task_container* taskContainer = buffer;
    printk("CORE.C: In send_task %llu \n", user_cmd->a);

    //float *A = (float*)malloc(128*sizeof(float))

    if(copy_from_user(&cmd, user_cmd, sizeof(cmd))!=0){
        printk("Could not copy");
        return -EFAULT;
    };
    
    //float *ptr = (float*)malloc(sizeof(float));
    float p;
    copy_from_user(&p,cmd.b,sizeof(float)); 
    printk("CORE.c value of PPPPP %f", p);
    printk("CORE.C: cmd val %p",(cmd.b+0));

    testing = (struct task*)kmalloc(sizeof(struct task), GFP_KERNEL);
    testing->data = &cmd;
    testing->next = NULL;
    taskContainer = (struct task_container*)kmalloc(sizeof(struct task_container), GFP_KERNEL);
    taskContainer->main = testing;
    taskContainer->next = NULL;
    buffer = taskContainer;
    //printk("**********************************************Test INPUTS 2: %llu", buffer->main->data);
    kfree(testing);
    printk("**********************************************End of the send_task**********************************************");
    return 0;
}

/**
 * Return until all outstanding tasks for the calling process are done
 */
int blockmma_sync(struct blockmma_cmd __user *user_cmd)
{
    return 0;
}

/**
 * Return the task id of a task to the caller/accelerator to perform computation.
 */
int blockmma_get_task(struct blockmma_hardware_cmd __user *user_cmd)
{

    struct blockmma_hardware_cmd cmd;
    struct task_container* head = buffer;
    bool flag = (head->main != 0 && head->main != NULL) ? true : false;

    if(flag){
        struct blockmma_cmd* currentNode = head->main->data;
        // struct task* currentNode = head->main;
        printk("CORE.C: In get_task Value a here -> %llu  with size %llu",user_cmd->a, sizeof(user_cmd->a));
        printk("CORE.C: In get_task Value b here -> %llu  with size %llu",user_cmd->b, sizeof(user_cmd->b));
        copy_from_user(&cmd, user_cmd, sizeof(cmd));

        //copy_to_user((void *)cmd.a, (void *)currentNode->a, sizeof(currentNode->a));
        //copy_to_user((void *)cmd.b, (void *)currentNode->b, sizeof(currentNode->b));

        // printk("Current id in gettask is: %llu", curTask.tid);
        // return curTask.tid;
        
        if (count > 0) {
            count--;
        }

        return count;
    }

    return flag;
}


/**
 * Return until the task specified in  the command is done.
 */
int blockmma_comp(struct blockmma_hardware_cmd __user *user_cmd)
{
    return 0;
}

/*
 * Tell us who wrote the module
 */
int blockmma_author(struct blockmma_hardware_cmd __user *user_cmd)
{
    struct blockmma_hardware_cmd cmd;

    char authors[] = "THIS IS WORKING";
    
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1;
    }

    copy_to_user((void *)cmd.a, (void *)authors, sizeof(authors));
    return 0;
}

int blockmma_init(void)
{
    int ret = 0;
    if ((ret = misc_register(&blockmma_dev)))
    {
        printk(KERN_ERR "Unable to register \"blockmma\" misc device\n");
        return ret;
    }
    printk("BlockMMA kernel module installed\n");
    return ret;
}

void blockmma_exit(void)
{
    printk("BlockMMA removed\n");

    buffer->main = NULL;
    kfree(buffer);

    misc_deregister(&blockmma_dev);
}

