# NCTU_OS_2015_HW4
This is the homework 4 of Introduction to Operating System in the fall semester, 2015.

What I had done were:
* Write my own hexdump and hexedit tools
* Aquire the knowledge of FAT32
* List all files in file system (August 9, 2018)

## Overview and Purpose
The aim of this assignment is to familiarize you with FAT32 file system, and to help you to understand underlying layout of that, you will need to implement your own hexdump and hexedit tools by using file system calls (such as open, read, write, lseek).

Then you will also need to design other tools or programs that will help you to solve the next few problems in this assignment.

## Problem 0: Write you own hexdump and hexedit tools
0. fork repository from `chalos/NCTU_OS_2015_HW4`
1. fill in the blank on *.c files
2. $ cd myHexDump/ or cd myHexEdit/
3. $ make
4. $ cd ../out #your executable hexDump or hexEdit will at here

### HexDump tools
You are asked to write myHexDump by your own. myHexDump is able to read a
block of data in block device and print it to console in readable format.
myHexDump accepts 3 arguments: Device/Image Location, Offset, Size

Usage: `$ ./myHexDump {Device/Image Location} {Offset} {Size}`

### HexEdit tools
You are asked to write myHexEdit, which is able to change value of a block of data in block device.
myHexEdit accepts at least 3 arguments: Device/Image Location, Offset, Values, …

Usage: `$ ./myHexEdit {Device/Image Location} {Offset} {Value1} [ {Value2} ..]`

## Problem 1: List all files in file system
This problem is designed to let you familiar with **Directory Entries** of FAT32 and **Long File Names** of vFAT. You are going to __list all the directories and files (URI format)__ in the pseudo block device you created. The entries you listed should be in **LFN (Long File Names)** form and **dot entry** should be ignored.

## Problem 2: Write a simple malware
You are going to write a malware, lets call it “32’s Nightmare”.

32’s Nightmare will __replace the content__ of files in the block device, by a __list of repeated strings__ ("{student_id} rocks!"), which has the __same size__ as the origin file size. And 32’s Nightmare will also __ignore deleted__ file data. 

32’s Nightmare will __not replace Reserved Area, FAT Area, and Directory Entries__.

## Problem 3: Recover all deleted files in file system
In FAT32, the remove action on a file, will only remove the record from **File Allocating Table** and put a **flag (0xE5)** on its first byte of **directory entry** and **LFN entries**. So you are able to do __recovery on removed file__ by doing reverse action on it.

In problem 3, you are asked to do recovery on the block device that all its files are (accidently) deleted by: `$ rm -rf mountPoint/`

# License
MIT
