.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	// [esp  + 8] = 'thread' parameter for 'f'
	// [esp  + 4] = 0 (dummy return address)
	// [esp  + 0] = 'f'
	// stack is now (16-12)-byte aligned

	// push [esp + foo]: address of esp is used _before_ decrementing!
	push	8(%esp)	// 'thread' parameter for 'f'
	push	$0	// 0 (dummy return address)

	// [esp + 16] = thread parameter for 'f'   << not relevant anymore
	// [esp + 12] = 0 (dummy return address)   << not relevant anymore
	// [esp +  8] = 'f'                        << not relevant anymore
	// [esp +  4] = thread parameter for 'f'
	// [esp +  0] = 0
	// stack is now (16-4)-byte aligned -- as required by function entry
	jmp	*8(%esp) // jmp f
