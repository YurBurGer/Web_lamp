#!env python3


from socket import *
cs = socket(AF_INET, SOCK_DGRAM)
cs.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
#cs.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
cs.sendto('WebLamp announce!', ('192.168.80.177', 8888))
