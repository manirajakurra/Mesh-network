This project is a simple monitor program for the STM32 family.

The Simple monitor program provides a command line parser, with
facilities for the user to add new commands on a module by module
basis.  The parser is capable of recognizing one word commands, and
calling a function that implements that command.  The function may
then retrieve additional arguments from the command line, and act
accordingly.  Online brief help is provided automatically, extended
help is available optionally on a command by command basis.  New
commands are added to the parser using the 'ADD_CMD()' macro.  See the
source code for examples of how to use this functionality.  Each
command function returns a value to indicate success or failure.  The
user is ecouraged to take advantage of the return values to aid in
diagnostics. New return values can be defined by adding them to the
file 'src/monitor/command_codes.inc'.

In addition to the parser, there is a simple task executive built in.
Similar to the parser, the task executive uses a macro to add tasks to
the master table. A task is defined by having two optional
functions. The first function is an initialization function, which is
called at startup.  The second is the actual task function, which will
be called repeatedly from the main loop. To mark one or both of the
functions as unused, simply set the function pointer to NULL. At run
time, both functions are supplied with a single void pointer 'data'
argument. The value of this pointer can be supplied as part of the
'ADD_TASK()' macro. This pointer is useful for supplying task local
storage. The parser task is implemented using this scheme.  Note that
the added tasks are called in alphabetic order, there is no way to
guarentee a particular startup order.  If a particular order is
needed, it is necessary to call the startup functions manually.  No
task control is provided.  Tasks are always in the running state.

Building the code is done by typing 'make' in the src directory.  To
load the code onto the target, issue 'make program'.  To invoke the
GNU debugger on the code, type 'make debug'.  This gives a command
line debug tool. For a graphical debug experience, type 'make
ddddebug'. This will launch 'ddd', a graphical front end to GDB. In
all cases, hardware breakpoints can be used to debug and single step
code.

A default IRQ handler has been provided that dumps some state to the
terminal, which includes the IRQ number, this can aid in debugging
unmapped interrupts, or when the user IRQ handler function has been
given the wrong name.  Additionaly, the watchdog timer has been
enabled at startup.  User code that blocks will cause a timeout,
followed by a system reset.

The structure of the simple monitor program is one of a
foreground/background system.  The main loop of the program
initializes all the peripherals, enables interrupts, sets up I/O
conditions, and then calls a series of 'Tasks' in sequence.  The user
can extend this by adding their own background tasks to the main loop.
Caution should be exercised by the user to ensure that every task
returns back to the main loop if there is no work to do. Any task that
blocks waiting for an I/O event to occur will stall the whole
system. (For example, calling the delay() function inside your task is
frowned upon, use a timer or virtual timer instead.)

To aid in configuring the hardware, CubeMX has been used to generate
template coode in src/cubemx directory.  To modify the configuration,
load the '.ioc' file in that subdirectory into CubeMX, and regenerate
the code.  Note that the makefile in the main directory does not get
automatically updated if new HAL files are added. It is up to the user
to add them manually in the HAL section.
