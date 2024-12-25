![github repo badge: Language](https://img.shields.io/badge/Language-C-181717?color=red) ![github repo badge: Testing](https://img.shields.io/badge/Testing-Criterion-181717?color=orange)

# PBX_Telly_System

A multi-threaded chat server in C simulating Private Branch Exchange was created for my System Fundamentals II class. We were provided the test cases shown here, a demo application to see the server's behavior, and comments for function stubs but otherwise, the actual implementation found within main.c, server.c, pbx.c, and tu.c were my own. 

## Design/Features 

The design of our server is a simple multi-threaded network server where the main thread sets up the socket for listening and once connections are accepted, client threads are created to handle each connection. The client threads provide a service loop where the user can send a message for a corresponding action and receive a response. Telnet was used to connect to the server. 

## Usage  

First we must start up the server on a specific port by running the following in our terminal: 

```
bin/pbx -p PORT#
```

Then we can connect to this server as a client in another terminal by running: 

```
telnet localhost PORT#
```

You can now issue the following commands to the server, simulating a private telephone: 

```
pickup
hangup 
dial #, where # is the number of the extension to be dialed.
chat ...arbitrary text...
```

Typically a conversation without errors would go like so, pickup -> dial # -> dialed picks up -> chat x times -> hangup and repeat!  

Explaining each scenario would take up a lot of time and space, so I'll avoid doing that as playing around with the server and reading the code
gives a clear idea of what can be done. 

## Building and Testing     

PBX_Telly_System can be built using the provided make files and running ```make clean && make all```. 

The testing framework used was [criterion](https://github.com/Snaipe/Criterion), so this must be installed before attempting to run the tests using: 

```bin/pbx_tests -j1``` 

*Note: The -j1 flag must be passed in otherwise all the tests will fail! This is because without it only one server will be able to bind the port number 
being used and the others will fail!* 

## Acknowledgements 

A lot of my understanding of multi-threaded servers and figuring out such an implementation can be attributed to the excellent explanations that can be found in [*Computer Systems: A Programmer's Perspective*](http://csapp.cs.cmu.edu/3e/home.html) by Randal E. Bryant and David R. O'Hallaron
