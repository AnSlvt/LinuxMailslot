## Project - Linux Mail slots 

This specification related the implementation of a special device file that is accessible according to FIFO style semantic (via open/close/read/write services), but offering an execution semantic of read/write services such that any segment that is posted to the stream associated with the file is seen as an independent data unit (a message), thus being posted and delivered atomically (all or nothing) and in data separation (with respect to other segments) to the reading threads. 

The device file needs to be multi-instance (by having the possibility to manage at most 256 different instances) so that mutiple FIFO style streams (characterized by the above semantic) can be concurrently accessed by active processes/threads. 

The device file needs to also support ioctl commands in order to define the run time behavior of any I/O session targeting it (such as whether read and/or write operations on a session need to be performed according to blocking or non-blocking rules). 

Parameters that are left to the designer, which should be selected via reasonable policies, are: 
    the maximum size of managed data-units (this might also be made tunable via ioctl up to an absolute upper limit) 
    the maximum storage that can be (dynamically) reserved for any individual mail slot 
    the range of device file minor numbers supported by the driver (it could be the interval [0-255] or not) 

NOTE: this project deals with implementing within Linux services similar to those that are offered by Windows "mail slots", which is the reason for the selected project name 
