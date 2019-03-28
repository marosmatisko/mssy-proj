# MSSY-project
Master branch - packet library for gateway & nodes.   
Gateway branch - implementation of gateway functions.   

## FAQ: 
**Q:** How to use?   
**A:** Add to you main.h:   
``` c
    #include "device.h"
    #include "packet_parser.h"
    
    void APP_WriteString(char *string);
```
and to your main.c:
``` c
    void APP_WriteString(char *string) {
	int index = 0;
	while (string[index] != '\0') {
		HAL_UartWriteByte(string[index]);
		++index;
	}
  }   
``` 
APP_WriteString is used for testing & debug purposes. Then in packet_parser.c add call to your functions with every packet structure.   
   
   
**Q:** What if I do not want to use this debug?   
**A:** Just comment in packet_parser.h
``` c
//extern void APP_WriteString(char *string);
``` 
and in packet_parser.c
``` c
//#define DEBUGGING
```    
   
   
**Q:** What are that files for?   
**A:** Take a look on table below.      
## Files
| Option | Description |
| ------ | ----------- |
| device.h  | Definition of devices & device state machine. |
| packet_parser.c | Functions for serialization & parsing packets. |
| packet_parser.h | Definition of functions for serialization & parsing packets. |
| packets.h | Definition of packet structures. |
