_start:
        li sp, 0xFFFE

        li t0, 0
        li t1, 1

        li t3, 0
        li s0, 10
for_check:
        lt t2, t3, s0
        jz t2, for_done
for_body:
        add t2, t0, t1
        mv t0, t1
        mv t1, t2

        st t0, sp, 0
        subi sp, sp, 2

        addi t3, t3, 1
        j for_check
for_done:       
        halt 0
