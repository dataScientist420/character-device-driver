/******************************************************************************
                          ____     ____                                                 
                 		 / __/	  / __/__  __ _____________                            
                		/ /_	 _\ \/ _ \/ // / __/ __/ -_|                           
               			\__/	/___/\___/\___/_/  \__/\__/                            
                                                                                                                                                                          
*******************************************************************************
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
 @file: main.c
 @author: Victor Neville
 @description: Simple program which tests a character device driver. 	  			  
 @date: 13-05-2017
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>

/* CONSTANTS */
#define DEVICE_FILE "/dev/s33drax"
#define LENGTH      1024
#define READ        'r'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE

/******************************************************************************
 Allows the user to enter an option on the keyboard.
******************************************************************************/
char get_option() 
{
    char option;
    
    do {
        system("clear");
        printf("DEVICE OPTIONS:\n\n- Read(%c)\n- Write(%c)\n- Quit(%c)\n", 
                READ, WRITE, QUIT);
        option = tolower(getchar());
        __fpurge(stdin);
    } while (option != READ && option != WRITE && option != QUIT);

    return option;
}

/******************************************************************************
 Entry point of the process.
******************************************************************************/
int main()
{
    int fd, done = FALSE;
    char write_buf[LENGTH], read_buf[LENGTH], command[LENGTH/8];

    /* Give permission to use the device file */
    sprintf(command, "sudo chmod 777 %s", DEVICE_FILE);
    system(command);

    assert(fd = open(DEVICE_FILE, O_RDWR));
    
    while (!done) {
        switch(get_option()) {
        case READ:
            read(fd, read_buf, LENGTH);
            printf("device: %s\n", read_buf);
            getchar();
            break;
        case WRITE:
            printf("%s", "Enter data: ");
            scanf("%1023[^\n]s", write_buf);
            __fpurge(stdin);
            write(fd, write_buf, LENGTH);
            break;
        case QUIT:
            done = TRUE;
            break;
        }
    }
    close(fd);
    return 0;
}