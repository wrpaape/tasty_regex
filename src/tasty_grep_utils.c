#include "tasty_grep_utils.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
/* open (absolute or relative path, provide mode) */
extern inline void
file_open_muffle(int *const restrict file_descriptor,
		 const char *const path,
		 const int open_flag,
		 const mode_t mode);
extern inline bool
file_open_report(int *const restrict file_descriptor,
		 const char *const path,
		 const int open_flag,
		 const mode_t mode,
		 const char *restrict *const restrict failure);

/* read from open file descriptor */
extern inline void
file_read_muffle(const int file_descriptor,
		 void *const restrict buffer,
		 const ssize_t size);
extern inline bool
file_read_report(const int file_descriptor,
		 void *const restrict buffer,
		 const ssize_t size,
		 const char *restrict *const restrict failure);

/* close an open file descriptor */
extern inline void
file_close_muffle(const int file_descriptor);
extern inline bool
file_close_report(const int file_descriptor,
		  const char *restrict *const restrict failure);
