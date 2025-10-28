        .loc 0x100
_start:
        li sp, 0xFFFE

        li a0, n
        li a1, x

        call fibonacci

        halt 0


        .loc 0x200
fibonacci:
        ld a0, a0, 0            ; Count of values to compute

        li t0, 0                ; Initial value of A
        li t1, 1                ; Initial value of B

        li t2, 0                ; Load initial value of loop index variable
for_check:
        lt t3, t2, a0           ; t3 = i < n (whether loop is done)
        j0 t3, for_done
for_body:
        add t3, t0, t1          ; temp = A + B
        mv t0, t1               ; A = B
        mv t1, t3               ; B = temp

        st t0, a1, 0            ; Store latest fibonacci number into int* pointer

        addi t2, t2, 1          ; Increment loop index variable
        j for_check             ; Next loop iteration
for_done:
        ret


        .loc 0x6000
n:
        .half 20
x:
        .half 0
