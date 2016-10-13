#ifndef TASTY_REGEX_TASTY_GREP_UTILS_H_
#define TASTY_REGEX_TASTY_GREP_UTILS_H_

/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#ifdef WIN32
#	include <io.h>		/* _open, _read, _write, _close */
#else
#	include <fcntl.h>	/* open */
#	include <sys/types.h>	/* needed for read */
#	include <sys/uio.h>	/* needed for read */
#endif /* ifdef WIN32 */
#include <unistd.h>		/* STDOUT/IN/ERR_FILENO, read, write, close */
#include <errno.h>		/* errno */

/* helper macros
 * ────────────────────────────────────────────────────────────────────────── */


/* wrapper macros
 * ────────────────────────────────────────────────────────────────────────── */
#ifdef WIN32
/* open a file */
#	define FILE_OPEN(PATH,						\
			 OPEN_FLAG,					\
			 MODE)						\
	_open(PATH,							\
	      OPEN_FLAG,						\
	      MODE)
/* read a file */
#	define FILE_READ(FILE_DESCRIPTOR,				\
			 BUFFER,					\
			 SIZE)						\
	_read(FILE_DESCRIPTOR,						\
	      BUFFER,							\
	      (unsigned int) (SIZE))
/* close a file */
#	define FILE_CLOSE(FILE_DESCRIPTOR)				\
	_close(FILE_DESCRIPTOR)

#else
/* open a file */
#	define FILE_OPEN(PATH,						\
			 OPEN_FLAG,					\
			 MODE)						\
	open(PATH,							\
	     OPEN_FLAG,							\
	     MODE)
/* read a file */
#	define FILE_READ(FILE_DESCRIPTOR,				\
			 BUFFER,					\
			 SIZE)						\
	read(FILE_DESCRIPTOR,						\
	     BUFFER,							\
	     (size_t) SIZE)
/* close a file */
#	define FILE_CLOSE(FILE_DESCRIPTOR)				\
	close(FILE_DESCRIPTOR)
#endif /* ifdef WIN32 */



/* API
 * ────────────────────────────────────────────────────────────────────────── */
/* open (absolute or relative path, provide mode) */
inline void
file_open_muffle(int *const restrict file_descriptor,
		 const char *const path,
		 const int open_flag,
		 const mode_t mode)
{
	*file_descriptor = FILE_OPEN(path,
				     open_flag,
				     mode);
}

inline bool
file_open_report(int *const restrict file_descriptor,
		 const char *const path,
		 const int open_flag,
		 const mode_t mode,
		 const char *restrict *const restrict failure)
{
	*file_descriptor = FILE_OPEN(path,
				     open_flag,
				     mode);

	if (LIKELY(*file_descriptor >= 0))
		return true;

	switch (errno) {
	case EACCES:
		*failure = "open failure (one of the following):\n"
			   "\t- Search permission is denied for a component of "
			   "the path prefix."
			   "\t- The required permissions (for reading and/or "
			   "writing) are denied for the given flags.\n"
			   "\t- 'O_CREAT' is specified, the file does not exist"
			   ", and the directory in which it is to be created "
			   "does not permit writing.\n"
			   "\t- 'O_TRUNC' is specified and write permission is "
			   "denied.\n";
		return false;
	case EAGAIN:
		*failure = "open failure: path specifies the slave side of a "
			   "locked pseudo-terminal device.\n";
		return false;
	case EDQUOT:
		*failure = "open failure (one of the following):\n"
			   "\t- 'O_CREAT' is specified, the file does not exist"
			   ", and the directory in which the entry for the new "
			   "file is being placed cannot be extended because the"
			   " user's quota of disk blocks on the file system "
			   "containing the directory has been exhausted.\n"
			   "\t- 'O_CREAT' is specified, the file does not exist"
			   ", and the user's quota of inodes on the file system"
			   " on which the file is being created has been "
			   "exhausted.\n";
		return false;
	case EEXIST:
		*failure = "open failure: 'O_CREAT' and 'O_EXCL' are specified "
			   "and the file exists.\n";
		return false;
	case EFAULT:
		*failure = "open failure: 'path' points outside the process's "
			   "allocated address space.\n";
		return false;
	case EINTR:
		*failure = "open failure: The open() operation was interrupted "
			   "by a signal.\n";
		return false;
	case EINVAL:
		*failure = "open failure: The value of 'open_flag' is not valid"
			   ".\n";
		return false;
	case EIO:
		*failure = "open failure: An I/O error occurred while making "
			   "the directory entry or allocating the inode for '"
			   "O_CREAT'.\n";
		return false;
	case EISDIR:
		*failure = "open failure: The named file is a directory, and "
			   "the arguments specify that it is to be opened for"
			   " writing.\n";
		return false;
	case ELOOP:
		*failure = "open failure: (one of the following):\n"
			   "\t- Too many symbolic links were encountered in "
			   "translating the pathname. This is taken to be "
			   "indicative of a looping symbolic link.\n"
			   "\t- 'O_NOFOLLOW' was specified and the target is a "
			   "symbolic link.\n";
		return false;
	case EMFILE:
		*failure = "open failure: The process has already reached its "
			   "limit for open file descriptors.\n";
		return false;
	case ENAMETOOLONG:
		*failure = "open failure: A component of a pathname exceeds "
			   "{NAME_MAX} characters, or an entire path name "
			   "exceeded {PATH_MAX} characters.\n";
		return false;
	case ENFILE:
		*failure = "open failure: The system file table is full.\n";
		return false;
	case ENOENT:
		*failure = "open failure: (one of the following):\n"
			   "\t- 'O_CREAT' is not set and the named file does "
			   "not exist.\n"
			   "\t- A component of the path name that must exist "
			   "does not exist.\n";
		return false;
	case ENOSPC:
		*failure = "open failure: (one of the following):\n"
			   "\t- 'O_CREAT' is specified, the file does not exist"
			   ", and the directory in which the entry for the new "
			   "file is being placed cannot be extended because "
			   "there is no space left on the file system "
			   "containing the directory.\n"
			   "\t- 'O_CREAT' is specified, the file does not exist"
			   ", and there are no free inodes on the file system "
			   "on which the file is being created.\n";
		return false;
	case ENOTDIR:
		*failure = "open failure: (one of the following):\n"
			   "\t- A component of the path prefix is not a "
			   "directory.\n"
			   "\t- The path argument is not an absolute path.\n";
		return false;
	case ENXIO:
		*failure = "open failure: (one of the following):\n"
			   "\t- The named file is a character-special or block-"
			   "special file and the device associated with this "
			   "special file does not exist.\n"
			   "\t- 'O_NONBLOCK' and 'O_WRONLY' are set, the file "
			   "is a FIFO, and no process has it open for reading."
			   "\n";
		return false;
	case EOPNOTSUPP:
		*failure = "open failure: (one of the following):\n"
			   "\t- 'O_SHLOCK' or 'O_EXLOCK' is specified, but the "
			   "underlying filesystem does not support locking.\n"
			   "\t- An attempt was made to open a socket (not "
			   "currently implemented).\n";
		return false;
	case EOVERFLOW:
		*failure = "open failure: The named file is a regular file and "
			   "its size does not fit in an object of type 'off_t'"
			   ".\n";
		return false;
	case EROFS:
		*failure = "open failure: The named file resides on a read-only"
			   " file system, and the file was to be modified.\n";
		return false;
	case ETXTBSY:
		*failure = "open failure: The file is a pure procedure (shared "
			   "text) file that is being executed and the open() "
			   "call requests write access.\n";
		return false;
	case EBADF:
		*failure = "open failure: The path argument does not specify an"
			   " absolute path.\n";
		return false;
	default:
		*failure = "open failure: unknown\n";
		return false;
	}
}


/* read from open file descriptor */
inline void
file_read_muffle(const int file_descriptor,
		 void *const restrict buffer,
		 const ssize_t size)
{
	(void) FILE_READ(file_descriptor,
			 buffer,
			 size);
}

inline bool
file_read_report(const int file_descriptor,
		 void *const restrict buffer,
		 const ssize_t size,
		 const char *restrict *const restrict failure)
{
	const ssize_t size_read = FILE_READ(file_descriptor,
					    buffer,
					    size);

	if (LIKELY(size_read == size))
		return true;

	if (size_read >= 0l) {
		*failure = "read failure: failed to read specified 'size' bytes"
			   ".\n";
		return false;
	}

	switch (errno) {
	case EAGAIN:
		*failure = "read failure: The file was marked for non-blocking "
			   "I/O, and no data were ready to be read.\n";
		return false;
	case EBADF:
		*failure = "read failure: 'file_descriptor' is not a valid file"
			   " or socket descriptor open for reading.\n";
		return false;
	case EFAULT:
		*failure = "read failure: 'buffer' points outside the allocated"
			   " address space.\n";
		return false;
	case EINTR:
		*failure = "read failure: A read from a slow device was "
			   "interrupted before any data arrived by the delivery"
			   " of a signal.\n";
		return false;
	case EINVAL:
		*failure = "read failure: The pointer associated with '"
			   "file_descriptor' was negative.\n";
		return false;
	case EIO:
		*failure = "read failure (one of the following):\n"
			   "\t- An I/O error occurred while reading from the "
			   "file system.\n"
			   "\t- The process group is orphaned.\n"
			   "\t- The file is a regular file, 'size' is greater"
			   " than 0, the starting position is before the end-of"
			   "-file, and the starting position is greater than or"
			   " equal to the offset maximum established for the "
			   "open file descriptor associated with '"
			   "file_descriptor'.\n";
		return false;
	case EISDIR:
		*failure = "read failure: An attempt was made to read a "
			   "directory.\n";
		return false;
	case ENOBUFS:
		*failure = "read failure: An attempt to allocate a memory "
			   "buffer failed.\n";
		return false;
	case ENOMEM:
		*failure = "read failure: Insufficient memory is available.\n";
		return false;
	case ENXIO:
		*failure = "read failure (one of the following):\n"
			   "\t- An action was requested of a device that does "
			   "not exist.\n"
			   "\t- A requested action cannot be performed by the "
			   "device.\n";
		return false;
	case ECONNRESET:
		*failure = "read failure: The connection was closed by the peer"
			   " during a read attempt on a socket.\n";
		return false;
	case ENOTCONN:
		*failure = "read failure: A read was attempted on an "
			   "unconnected socket.\n";
		return false;
	case ETIMEDOUT:
		*failure = "read failure: A transmission timeout occurs during "
			   "a read attempt on a socket.\n";
		return false;
	default:
		*failure = "read failure: unknown\n";
		return false;
	}
}



/* close an open file descriptor */
inline void
file_close_muffle(const int file_descriptor)
{
	(void) FILE_CLOSE(file_descriptor);
}

inline bool
file_close_report(const int file_descriptor,
		  const char *restrict *const restrict failure)
{
	if (LIKELY(FILE_CLOSE(file_descriptor)) == 0)
		return true;

	switch (errno) {
	case EBADF:
		*failure = "close failure: 'file_descriptor' is not a valid, "
			   "active file descriptor.\n";
		return false;
	case EINTR:
		*failure = "close failure: Execution was interrupted by a "
			   "signal.\n";
		return false;
	case EIO:
		*failure = "close failure: A previously-uncommitted write "
			   "encountered an input/output error.\n";
		return false;
	default:
		*failure = "close failure: unknown\n";
		return false;
	}
}
#endif /* ifndef TASTY_REGEX_TASTY_GREP_UTILS_H_ */
