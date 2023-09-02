For the d*.c examples use sample.xml and for e*.c use e.xml. If you try
to use the e*.c examples with sample.xml they may segfault because they
look for specific nodes.

Also, these examples rely on -DMSGNO being defined. If you want
stack-trace-like messages from the DOMC package it will be necessary to
rebuild it too with -DMSGNO defined.

