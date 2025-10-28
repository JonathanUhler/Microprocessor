        .loc 0x100
_start:
        li sp, 0xFFFE

        li a0, array
        li a1, array_size
        ld a1, a1, 0

        call main

        halt 0


        .loc 0x200
main:
        li t0, 0                ; Outer loop index variable
L_outer_loop_check:
        lt t2, t0, a1
        j0 t2, L_outer_loop_done
L_outer_loop_body:

        mv t1, t0                ; Inner loop index variable
L_inner_loop_check:
        lt t2, t1, a1
        j0 t2, L_inner_loop_done
L_inner_loop_body:
        slli t5, t0, 1          ; Offset in number of bytes for outer loop value
        add t5, t5, a0          ; Address of arr[outer loop index]
        slli t6, t1, 1          ; Offset in number of bytes for inner loop value
        add t6, t6, a0          ; Address of arr[inner loop index]

        ld t3, t5, 0            ; arr[outer loop index]
        ld t4, t6, 0            ; arr[inner loop index]

        gt t2, t3, t4           ; If arr[outer loop index] > arr[inner loop index]
        j0 t2, L_swap_done
L_swap:
        st t3, t6, 0
        st t4, t5, 0
L_swap_done:    

        addi t1, t1, 1
        j L_inner_loop_check
L_inner_loop_done:

        addi t0, t0, 1
        j L_outer_loop_check
L_outer_loop_done:
        ret


        .loc 0x6000
array:
        .half 33
        .half 91
        .half 96
        .half 40
        .half 5
        .half 32
        .half 26
        .half 2
        .half 56
        .half 44
array_size:
        .half 10
