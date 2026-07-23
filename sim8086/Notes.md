# Intel 8086 Simulator Notes

## Enhancements
- We don't have any protections in place (no checks, asserts etc.)
    - Check of bytes exceed boundaries when decoding
    - No checks against bad decode
    - No asserts in areas where conditions MUST hold true
- Global memory is not great for shippable product, need to move that into a different spot and pass in memory pointer to functions 
    - This allows functions to be more testable 
- Technically, we are not modeling 8086 memory properly (no segmenting or segmented access), need to update this to match the memory model 
- CPU is missing registes and no concept of flags 
- Can we clean up structs, struct naming, and field naming? 
- Do we need to have register macros in the table file? Can we just use Register enum?
- Can register offsets be handled better? Something other than LO, HI, FULL? Maybe better name for index register in Effective Address calculation?
- Can we simplify the instruction struct to just Op, Operands, and Size?
- We always use defaults in Switch statements, but this may be bad choice. Need to be more strategic about when we use them. We may want compiler to notify us when a switch is missing cases
- Leverage the built in sign operator for printf `%+d`
- Everything in instruction table should be macros, and table should be easy to skim read and understand 
- Need to leverage pointers more, we heavily rely on passing by value
- Can we optimize the bit extraction to be a bit cleaner and simpler?
- Can we simplify all of the has__ and data extraction piece after the bit extraction loop. Lots of uint8_t variables there that are probably unnecessary. 
- Need to start moving like functionality to their own header/cpp file to make it easier to read/understand project. \
- Can we simplify the disassembly by removing the two pass system and just doing one pass?
- We need to determine a better way to handle jmps and IP increments