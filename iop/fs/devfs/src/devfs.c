/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# DevFS source file
*/

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "loadcore.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"
#include "intrman.h"
#include "stdio.h"
#include "sysclib.h"
#include "devfs.h"
#include "sysmem.h"
#include "sys/stat.h"
#include "stdio.h"

#ifdef USE_IOMAN
#include "ioman.h"
#else
#include "iomanX.h"
#endif

#define MODNAME "devfs"
IRX_ID(MODNAME, 1, 1);

#define M_PRINTF(format, args...)       printf(MODNAME ": " format, ## args)

#define BANNER "DEVFS %s\n"
#define VERSION "v0.1"
#define MAX_OPENFILES 32
#define MAX_OPEN_DIRFILES 16

struct irx_export_table _exp_devfs;

iop_device_t devfs_device;
iop_device_ops_t devfs_ops;
typedef int (*dummy_func)(void);

typedef struct

{
  void *data;
  s32 valid;
  u32 mode;
  u32 open_refcount;
  devfs_loc_t extent;
  struct _ioman_data *open_files[MAX_OPENFILES]; 
} subdev_t;

typedef struct _devfs_device
{
  struct _devfs_device *forw, *back; /* Pointers to the next device in the chain */
  HDEV hDev;
  devfs_node_t node;
  subdev_t subdevs[DEVFS_MAX_SUBDEVS];
  u32 subdev_count;
  u32 open_refcount;
} devfs_device_t;
   
typedef struct _ioman_data
{
   HDEV hDev;
   devfs_device_t *dev;  /* Pointer to the main device, set to NULL if device closed */ 
   int subdev;
   u32 mode;
   devfs_loc_t loc;
} ioman_data_t;

devfs_device_t *root_device; /* Pointer to the root device */
HDEV dev_count;

typedef struct _directory_file

{
   HDEV hDev; /* Should always be set to -1 */
   u32 devno; /* The number of the file being listed */
   u32 opened; /* If this directory file is in an opened state */
} directory_file_t;

directory_file_t open_dirfiles[MAX_OPEN_DIRFILES];

/** Simple function to convert a number to a string 
    
    @param subdev: The number to conver.
    @returns A constant string containing the number. Returns an 
    empty string if not possible to convert.
*/
const char *devfs_subdev_to_str(int subdev)

{
   static char name[4]; /* Supports up to 999 subdevices */
   int loop;

   if((subdev > 999) || (subdev < 0))
   {
      name[0] = 0;
   }
   else
   { 
      loop = 0;
      name[loop++] = ((subdev / 100) % 10) + '0';
      name[loop++] = ((subdev / 10) % 10) + '0';
      name[loop++] = (subdev % 10) + '0';
      name[loop++] = 0;
   }

   loop = 0;
   if(name[0] == '0') loop = 1;
   if((name[0] == '0') && (name[1] == '0')) loop = 2;

   return &name[loop];
}

/** Scans the device list and fills in the corresponding directory entry
    
    @param dirent: Pointer to a ::iox_dirent_t structure.
    @param devno: The sequential device number to read

    @returns Always 0
*/

int devfs_fill_dirent(iox_dirent_t *dirent, int devno)

{
   devfs_device_t *dev_scan;
   int curr_no = 0;
   u32 subdev_loop;
   
   dev_scan = root_device;
   
   while(dev_scan != NULL)
   { 
     if((curr_no + dev_scan->subdev_count) > devno)
     {
        for(subdev_loop = 0; subdev_loop < DEVFS_MAX_SUBDEVS; subdev_loop++)
        {
           if(dev_scan->subdevs[subdev_loop].valid)
           {
              if(curr_no == devno)
              {
                 break;
              }
              curr_no++;
           }
        }
   
        if(subdev_loop < DEVFS_MAX_SUBDEVS)
        {
           memset(dirent, 0, sizeof(iox_dirent_t));
           dirent->stat.size = dev_scan->subdevs[subdev_loop].extent.loc32[0];
           dirent->stat.hisize = dev_scan->subdevs[subdev_loop].extent.loc32[1];
           dirent->stat.mode = FIO_S_IFREG;
           if(dev_scan->subdevs[subdev_loop].mode & DEVFS_MODE_R)
           {
              dirent->stat.mode |= FIO_S_IRUSR;
           }
           if(dev_scan->subdevs[subdev_loop].mode & DEVFS_MODE_W)
           {
              dirent->stat.mode |= FIO_S_IWUSR;
           }
           dirent->name[0] = 0;
           strcpy(dirent->name, dev_scan->node.name);
           strcat(dirent->name, devfs_subdev_to_str(subdev_loop));
        
           return strlen(dirent->name);
        }
     }
     
     curr_no += dev_scan->subdev_count;

     dev_scan = dev_scan->forw;
   }
   
   return 0;
}

/** Creates a new device based on the node data supplied 
 
    @param node: Pointer to a ::devfs_node_t structure
    
    @returns A new device structure or NULL on error.
*/
devfs_device_t *devfs_create_device(const devfs_node_t *node)

{
   devfs_device_t *dev;

   if((node == NULL) || (node->name == NULL))
   { 
      return NULL;
   }

   dev = AllocSysMemory(ALLOC_FIRST, sizeof(devfs_device_t), NULL);
   if(dev == NULL)
   {
      printf("create_device: Alloc Failed\n");
      return NULL;
   }

   memset(dev, 0, sizeof(devfs_device_t));
   memcpy(&dev->node, node, sizeof(devfs_node_t));
   if(dev->node.name != NULL)
   {
      dev->node.name = AllocSysMemory(ALLOC_FIRST, strlen(node->name) + 1, NULL);
      if(dev->node.name == NULL)
      { 
         FreeSysMemory(dev);
         return NULL;
      }
      memcpy(dev->node.name, node->name, strlen(node->name) + 1);
   }
 
   if(dev->node.desc != NULL)
   {
      dev->node.desc = AllocSysMemory(ALLOC_FIRST, strlen(node->desc) + 1, NULL);
      if(dev->node.name == NULL)
      {
         FreeSysMemory(dev->node.name);
         FreeSysMemory(dev);
         return NULL;
      }
      memcpy(dev->node.desc, node->desc, strlen(node->desc) + 1);
   }

   return dev; 
}

/** Deletes a device 
    @param dev: Pointer to a previously allocated device structure
    @returns Always 0
*/

int devfs_delete_device(devfs_device_t *dev)

{
  if(dev != NULL)
  {
     if(dev->node.name)
     {
        FreeSysMemory(dev->node.name);
     }
 
     if(dev->node.desc)
     {
        FreeSysMemory(dev->node.desc);
     }

     FreeSysMemory(dev);
  }

  return 0;
}

/** Trys to find a device with a matching name. 

    The name can be a full device name with subdev on the end, the function should
    still find the device.
    @param name: The name of the device to find
    @returns A pointer to the device, if not found returns NULL
*/
devfs_device_t *devfs_find_devicename(const char *name)

{
   devfs_device_t *dev_scan;

   if(name == NULL)
   {
      return NULL;
   }

   dev_scan = root_device;

   while(dev_scan != NULL)
   {
     /* Ensure name is at least less than or equal to the one we want to find */
     if(strlen(dev_scan->node.name) <= strlen(name))
     {
        if(strncmp(dev_scan->node.name, name, strlen(dev_scan->node.name)) == 0)
        {
           return dev_scan;
        } 
     }
     dev_scan = dev_scan->forw;
   }

   return NULL;
}

/** Finds a device from its device handle.

    @param hDev: Handle to the device.
    @returns A pointer to the device, if not found returns NULL
*/
devfs_device_t *devfs_find_deviceid(HDEV hDev)

{
   devfs_device_t *dev_scan;

   dev_scan = root_device;

   while(dev_scan != NULL)
   {
     if(dev_scan->hDev == hDev)
     {
        return dev_scan;
     }
     dev_scan = dev_scan->forw;
   }

   return NULL;
}

/** Dummy ioman handler 
    @returns Always returns -1
*/
int devfs_dummy(void)

{
   printf("devfs_dummy\n");
   return -1;
}

/** ioman init handler
    @returns Always returns 0
*/
int devfs_init(iop_device_t *dev)

{
   printf("devfs_init dev=%p\n", dev);
   return 0;
}

/** ioman deinit handler
    @returns Always returns 0
*/
int devfs_deinit(iop_device_t *dev)

{
   printf("devfs_deinit dev=%p\n", dev);
   return 0;
}

/** ioman open handler
    @oaram file: Pointer to the ioman file structure
    @param name: Name of file to open
    @param mode: Open file mode settings
    @returns 0 if success, -1 on error
*/
int devfs_open(iop_file_t *file, const char *name, int mode, ...)

{
   devfs_device_t *dev;
   int loop;
   int fn_offset = 0; 

   //printf("devfs_open file=%p name=%s mode=%d\n", file, name, mode);
   if(name == NULL)
   {
      M_PRINTF("open: Name is NULL\n");
      return -1;
   }
  
   if((name[0] == '\\') || (name[0] == '/'))
   {
      fn_offset = 1;
   } 

   dev = devfs_find_devicename(name + fn_offset);

   if(dev != NULL)
   {
      char *endp;
      int subdev;
      int name_len;
      
      name_len = strlen(dev->node.name);

      if(name_len == strlen(name)) /* If this is has not got a subnumber */
      {
        M_PRINTF("open: No subdevice number in filename %s\n", name);
        return -1;
      }

      subdev = strtoul(&name[name_len + fn_offset], &endp, 10);
      if(((subdev == 0) && (name[name_len + fn_offset] != '0')) 
        || (subdev >= DEVFS_MAX_SUBDEVS))
         /* Invalid number */
      {
         M_PRINTF("open: Invalid subdev number %d\n", subdev);
         return -1;
      }

      if(*endp)
      /* Extra charactes after filename */
      {
         M_PRINTF("open: Invalid filename\n");
         return -1;
      }

      if(!dev->subdevs[subdev].valid)
      /* No subdev */
      {
        M_PRINTF("open: No subdev registered\n");
        return -1;
      }

      if((dev->subdevs[subdev].mode & DEVFS_MODE_EX) 
        && (dev->subdevs[subdev].open_refcount > 0))
      /* Already opened in exclusive mode */
      {
         M_PRINTF("open: Exclusive subdevice already opened\n");
         return -1;
      }

      if(dev->subdevs[subdev].open_refcount == MAX_OPENFILES)
      {
         M_PRINTF("open: Reached open file limit for sub device\n");
      }

      /* Tried to open read but not allowed */
      if(((mode & O_RDONLY) || ((mode & O_RDWR) == O_RDWR)) 
        && !(dev->subdevs[subdev].mode & DEVFS_MODE_R))
      {
         M_PRINTF("open: Read mode requested but not permitted\n");
         return -1;
      }

      if(((mode & O_WRONLY) || ((mode & O_RDWR) == O_RDWR)) 
        && !(dev->subdevs[subdev].mode & DEVFS_MODE_W))
      {
         M_PRINTF("open: Write mode requested but not permitted\n");
         return -1;
      }

      file->privdata = AllocSysMemory(ALLOC_FIRST, sizeof(ioman_data_t), NULL);
      if(file->privdata == NULL)
      {
         M_PRINTF("open: Allocation failure\n");
         return -1;
      }

      ((ioman_data_t *) file->privdata)->hDev = dev->hDev;
      ((ioman_data_t *) file->privdata)->subdev = subdev;
      ((ioman_data_t *) file->privdata)->loc.loc64 = 0;
      ((ioman_data_t *) file->privdata)->dev = dev;
      ((ioman_data_t *) file->privdata)->mode = mode;

      dev->subdevs[subdev].open_refcount++;
      loop = 0;
      while((loop < MAX_OPENFILES) && (dev->subdevs[subdev].open_files[loop]))
      {
          loop++;
      }
      if(loop == MAX_OPENFILES)
      {
         FreeSysMemory(file->privdata);
         M_PRINTF("open: Inconsistency between number of open files and available slot\n");
         return -1;
      }

      dev->subdevs[subdev].open_files[loop] = file->privdata;  
      dev->open_refcount++;
      M_PRINTF("open: Opened device %s subdev %d\n", dev->node.name, subdev);
   }
   else
   {
      M_PRINTF("open: Couldn't find the device\n");
      return -1;
   }
   
   return 0;
}

/** ioman close handler
  
    @param file: Pointer to an ioman file structure
    @returns 0 on success, else -1
*/
int devfs_close(iop_file_t *file)

{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;
   int loop;

   if((data) && (data->hDev != INVALID_HDEV)) /* Invalid HDEV is a dir listing */
   {
      dev = data->dev;
   
      if(dev != NULL)
      {
         if(dev->subdevs[data->subdev].open_refcount > 0)
         {
             dev->subdevs[data->subdev].open_refcount--;
         }
         if(dev->open_refcount > 0)
         {
            dev->open_refcount--;
         }

         loop = 0;
         while((loop < MAX_OPENFILES) 
               && (dev->subdevs[data->subdev].open_files[loop] != data))
         {
            loop++;
         }
 
         if(loop != MAX_OPENFILES)
         { 
            dev->subdevs[data->subdev].open_files[loop] = NULL;
         }         
         else
         { 
            M_PRINTF("close: Could not find opened file\n");
         }
 
         M_PRINTF("close: Closing device %s subdev %d\n", dev->node.name, data->subdev);
         FreeSysMemory(data);
      }
      else
      {
         FreeSysMemory(data);
         file->privdata = NULL; 
         return -1;
      }
   } 
   else
   { 
      return -1;
   }

   return 0;
}

/** ioman ioctl handler
    @param file: Pointer to an ioman file structure
    @param cmd: ioctl command number
    @param args: Pointer to a buffer containing any arguments
    @returns >= 0 Defined by the type of command, -1 on error
*/
int devfs_ioctl(iop_file_t *file, unsigned long cmd, void *args)

{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;
            
   if(cmd == DEVFS_IOCTL_GETDESC)
   {
      return -1; /* ioctl cannot support this call */
   } 

   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {  
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }

      if(dev->node.ioctl != NULL)
      {
         devfs_info_t dev_info;

         dev_info.data = dev->subdevs[data->subdev].data;
         dev_info.subdev = data->subdev;
         dev_info.mode = data->mode;
         dev_info.loc = data->loc;
         return dev->node.ioctl(&dev_info, DEVFS_IOCTL_TYPE_1, cmd, args, 0, NULL, 0);
      }

   }

   return -1;
}
 
/** ioman ioctl2 handler
    @param file: Pointer to an ioman file structure
    @param cmd: ioctl command number
    @param args: Pointer a buffer containing command arguments
    @param arglen: Length of args buffer
    @param buf: Pointer to a return buffer
    @param buflen: Length of buf
    @returns >= 0 Depends on command, -1 on error
*/
int devfs_ioctl2(iop_file_t *file, int cmd, void *args, unsigned int arglen, void *buf, unsigned int buflen)

{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;

   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }

      if((cmd == DEVFS_IOCTL_GETDESC) && (buf) && (buflen >= DEVFS_MAX_DESC_LENGTH))
      {
         if(dev->node.desc)
         {
            strncpy(buf, dev->node.desc, buflen-1);
            *((u8 *) (buf + buflen)) = 0;
            return strlen(buf);
         }
         
         return 0;
      }
            
      if(dev->node.ioctl != NULL)
      {
         devfs_info_t dev_info;

         dev_info.data = dev->subdevs[data->subdev].data;
         dev_info.subdev = data->subdev;
         dev_info.mode = data->mode;
         dev_info.loc = data->loc;
         return dev->node.ioctl(&dev_info, DEVFS_IOCTL_TYPE_2, cmd, args, arglen, buf, buflen);
      }

   }

   return -1;
}

/** ioman read handler
    @param file: Pointer to an ioman file structure
    @param buf: Buffer to read data to
    @param len: Length of data to read
    @returns Length of data read. 0 on eof or file pointer out of range. -1 on error.
*/
int devfs_read(iop_file_t *file, void *buf, int len)

{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;

   //printf("devfs_read file=%p buf=%p len=%d\n", file, buf, len);

   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }

      if((dev->node.read != NULL) 
        && (dev->subdevs[data->subdev].mode & DEVFS_MODE_R))
      {
         devfs_info_t dev_info;
         int bytes_read;

         dev_info.data = dev->subdevs[data->subdev].data;
         dev_info.subdev = data->subdev;
         dev_info.mode = data->mode;
         dev_info.loc = data->loc;
         bytes_read = dev->node.read(&dev_info, buf, len); 
         if(bytes_read > 0)
         { 
            data->loc.loc64 += (u64) bytes_read;
         }
         return bytes_read;
      } 
   }
  
   return -1;
}

/** ioman write handler
    @param file: Pointer to an ioman file structure
    @param buf: Pointer to the buffer to write
    @param len: Length of data to write
    @returns Number of bytes written, 0 on eof or -1 on error
*/
int devfs_write(iop_file_t *file, void *buf, int len)

{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;

   //printf("devfs_write file=%p buf=%p len=%d\n", file, buf, len);
   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }

      if((dev->node.write != NULL)
        && (dev->subdevs[data->subdev].mode & DEVFS_MODE_W))
      {
         devfs_info_t dev_info;
         int bytes_written;

         dev_info.data = dev->subdevs[data->subdev].data;
         dev_info.subdev = data->subdev;
         dev_info.mode = data->mode;
         dev_info.loc = data->loc;
         bytes_written = dev->node.write(&dev_info, buf, len);
         if(bytes_written > 0)
         {
            data->loc.loc64 += (u64) bytes_written;
         }
         return bytes_written; 
      }
   }

   return -1;
}

/** ioman lseek handler
    @param file: Pointer to a ioman file structure
    @param loc: Location to seek to
    @param whence: Seek base. 
    @returns location seeked to, -1 on error.
*/
int devfs_lseek(iop_file_t *file, long loc, int whence)
{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;

   //printf("devfs_lseek file=%p loc=%ld whence=%d\n", file, loc, whence);
   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }
      
      switch(whence)
      {
        case SEEK_SET: if(loc > 0)
                         data->loc.loc64 = (u64) loc;
                       break;
        case SEEK_CUR: if(loc > 0)
                         data->loc.loc64 += (u64) loc;
                       else if(((s64) -loc) < data->loc.loc64)
                         data->loc.loc64 += (s64) loc;
                       break;
        case SEEK_END: if((loc > 0) 
                       && (dev->subdevs[data->subdev].extent.loc64 >= (u64) loc))
                         data->loc.loc64 = dev->subdevs[data->subdev].extent.loc64 - (u64) loc; 
                       break;
        default:       return -1;
      }
   } 
   else
   {
      return -1;
   }

   return data->loc.loc32[0];
}

/** ioman lseek64 handler
    @param file: Pointer to an ioman file structure 
    @param loc: 64bit seek location
    @param whence: Seek base
*/
int devfs_lseek64(iop_file_t *file, long long loc, int whence)
{
   devfs_device_t *dev;
   ioman_data_t *data = (ioman_data_t *) file->privdata;

   //printf("devfs_lseek64 file=%p loc= whence=%d\n", file, whence);
   data = file->privdata;
   if((data) && (data->hDev != INVALID_HDEV))
   {
      dev = data->dev;
      if(dev == NULL) /* Device has been closed from under us */
      {
         /* Delete data to try and prevent a memory leak */
         FreeSysMemory(data);
         file->privdata = NULL;
         return -1;
      }

      switch(whence)
      {
        case SEEK_SET: if(loc > 0)
                         data->loc.loc64 = (u64) loc;
                       break;
        case SEEK_CUR: if(loc > 0)
                         data->loc.loc64 += (u64) loc;
                       else if(((s64) -loc) < data->loc.loc64)
                         data->loc.loc64 += (s64) loc;
                       break;
        case SEEK_END: if((loc > 0)                                                                                  && (dev->subdevs[data->subdev].extent.loc64 >= (u64) loc))
                         data->loc.loc64 = dev->subdevs[data->subdev].extent.loc64 - (u64) loc; 
                       break;
        default:       return -1;
      }

   }
   else
   {
      return -1;
   }

   return data->loc.loc32[0];
}

/** ioman dopen handler
    @param file: Pointer to an ioman file structure
    @param name: Name of directory to open
    @returns 0 if success, -1 on error
*/
int devfs_dopen(iop_file_t *file, const char *name)
{
   int dir_loop;

   if((name == NULL) && (name[0] != '\\') && (name[0] != '/'))
   {
      M_PRINTF("dopen: Not a valid directory name\n");
      return -1;
   }

   //M_PRINTF("dopen: file=%p name=%s\n", file, name);
   for(dir_loop = 0; dir_loop < MAX_OPEN_DIRFILES; dir_loop++)
   {
      if(!open_dirfiles[dir_loop].opened)
      {
         open_dirfiles[dir_loop].devno = 0;
         open_dirfiles[dir_loop].hDev = INVALID_HDEV;
         file->privdata = &open_dirfiles[dir_loop];
         return 0;
      }
   }

   return -1;
}

/** ioman dclose handler
    @param file: Pointer to an ioman file structure
    @returns Alwats returns 0
*/
int devfs_dclose(iop_file_t *file)
{
   directory_file_t *dir;

   //M_PRINTF("dclose: file=%p\n", file);
   dir = (directory_file_t *) file->privdata;

   if((dir) && (dir->hDev == INVALID_HDEV))
   {
      dir->opened = 0;
      file->privdata = NULL;
   } 

   return 0;
}

int devfs_dread(iop_file_t *file, void *buf)
{
   directory_file_t *dir;
   int ret = 0;

   //M_PRINTF("dread: file=%p buf=%p\n", file, buf);
   dir = (directory_file_t *) file->privdata;
   if((dir) && (dir->hDev == INVALID_HDEV) && (buf))
   {
      ret = devfs_fill_dirent((iox_dirent_t *) buf, dir->devno);
      if(ret)
      { 
         dir->devno++;
      }
   }
  
   return ret;
}

/** ioman getstat handler
    @param file: Pointer to an ioman file structure
    @param name: Name of the file to stat
    @param buf: Buffer to receive the dirent
    @returns 0 on success, -1 on error
*/
int devfs_getstat(iop_file_t *file, const char *name, void *buf)

{
   devfs_device_t *dev;
   iox_dirent_t *dirent = (iox_dirent_t *) buf;
   int fn_offset = 0;

   if(name == NULL)
   {
      return -1;
   }

   if((name[0] == '\\') || (name[0] == '/'))
   {  
      fn_offset = 1;
   }    
  
   if(buf == NULL)
   {
      return -1;
   }
      
   dev = devfs_find_devicename(name + fn_offset);

   if(dev != NULL)
   {    
      char *endp;
      int subdev;
      int name_len;
         
      name_len = strlen(dev->node.name);

      if(name_len == strlen(name)) /* If this is has not got a subnumber */
      {
        M_PRINTF("getstat: No subdevice number in filename %s\n", name);
        return -1;
      }
      
      subdev = strtoul(&name[name_len + fn_offset], &endp, 10);
      if(((subdev == 0) && (name[name_len + fn_offset] != '0'))
        || (subdev >= DEVFS_MAX_SUBDEVS))
         /* Invalid number */
      {  
         M_PRINTF("getstat: Invalid subdev number %d\n", subdev);
         return -1;
      }
        
      if(*endp)
      /* Extra charactes after filename */
      {  
         M_PRINTF("getstat: Invalid filename\n");
         return -1;
      }

      if(!dev->subdevs[subdev].valid)
      {
        M_PRINTF("getstat: No subdev registered\n");
        return -1;
      } 

      memset(dirent, 0, sizeof(iox_dirent_t));
      dirent->stat.size = dev->subdevs[subdev].extent.loc32[0];
      dirent->stat.hisize = dev->subdevs[subdev].extent.loc32[1];
      dirent->stat.mode = FIO_S_IFREG;
      if(dev->subdevs[subdev].mode & DEVFS_MODE_R)
      {
         dirent->stat.mode |= FIO_S_IRUSR;
      }
      if(dev->subdevs[subdev].mode & DEVFS_MODE_W)
      {
         dirent->stat.mode |= FIO_S_IWUSR;
      }
      dirent->name[0] = 0;
      strcpy(dirent->name, dev->node.name);
      strcat(dirent->name, devfs_subdev_to_str(subdev));
   }
      
   return 0;
}

/** DevFS initialise function
    @returns >= 0 on success, -1 on error
*/
int init_devfs(void)

{
   int dummy_loop;
   dummy_func *dummy; 

   root_device = NULL;
   dev_count = 0;

   /* Set all io handlers to dummy values */
   dummy = (dummy_func *) &devfs_ops;
   for(dummy_loop = 0; dummy_loop < (sizeof(iop_device_ops_t) / sizeof(dummy)); dummy_loop++)
   {
      dummy[dummy_loop] = devfs_dummy;
   }
   memset(open_dirfiles, 0, sizeof(directory_file_t) * MAX_OPEN_DIRFILES);
 
   devfs_device.name = "devfs";
   devfs_device.type = IOP_DT_FS | IOP_DT_FSEXT;
   devfs_device.version = 0x100;
   devfs_device.desc = "PS2 Device FS Driver";
   devfs_device.ops = &devfs_ops;
   devfs_ops.init = devfs_init; 
   devfs_ops.deinit = devfs_deinit;
   devfs_ops.open = devfs_open;
   devfs_ops.read = devfs_read;
   devfs_ops.close = devfs_close;
   devfs_ops.dopen = devfs_dopen;
   devfs_ops.dclose = devfs_dclose;
   devfs_ops.dread = devfs_dread;
   devfs_ops.ioctl = devfs_ioctl;
   devfs_ops.ioctl2 = devfs_ioctl2;
   devfs_ops.getstat = devfs_getstat;
   DelDrv("devfs");

   return AddDrv(&devfs_device);
}

/** Main start function of the module
    @param argc: Unused
    @param argv: Unused
    @returns 0 on success, -1 on error
*/
int _start(int argc, char **argv)
{
   int res = 1;

   printf(BANNER, VERSION);

   if ((res = RegisterLibraryEntries(&_exp_devfs)) != 0) {
      M_PRINTF("Library is already registered, exiting.\n");
      printf("res=%d\n", res);
   }
   else
   {
      res = init_devfs();
   }

   M_PRINTF("devfs_device_t size=%d\n", sizeof(devfs_device_t));
   M_PRINTF("Driver loaded.\n");

   return res;
}

/** Function to check a device name only contains alphabetic characters
    @param name: The name to check.
    @returns 0 if the name is invalid, else 1
*/
int devfs_check_devname(const char *name)
/* Valid device name uses alphabetic characters only */

{
   int str_loop;

   if((name == NULL) || (name[0] == 0))
   {
      return 0;
   }
  
   if(strlen(name) > DEVFS_MAX_DEVNAME_LENGTH)
   {
      return 0;
   }

   str_loop = 0;

   while(name[str_loop]) 
   {
      char ch;

      ch = name[str_loop];
      if(((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')))
      {
         str_loop++;
      }
      else
      {
         return 0;
      }
   } 

   return 1;
}

/** Adds a new device to the filing system
    @param node: Pointer to a ::devfs_node_t structure
    @returns A device handle is returned if the device was added. 
    On error INVALID_HDEV is returned
*/
HDEV DevFSAddDevice(const devfs_node_t *node)

{
   devfs_device_t *dev = NULL;
   devfs_device_t *dev_scan;

   //printf("AddDevice node=%p\n", node);
   if(node == NULL)
   {
      M_PRINTF("AddDevice: node == NULL\n");
      return INVALID_HDEV;
   }

   if(!devfs_check_devname(node->name))
   {
      M_PRINTF("AddDevice: node name invalid\n");
      return INVALID_HDEV;
   }

   if(devfs_find_devicename(node->name))
   {
     M_PRINTF("AddDevice: cannot add new device. Already exists\n");
     return INVALID_HDEV;
   }

   dev = devfs_create_device(node);
   if(dev == NULL)
   {
      M_PRINTF("AddDevice: failed to allocate device structure\n");
      return INVALID_HDEV;
   }
   if(root_device == NULL)
   {
      root_device = dev;
   } 
   else
   {
      dev_scan = root_device;
      while(dev_scan->forw) /* Scan the device linked list */
      {
         dev_scan = dev_scan->forw;
      }
      dev_scan->forw = dev;
      dev->back = dev_scan;
   }
 
   dev->hDev = dev_count++;
   if(dev_count == INVALID_HDEV) dev_count++;
   M_PRINTF("AddDevice: Registered device (%s)\n", node->name);

   return dev->hDev;
}

/** Deletes an previously opened device.
    @param hDev: Handle to the device to delete
    @returns 0 if device deleted, -1 on error
*/
int DevFSDelDevice(HDEV hDev)

{
   devfs_device_t *dev;
   int ret = -1;
   devfs_device_t *forw, *back;
 
   //printf("DelDevice hDev=%d\n", hDev);
   dev = devfs_find_deviceid(hDev);
   if(dev)
   {
      int subdev_loop;
      int openfile_loop;

      forw = dev->forw;
      back = dev->back;

      /* Delete the device even if open files */
      for(subdev_loop = 0; subdev_loop < DEVFS_MAX_SUBDEVS; subdev_loop++)
      {
         for(openfile_loop = 0; openfile_loop < MAX_OPENFILES; openfile_loop++)
         {
            if(dev->subdevs[subdev_loop].open_files[openfile_loop])
            {
               dev->subdevs[subdev_loop].open_files[openfile_loop]->dev = NULL;
               /* Set the device reference to NULL */
            }
         }
      }
      devfs_delete_device(dev);

      if(back)
      {
         back->forw = forw;
      }
      else
      {
         root_device = NULL;
      }
	  
      if(forw)
      {
         forw->back = back; 
      }
	  
      ret = 0;
   }
   
   return ret;
}

/** Adds a sub device to a previously opened device
    @param hDev: Handle to an opened device
    @param subdev_no: The number of the subdevice. Can be 0 to DEVFS_MAX_SUBDEVS
    @param extent: A 64bit extent which reflects the size of the underlying device 
    @param data: Pointer to some private data to associate with this sub device
    @returns 0 if sub device added, else -1
*/
int DevFSAddSubDevice(HDEV hDev, u32 subdev_no, s32 mode, devfs_loc_t extent, void *data)

{
   devfs_device_t *dev;
 
   //M_PRINTF("AddSubDevice hDev=%d subdev_no=%d data=%p\n", hDev, subdev_no, data);
   dev = devfs_find_deviceid(hDev);
   if(dev != NULL)
   {
      if(subdev_no >= DEVFS_MAX_SUBDEVS)
      {
         M_PRINTF("AddSubDevice: Sub device number too big\n");
         return -1; 
      }
  
      if(dev->subdevs[subdev_no].valid)
      {
         M_PRINTF("AddSubDevice: Sub device already created\n");
         return -1;
      } 

      dev->subdevs[subdev_no].data = data;
      dev->subdevs[subdev_no].valid = 1;
      dev->subdevs[subdev_no].mode = mode;
      dev->subdevs[subdev_no].open_refcount = 0;
      dev->subdevs[subdev_no].extent = extent;
      memset(dev->subdevs[subdev_no].open_files, 0, sizeof(ioman_data_t *) * MAX_OPENFILES); 
      dev->subdev_count++;
   } 
   else
   {
      M_PRINTF("AddSubDevice: Couldn't find device ID\n");
      return -1;
   }   

   return 0;
}

/** Deletes a sub device.
    @param hDev: Handle to an opened device.
    @param subdev_no: The number of the subdevice to delete.
    @returns 0 if device deleted. -1 on error.
*/
int DevFSDelSubDevice(HDEV hDev, u32 subdev_no)

{
   devfs_device_t *dev;
   int openfile_loop;

   //printf("DelSubDevice hDev=%d subdev_no=%d\n", hDev, subdev_no);
   dev = devfs_find_deviceid(hDev);
   if(dev != NULL)
   {
      if(subdev_no >= DEVFS_MAX_SUBDEVS)
      {
         M_PRINTF("DelSubDevice: Sub device number too big\n");
         return -1; 
      }
  
      if(!dev->subdevs[subdev_no].valid)
      {
         M_PRINTF("DelSubDevice: Sub device not created\n");
         return -1;
      } 

      dev->subdevs[subdev_no].data = NULL;
      dev->subdevs[subdev_no].valid = 0;
      dev->subdevs[subdev_no].mode = 0;
      dev->subdevs[subdev_no].open_refcount = 0;
      dev->subdevs[subdev_no].extent.loc64= 0;
      for(openfile_loop = 0; openfile_loop < MAX_OPENFILES; openfile_loop++)
      {
         if(dev->subdevs[subdev_no].open_files[openfile_loop])
         {
           dev->subdevs[subdev_no].open_files[openfile_loop]->dev = NULL;
           dev->subdevs[subdev_no].open_files[openfile_loop] = NULL;
         }
      } 
      dev->subdev_count--;
   }

   return 0;
}
