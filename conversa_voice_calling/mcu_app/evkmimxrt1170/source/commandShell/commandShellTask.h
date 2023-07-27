/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __COMMAND_SHELL_TASK_H__
#define __COMMAND_SHELL_TASK_H__


/*******************************************************************************
 * Definitions
 ******************************************************************************/
// App schell task which will be suspend after init OSA schell task
#define APP_SHELL_TASK_STACK_SIZE  (1024)
#define APP_SHELL_TASK_PRIORITY     1  			// configMAX_PRIORITIES-1 is the most priority task and idle task priority is 0

// OSA shell task SHELL_TASK_STACK_SIZE  and SHELL_TASK_PRIORITY define in project preprocessor


/*******************************************************************************
 * Structure
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void APP_Shell_Task(void *param);

#endif /* __COMMAND_SHELL_TASK_H__ */
