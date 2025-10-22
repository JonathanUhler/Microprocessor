_start:
        li sp, 0xFFFE           ; Load the stack pointer

        li t0, 0                ; Load initial value of A
        li t1, 1                ; Load initial value of B

        li t3, 0                ; Load initial value of loop index variable
        li s0, 10               ; Load final value of loop index variable (exclusive)
for_check:
        lt t2, t3, s0           ; t2 = i < 10 (whether loop is done)
        jz t2, for_done
for_body:
        add t2, t0, t1          ; temp = A + B
        mv t0, t1               ; A = B
        mv t1, t2               ; B = temp

        st t0, sp, 0            ; Store latest fibonacci number onto the stack
        subi sp, sp, 2          ; Decrement stack pointer

        addi t3, t3, 1          ; Increment loop index variable
        j for_check             ; Next loop iteration
for_done:       
        halt 0                  ; Loop is done
