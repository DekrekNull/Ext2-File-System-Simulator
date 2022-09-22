# Ext2-File-System-Simulator
<p>This is a Ext2 file system simulator that I created as a final project for a class at WSU: CptS 360 Systems Programming C/C++.</p>
This project was developed in a Linux environment and will likely only function properly in one. It is written entirely in the C language, and is intended to simulate the basic functionality of an Ext2 file system using virtual disks. The following command line commands are funcitional in the simulator: <br>
- <b>ls:</b> List the current working directory. By default it prints the detailed list (similar to ls -l).
- <b>cd:</b> Takes a pathname as an argument and changes the current directory to that path.
- <b>pwd:</b> Prints the path to the current working directory e.g C://Users/Derek/Documents.
- <b>mkdir:</b> Takes a path as an argument and creates a new directory at that path if it doesn't already exist.
- <b>creat:</b> Similar to mkdir, but instead of a directory it creates a new file.
- <b>rmdir:</b> Takes a path as an argument. If a directory exists at the path and has no dependencies, it is permanently deleted.
- <b>link:</b> Takes two paths as arguments and, if both are existing files, creates a hard link between the two. Cannot hard link directories.
- <b>unlink:</b> Takes two paths as arguments and, if both are existing files that are linked, removes the link between them.
- <b>symlink:</b> Takes two paths as arguments and creates a sybolic link at the second path that is linked to the first path. The symbolic link is basically a shortcut to what it is linked to.
- <b>readlink:</b> Takes a path and a buffer as arguments and, if the path is a sybolic link, reads the length of the file into the buffer. This is basically just to confirm that the symbolic link is working correctly.
- <b>open:</b> Takes a path as an argument and, if a file exists at the path, opens the file at the path.
- <b>close:</b> Takes an int as an argument and, if one is open, closes the file at the given file descriptor.
- <b>cat:</b> Takes a path as an argument and, if a file exists at that path, opens the file and prints it to the console.
- <b>cp:</b> Takes two paths as an argument and, if a file exists at the first path, opens the file and copies it to a file at the second path, creating a file if one does not already exist.
- <b>mount:</b> Takes a file system (virtual disk in this case) and path as arguments and mounts the file system to the directory. (Incomplete)
- <b>unmount:</b> Takes a file system as an argument, determines if the file system is mounted to the current file system, then unmounts the given file system. (incomplete).
<p>This project is my best examply of taking existing code and making it my own. The project was laid out in the cited text book by K.C. Wang (who was also the professor) and included chunks of code and explanations of what the expected functionality is. It was my job to take this and create the final product. This best shows my ability to independently work through and solve complex problems laid out for me by someone else. I did not prioritize documentation for this project pprimarily because it was an incredibly time consuming project and documentation had no affect on my grades. My other projects do a much better job at showing my ability to make use of effective documentation, especially my Spreadsheet project.
</p>
## The following two sources were used in this project:
- WANG, K. C. Systems Programming in UNIX/Linux. SPRINGER, 2019. 
- Poirier, Dave. The Second Extended File System, https://www.nongnu.org/ext2-doc/ext2.html. 
## Collaborators: None
