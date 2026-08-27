
bits 16

# Test - 16-bit to 16-bit register add, no carry, overflow, negative, or zero

# Test - 8-bit to 8-bit register add, no carry, overflow, negative, or zero

# Test - 16-bit to 16-bit register add, overflow set 

# Test - 16-bit to 16-bit register add, carry set

# Test - 16-bit to 16-bit register add, zero set (carry optional)

# Test - 8-bit to 8-bit register add, overflow set 

# Test - 8-bit to 8-bit register add, carry set

# Test - 8-bit to 8-bit register add, zero set (carry optional)


MOV bl, 0xFF
MOV al, 0x1
MOV cl, 0xA
ADD al, bl
ADC al, cl