#pragma once

/**
 * Virtual Memory Manager
 * 
 * This is an extra abstraction of "paging"
 * It should handle issues like page faults, allocating/freeing pages (while paging maps/unmaps it)
 * Manage permissions, swapping and later sharing between processes
 *
 * When a process (not kernel!) page faults,
 * it should determine whether it's valid memory (like malloc or just random memory access)
 * loads a page from disk (swapin) or kill the process if it's an illegal access (like kernel memory)
 *
 * It should also init paging and the pageHeap
 */