# S-Talk

This program consists of a chat application between to consoles on the same network that is written in C that I wrote for a class.


## Program Structure

The program is splot into modules.  The input module accepts input from the user, the sender module sends input from the user, the receiver module receives input and the display module displays received input to the screen.

## Sample Commands / Getting Started

```
make all
./s-talk 12345 <Your-Compute-Name-Here> 22110 # Execute this in first console (This means the program is listening on port 12345 and sending to the named computer on port 22110)
./s-talk 22110 <Your-Compute-Name-Here> 12345 # Execute this in second console (This means the program is listening on port 22110 and sending to the named computer on port 12345)
...
Type messages here
...
! # (Typing "!" shutdowns the program on both terminals)
```

```
make all
./s-talk 22110 <Computer-Name> 12345 >> someOutputData.txt  # Writes output to "someOutputData.txt"
```

```
make all
cat someTestData.txt | ./s-talk 12345 aidan-MS-7B93 22110 # Sends all text in "someTestData.txt".  Note: any file sent must contain "!" in a newline at the end of the file.
```


